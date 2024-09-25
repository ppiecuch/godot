
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define GL_RGB 3

typedef struct {
	const char *label;
	const char *image;
	const unsigned char *pixels;
	int char_w, char_h, size, width, height, channels;
} EmbedImageItem;

extern const unsigned char font_8x16[], font_8x12[], font_8x8[], font_7x9[], font_4x6[];
const EmbedImageItem embed_debug_font[] = {
	{ "font_8x16", "dos-8x16.bmp", font_8x16, 8, 16, 196608, 128, 512, GL_RGB },
	{ "font_8x12", "dos-8x12.bmp", font_8x12, 8, 12, 147456, 128, 384, GL_RGB },
	{ "font_8x8", "dos-8x8.bmp", font_8x8, 8, 8, 98304, 128, 256, GL_RGB },
	{ "font_7x9", "dos-7x9.bmp", font_7x9, 7, 9, 96768, 112, 288, GL_RGB },
	{ "font_4x6", "dos-4x6.bmp", font_4x6, 4, 6, 36864, 64, 192, GL_RGB },
	{ NULL, NULL, 0, 0, 0, 0, 0, 0 }
};

#include "dos_font_data_rgb.h"

int main() {
	FILE *out = fopen("dos_font_data_8.h", "w");
	if (out == 0) {
		printf("Cannot open dos_font_data_8.h!\n");
		exit(1);
	}
	EmbedImageItem *el = (EmbedImageItem *)&embed_debug_font[0];
	int index = 1;
	while (el->image) {
		printf("Converting %s ..\n", el->image);
		fprintf(out,
				"/* %s contains file \"%s\". */\n"
				"const unsigned char %s[] =\n"
				"{",
				el->label, el->image, el->label);

		assert(el->size % 3 == 0);

		for (int b = 0; b < el->size; b += 3) {
			if (b % 10 == 0)
				fprintf(out, "\n\t");
			fprintf(out, "0x%02x", el->pixels[b]);
			if (b + 3 != el->size)
				fprintf(out, ",");
		}

		fprintf(out, "\n};\n\n");
		el++;
		index++;
	}
}
