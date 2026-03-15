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

#include "common/gd_core.h"
#include "core/error_macros.h"
#include "core/math/math_defs.h"
#include "core/math/math_funcs.h"
#include "core/os/memory.h"
#include "core/print_string.h"
#include "core/script_language.h"
#include "core/typedefs.h"
#include "editor/audio_stream_preview.h"
#include "scene/gui/progress_bar.h"
#include "scene/resources/audio_stream_sample.h"
#include "scene/resources/resource_format_text.h"

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

typedef struct _vector3 {
	double x, y, z;
} vector3;

typedef void PSPercentCallback(float percent, void *userdata);

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
static size_t ps_metal_obj_render(int rate, PSMetalObj *obj, int innode, int outnode, double speed, double damp, int compress, double velocity, int len, double *samples, PSPercentCallback *cb, double att, void *userdata);

size_t ps_metal_obj_render_tube(int rate, int height, int circum, double tension, double speed, double damp, int compress, double velocity, int len, double *samples, PSPercentCallback *cb, double att, void *userdata);
size_t ps_metal_obj_render_rod(int rate, int length, double tension, double speed, double damp, int compress, double velocity, int len, double *samples, PSPercentCallback *cb, double att, void *userdata);
size_t ps_metal_obj_render_plane(int rate, int length, int width, double tension, double speed, double damp, int compress, double velocity, int len, double *samples, PSPercentCallback *cb, double att, void *userdata);

int save_pres(PSPhysMod *model, const char *fname, char **errmsg);
int load_pres(PSPhysMod *model, const char *fname, char **errmsg);

// Utility functions.

