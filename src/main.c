#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vidi.h"

#define CAM_FOV 30

typedef union {
	struct {
		uint8_t r, g, b;
	};
	uint8_t v[3];
} rgb_t;


typedef struct {
	size_t width, height;
	rgb_t* buf;
} frame_t;


void downsample(frame_t from, frame_t to)
{
	size_t dw = from.width / to.width;
	size_t dh = from.height / to.height;

	for (size_t r = 0; r < to.height; r++)
	for (size_t c = 0; c < to.width; c++)
	{
		size_t f_r = dh * r;
		size_t f_c = dw * c;

		(to.buf + r * to.width)[c] = (from.buf + f_r * from.width)[f_c];
	}
}


int main (int argc, const char* argv[])
{
	// define a configuration object for a camera, here
	// you can request frame size, pixel format, frame rate
	// and the camera which you wish to use.
	vidi_cfg_t cam = {
		.width = 640,
		.height = 480,
		.frames_per_sec = 60,
		.path = argv[1]
	};

	// vidi_config is used to open and configure the camera
	// device, but can also be used to reconfigure an
	// existing vidi_cfg_t camera instance.
	assert(0 == vidi_config(&cam));
	
	rgb_t last_frame[480][640] = {};
	rgb_t frame[480][640] = {};
	char display[32][128] = {};

	int rows_drawn = 0;
	int targ_x = 0;
	int targ_y = 0;

	fputs("\033[?25l", stderr);

	// request the camera to capture a frame
	vidi_request_frame(&cam);

	while (1)
	{
		// this function blocks until a frame pointer is returned
		uint8_t* raw_frame = vidi_wait_frame(&cam);
		size_t row_size = vidi_row_bytes(&cam);

		// request the camera to capture a frame
		vidi_request_frame(&cam);

		for (int r = 0; r < 480; r++)
		{
			memcpy(frame[r], raw_frame + (r * row_size), row_size);
		}

		const char spectrum[] = "  .,':;|[{+*x88";

		// process
		int com_x = 0, com_y = 0;
		int com_points = 0;
		const int dr = 480 / 32;
		const int dc = 640 / 128;
		for (int r = 0; r < 32; r++)
		{
			for (int c = 0; c < 128; c++)
			{
				int ri = r * dr, ci = c * dc;
				int last_grey = (last_frame[ri][ci].r + last_frame[ri][ci].g + last_frame[ri][ci].b) / 3;
				int grey = (frame[ri][ci].r + frame[ri][ci].g + frame[ri][ci].b) / 3;
				int delta = abs(grey-last_grey);

				int idx = delta / 18;
				display[r][c] = spectrum[idx];

				if (idx >= 2)
				{
					com_x += c;
					com_y += r;
					com_points++;
				}
			}
		}

		if (com_points > 0)
		{
			int x = com_x / com_points;
			int y = com_y / com_points;

			targ_x = (x + targ_x * 4) / 5;
			targ_y = (y + targ_y * 4) / 5;			
		}

		display[targ_y][targ_x] = '#';

		{ // Erase the previously drawn rows
			static char move_up[16] = {};
			sprintf(move_up, "\033[%dA", rows_drawn);
			fprintf(stderr, "%s", move_up);
			rows_drawn = 0;
		}

		// display
		for (int r = 0; r < 32; r++)
		{
			for (int c = 0; c < 128; c++)
			{
				fputc(display[r][c], stderr);
			}
			fputc('\n', stderr);
			rows_drawn += 1;
		}

		printf("COM points: %d   \n", com_points);
		printf("COM (%d, %d)\n", targ_x, targ_y);
		rows_drawn+=2;

		memcpy(last_frame, frame, sizeof(frame));
	}

	return 0;
}
