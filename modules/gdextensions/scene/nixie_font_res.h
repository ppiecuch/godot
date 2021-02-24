#ifndef _nixie_font_defined_
#define _nixie_font_defined_

#if defined(__cplusplus)
extern "C" {
#endif

extern const unsigned char
	nixie_font_1[];

typedef struct {
	const char *image;
	const unsigned char *pixels;
	int size, width, height, channels;
	} EmbedImageItem;

const static EmbedImageItem embed_nixie_font[] = {
	{"nixie_font.png", nixie_font_1, 258336, 184, 468, 3},
	{NULL, NULL, 0, 0, 0, 0}
};
const static int embed_nixie_font_count = 1;

#if defined(__cplusplus)
}
#endif
#endif // of nixie_font
