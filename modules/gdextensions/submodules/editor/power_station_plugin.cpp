/**************************************************************************/
/*  power_station_plugin.cpp                                              */
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

// Power Station Glib PhyMod Library
// Copyright (c) 2000 David A. Bartold

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "power_station_plugin.h"

#include "core/error_macros.h"
#include "core/math/math_defs.h"
#include "core/math/math_funcs.h"
#include "core/os/memory.h"
#include "core/typedefs.h"

PSState _ps_defaults = {
	0,
	10,
	5,
	5,
	7,
	9,
	4,
	0.2,
	0.05,
	0,
	1,
	1,
	{},
	nullptr,
	0,
	90,
	0,
	false,
	0,
	0,
};

typedef void PSPercentCallback(float percent, void *userdata);

typedef struct _vector3 {
	double x, y, z;
} vector3;

typedef struct _PSMetalObjNode {
	int anchor;
	vector3 pos;
	vector3 vel;

	int num_neighbors;
	struct _PSMetalObjNode *neighbors[1];
} PSMetalObjNode;

typedef struct _PSMetalObj {
	int num_nodes;
	PSMetalObjNode *nodes[1];
} PSMetalObj;

typedef enum {
	PS_OBJECT_TUBE = 0,
	PS_OBJECT_ROD,
	PS_OBJECT_PLANE,
	PS_NUM_OBJECTS
} PSObjType;

typedef struct _PSPhysMod {
	PSObjType obj_type;
	bool decay_is_used;
	double decay_value;
	int height, circum, length;
	int plane_length, plane_width;
	int innode, outnode;
	double tenseness, speed, damping;
	int actuation;
	double velocity;
	bool stop;
	PSPercentCallback percent_callback;
} PSPhysMod;

PSMetalObj *ps_metal_obj_new_tube(int height, int circum, double tension);
PSMetalObj *ps_metal_obj_new_rod(int height, double tension);
PSMetalObj *ps_metal_obj_new_plane(int length, int width, double tension);
PSMetalObj *ps_metal_obj_new_hypercube(int dimensions, int size, double tension);

static void ps_metal_obj_perturb(PSMetalObj *obj, double speed, double damp);

int save_pres(PSPhysMod *model, const char *fname, char **errmsg);
int load_pres(PSPhysMod *model, const char *fname, char **errmsg);

static size_t ps_metal_obj_render(int rate, PSMetalObj *obj, int innode, int outnode, double speed, double damp, int compress, double velocity, int len, double *samples, PSPercentCallback *cb, double att, void *userdata);

size_t ps_metal_obj_render_tube(int rate, int height, int circum, double tension, double speed, double damp, int compress, double velocity, int len, double *samples, PSPercentCallback *cb, double att, void *userdata);
size_t ps_metal_obj_render_rod(int rate, int length, double tension, double speed, double damp, int compress, double velocity, int len, double *samples, PSPercentCallback *cb, double att, void *userdata);
size_t ps_metal_obj_render_plane(int rate, int length, int width, double tension, double speed, double damp, int compress, double velocity, int len, double *samples, PSPercentCallback *cb, double att, void *userdata);

// Utility functions.

_FORCE_INLINE_ int16_t double_to_s16(double d) {
	if (d >= 1.0)
		return 32767;
	else if (d <= -1.0)
		return -32768;
	return (int16_t)((d + 1.0) * 32767.5 - 32768.0);
}

static _FORCE_INLINE_ double s16_to_double(int16_t i) {
	return (((double)i) + 32768.0) / 32767.5 - 1.0;
}

static _FORCE_INLINE_ int16_t get_s16(FILE *in) {
	int16_t a = fgetc(in);
	return (fgetc(in) << 8) | a;
}

static _FORCE_INLINE_ void put_s16(FILE *out, int16_t i) {
	fputc(i, out);
	fputc(i >> 8, out);
}

// Core functions.

static PSMetalObjNode *ps_metal_obj_node_new(int neighbors) {
	int size = sizeof(PSMetalObjNode) + sizeof(PSMetalObjNode *) * (neighbors - 1);
	PSMetalObjNode *n = (PSMetalObjNode *)memalloc(size);
	if (n == nullptr) {
		return nullptr;
	}
	memset(n, 0, sizeof(PSMetalObjNode));
	n->num_neighbors = neighbors;
	return n;
}

static void ps_metal_obj_node_free(PSMetalObjNode *n) {
	if (n != nullptr) {
		memdelete(n);
	}
}

static PSMetalObj *ps_metal_obj_new(int size) {
	int byte_size = sizeof(PSMetalObj) + sizeof(PSMetalObjNode *) * (size - 1);
	PSMetalObj *obj = (PSMetalObj *)memalloc(byte_size);
	if (obj == nullptr) {
		return nullptr;
	}
	memset(obj, 0, sizeof(PSMetalObj));
	obj->num_nodes = size;
	return obj;
}

