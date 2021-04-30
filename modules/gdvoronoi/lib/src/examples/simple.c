/*************************************************************************/
/*  simple.c                                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

// Contribution by: Abe Tusk https://github.com/abetusk
// To compile:
// gcc -Wall -Weverything -Wno-float-equal src/examples/simple.c -Isrc -o simple
//
// About:
//
// This example outputs 10 random 2D coordinates, and all the generated edges, to standard output.
// Note that the edges have duplicates, but you can easily filter them out.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JC_VORONOI_IMPLEMENTATION
// If you wish to use doubles
//#define JCV_REAL_TYPE double
//#define JCV_FABS fabs
//#define JCV_ATAN2 atan2
//#define JCV_CEIL ceil
//#define JCV_FLOOR floor
//#define JCV_FLT_MAX 1.7976931348623157E+308
#include "jc_voronoi.h"

#define NPOINT 10

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;

	int i;
	jcv_rect bounding_box = { { 0.0f, 0.0f }, { 1.0f, 1.0f } };
	jcv_diagram diagram;
	jcv_point points[NPOINT];
	const jcv_site *sites;
	jcv_graphedge *graph_edge;

	memset(&diagram, 0, sizeof(jcv_diagram));

	srand(0);
	for (i = 0; i < NPOINT; i++) {
		points[i].x = (float)(rand() / (1.0f + RAND_MAX));
		points[i].y = (float)(rand() / (1.0f + RAND_MAX));
	}

	printf("# Seed sites\n");
	for (i = 0; i < NPOINT; i++) {
		printf("%f %f\n", (double)points[i].x, (double)points[i].y);
	}

	jcv_diagram_generate(NPOINT, (const jcv_point *)points, &bounding_box, &diagram);

	printf("# Edges\n");
	sites = jcv_diagram_get_sites(&diagram);
	for (i = 0; i < diagram.numsites; i++) {

		graph_edge = sites[i].edges;
		while (graph_edge) {
			// This approach will potentially print shared edges twice
			printf("%f %f\n", (double)graph_edge->pos[0].x, (double)graph_edge->pos[0].y);
			printf("%f %f\n", (double)graph_edge->pos[1].x, (double)graph_edge->pos[1].y);
			graph_edge = graph_edge->next;
		}
	}

	jcv_diagram_free(&diagram);
}