static _FORCE_INLINE_ int16_t double_to_s16(double d) {
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

// Now len means _maximal_ lenght if the given attenuation will not be reached;
// for disabling stopping at given attenuation, use attenuation = 0.0.
// Attenuation is given in dB, att = 60.0 means render will be stopped after
// the mean amplitude reach the value of -60 dB.

static size_t ps_metal_obj_render(int rate, PSMetalObj *obj, int innode, int outnode, double speed, double damp, int compress, double velocity, int len, double *samples, PSPercentCallback *cb, double att, void *userdata) {
	double stasis;
	double sample;

	if (compress) {
		stasis = obj->nodes[outnode]->pos.z;
		obj->nodes[innode]->pos.z += velocity;
	} else {
		stasis = obj->nodes[outnode]->pos.x;
		obj->nodes[innode]->pos.x += velocity;
	}

	damp = Math::pow(0.5, 1.0 / (damp * rate));

	double curr_att = 0, hipass = 0, lowpass = 0, maxamp = 0;
	double hipass_coeff = Math::pow(0.5, 5.0 / rate);
	double lowpass_coeff = 1 - 20.0 / rate; // 50 ms integrator
	double maxvol = 0.001;
	size_t real_len = 0;
	for (int i = 0; i < len; i++) {
		ps_metal_obj_perturb(obj, speed, damp);
		if (compress) {
			sample = obj->nodes[outnode]->pos.z - stasis;
		} else {
			sample = obj->nodes[outnode]->pos.x - stasis;
		}
		hipass = hipass_coeff * hipass + (1 - hipass_coeff) * sample;
		samples[i] = sample - hipass;
		if (Math::abs(samples[i]) > maxvol) {
			maxvol = Math::abs(samples[i]);
		}
		lowpass = lowpass_coeff * lowpass + (1 - lowpass_coeff) * Math::abs(samples[i]);
		if (maxamp < lowpass) {
			maxamp = lowpass;
		}
		if (att < 0) {
			if (maxamp > 0) {
				curr_att = 20 * Math::log10(lowpass / maxamp);
			}
			if (!(i & 1023)) {
				if (cb != nullptr) {
					float p1 = ((float)i) / len;
					float p2 = curr_att / att;
					cb(MAX(p1, p2), userdata);
				}
			}
			if (curr_att <= att) {
				break;
			}
		} else {
			if (!(i & 1023)) {
				if (cb != nullptr)
					cb(((float)i) / len, userdata);
			}
		}
		real_len++;
	}

	maxvol = 1.0 / maxvol;
	for (int i = 0; i < real_len; i++) {
		samples[i] *= maxvol;
	}
	return real_len;
}

size_t ps_metal_obj_render_tube(int rate, int height, int circum, double tension, double speed, double damp, int compress, double velocity, int len, double *samples, PSPercentCallback *cb, double att, void *userdata) {
	PSMetalObj *obj = ps_metal_obj_new_tube(height, circum, tension);
	int innode = circum + circum / 2;
	int outnode = (height - 2) * circum;
	size_t lgth = ps_metal_obj_render(rate, obj, innode, outnode, speed, damp, compress, velocity, len, samples, cb, att, userdata);
	ps_metal_obj_free(obj);
	return lgth;
}

size_t ps_metal_obj_render_rod(int rate, int length, double tension, double speed, double damp, int compress, double velocity, int len, double *samples, PSPercentCallback *cb, double att, void *userdata) {
	PSMetalObj *obj = ps_metal_obj_new_rod(length, tension);
	int innode = 1;
	int outnode = length - 2;
	size_t lgth = ps_metal_obj_render(rate, obj, innode, outnode, speed, damp, compress, velocity, len, samples, cb, att, userdata);
	ps_metal_obj_free(obj);
	return lgth;
}

size_t ps_metal_obj_render_plane(int rate, int length, int width, double tension, double speed, double damp, int compress, double velocity, int len, double *samples, PSPercentCallback *cb, double att, void *userdata) {
	PSMetalObj *obj = ps_metal_obj_new_plane(length, width, tension);
	int innode = 1;
	int outnode = (length - 1) * width - 1;
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

static bool do_render(PSState *ps) {
	LocalVector<double> data(SAMPLE_RATE * ps->sample_length);
	const double decay = ps->decay_is_used ? ps->decay_value : 0;

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

	ERR_FAIL_COND_V(size > data.size(), false);

	ps->samples.resize(data.size());
	for (int i = 0; i < size; i++) {
		ps->samples[i] = double_to_s16(data[i]);
	}

	return true;
}

// UI description
static const char *_ps_ui = R"TSCN(
[gd_scene load_steps=2 format=2]

[node name="window" type="AcceptDialog"]
margin_right = 300.0
margin_bottom = 240.0
rect_min_size = Vector2( 640, 560 )
focus_mode = 2
window_title = "Power Station Editor"
resizable = true

[node name="body" type="VBoxContainer" parent="."]
margin_left = 8.0
margin_top = 8.0
margin_right = 632.0
margin_bottom = 524.0
size_flags_horizontal = 3
size_flags_vertical = 3
custom_constants/separation = 12
__meta__ = {
"_edit_use_anchors_": false
}

[node name="objtype_body" type="HBoxContainer" parent="body"]

[node name="objtype_label" type="Label" parent="body/objtype_body"]
text = "Object type"

[node name="objtype_value" type="OptionButton" parent="body/objtype_body"]
size_flags_horizontal = 3
items = [ "Tube", null, false, 0, null, "Rod", null, false, 1, null, "Plane", null, false, 2, null ]
selected = 0

[node name="controls" type="GridContainer" parent="body"]
size_flags_horizontal = 3
layout_mode = 2
columns = 3

[node name="height_label" type="Label" parent="body/controls"]
text = "Height"

[node name="height_value" type="HScrollBar" parent="body/controls"]
min_value = 3
max_value = 50
step = 1
value = 10
size_flags_horizontal = 3

[node name="height_value_label" type="Label" parent="body/controls"]
rect_min_size = Vector2( 80, 1 )
text = "10"

[node name="circumference_label" type="Label" parent="body/controls"]
text = "Circumference"

[node name="circumference_value" type="HScrollBar" parent="body/controls"]
min_value = 3
max_value = 20
step = 1
value = 5
size_flags_horizontal = 3

[node name="circumference_value_label" type="Label" parent="body/controls"]
rect_min_size = Vector2( 80, 1 )
text = "5"

[node name="length_label" type="Label" parent="body/controls"]
text = "Length (Rod)"

[node name="length_value" type="HScrollBar" parent="body/controls"]
min_value = 3
max_value = 50
step = 1
value = 5
size_flags_horizontal = 3

[node name="length_value_label" type="Label" parent="body/controls"]
rect_min_size = Vector2( 80, 1 )
text = "5"

[node name="plane_length_label" type="Label" parent="body/controls"]
text = "Plane length"

[node name="plane_length_value" type="HScrollBar" parent="body/controls"]
min_value = 3
max_value = 20
step = 1
value = 7
size_flags_horizontal = 3

[node name="plane_length_value_label" type="Label" parent="body/controls"]
rect_min_size = Vector2( 80, 1 )
text = "7"

[node name="plane_width_label" type="Label" parent="body/controls"]
text = "Plane width"

[node name="plane_width_value" type="HScrollBar" parent="body/controls"]
min_value = 3
max_value = 20
step = 1
value = 9
size_flags_horizontal = 3

[node name="plane_width_value_label" type="Label" parent="body/controls"]
rect_min_size = Vector2( 80, 1 )
text = "9"

[node name="tenseness_label" type="Label" parent="body/controls"]
text = "Tenseness"

[node name="tenseness_value" type="HScrollBar" parent="body/controls"]
min_value = 0.1
max_value = 10
step = 0.1
value = 4
size_flags_horizontal = 3

[node name="tenseness_value_label" type="Label" parent="body/controls"]
rect_min_size = Vector2( 80, 1 )
text = "4.0"

[node name="speed_label" type="Label" parent="body/controls"]
text = "Speed"

[node name="speed_value" type="HScrollBar" parent="body/controls"]
min_value = 0.01
max_value = 1
step = 0.01
value = 0.2
size_flags_horizontal = 3

[node name="speed_value_label" type="Label" parent="body/controls"]
rect_min_size = Vector2( 80, 1 )
text = "0.20"

[node name="damping_label" type="Label" parent="body/controls"]
text = "Damping"

[node name="damping_value" type="HScrollBar" parent="body/controls"]
min_value = 0.01
max_value = 1
step = 0.01
value = 0.05
size_flags_horizontal = 3

[node name="damping_value_label" type="Label" parent="body/controls"]
rect_min_size = Vector2( 80, 1 )
text = "0.05"

[node name="actuation_label" type="Label" parent="body/controls"]
text = "Actuation (compress)"

[node name="actuation_value" type="CheckBox" parent="body/controls"]

[node name="fill_actuation" type="Label" parent="body/controls"]

[node name="velocity_label" type="Label" parent="body/controls"]
text = "Velocity"

[node name="velocity_value" type="HScrollBar" parent="body/controls"]
min_value = 0.1
max_value = 10
step = 0.1
value = 1
size_flags_horizontal = 3

[node name="velocity_value_label" type="Label" parent="body/controls"]
rect_min_size = Vector2( 80, 1 )
text = "1.0"

[node name="sample_length_label" type="Label" parent="body/controls"]
text = "Sample length (s)"

[node name="sample_length_value" type="HScrollBar" parent="body/controls"]
min_value = 0.5
max_value = 10
step = 0.1
value = 1
size_flags_horizontal = 3

[node name="sample_length_value_label" type="Label" parent="body/controls"]
rect_min_size = Vector2( 80, 1 )
text = "1.0"

[node name="decay_body" type="HBoxContainer" parent="body"]

[node name="decay_label" type="Label" parent="body/decay_body"]
text = "Decay"

[node name="decay_enabled" type="CheckBox" parent="body/decay_body"]

[node name="decay_value_label" type="Label" parent="body/decay_body"]
text = "Decay (dB)"

[node name="decay_value" type="HScrollBar" parent="body/decay_body"]
min_value = -120
max_value = 0
step = 1
value = 0
size_flags_horizontal = 3

[node name="decay_value_value_label" type="Label" parent="body/decay_body"]
rect_min_size = Vector2( 80, 1 )
text = "0"

[node name="output_body" type="HBoxContainer" parent="body"]

[node name="outputfile_label" type="Label" parent="body/output_body"]
text = "Save as file"

[node name="outputfile_value" type="LineEdit" parent="body/output_body"]
size_flags_horizontal = 3
text = "metal_sound.wav"

[node name="save" type="Button" parent="body/output_body"]
text = " Save "

[node name="progress_body" type="HBoxContainer" parent="body"]

[node name="generate" type="Button" parent="body/progress_body"]
text = " Generate "

[node name="progress" type="ProgressBar" parent="body/progress_body"]
size_flags_horizontal = 3

[node name="preview" type="AudioStreamPlayerControl" parent="body"]
size_flags_horizontal = 3
size_flags_vertical = 3

[connection signal="about_to_show" from="." to="." method="_on_window_about_to_show"]
[connection signal="popup_hide" from="." to="." method="_on_window_popup_hide"]
[connection signal="resized" from="." to="." method="_on_window_resized"]
[connection signal="visibility_changed" from="." to="." method="_on_window_visibility_changed"]
[connection signal="item_selected" from="body/objtype_body/objtype_value" to="." method="_on_objtype_selected"]
[connection signal="value_changed" from="body/controls/height_value" to="." method="_on_value_changed" binds= [ "height" ]]
[connection signal="value_changed" from="body/controls/circumference_value" to="." method="_on_value_changed" binds= [ "circumference" ]]
[connection signal="value_changed" from="body/controls/length_value" to="." method="_on_value_changed" binds= [ "length" ]]
[connection signal="value_changed" from="body/controls/plane_length_value" to="." method="_on_value_changed" binds= [ "plane_length" ]]
[connection signal="value_changed" from="body/controls/plane_width_value" to="." method="_on_value_changed" binds= [ "plane_width" ]]
[connection signal="value_changed" from="body/controls/tenseness_value" to="." method="_on_value_changed" binds= [ "tenseness" ]]
[connection signal="value_changed" from="body/controls/speed_value" to="." method="_on_value_changed" binds= [ "speed" ]]
[connection signal="value_changed" from="body/controls/damping_value" to="." method="_on_value_changed" binds= [ "damping" ]]
[connection signal="value_changed" from="body/controls/velocity_value" to="." method="_on_value_changed" binds= [ "velocity" ]]
[connection signal="value_changed" from="body/controls/sample_length_value" to="." method="_on_value_changed" binds= [ "sample_length" ]]
[connection signal="value_changed" from="body/decay_body/decay_value" to="." method="_on_value_changed" binds= [ "decay" ]]
[connection signal="pressed" from="body/progress_body/generate" to="." method="_on_generate_pressed"]
[connection signal="pressed" from="body/output_body/save" to="." method="_on_save_pressed"]
)TSCN";

// BEGIN Power Station generator

typedef PassthroughScript<AcceptDialog, PowerStationGenerator> PowerStationUIScriptInstanceBase;

class PowerStationUIScript : public PowerStationUIScriptInstanceBase {
	GDCLASS(PowerStationUIScript, PowerStationUIScriptInstanceBase)

	_THREAD_SAFE_CLASS_

public:
	PowerStationUIScript(PowerStationGenerator *p_recv) {
		set_receiver(p_recv);
	}
};

// Property accessors

void PowerStationGenerator::set_obj_type(int p_type) {
	ERR_FAIL_INDEX(p_type, 3);
	if (state.obj_type != p_type) {
		state.obj_type = p_type;
		if (auto_generate) {
			call_deferred("generate");
		}
	}
}

int PowerStationGenerator::get_obj_type() const {
	return state.obj_type;
}

void PowerStationGenerator::set_height(int p_height) {
	ERR_FAIL_COND(p_height < 3);
	if (state.height != p_height) {
		state.height = p_height;
		if (auto_generate) {
			call_deferred("generate");
		}
	}
}

int PowerStationGenerator::get_height() const {
	return state.height;
}

void PowerStationGenerator::set_circumference(int p_circum) {
	ERR_FAIL_COND(p_circum < 3);
	if (state.circum != p_circum) {
		state.circum = p_circum;
		if (auto_generate) {
			call_deferred("generate");
		}
	}
}

int PowerStationGenerator::get_circumference() const {
	return state.circum;
}

void PowerStationGenerator::set_length(int p_length) {
	ERR_FAIL_COND(p_length < 3);
	if (state.length != p_length) {
		state.length = p_length;
		if (auto_generate) {
			call_deferred("generate");
		}
	}
}

int PowerStationGenerator::get_length() const {
	return state.length;
}

void PowerStationGenerator::set_plane_length(int p_length) {
	ERR_FAIL_COND(p_length < 3);
	if (state.plane_length != p_length) {
		state.plane_length = p_length;
		if (auto_generate) {
			call_deferred("generate");
		}
	}
}

int PowerStationGenerator::get_plane_length() const {
	return state.plane_length;
}

void PowerStationGenerator::set_plane_width(int p_width) {
	ERR_FAIL_COND(p_width < 3);
	if (state.plane_width != p_width) {
		state.plane_width = p_width;
		if (auto_generate) {
			call_deferred("generate");
		}
	}
}

int PowerStationGenerator::get_plane_width() const {
	return state.plane_width;
}

void PowerStationGenerator::set_tenseness(real_t p_val) {
	ERR_FAIL_COND(p_val <= 0);
	if (state.tenseness != p_val) {
		state.tenseness = p_val;
		if (auto_generate) {
			call_deferred("generate");
		}
	}
}

real_t PowerStationGenerator::get_tenseness() const {
	return state.tenseness;
}

void PowerStationGenerator::set_speed(real_t p_val) {
	ERR_FAIL_COND(p_val <= 0);
	if (state.speed != p_val) {
		state.speed = p_val;
		if (auto_generate) {
			call_deferred("generate");
		}
	}
}

real_t PowerStationGenerator::get_speed() const {
	return state.speed;
}

void PowerStationGenerator::set_damping(real_t p_val) {
	ERR_FAIL_COND(p_val <= 0);
	if (state.damping != p_val) {
		state.damping = p_val;
		if (auto_generate) {
			call_deferred("generate");
		}
	}
}

real_t PowerStationGenerator::get_damping() const {
	return state.damping;
}

void PowerStationGenerator::set_actuation(int p_val) {
	ERR_FAIL_COND(p_val < 0 || p_val > 1);
	if (state.actuation != p_val) {
		state.actuation = p_val;
		if (auto_generate) {
			call_deferred("generate");
		}
	}
}

int PowerStationGenerator::get_actuation() const {
	return state.actuation;
}

void PowerStationGenerator::set_velocity(real_t p_val) {
	ERR_FAIL_COND(p_val <= 0);
	if (state.velocity != p_val) {
		state.velocity = p_val;
		if (auto_generate) {
			call_deferred("generate");
		}
	}
}

real_t PowerStationGenerator::get_velocity() const {
	return state.velocity;
}

void PowerStationGenerator::set_sample_length(real_t p_val) {
	ERR_FAIL_COND(p_val <= 0);
	if (state.sample_length != p_val) {
		state.sample_length = p_val;
		if (auto_generate) {
			call_deferred("generate");
		}
	}
}

real_t PowerStationGenerator::get_sample_length() const {
	return state.sample_length;
}

void PowerStationGenerator::set_decay_enabled(bool p_val) {
	if (state.decay_is_used != p_val) {
		state.decay_is_used = p_val;
		if (auto_generate) {
			call_deferred("generate");
		}
	}
}

bool PowerStationGenerator::is_decay_enabled() const {
	return state.decay_is_used;
}

void PowerStationGenerator::set_decay_value(real_t p_val) {
	if (state.decay_value != p_val) {
		state.decay_value = p_val;
		if (auto_generate) {
			call_deferred("generate");
		}
	}
}

real_t PowerStationGenerator::get_decay_value() const {
	return state.decay_value;
}

void PowerStationGenerator::set_sound_quality(int p_quality) {
	ERR_FAIL_INDEX(p_quality, 2);
	sample_format = p_quality;
}

int PowerStationGenerator::get_sound_quality() const {
	return sample_format;
}

void PowerStationGenerator::set_output_file(const String &p_path) {
	output_file = p_path;
}

String PowerStationGenerator::get_output_file() const {
	return output_file;
}

void PowerStationGenerator::generate() {
	do_render(&state);
	emit_signal("sound_process_ready", get_samples());
}

Ref<AudioStreamSample> PowerStationGenerator::get_samples() const {
	static const AudioStreamSample::Format _fmt[] = { AudioStreamSample::FORMAT_8_BITS, AudioStreamSample::FORMAT_16_BITS };
	Ref<AudioStreamSample> sample;
	sample.instance();

	const int num_samples = state.samples.size();
	if (num_samples > 0) {
		PoolVector<uint8_t> data;
		if (sample_format == 0) {
			// 8-bit
			data.resize(num_samples);
			PoolVector<uint8_t>::Write w = data.write();
			for (int i = 0; i < num_samples; i++) {
				double d = s16_to_double(state.samples[i]);
				w[i] = (uint8_t)CLAMP((int)((d + 1.0) * 127.5), 0, 255);
			}
		} else {
			// 16-bit
			data.resize(num_samples * 2);
			PoolVector<uint8_t>::Write w = data.write();
			int16_t *dst = (int16_t *)w.ptr();
			for (int i = 0; i < num_samples; i++) {
				dst[i] = state.samples[i];
			}
		}
		sample->set_data(data);
	}

	sample->set_format(_fmt[sample_format]);
	sample->set_mix_rate(SAMPLE_RATE);
	sample->set_loop_mode(AudioStreamSample::LOOP_DISABLED);
	sample->set_loop_begin(0);
	sample->set_loop_end(0);
	sample->set_stereo(false);

	return sample;
}

#ifdef TOOLS_ENABLED
AcceptDialog *PowerStationGenerator::load_ui() {
	if (!dlg) {
		ResourceFormatLoaderText rl;
		Ref<PackedScene> ui = rl.load_from_data(_ps_ui, "power_station_ui.tscn");
		if (ui) {
			dlg = cast_to<AcceptDialog>(ui->instance());
			dlg_script = newref(PowerStationUIScript, this);
			dlg->set_script(dlg_script.get_ref_ptr());
			if ((cleanup = memnew(Timer))) {
				cleanup->set_one_shot(false);
				cleanup->set_wait_time(1);
				cleanup->set_timer_process_mode(Timer::TIMER_PROCESS_IDLE);
				cleanup->connect("timeout", this, "_cleanup_ui");
				dlg->add_child(cleanup);
			}
		}
	}
	return dlg;
}

void PowerStationGenerator::open_ui() {
	dlg->popup_centered_ratio(0.25);
}

void PowerStationGenerator::_cleanup_ui() {
	ERR_FAIL_NULL(dlg);
	if (ProgressBar *bar = cast_to<ProgressBar>(dlg->get_node_or_null(String("body/progress_body/progress")))) {
		bar->set_value(0);
	}
}

void PowerStationGenerator::_on_sound_progress(real_t p_progress) {
	ERR_FAIL_NULL(dlg);
	if (ProgressBar *bar = cast_to<ProgressBar>(dlg->get_node_or_null(String("body/progress_body/progress")))) {
		bar->set_value(p_progress * 100);
	}
}

void PowerStationGenerator::_on_sound_ready(Ref<AudioStreamSample> sound) {
	cleanup->start();
	if (AudioStreamPlayerControl *player = cast_to<AudioStreamPlayerControl>(dlg->get_node_or_null(String("body/preview")))) {
		player->set_stream(sound);
	}
}

void PowerStationGenerator::_on_generate_pressed() {
	generate();
}

void PowerStationGenerator::_on_save_pressed() {
	if (LineEdit *edt = cast_to<LineEdit>(dlg->get_node_or_null(String("body/output_body/outputfile_value")))) {
		if (!edt->get_text().empty()) {
			get_samples()->save_to_wav(edt->get_text());
		}
	}
}

void PowerStationGenerator::_on_objtype_selected(int p_index) {
	set_obj_type(p_index);
}

void PowerStationGenerator::_on_value_changed(float value, String node) {
	const String value_node = "body/controls/" + node + "_value";
	const String label_node = "body/controls/" + node + "_value_label";

	// Handle decay slider separately (it lives in decay_body)
	if (node == "decay") {
		if (HScrollBar *scr = cast_to<HScrollBar>(dlg->get_node_or_null(String("body/decay_body/decay_value")))) {
			const float v = scr->get_value();
			if (Label *lbl = cast_to<Label>(dlg->get_node_or_null(String("body/decay_body/decay_value_value_label")))) {
				lbl->set_text(itos(v));
			}
			state.decay_value = v;
			_change_notify();
		}
		return;
	}

	if (Label *lbl = cast_to<Label>(dlg->get_node_or_null(label_node))) {
		if (HScrollBar *scr = cast_to<HScrollBar>(dlg->get_node_or_null(value_node))) {
			const float v = scr->get_value();
			if (scr->get_step() == 1) { // int
				lbl->set_text(itos(v));
			} else {
				lbl->set_text(String::num(v, 2));
			}
			if (node == "height") {
				state.height = v;
			} else if (node == "circumference") {
				state.circum = v;
			} else if (node == "length") {
				state.length = v;
			} else if (node == "plane_length") {
				state.plane_length = v;
			} else if (node == "plane_width") {
				state.plane_width = v;
			} else if (node == "tenseness") {
				state.tenseness = v;
			} else if (node == "speed") {
				state.speed = v;
			} else if (node == "damping") {
				state.damping = v;
			} else if (node == "velocity") {
				state.velocity = v;
			} else if (node == "sample_length") {
				state.sample_length = v;
			}
			_change_notify();
		}
	}
}

void PowerStationGenerator::_on_window_about_to_show() {
}

void PowerStationGenerator::_on_window_popup_hide() {
}

void PowerStationGenerator::_on_window_resized() {
}

void PowerStationGenerator::_on_window_visibility_changed() {
}
#endif

void PowerStationGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_obj_type", "type"), &PowerStationGenerator::set_obj_type);
	ClassDB::bind_method(D_METHOD("get_obj_type"), &PowerStationGenerator::get_obj_type);
	ClassDB::bind_method(D_METHOD("set_height", "height"), &PowerStationGenerator::set_height);
	ClassDB::bind_method(D_METHOD("get_height"), &PowerStationGenerator::get_height);
	ClassDB::bind_method(D_METHOD("set_circumference", "circum"), &PowerStationGenerator::set_circumference);
	ClassDB::bind_method(D_METHOD("get_circumference"), &PowerStationGenerator::get_circumference);
	ClassDB::bind_method(D_METHOD("set_length", "length"), &PowerStationGenerator::set_length);
	ClassDB::bind_method(D_METHOD("get_length"), &PowerStationGenerator::get_length);
	ClassDB::bind_method(D_METHOD("set_plane_length", "length"), &PowerStationGenerator::set_plane_length);
	ClassDB::bind_method(D_METHOD("get_plane_length"), &PowerStationGenerator::get_plane_length);
	ClassDB::bind_method(D_METHOD("set_plane_width", "width"), &PowerStationGenerator::set_plane_width);
	ClassDB::bind_method(D_METHOD("get_plane_width"), &PowerStationGenerator::get_plane_width);
	ClassDB::bind_method(D_METHOD("set_tenseness", "val"), &PowerStationGenerator::set_tenseness);
	ClassDB::bind_method(D_METHOD("get_tenseness"), &PowerStationGenerator::get_tenseness);
	ClassDB::bind_method(D_METHOD("set_speed", "val"), &PowerStationGenerator::set_speed);
	ClassDB::bind_method(D_METHOD("get_speed"), &PowerStationGenerator::get_speed);
	ClassDB::bind_method(D_METHOD("set_damping", "val"), &PowerStationGenerator::set_damping);
	ClassDB::bind_method(D_METHOD("get_damping"), &PowerStationGenerator::get_damping);
	ClassDB::bind_method(D_METHOD("set_actuation", "val"), &PowerStationGenerator::set_actuation);
	ClassDB::bind_method(D_METHOD("get_actuation"), &PowerStationGenerator::get_actuation);
	ClassDB::bind_method(D_METHOD("set_velocity", "val"), &PowerStationGenerator::set_velocity);
	ClassDB::bind_method(D_METHOD("get_velocity"), &PowerStationGenerator::get_velocity);
	ClassDB::bind_method(D_METHOD("set_sample_length", "val"), &PowerStationGenerator::set_sample_length);
	ClassDB::bind_method(D_METHOD("get_sample_length"), &PowerStationGenerator::get_sample_length);
	ClassDB::bind_method(D_METHOD("set_decay_enabled", "val"), &PowerStationGenerator::set_decay_enabled);
	ClassDB::bind_method(D_METHOD("is_decay_enabled"), &PowerStationGenerator::is_decay_enabled);
	ClassDB::bind_method(D_METHOD("set_decay_value", "val"), &PowerStationGenerator::set_decay_value);
	ClassDB::bind_method(D_METHOD("get_decay_value"), &PowerStationGenerator::get_decay_value);
	ClassDB::bind_method(D_METHOD("set_sound_quality", "quality"), &PowerStationGenerator::set_sound_quality);
	ClassDB::bind_method(D_METHOD("get_sound_quality"), &PowerStationGenerator::get_sound_quality);
	ClassDB::bind_method(D_METHOD("set_output_file", "filename"), &PowerStationGenerator::set_output_file);
	ClassDB::bind_method(D_METHOD("get_output_file"), &PowerStationGenerator::get_output_file);
	ClassDB::bind_method(D_METHOD("generate"), &PowerStationGenerator::generate);

#ifdef TOOLS_ENABLED
	ClassDB::bind_method(D_METHOD("_cleanup_ui"), &PowerStationGenerator::_cleanup_ui);
	ClassDB::bind_method(D_METHOD("_on_generate_pressed"), &PowerStationGenerator::_on_generate_pressed);
	ClassDB::bind_method(D_METHOD("_on_save_pressed"), &PowerStationGenerator::_on_save_pressed);
	ClassDB::bind_method(D_METHOD("_on_sound_progress"), &PowerStationGenerator::_on_sound_progress);
	ClassDB::bind_method(D_METHOD("_on_sound_ready", "sound"), &PowerStationGenerator::_on_sound_ready);
	ClassDB::bind_method(D_METHOD("_on_value_changed", "value"), &PowerStationGenerator::_on_value_changed);
	ClassDB::bind_method(D_METHOD("_on_objtype_selected", "index"), &PowerStationGenerator::_on_objtype_selected);
	ClassDB::bind_method(D_METHOD("_on_window_about_to_show"), &PowerStationGenerator::_on_window_about_to_show);
	ClassDB::bind_method(D_METHOD("_on_window_popup_hide"), &PowerStationGenerator::_on_window_popup_hide);
	ClassDB::bind_method(D_METHOD("_on_window_resized"), &PowerStationGenerator::_on_window_resized);
	ClassDB::bind_method(D_METHOD("_on_window_visibility_changed"), &PowerStationGenerator::_on_window_visibility_changed);
#endif

	ADD_PROPERTY(PropertyInfo(Variant::INT, "obj_type", PROPERTY_HINT_ENUM, "Tube,Rod,Plane"), "set_obj_type", "get_obj_type");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "height", PROPERTY_HINT_RANGE, "3,50,1"), "set_height", "get_height");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "circumference", PROPERTY_HINT_RANGE, "3,20,1"), "set_circumference", "get_circumference");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "length", PROPERTY_HINT_RANGE, "3,50,1"), "set_length", "get_length");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "plane_length", PROPERTY_HINT_RANGE, "3,20,1"), "set_plane_length", "get_plane_length");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "plane_width", PROPERTY_HINT_RANGE, "3,20,1"), "set_plane_width", "get_plane_width");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "tenseness", PROPERTY_HINT_RANGE, "0.1,10,0.1"), "set_tenseness", "get_tenseness");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "speed", PROPERTY_HINT_RANGE, "0.01,1,0.01"), "set_speed", "get_speed");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "damping", PROPERTY_HINT_RANGE, "0.01,1,0.01"), "set_damping", "get_damping");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "actuation", PROPERTY_HINT_RANGE, "0,1,1"), "set_actuation", "get_actuation");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "velocity", PROPERTY_HINT_RANGE, "0.1,10,0.1"), "set_velocity", "get_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "sample_length", PROPERTY_HINT_RANGE, "0.5,10,0.1"), "set_sample_length", "get_sample_length");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "decay_enabled"), "set_decay_enabled", "is_decay_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "decay_value", PROPERTY_HINT_RANGE, "-120,0,1"), "set_decay_value", "get_decay_value");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "sound_quality", PROPERTY_HINT_ENUM, "8 bit,16 bit"), "set_sound_quality", "get_sound_quality");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "output_file"), "set_output_file", "get_output_file");

	ADD_SIGNAL(MethodInfo("sound_process_progress", PropertyInfo(Variant::REAL, "progress")));
	ADD_SIGNAL(MethodInfo("sound_process_ready", PropertyInfo(Variant::OBJECT, "sound")));
}

