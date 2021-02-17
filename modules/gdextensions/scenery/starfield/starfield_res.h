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
	starfield_10[],
	starfield_11[],
	starfield_12[],
	starfield_13[],
	starfield_14[],
	starfield_15[];

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
	{"star08.png", starfield_8, 5808, 44, 44, 3},
	{"star09.png", starfield_9, 2028, 26, 26, 3},
	{"star10.png", starfield_10, 4788, 38, 42, 3},
	{"star11.png", starfield_11, 6348, 46, 46, 3},
	{"star12.png", starfield_12, 1452, 22, 22, 3},
	{"frame0.png", starfield_13, 12288, 64, 64, 3},
	{"frame1.png", starfield_14, 12288, 64, 64, 3},
	{"frame2.png", starfield_15, 12288, 64, 64, 3},
	{NULL, NULL, 0, 0, 0, 0}
};
const static int embed_starfield_res_count = 15;

#if defined(__cplusplus)
}
#endif
#endif // of starfield_res
