#ifndef _starfield_res_defined_
#define _starfield_res_defined_

#if defined(__cplusplus)
extern "C" {
#endif

extern const unsigned char
	starfield_1[],
	starfield_2[],
	starfield_3[],
	starfield_4[],
	starfield_5[],
	starfield_6[],
	starfield_7[],
	starfield_8[],
	starfield_9[],
	starfield_10[];

typedef struct {
	const char *image;
	const unsigned char *pixels;
	int size, width, height, channels;
	} EmbedImageItem;

const static EmbedImageItem embed_starfield_res[] = {
	{"star01.png", starfield_1, 3072, 32, 32, 3},
	{"star02.png", starfield_2, 768, 16, 16, 3},
	{"star03.png", starfield_3, 192, 8, 8, 3},
	{"star04.png", starfield_4, 768, 16, 16, 3},
	{"star05.png", starfield_5, 192, 8, 8, 3},
	{"star06.png", starfield_6, 768, 16, 16, 3},
	{"star07.png", starfield_7, 768, 16, 16, 3},
	{"frame0.png", starfield_8, 12288, 64, 64, 3},
	{"frame1.png", starfield_9, 12288, 64, 64, 3},
	{"frame2.png", starfield_10, 12288, 64, 64, 3},
	{NULL, NULL, 0, 0, 0, 0}
};
const static int embed_starfield_res_count = 10;

#if defined(__cplusplus)
}
#endif
#endif // of starfield_res