PowerStationGenerator::PowerStationGenerator() {
	state = _ps_defaults;
	sample_format = 1;
	auto_generate = false;
	dlg = nullptr;
	cleanup = nullptr;

#ifdef TOOLS_ENABLED
	connect("sound_process_progress", this, "_on_sound_progress");
	connect("sound_process_ready", this, "_on_sound_ready");
#endif
}

// END Power Station generator

// BEGIN Godot editor plugin

void PowerStationEditorPlugin::add_icons_menu_item(const String &p_name, const String &p_callback) {
	if (int(Engine::get_singleton()->get_version_info()["hex"]) >= 0x030100) {
		add_tool_menu_item(p_name, this, p_callback);
	}
}

void PowerStationEditorPlugin::remove_icons_menu_item(const String &p_name) {
	if (int(Engine::get_singleton()->get_version_info()["hex"]) >= 0x030100) {
		remove_tool_menu_item(p_name);
	}
}

void PowerStationEditorPlugin::_on_show_ps_editor_pressed(Variant p_null) {
	gen->open_ui();
}

void PowerStationEditorPlugin::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			if (AcceptDialog *dlg = gen->load_ui()) {
				get_editor_interface()->get_base_control()->add_child(dlg);
			}
		} break;
		case NOTIFICATION_ENTER_TREE: {
			add_icons_menu_item("Power Station Editor", "_on_show_ps_editor_pressed");
		} break;
		case NOTIFICATION_EXIT_TREE: {
			remove_icons_menu_item("Power Station Editor");
		} break;
	}
}

void PowerStationEditorPlugin::generate() {
	gen->generate();
}

void PowerStationEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_show_ps_editor_pressed"), &PowerStationEditorPlugin::_on_show_ps_editor_pressed);
}

PowerStationEditorPlugin::PowerStationEditorPlugin(EditorNode *p_node) {
	editor = p_node;
	gen = newref(PowerStationGenerator);
}

// END
