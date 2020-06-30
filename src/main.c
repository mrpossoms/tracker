#include "vidi.h"

typedef union {
	struct {
		uint8_t r, g, b;
	};
	uint8_t v[3];
} rgb_t;
typedef struct { 
	uint8_t* frame,
	size_t width, height;
	size_t bytes_pixel;
} frame_t; 


void downsample(frame_t from, frame_t to)
{
	if (from.width < to.width) { return; }
	if (from.height < to.height) { return; }

	int dw = from.width / to.width;
	int dh = from.height / to.height;

	for 
}


int main (int argc, const char* argv[])
{
	vidi_cfg_t cam = {
		.width = 640,
		.height = 480,
		.frames_per_sec = 60,
		.path = argv[1]
	};

	assert(0 == vidi_config(&cam));
	

	return 0;
}