static void ps_metal_obj_free(PSMetalObj *obj) {
	if (obj != nullptr) {
		for (int i = 0; i < obj->num_nodes; i++) {
			ps_metal_obj_node_free(obj->nodes[i]);
		}
		memdelete(obj);
	}
}

PSMetalObj *ps_metal_obj_new_tube(int height, int circum, double tension) {
	PSMetalObj *obj = ps_metal_obj_new(height * circum);
	if (obj == nullptr) {
		return nullptr;
	}
	double radius = 0.5 / Math::cos((M_PI * (circum - 2)) / circum / 2.0);
	int n = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < circum; x++) {
			double angle = x * 2.0 * M_PI / circum;
			PSMetalObjNode *inode = ps_metal_obj_node_new((y == 0 || y == height - 1) ? 3 : 4);
			if (inode == nullptr) {
				ps_metal_obj_free(obj);
				return nullptr;
			}
			obj->nodes[n] = inode;
			inode->pos.x = Math::cos(angle) * radius;
			inode->pos.y = Math::sin(angle) * radius;
			inode->pos.z = y * tension;
			if (y == height - 1 || y == 0) {
				inode->anchor = true;
			}
			n++;
		}
	}
	n = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < circum; x++) {
			PSMetalObjNode *inode = obj->nodes[n];
			if (x == 0) {
				inode->neighbors[0] = obj->nodes[y * circum + circum - 1];
			} else {
				inode->neighbors[0] = obj->nodes[n - 1];
			}
			if (x == circum - 1) {
				inode->neighbors[1] = obj->nodes[y * circum];
			} else {
				inode->neighbors[1] = obj->nodes[n + 1];
			}
			if (y == 0) {
				inode->neighbors[2] = obj->nodes[n + circum];
			} else if (y == height - 1) {
				inode->neighbors[2] = obj->nodes[n - circum];
			} else {
				inode->neighbors[2] = obj->nodes[n + circum];
				inode->neighbors[3] = obj->nodes[n - circum];
			}
			n++;
		}
	}
	return obj;
}

PSMetalObj *ps_metal_obj_new_rod(int height, double tension) {
	PSMetalObj *obj = ps_metal_obj_new(height);
	if (obj == nullptr) {
		return nullptr;
	}
	for (int i = 0; i < height; i++) {
		PSMetalObjNode *inode = ps_metal_obj_node_new((i == 0 || i == height - 1) ? 1 : 2);
		if (inode == nullptr) {
			ps_metal_obj_free(obj);
			return nullptr;
		}
		obj->nodes[i] = inode;

		inode->pos.x = inode->pos.y = 0.0;
		inode->pos.z = i * tension;
	}
	for (int i = 0; i < height; i++) {
		PSMetalObjNode *inode = obj->nodes[i];
		if (i == 0) {
			inode->neighbors[0] = obj->nodes[1];
			inode->anchor = true;
		} else if (i == height - 1) {
			inode->neighbors[0] = obj->nodes[i - 1];
			inode->anchor = true;
		} else {
			inode->neighbors[0] = obj->nodes[i - 1];
			inode->neighbors[1] = obj->nodes[i + 1];
		}
	}
	return obj;
}

PSMetalObj *ps_metal_obj_new_plane(int length, int width, double tension) {
	PSMetalObj *obj = ps_metal_obj_new(length * width);
	if (obj == nullptr) {
		return nullptr;
	}
	int n = 0;
	for (int y = 0; y < length; y++)
		for (int x = 0; x < width; x++) {
			int count = 8;
			if (y == 0 || y == length - 1) {
				count = 5;
			}
			if (x == 0 || x == width - 1) {
				if (count == 5) {
					count = 3;
				} else {
					count = 5;
				}
			}
			PSMetalObjNode *inode = ps_metal_obj_node_new(count);
			if (inode == nullptr) {
				ps_metal_obj_free(obj);
				return nullptr;
			}
			obj->nodes[n] = inode;

			inode->pos.x = 0;
			inode->pos.y = x;
			inode->pos.z = y * tension;

			n++;
		}

	obj->nodes[0]->anchor = true;
	obj->nodes[width - 1]->anchor = true;
	obj->nodes[(length - 1) * width]->anchor = true;
	obj->nodes[(length - 1) * width + width - 1]->anchor = true;

	n = 0;
	for (int y = 0; y < length; y++) {
		for (int x = 0; x < width; x++) {
			PSMetalObjNode *inode = obj->nodes[n];
			int count = 0;
			for (int dy = -1; dy <= 1; dy++) {
				for (int dx = -1; dx <= 1; dx++) {
					if (x + dx >= 0 && x + dx < width && y + dy >= 0 && y + dy < length && (dx != 0 || dy != 0)) {
						inode->neighbors[count++] = obj->nodes[(y + dy) * width + x + dx];
					}
				}
			}
			n++;
		}
	}
	return obj;
}

