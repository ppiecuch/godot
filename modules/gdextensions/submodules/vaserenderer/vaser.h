/*************************************************************************/
/*  vaser.h                                                              */
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

#ifndef VASER_H
#define VASER_H

/* Vase Renderer first draft, version 0.3 */

#include "core/color.h"
#include "core/math/math_decl.h"
#include "core/math/vector2.h"

namespace VASEr {

struct gradient_stop {
	real_t t; //position
	char type; //GS_xx
	union {
		Color color;
		real_t weight;
	};
};
const char GS_none = 0;
const char GS_rgba = 1;
const char GS_rgb = 2; //rgb only
const char GS_alpha = 3;
const char GS_weight = 4;
struct gradient {
	gradient_stop *stops; //array must be sorted in ascending order of t
	int length; //number of stops
	char unit; //use_GD_XX
};
const char GD_ratio = 0; //default
const char GD_length = 1;

class renderer {
public:
	static void init();
	static void before();
	static void after();
};

struct tessellator_opt {
	//set the whole structure to 0 will give default options
	bool triangulation;
	char parts; //use TS_xx
	bool tessellate_only;
	void *holder; //used as (VASErin::vertex_array_holder*) if tessellate_only is true
};
//for tessellator_opt.parts
const char TS_core_fade = 0; //default
const char TS_core = 1;
const char TS_outer_fade = 2;
const char TS_inner_fade = 3;

struct polyline_opt {
	//set the whole structure to 0 will give default options
	const tessellator_opt *tess;
	char joint; //use PLJ_xx
	char cap; //use PLC_xx
	bool feather;
	real_t feathering;
	bool no_feather_at_cap;
	bool no_feather_at_core;
};
//for polyline_opt.joint
const char PLJ_miter = 0; //default
const char PLJ_bevel = 1;
const char PLJ_round = 2;
//for polyline_opt.cap
const char PLC_butt = 0; //default
const char PLC_round = 1;
const char PLC_square = 2;
const char PLC_rect = 3;
const char PLC_both = 0; //default
const char PLC_first = 10;
const char PLC_last = 20;
const char PLC_none = 30;

void polyline(const Vector2 *, const Color *, const real_t *, int length, const polyline_opt *);
void polyline(const Vector2 *, Color, real_t W, int length, const polyline_opt *); //constant color and weight
void polyline(const Vector2 *, const Color *, real_t W, int length, const polyline_opt *); //constant weight
void polyline(const Vector2 *, Color, const real_t *W, int length, const polyline_opt *); //constant color
void segment(Vector2, Vector2, Color, Color, real_t W1, real_t W2, const polyline_opt *);
void segment(Vector2, Vector2, Color, real_t W, const polyline_opt *); //constant color and weight
void segment(Vector2, Vector2, Color, Color, real_t W, const polyline_opt *); //constant weight
void segment(Vector2, Vector2, Color, real_t W1, real_t W2, const polyline_opt *); //const color

struct polybezier_opt {
	//set the whole structure to 0 will give default options
	const polyline_opt *poly;
};

void polybezier(const Vector2 *, const gradient *, int length, const polybezier_opt *);
void polybezier(const Vector2 *, Color, real_t W, int length, const polybezier_opt *);

} //namespace VASEr

#endif // VASER_H
