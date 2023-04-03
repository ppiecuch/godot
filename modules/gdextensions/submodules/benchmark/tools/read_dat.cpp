/**************************************************************************/
/*  read_dat.cpp                                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("%s: Filename required.\n", argv[0]);
		exit(1);
	}

	FILE *file = fopen(argv[1], "r");

	if (file == nullptr) {
		printf("%s: Cannot open '%s'.\n", argv[0], argv[1]);
		exit(2);
	}

	uint32_t verts_num = -1, indexes_num = -1;
	float *verts = nullptr;
	uint16_t *indexes = nullptr;

	fread(&verts_num, sizeof(uint32_t), 1, file);
	uint32_t verts_size = verts_num * sizeof(float) * 8;
	printf("%s: %d vertexes, %d bytes of data.\n", argv[0], verts_num, verts_size);
	verts = (float *)malloc(verts_size);
	fread(verts, verts_size, 1, file);
	fread(&indexes_num, sizeof(uint32_t), 1, file);
	uint32_t indexes_size = indexes_num * sizeof(uint16_t);
	printf("%s: %d indexes, %d bytes of data.\n", argv[0], indexes_num, indexes_size);
	indexes = (uint16_t *)malloc(indexes_size);
	fread(indexes, indexes_size, 1, file);
	fclose(file);
}