PSMetalObj *ps_metal_obj_new_hypercube(int dimensions, int size, double tension) {
	int count = 1;
	for (int i = 0; i < dimensions; i++) {
		count *= size;
	}
	PSMetalObj *obj = ps_metal_obj_new(count);
	for (int i = 0; i < count; i++) {
		int value = i;
		int neighbors = 0;
		for (int j = 0; j < dimensions; j++) {
			int pos = value % size;
			if (pos > 0) {
				neighbors++;
			}
			if (pos < size - 1) {
				neighbors++;
			}
			value /= size;
		}
		obj->nodes[i] = ps_metal_obj_node_new(neighbors);
	}
	for (int i = 0; i < count; i++) {
		bool anchor = false;
		int neighbors = 0;
		int offset = 1;
		int value = i;
		for (int j = 0; j < dimensions; j++) {
			int pos = value % size;
			if (pos > 0) {
				obj->nodes[i]->neighbors[neighbors++] = obj->nodes[i - offset];
			} else {
				anchor = true;
			}
			if (pos < size - 1) {
				obj->nodes[i]->neighbors[neighbors++] = obj->nodes[i + offset];
			} else {
				anchor = true;
			}
			offset *= size;
			value /= size;
		}
		obj->nodes[i]->anchor = false;
		(void)anchor;
	}
	return obj;
}

static void ps_metal_obj_perturb(PSMetalObj *obj, double speed, double damp) {
	vector3 sum;
	vector3 dif;

	for (int i = 0; i < obj->num_nodes; i++) {
		PSMetalObjNode *inode = obj->nodes[i];
		if (!inode->anchor) {
			sum.x = sum.y = sum.z = 0.0;

			for (int j = 0; j < inode->num_neighbors; j++) {
				dif.x = inode->pos.x - inode->neighbors[j]->pos.x;
				dif.y = inode->pos.y - inode->neighbors[j]->pos.y;
				dif.z = inode->pos.z - inode->neighbors[j]->pos.z;

				double temp = 1.0 - Math::sqrt((dif.x * dif.x) + (dif.y * dif.y) + (dif.z * dif.z));

				sum.x += dif.x * temp;
				sum.y += dif.y * temp;
				sum.z += dif.z * temp;
			}
			double sprinps_k = 1;
			inode->vel.x = (inode->vel.x + sprinps_k * sum.x * speed) * damp;
			inode->vel.y = (inode->vel.y + sprinps_k * sum.y * speed) * damp;
			inode->vel.z = (inode->vel.z + sprinps_k * sum.z * speed) * damp;
		}
	}

	for (int i = 0; i < obj->num_nodes; i++) {
		PSMetalObjNode *inode = obj->nodes[i];
		if (!inode->anchor) {
			inode->pos.x += inode->vel.x * speed;
			inode->pos.y += inode->vel.y * speed;
			inode->pos.z += inode->vel.z * speed;
		}
	}
}

size_t ps_metal_obj_render_tube(int rate, int height, int circum, double tension, double speed, double damp, int compress, double velocity, int len, double *samples, PSPercentCallback *cb, double att, void *userdata) {
	PSMetalObj *obj = ps_metal_obj_new_tube(height, circum, tension);
	int innode = circum + circum / 2;
	int outnode = (height - 2) * circum;
	size_t lgth = ps_metal_obj_render(rate, obj, innode, outnode, speed, damp, compress, velocity, len, samples, cb, att, userdata);
	ps_metal_obj_free(obj);
	return lgth;
}

// Callbacks functions.

#define SAMPLE_RATE 44100

static void percent_callback(float p, void *userdata) {
	ERR_FAIL_NULL(userdata);
	PSState *s = (PSState *)userdata;
	s->progress = p;
}

static void do_render(PSState *ps) {
	LocalVector<double> data(SAMPLE_RATE * ps->sample_length);
	double decay = ps->decay_is_used ? ps->decay_value : 0;

	size_t size = data.size();
	switch (ps->obj_type) {
		case PS_OBJECT_TUBE: {
			size = ps_metal_obj_render_tube(SAMPLE_RATE, ps->height, ps->circum, ps->tenseness, ps->speed, ps->damping, ps->actuation, ps->velocity, size, data.ptr(), percent_callback, decay, ps);
		} break;
		case PS_OBJECT_ROD: {
			size = ps_metal_obj_render_rod(SAMPLE_RATE, ps->length, ps->tenseness, ps->speed, ps->damping, ps->actuation, ps->velocity, size, data.ptr(), percent_callback, decay, ps);
		} break;
		case PS_OBJECT_PLANE: {
			size = ps_metal_obj_render_plane(SAMPLE_RATE, ps->plane_length, ps->plane_width, ps->tenseness, ps->speed, ps->damping, ps->actuation, ps->velocity, size, data.ptr(), percent_callback, decay, ps);
		} break;
	}

	ERR_FAIL_COND(size > data.size());

	ps->samples.resize(data.size());
	for (int i = 0; i < size; i++) {
		ps->samples[i] = double_to_s16(data[i]);
	}
}
