/*************************************************************************/
/*  write_dat.cpp                                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const int VERTEX_SIZE = sizeof(float)*8;

void save_dat(const char *app, const char *name, const float verts[], uint32_t verts_num, const uint16_t indexes[], uint32_t indexes_num) {
	FILE *file = fopen(name, "w");
	if (file == nullptr) {
		printf("%s: Cannot write '%s'.\n", app, name);
		exit(1);
	}
	fwrite(&verts_num, sizeof(uint32_t), 1, file);
	uint32_t verts_size = verts_num * VERTEX_SIZE;
	printf("%s: %d vertexes, %d bytes of data.\n", app, verts_num, verts_size);
	fwrite(verts, verts_size, 1, file);
	fwrite(&indexes_num, sizeof(uint32_t), 1, file);
	uint32_t indexes_size = indexes_num * sizeof(uint16_t);
	printf("%s: %d indexes, %d bytes of data.\n", app, indexes_num, indexes_size);
	fwrite(indexes, indexes_size, 1, file);
	fclose(file);
}

// #include "cube_model.h"
// #include "frog_model.h"

int main(int argc, char *argv[]) {
	// save_dat(argv[0], "cube.dat", cube_verts, sizeof(cube_verts)/VERTEX_SIZE, cube_indices, sizeof(cube_indices)/sizeof(uint16_t));
	// save_dat(argv[0], "frog.dat", frog_verts, sizeof(frog_verts)/VERTEX_SIZE, frog_indices, sizeof(frog_indices)/sizeof(uint16_t));
}
