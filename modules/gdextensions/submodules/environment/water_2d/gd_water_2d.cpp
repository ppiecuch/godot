/*************************************************************************/
/*  gd_water_2d.cpp                                                      */
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

#include "gd_water_2d.h"

#include "common/gd_core.h"
#include "common/gd_pack.h"
#include "core/os/keyboard.h"
#include "core/os/os.h"
#include "scene/main/viewport.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <iostream>
#include <string>

// define some constants
#ifndef M_PI
#define M_PI Math_Pi
#endif

template <typename T>
T create_poolarray(size_t presize) {
	T data;
	data.resize(presize);
	return data;
}

template <typename T>
PoolVector<T> create_poolarray(T *buf_ptr, size_t buf_size) {
	PoolVector<T> data;
	data.resize(buf_size);
	memcpy(data.write().ptr(), buf_ptr, buf_size * sizeof(T));
	return data;
}

// https://www.khronos.org/opengl/wiki/Texture_Combiners
// https://www.gamedev.net/tutorials/programming/graphics/creating-a-glsl-library-r2428
// GL_DECAL: C = Cf * (1 – At) + Ct * At ; A = Af

// clang-format off
const char *_material_shaders[] = { R"(
	shader_type canvas_item;

	uniform sampler2D envmap;

	float map(float value, float min1, float max1, float min2, float max2) {
		return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
	}

	void fragment() {
		vec4 skin = texture(TEXTURE, UV);
		vec4 env = texture(envmap, UV2);

		vec3 color = mix(skin.rgb, env.rgb, map(env.a, 0.5, 1.0, 0.1, 1));
		COLOR *= vec4(color, skin.a);
	}
)", R"(
	shader_type canvas_item;

	uniform sampler2D envmap;

	void fragment() {
		vec4 skin = texture(TEXTURE, UV);
		vec4 env = texture(envmap, UV2);

		vec3 modulate = skin.rgb * env.rgb;
		COLOR *= vec4(mix(modulate.rgb, skin.rgb, env.a), skin.a);
	}
)", R"(
	shader_type canvas_item;

	uniform sampler2D envmap;

	void fragment() {
		vec4 skin = texture(TEXTURE, UV);
		vec4 env = texture(envmap, UV2);

		vec3 color = mix(skin.rgb, env.rgb, env.a);
		COLOR *= vec4(color, skin.a);
	}
)", R"(
	shader_type canvas_item;

	uniform sampler2D envmap;

	float map(float value, float min1, float max1, float min2, float max2) {
		return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
	}

	void fragment() {
		vec4 skin = texture(TEXTURE, UV);
		vec4 env = texture(envmap, UV2);

		if (UV.x > 0.66) {
			vec3 color = mix(skin.rgb, env.rgb, map(env.a, 0.5, 1.0, 0.1, 1));
			COLOR *= vec4(color, skin.a);
		} else if (UV.x > 0.33) {
			vec3 modulate = skin.rgb * env.rgb;
			COLOR *= vec4(mix(modulate.rgb, skin.rgb, env.a), skin.a);
		} else {
			vec3 color = mix(skin.rgb, env.rgb, env.a);
			COLOR *= vec4(color, skin.a);
		}
	}
)" };
// clang-format on

/// Private methods

// https://blog.demofox.org/2017/05/29/when-random-numbers-are-too-random-low-discrepancy-sequences/
static Vector<Point2> _get_samples(int num, size_t basex, size_t basey, const Rect2 &view) {
	// calculate the sample points
	Vector<Point2> ret;
	ret.resize(num);
	auto samples = ret.write;
	for (size_t i = 0; i < num; ++i) {
		samples[i].x = 0; // x axis
		{
			real_t denominator = basex;
			size_t n = i;
			while (n > 0) {
				const size_t multiplier = n % basex;
				samples[i].x += multiplier / denominator;
				n = n / basex;
				denominator *= basex;
			}
		}
		samples[i].y = 0; // y axis
		{
			real_t denominator = basey;
			size_t n = i;
			while (n > 0) {
				const size_t multiplier = n % basey;
				samples[i].y += multiplier / denominator;
				n = n / basey;
				denominator *= basey;
			}
		}
	}
	for (size_t i = 0; i < num; ++i) {
		samples[i].x = view.position.x + samples[i].x * view.size.width;
		samples[i].y = view.position.y + samples[i].y * view.size.height;
	}
	return ret;
}

static Ref<Texture> make_texture_from_data(const uint8_t *p_data, size_t p_data_len, int p_flags, const String &p_name) {
	String fn = "user://__water2d_" + String::num(p_flags) + "_" + p_name; // cached texture
	if (ResourceLoader::exists(fn + ".tex")) {
		return ResourceLoader::load(fn + ".tex", "Texture");
	} else {
		Ref<Image> image = memnew(Image(p_data, p_data_len));
		Ref<ImageTexture> texture = memnew(ImageTexture);
		texture->create_from_image(image, p_flags);
		texture->set_name(p_name);

		ResourceSaver::save(fn + ".tex", texture);

		return texture;
	}
}

#define IRAND() Math::rand()
#define TS() OS::get_singleton()->get_ticks_msec()

namespace {
enum {
	ARRAY_MESH_VERTEX,
	ARRAY_MESH_SKIN_UV,
	ARRAY_MESH_ENVMAP_UV,
	ARRAY_MESH_INDEX,
	ARRAY_MESH_WIREFRAME_COLOR,
	ARRAY_MESH_WIREFRAME_INDEX,
	ARRAY_MESH_MAX,
};
}

// initial conditions: every heights at zero
template <int WaterSize>
void WaterRipples<WaterSize>::init() {
	memset(*p1, 0, BufferSpace);
	memset(*p2, 0, BufferSpace);

	memset(smooth, 0, BufferSpace);

	prebuild_water(); // prebuild geometric model
}

// trace a hole at normalized [0..1] coordinates
template <int WaterSize>
void WaterRipples<WaterSize>::set_wave(real_t p_x, real_t p_y, int p_amp) {
	const int x = Math::fposmod(p_x, 1) * WaterSize;
	const int y = Math::fposmod(p_y, 1) * WaterSize;

	print_verbose(vformat("(WaterRipples) set wave at cell. %dx%d", x, y));

	(*p1)[x][y] -= p_amp;
}

// trace a hole following parametric curves
template <int WaterSize>
void WaterRipples<WaterSize>::run_wave(real_t p_phase, real_t p_cos, real_t p_sin, int p_amp) {
	const real_t r = (angle * M_PI) / 1024;

	// [TODO] Ripple types:
	// https://github.com/lequangios/Ripple-Cocos2dx/blob/e7e0a379ed20c2a033c802e0edc3309e7757eea5/CCRippleSprite.cpp#L290

	const int x = Math::fposmod(Math::cos(p_cos * r + p_phase), 1) * WaterSize;
	const int y = Math::fposmod(Math::sin(p_sin * r + p_phase), 1) * WaterSize;

	print_verbose(vformat("(WaterRipples) run wave at cell. %dx%d", x, y));

	(*p1)[x][y] -= p_amp;
}

// trace a random hole
template <int WaterSize>
void WaterRipples<WaterSize>::random_wave() {
	const int x = IRAND() % WaterSize + 1;
	const int y = IRAND() % WaterSize + 1;
	const int amp = IRAND() & 127;
	print_verbose(vformat("(WaterRipples) ranom wave at cell. %dx%d (amp: %d)", x, y, amp));
	(*p1)[IRAND() % WaterSize + 1][IRAND() % WaterSize + 1] -= amp;
}

// update to next state of fluid model
template <int WaterSize>
void WaterRipples<WaterSize>::update() {
	angle = (angle + 2) & 1023; // new angle for parametric curves

	new_water(); // fluid update
	smooth_water(); // smoothing
}

// physical calculus for fluid model
template <int WaterSize>
void WaterRipples<WaterSize>::new_water() {
	// discretized differential equation
	for (int x = 1; x <= WaterSize; x++) {
		for (int y = 0; y <= WaterSize; y++) {
			(*p1)[x][y] = (((*p2)[x - 1][y] + (*p2)[x + 1][y] + (*p2)[x][y - 1] + (*p2)[x][y + 1]) >> 1) - (*p1)[x][y];
			(*p1)[x][y] -= (*p1)[x][y] >> 4;
		}
	}

	// copy borders to make the map periodic
	memcpy(&(*p1)[0][0], &(*p1)[1][0], BufferRowSpace);
	memcpy(&(*p1)[WaterSize + 1][0], &(*p1)[1][0], BufferRowSpace);

	for (int x = 0, *ptr = &(*p1)[0][0]; x <= WaterSize; x++, ptr += BufferSize) {
		ptr[0] = ptr[1];
		ptr[WaterSize + 1] = ptr[1];
	}

	// swap buffers t and t-1, we advance in time
	std::swap(p1, p2);
}

// filter and smooth producted values
template <int WaterSize>
void WaterRipples<WaterSize>::smooth_water() {
	for (int x = 1; x < WaterSize + 1; x++) {
		for (int y = 1; y < WaterSize + 1; y++) {
			smooth[x][y] = (3 * (*p1)[x][y] + 2 * (*p1)[x + 1][y] + 2 * (*p1)[x][y + 1] + (*p1)[x + 1][y + 1]) >> 3;
		}
	}
	for (int i = 1; i < 4; i++) {
		for (int x = 1; x < WaterSize + 1; x++) {
			for (int y = 1; y < WaterSize + 1; y++) {
				smooth[x][y] = (3 * smooth[x][y] + 2 * smooth[x + 1][y] + 2 * smooth[x][y + 1] + smooth[x + 1][y + 1]) >> 3;
			}
		}
	}
}

// pre-building of a geometric model
template <int WaterSize>
void WaterRipples<WaterSize>::prebuild_water(void) {
	print_verbose("(WaterRipples) prebuilding water geometry ..");

	// vertices calculus -> we already know x and y
	// calculus of background texture coordinates
	for (int x = 0; x < GridSize; x++) {
		const real_t xmin = x / 2.0;
		for (int y = 0; y < GridSize; y++) {
			const real_t ymin = y / 2.0;
			sommet[x][y] = { xmin, ymin, 0 };
			uvmap[x][y] = { x * CellSize, y * CellSize };
		}
	}

	// normals to faces calculus: z component is constant
	for (int x = 0; x < GridSize; x++) {
		for (int y = 0; y < GridSize; y++) {
			normal[x][y].z = NormZ;
		}
	}

	// calculate normals to vertices (z component only)
	for (int x = 1; x < GridLast; x++) {
		for (int y = 1; y < GridLast; y++) {
			// snormal[x][y].z = normal[x - 1][y].z + normal[x + 1][y].z + normal[x][y -1].z + normal[x][y + 1].z;
			// snormal[x][y].z = 0.01 + 0.01 + 0.01 + 0.01 = 0.04;
			snormal[x][y].z = NormZ * 4;
		}
	}

	// copy borders of the map (z component only) for periodicity
	for (int x = 0; x < GridSize; x++) {
		snormal[x][0].z = normal[x][0].z;
		snormal[x][GridLast].z = normal[x][GridLast].z;
	}

	memcpy((void *)&snormal[0][0], (const void *)&normal[0][0], GridRowSpace);
	memcpy((void *)&snormal[GridLast][0], (const void *)&normal[GridLast][0], GridRowSpace);
}

// construction of a geometric model
template <int WaterSize>
void WaterRipples<WaterSize>::build_water() {
	const real_t zf = size_factor;
	// calculate vertices: z component
	for (int x = 0; x < WaterSize; x++) {
		for (int y = 0; y < WaterSize; y++) {
			const real_t h1 = smooth[x + 1][y + 1] / zf;
			sommet[x * 2][y * 2].z = (h1 < 0) ? 0 : h1;
		}
	}
	// construct vertices in-between
	for (int x = 0; x <= GridLast; x += 2) { // even rows
		for (int y = 1; y <= GridLast - 1; y += 2) { // odd columns
			sommet[x][y].z = (sommet[x][y - 1].z + sommet[x][y + 1].z) / 2;
		}
	}

	// construct vertices in-between
	for (int x = 1; x <= GridLast - 1; x += 2) { // even rows
		for (int y = 0; y <= GridLast; y++) { // every columns
			sommet[x][y].z = (sommet[x - 1][y].z + sommet[x + 1][y].z) / 2;
		}
	}

	// calculate normals to faces : components x and y
	// -> simplified cross product knowing that we have a distance of 1.0 between
	//    each fluid cells.
	for (int x = 0; x < GridLast; x++) {
		for (int y = 0; y < GridSize - 1; y++) {
			normal[x][y].x = 0.1 * (sommet[x][y].z - sommet[x + 1][y].z);
			normal[x][y].y = 0.1 * (sommet[x][y].z - sommet[x][y + 1].z);
		}
	}

	// copy map borders(components x and y only) for periodicity
	memcpy((void *)&normal[GridLast][0], (void *)&normal[GridLast - 1][0], GridRowSpace);

	for (int x = 0; x < GridSize; x++) {
		normal[x][GridLast].x = normal[x][GridLast - 1].x;
		normal[x][GridLast].y = normal[x][GridLast - 1].y;
	}

	// calculate normals to vertices (components X and Y only)
	for (int x = 1; x < GridLast; x++) {
		for (int y = 1; y < GridLast; y++) {
			snormal[x][y].x = normal[x - 1][y].x + normal[x + 1][y].x + normal[x][y - 1].x + normal[x][y + 1].x;
			snormal[x][y].y = normal[x - 1][y].y + normal[x + 1][y].y + normal[x][y - 1].y + normal[x][y + 1].y;
		}
	}

	// copy map borders (components x and y only)
	for (int x = 0; x < GridSize; x++) {
		snormal[x][0].x = normal[x][0].x;
		snormal[x][0].y = normal[x][0].y;
		snormal[x][GridLast].x = normal[x][GridLast].x;
		snormal[x][GridLast].y = normal[x][GridLast].y;
	}

	memcpy((void *)&snormal[0][0], (const void *)&normal[0][0], GridRowSpace);
	memcpy((void *)&snormal[GridLast][0], (const void *)&normal[GridLast][0], GridRowSpace);

	constexpr real_t SNormZ = NormZ * 4;

	// calculate ourself normalization
	for (int x = 0; x < GridSize; x++) {
		for (int y = 0; y < GridSize; y++) {
			const real_t sqroot = Math::sqrt(snormal[x][y].x * snormal[x][y].x + snormal[x][y].y * snormal[x][y].y + SNormZ * SNormZ);
			snormaln[x][y].x = snormal[x][y].x / sqroot;
			snormaln[x][y].y = snormal[x][y].y / sqroot;
			snormaln[x][y].z = SNormZ / sqroot;
		}
	}

	// really simple version of a fake envmap generator
	for (int x = 0; x < GridSize; x++) {
		for (int y = 0; y < GridSize; y++) {
			// perturbate coordinates of background mapping with the components x,y of normals...
			// simulate refraction
			newuvmap[x][y].x = uvmap[x][y].x + 0.05 * snormaln[x][y].x;
			newuvmap[x][y].y = uvmap[x][y].y + 0.05 * snormaln[x][y].y;
			// trick: xy projection of normals -> assume reflection in direction of the normals
			// looks ok for non-plane surfaces
			envmap[x][y].x = 0.5 + snormaln[x][y].x * 0.45;
			envmap[x][y].y = 0.5 + snormaln[x][y].y * 0.45;
		}
	}
}

// build strip index for vertex arrays
template <int WaterSize>
void WaterRipples<WaterSize>::build_strip_index() {
	// array is (WaterSize * 2) x (WaterSize * 2)
	const int strip_width = GridSize - 2; // n points define n-2 triangles in a strip
	int *sindex_ptr = &sindex[0];

	// build index list
	for (int x = 0; x < strip_width; x++) { // vertical index in array
		*sindex_ptr++ = ((x + 1) * GridSize) + 1; // strip_width + 1 triangle strips
		for (int y = 1; y < strip_width; y++) { // horizontal index in array
			*sindex_ptr++ = (x * GridSize) + y;
			*sindex_ptr++ = ((x + 1) * GridSize) + y + 1;
		}
		*sindex_ptr++ = (x * GridSize) + strip_width;
	}
}

// build triangles index for vertex arrays
template <int WaterSize>
void WaterRipples<WaterSize>::build_tri_index() {
	int *sindex_ptr = &sindex[0];

	// build index list (column order)
	for (int y = 0; y < GridSize - 1; y++) {
		for (int x = 0; x < GridSize - 1; x++) {
			*sindex_ptr++ = y * GridSize + x;
			*sindex_ptr++ = y * GridSize + (x + 1);
			*sindex_ptr++ = (y + 1) * GridSize + x;

			*sindex_ptr++ = (y + 1) * GridSize + (x + 1);
			*sindex_ptr++ = (y + 1) * GridSize + x;
			*sindex_ptr++ = y * GridSize + (x + 1);
		}
	}

	// int l0 = 0;
	// int l1 = WaterSize * 4;
	// int l2 = l1;
	//
	// for (int y = 0; y < GridSize; y += 2) {
	// 	for (int x = 0; x < GridSize; x++) { // n-2 triangles (n points in 2 array lines)
	// 		if ((x & 1) == 1) {
	// 			*sindex_ptr++ = l1++;
	// 			l1++;
	// 		} else {
	// 			*sindex_ptr++ = l0++;
	// 			l0++;
	// 		}
	// 	}
	//
	// 	l0 = l2;
	// 	l1 = l2 + WaterSize * 4;
	// 	l2 = l1;
	// }
}

// build and display geometric model
template <int WaterSize>
Array WaterRipples<WaterSize>::build_mesh_data(const Rect2 *p_skin_region, const Rect2 *p_envmap_region, bool p_with_wireframe) {
	build_water();
	build_tri_index();

	const size_t indexes_num = (GridSize - 1) * (GridSize - 1) * 6;

	Array mesh_data;
	mesh_data.resize(ARRAY_MESH_MAX);

	mesh_data[ARRAY_MESH_VERTEX] = create_poolarray(&sommet[0][0], GridArea);

	if (p_with_wireframe) {
		// wireframe from triangles:

		PoolIntArray windex;
		windex.resize(indexes_num * 2);

		auto _windex = windex.write();
		for (int i = 0, j = 0; i < indexes_num; i += 3, j += 6) {
			_windex[j + 0] = sindex[i + 0];
			_windex[j + 1] = sindex[i + 1];
			_windex[j + 2] = sindex[i + 1];
			_windex[j + 3] = sindex[i + 2];
			_windex[j + 4] = sindex[i + 2];
			_windex[j + 5] = sindex[i + 0];
		}
		_windex.release();

		Color colors[GridSize][GridSize];
		for (int x = 0; x < GridSize; x++) {
			for (int y = 0; y < GridSize; y++) {
				if (sommet[x][y].z != 0) {
					const real_t z = CLAMP(sommet[x][y].z * 10, 0, 1);
					colors[x][y] = Color(z, 1 - z, 1 - z, 0.6);
				} else {
					colors[x][y] = Color(0, 1, 1, 0.3);
				}
			}
		}

		mesh_data[ARRAY_MESH_WIREFRAME_COLOR] = create_poolarray(&colors[0][0], GridArea);
		mesh_data[ARRAY_MESH_WIREFRAME_INDEX] = windex;
	}

	// normalize uv for mask texture atlas
	Vector2(*_newuvmap)[GridSize] = newuvmap;
	Vector2(*_envmap)[GridSize] = envmap;
	if (p_skin_region || p_envmap_region) {
		if (p_skin_region) {
			_newuvmap = new Vector2[GridSize][GridSize];
		}
		if (p_envmap_region) {
			_envmap = new Vector2[GridSize][GridSize];
		}
		for (int x = 0; x < GridSize; x++) {
			for (int y = 0; y < GridSize; y++) {
				// normalize uv coordinates for texture atlas
				if (p_skin_region) {
					_newuvmap[x][y] = p_skin_region->position + newuvmap[x][y] * p_skin_region->size;
				}
				if (p_envmap_region) {
					_envmap[x][y] = p_envmap_region->position + envmap[x][y] * p_envmap_region->size;
				}
			}
		}
	}

	mesh_data[ARRAY_MESH_SKIN_UV] = create_poolarray(&_newuvmap[0][0], GridArea);
	mesh_data[ARRAY_MESH_ENVMAP_UV] = create_poolarray(&_envmap[0][0], GridArea);
	mesh_data[ARRAY_MESH_INDEX] = create_poolarray(sindex, indexes_num);

	return mesh_data;
}

template <int WaterSize>
int WaterRipples<WaterSize>::get_grid_size() const {
	return GridSize;
}

template <int WaterSize>
void WaterRipples<WaterSize>::set_size_factor(int p_factor) {
	size_factor = p_factor;
}

template <int WaterSize>
int WaterRipples<WaterSize>::get_size_factor() const {
	return size_factor;
}

// define GridSize x GridSize columns of fluid
template <int WaterSize>
WaterRipples<WaterSize>::WaterRipples() {
	size_factor = 100;
	angle = 0;

	p1 = &water1; // FRONT map
	p2 = &water2; // BACK map

	// allocate buffers
	sommet = new Vector3[GridSize][GridSize]; // vertices vector
	normal = new Vector3[GridSize][GridSize]; // quads normals
	snormal = new Vector3[GridSize][GridSize]; // vertices normals (average)
	snormaln = new Vector3[GridSize][GridSize]; // normalized vertices normals

	uvmap = new Vector2[GridSize][GridSize]; // background texture coordinates
	newuvmap = new Vector2[GridSize][GridSize]; // perturbated background coordinates -> refraction
	envmap = new Vector2[GridSize][GridSize]; // envmap coordinates

	sindex = new int[(GridSize - 1) * (GridSize - 1) * 6]; // vertex array index
}

template <int WaterSize>
WaterRipples<WaterSize>::~WaterRipples(void) {
	delete[] sommet; // vertices vector
	delete[] normal; // quads normals
	delete[] snormal; // vertices normals (average)
	delete[] snormaln; // normalized vertices normals
	delete[] uvmap; // background texture coordinates
	delete[] newuvmap; // perturbated background coordinates -> refraction
	delete[] envmap; // envmap coordinates
	delete[] sindex; // vertex array index
}

/// Godot node

#ifdef TOOLS_ENABLED
Dictionary Water2D::_edit_get_state() const {
	Dictionary state = Node2D::_edit_get_state();
	state["view_size"] = get_view_size();
	return state;
}

void Water2D::_edit_set_state(const Dictionary &p_state) {
	Node2D::_edit_set_state(p_state);
	set_view_size(p_state["view_size"]);
}

Rect2 Water2D::_edit_get_rect() const {
	return Rect2(Point2(), get_view_size());
}

void Water2D::_edit_set_rect(const Rect2 &p_rect) {
	set_view_size(p_rect.size);
	_change_notify();
}

bool Water2D::_edit_use_rect() const {
	return true;
}
#endif

#define run_water(cmd)       \
	switch (level_quality) { \
		case 0:              \
			water0->cmd;     \
			break;           \
		case 1:              \
			water1->cmd;     \
			break;           \
		default:             \
		case 2:              \
			water2->cmd;     \
			break;           \
	}

#define run_water_r(cmd)        \
	switch (level_quality) {    \
		case 0:                 \
			return water0->cmd; \
		case 1:                 \
			return water1->cmd; \
		default:                \
		case 2:                 \
			return water2->cmd; \
	}

void Water2D::set_active(bool p_state) {
	active = p_state;
	if (is_visible_in_tree()) {
		set_process_internal(active);
	}
}

bool Water2D::is_active() const {
	return active;
}

void Water2D::set_quality_level(int p_quality) {
	level_quality = p_quality;
	update();
}

int Water2D::get_quality_level() const {
	return level_quality;
}

void Water2D::set_skin_texture(const Ref<Texture> &p_texture) {
	if (texture_skin != p_texture) {
		texture_skin = p_texture;
		_update_material |= UPDATE_WATER;
		update();
	}
}

Ref<Texture> Water2D::get_skin_texture() const {
	return texture_skin;
}

void Water2D::set_mask_texture(const Ref<Texture> &p_texture) {
	if (texture_mask != p_texture) {
		texture_mask = p_texture;
		_update_material |= UPDATE_ALL;
		update();
	}
}

Ref<Texture> Water2D::get_mask_texture() const {
	return texture_mask;
}

void Water2D::set_details_map(bool p_state) {
	details_map = p_state;
	_update_material |= UPDATE_DETAILS;
	update();
}

bool Water2D::is_details_map() const {
	return details_map;
}

void Water2D::set_caustics(bool p_state) {
	caustics = p_state;
	_update_material |= UPDATE_CAUSTICS;
	update();
}

bool Water2D::is_caustics() const {
	return caustics;
}

void Water2D::set_wireframe(bool p_state) {
	if (wireframe != p_state) {
		wireframe = p_state;
		update();
	}
}

bool Water2D::is_wireframe() const {
	return wireframe;
}

void Water2D::set_blend_mode(int p_mode) {
	ERR_FAIL_INDEX(p_mode, BLEND_MODES_NUM);
	if (blend_variant != p_mode) {
		blend_variant = p_mode;
		_update_material |= UPDATE_WATER;
	}
}

int Water2D::get_blend_mode() const {
	return blend_variant;
}

void Water2D::set_wave_speed_rate(real_t p_rate) {
	wave_speed_rate = p_rate;
}

real_t Water2D::get_wave_speed_rate() const {
	return wave_speed_rate;
}

void Water2D::set_wave_size_factor(int p_factor) {
	run_water(set_size_factor(p_factor));
}

int Water2D::get_wave_size_factor() const {
	run_water_r(get_size_factor());
}

void Water2D::set_caustics_speed_rate(real_t p_rate) {
	caustics_speed_rate = p_rate;
}

real_t Water2D::get_caustics_speed_rate() const {
	return caustics_speed_rate;
}

void Water2D::set_caustics_alpha(real_t p_alpha) {
	ERR_FAIL_COND(p_alpha <= 0 || p_alpha > 1);
	caustics_alpha = p_alpha;
	_update_material |= UPDATE_CAUSTICS;
	update();
}

real_t Water2D::get_caustics_alpha() const {
	return caustics_alpha;
}

void Water2D::set_wave(real_t p_x, real_t p_y, int p_amp) {
	run_water(set_wave(p_x, p_y, p_amp));
}

void Water2D::run_wave(real_t p_phase, real_t p_cos, real_t p_sin, int p_amp) {
	run_water(run_wave(p_phase, p_cos, p_sin, p_amp));
}

void Water2D::random_wave() {
	run_water(random_wave());
}

extern "C" const uint8_t envmap_png_data[];
extern "C" const size_t envmap_png_size;

extern "C" const uint8_t *causticsmaps[];
extern "C" const uint8_t *noisemaps[];

_FORCE_INLINE_ static std::pair<bool, Rect2> get_texture_region(const Ref<Texture> &p_texture) {
	Ref<AtlasTexture> atlas = p_texture;
	if (atlas.is_valid()) {
		return std::make_pair(true, atlas->get_region()); // true region
	}
	return std::make_pair(false, Rect2(0, 0, 1, 1)); // full rect
}

void Water2D::_changed_callback(Object *p_changed, const char *p_prop) {
#ifdef TOOLS_ENABLED
	if (p_changed == this) {
		if (strcmp(p_prop, "z-index") == 0) {
			if (mesh_item.is_valid()) {
				VS::get_singleton()->canvas_item_set_draw_index(mesh_item, get_index());
			}
			if (caustics_item.is_valid()) {
				VS::get_singleton()->canvas_item_set_draw_index(caustics_item, get_index());
			}
		}
	}
#endif
}

void Water2D::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_READY: {
			run_water(init());
			texture_envmap = make_texture_from_data(envmap_png_data, envmap_png_size, Texture::FLAGS_DEFAULT, "envmap");
			mesh_item = RID_PRIME(VS::get_singleton()->canvas_item_create());
			VS::get_singleton()->canvas_item_set_parent(mesh_item, get_canvas_item());
			VS::get_singleton()->canvas_item_set_draw_index(mesh_item, get_index());
			_update_material = UPDATE_ALL;
		} break;
		case NOTIFICATION_ENTER_TREE: {
			if (active) {
				set_process_internal(true);
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			set_process_internal(false);
		} break;
		case NOTIFICATION_DRAW: {
			std::pair<bool, Rect2> skin_reg = get_texture_region(texture_skin);
			std::pair<bool, Rect2> envmap_reg = get_texture_region(texture_envmap);
			auto build_mesh_data = [=](const Rect2 *p_skin_region, const Rect2 *p_envmap_region, bool p_with_wireframe) {
				run_water_r(build_mesh_data(skin_reg.first ? &skin_reg.second : nullptr, envmap_reg.first ? &envmap_reg.second : nullptr, wireframe));
			};
			Array mesh_data = build_mesh_data(skin_reg.first ? &skin_reg.second : nullptr, envmap_reg.first ? &envmap_reg.second : nullptr, wireframe);
			if (wireframe) { // wireframe
				if (!mesh_wireframe.is_valid()) {
					mesh_wireframe = newref(ArrayMesh);
				}
				Array w;
				w.resize(VS::ARRAY_MAX);
				w[VisualServer::ARRAY_VERTEX] = mesh_data[ARRAY_MESH_VERTEX];
				w[VisualServer::ARRAY_COLOR] = mesh_data[ARRAY_MESH_WIREFRAME_COLOR];
				w[VisualServer::ARRAY_INDEX] = mesh_data[ARRAY_MESH_WIREFRAME_INDEX];
				mesh_wireframe->clear_mesh();
				mesh_wireframe->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, w);
				draw_mesh(mesh_wireframe, Ref<Texture>(), texture_mask);
			}
			{ // skin and envmap
				if (details_map && !animation_details.is_valid()) {
					// load maps
					for (int m = 0; m < NUM_NORMALS; m++) {
						animation_details.frames.push_back(make_texture_from_data(noisemaps[m], -1, Texture::FLAGS_DEFAULT, "details" + String::num(m)));
					}
				}
				Array d;
				d.resize(VS::ARRAY_MAX);
				d[VisualServer::ARRAY_VERTEX] = mesh_data[ARRAY_MESH_VERTEX];
				d[VisualServer::ARRAY_TEX_UV] = mesh_data[ARRAY_MESH_SKIN_UV];
				d[VisualServer::ARRAY_TEX_UV2] = mesh_data[ARRAY_MESH_ENVMAP_UV];
				d[VisualServer::ARRAY_INDEX] = mesh_data[ARRAY_MESH_INDEX];
				mesh->clear_mesh();
				mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, d);
				if (_update_material & UPDATE_WATER || _update_material & UPDATE_DETAILS) {
					materials[blend_variant]->set_shader_param("envmap", texture_envmap);
					VS::get_singleton()->canvas_item_clear(mesh_item);
					VS::get_singleton()->canvas_item_set_material(mesh_item, materials[blend_variant]->get_rid());
					RID _skin = texture_skin ? texture_skin->get_rid() : RID();
					RID _mask = texture_mask ? texture_mask->get_rid() : RID();
					RID _details = (details_map && animation_details.is_valid()) ? animation_details.texture()->get_rid() : RID();
					VS::get_singleton()->canvas_item_add_mesh(mesh_item, mesh->get_rid(), Transform2D(), Color(1, 1, 1, 1), _skin, _details, _mask);
				}
			}
			if (caustics) { // caustics
				if (!animation_caustics.is_valid()) {
					// load maps
					for (int c = 0; c < NUM_CAUSTICS; c++) {
						animation_caustics.frames.push_back(make_texture_from_data(causticsmaps[c], -1, Texture::FLAGS_DEFAULT, "caust" + String::num(c)));
					}
				}
				if (!caustics_item.is_valid()) {
					caustics_item = RID_PRIME(VS::get_singleton()->canvas_item_create());
					VS::get_singleton()->canvas_item_set_parent(caustics_item, get_canvas_item());
					VS::get_singleton()->canvas_item_set_draw_index(caustics_item, get_index());
				}
				if (!mesh_caustics.is_valid()) {
					mesh_caustics = newref(ArrayMesh);
				}
				if (!material_caustics.is_valid()) {
					material_caustics = newref(CanvasItemMaterial);
					material_caustics->set_blend_mode(CanvasItemMaterial::BLEND_MODE_ADD);
					VS::get_singleton()->canvas_item_set_material(caustics_item, material_caustics->get_rid());
				}
				Array d;
				d.resize(VS::ARRAY_MAX);
				d[VisualServer::ARRAY_VERTEX] = mesh_data[ARRAY_MESH_VERTEX];
				d[VisualServer::ARRAY_TEX_UV] = mesh_data[ARRAY_MESH_SKIN_UV];
				d[VisualServer::ARRAY_INDEX] = mesh_data[ARRAY_MESH_INDEX];
				mesh_caustics->clear_mesh();
				mesh_caustics->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, d);
				if (_update_material & UPDATE_CAUSTICS) {
					VS::get_singleton()->canvas_item_clear(caustics_item);
					VS::get_singleton()->canvas_item_set_visible(caustics_item, true);
					RID _caustics = animation_caustics.is_valid() ? animation_caustics.texture()->get_rid() : RID();
					RID _mask = texture_mask ? texture_mask->get_rid() : RID();
					VS::get_singleton()->canvas_item_add_mesh(caustics_item, mesh_caustics->get_rid(), Transform2D(), Color(1, 1, 1, caustics_alpha), _caustics, RID(), _mask);
				}
			} else {
				if (_update_material & UPDATE_CAUSTICS) {
					VS::get_singleton()->canvas_item_set_visible(caustics_item, false);
				}
			}
			_update_material = 0;
		} break;
		case NOTIFICATION_VISIBILITY_CHANGED: {
			set_process_internal(active && is_visible());
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			static float water_progress = 0;
			const float delta = get_process_delta_time();
			water_progress += delta;
			if (water_progress > wave_speed_rate) {
				run_water(update());
				update();
				water_progress = 0;
			}
			if (caustics && animation_caustics.next(delta, caustics_speed_rate)) {
				_update_material |= UPDATE_CAUSTICS;
				update();
			}
			if (details_map && animation_details.next(delta, details_speed_rate)) {
				_update_material |= UPDATE_DETAILS;
				update();
			}
		} break;
	}
}

#ifdef TOOLS_ENABLED
void Water2D::toogle_demo_mode() {
	set_blend_mode(BLEND_MODE_DEMO);
}
#endif

void Water2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_active"), &Water2D::set_active);
	ClassDB::bind_method(D_METHOD("is_active"), &Water2D::is_active);
	ClassDB::bind_method(D_METHOD("set_quality_level"), &Water2D::set_quality_level);
	ClassDB::bind_method(D_METHOD("get_quality_level"), &Water2D::get_quality_level);
	ClassDB::bind_method(D_METHOD("set_skin_texture"), &Water2D::set_skin_texture);
	ClassDB::bind_method(D_METHOD("get_skin_texture"), &Water2D::get_skin_texture);
	ClassDB::bind_method(D_METHOD("set_mask_texture"), &Water2D::set_mask_texture);
	ClassDB::bind_method(D_METHOD("get_mask_texture"), &Water2D::get_mask_texture);
	ClassDB::bind_method(D_METHOD("set_wireframe"), &Water2D::set_wireframe);
	ClassDB::bind_method(D_METHOD("is_wireframe"), &Water2D::is_wireframe);
	ClassDB::bind_method(D_METHOD("set_blend_mode"), &Water2D::set_blend_mode);
	ClassDB::bind_method(D_METHOD("get_blend_mode"), &Water2D::get_blend_mode);
	ClassDB::bind_method(D_METHOD("set_details_map"), &Water2D::set_details_map);
	ClassDB::bind_method(D_METHOD("is_details_map"), &Water2D::is_details_map);
	ClassDB::bind_method(D_METHOD("set_wave_speed_rate"), &Water2D::set_wave_speed_rate);
	ClassDB::bind_method(D_METHOD("get_wave_speed_rate"), &Water2D::get_wave_speed_rate);
	ClassDB::bind_method(D_METHOD("set_wave_size_factor"), &Water2D::set_wave_size_factor);
	ClassDB::bind_method(D_METHOD("get_wave_size_factor"), &Water2D::get_wave_size_factor);
	ClassDB::bind_method(D_METHOD("set_caustics"), &Water2D::set_caustics);
	ClassDB::bind_method(D_METHOD("is_caustics"), &Water2D::is_caustics);
	ClassDB::bind_method(D_METHOD("set_caustics_speed_rate"), &Water2D::set_caustics_speed_rate);
	ClassDB::bind_method(D_METHOD("get_caustics_speed_rate"), &Water2D::get_caustics_speed_rate);
	ClassDB::bind_method(D_METHOD("set_caustics_alpha"), &Water2D::set_caustics_alpha);
	ClassDB::bind_method(D_METHOD("get_caustics_alpha"), &Water2D::get_caustics_alpha);

	ClassDB::bind_method(D_METHOD("set_wave", "x", "y", "amp"), &Water2D::set_wave);
	ClassDB::bind_method(D_METHOD("run_wave", "phase", "cos", "sin", "amp"), &Water2D::run_wave);
	ClassDB::bind_method(D_METHOD("random_wave"), &Water2D::random_wave);

#ifdef TOOLS_ENABLED
	ClassDB::bind_method(D_METHOD("toogle_demo_mode"), &Water2D::toogle_demo_mode);
#endif

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "is_active");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "wireframe"), "set_wireframe", "is_wireframe");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "details"), "set_details_map", "is_details_map");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "caustics"), "set_caustics", "is_caustics");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "caustics_speed_rate"), "set_caustics_speed_rate", "get_caustics_speed_rate");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "caustics_alpha", PROPERTY_HINT_RANGE, "0,1,0.05,or_greater"), "set_caustics_alpha", "get_caustics_alpha");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "wave_speed_rate"), "set_wave_speed_rate", "get_wave_speed_rate");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "wave_size_factor"), "set_wave_size_factor", "get_wave_size_factor");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "wave_blend_mode", PROPERTY_HINT_ENUM, "Mode1,Mode2,Mode3"), "set_blend_mode", "get_blend_mode");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "skin_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_skin_texture", "get_skin_texture");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mask_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), "set_mask_texture", "get_mask_texture");

	BIND_ENUM_CONSTANT_CUSTOM(BLEND_MODE1, "WATER_BLEND_MODE1");
	BIND_ENUM_CONSTANT_CUSTOM(BLEND_MODE2, "WATER_BLEND_MODE2");
	BIND_ENUM_CONSTANT_CUSTOM(BLEND_MODE3, "WATER_BLEND_MODE3");
}

Water2D::Water2D() {
	_update_material = 0;
	active = false;
	wireframe = false;
	details_map = false;
	caustics = true;
	caustics_speed_rate = 0.12;
	caustics_alpha = 0.5;
	wave_speed_rate = 0.1;
	details_speed_rate = 0.5;
	level_quality = 2;
	blend_variant = 0;

	water2 = newref(WaterRipples<50>);

	// water materials
	for (int m = 0; m < BLEND_MODES_NUM; m++) {
		materials[m] = newref(ShaderMaterial);
		Ref<Shader> shader = memnew(Shader);
		shader->set_code(_material_shaders[m]);
		materials[m]->set_shader(shader);
	}

	mesh = newref(ArrayMesh);
}

// https://www.codeproject.com/Articles/1073/Interactive-water-effect
//
// Introduction
// ------------
// This application demonstrates multitexturing effects and clever math optimizations to produce an interactive water effect.
//
// In the calculations used in the code, we work with the following assumptions:
//
// * Incompressible liquid: a given mass of the liquid occupies the same volume all the time whatever the shape it takes. There is conservation of volume.
// * The surface of the liquid can be represented by grid heights, i.e. we can see the surface of the liquid as being made up
//   of a series of vertical columns of liquid. We will be able to implement that by an N x M matrix giving the height of each point
//   of surface (N being the number of points in ordinate and M the number of points in x-coordinates). This approximation is valid only for not very
//   deep containers and is tolerable as long as the forces applied to the liquid are not too significant.
//   The liquid cannot splash and a wave cannot break (not bearings).
// * The vertical component of the speed of the particles of the liquid can be ignored. The model is not valid any more for stiff slopes or chasms.
// * The horizontal component of the speed of the liquid in a vertical column is roughly constant. The movement is uniform within
//   a vertical column. This model is not valid anymore when there are turbulences.
//
// These assumptions enable us to use a simplified form of the equations of the mechanics of fluids.
//
// To simplify the reasoning, we first of all will work in 2 dimensions (the height h depends on x-coordinate x and time t):
//
// [1] du(x,t)/dt + u(x,t).du(x,t)/dx + g*dh(x,t)/dx = 0
// [2] dp(x,t)/dt + d(u(x,t)p(x,t))/dx = d(h(x,t)-b(x))/dt + p(x,t).du(x,t)/dx + u(x,t)*dp(x,t)/dx = 0
//
// where g is gravitational acceleration, h(x, t) is the height of the surface of the liquid, b(x) is the height of the bottom of the container
// (we suppose that it does not vary according to time t), p(x, t) = h(x, t)-b(x) is the depth of the liquid, and u(x, t) is the horizontal
// speed of a vertical column of liquid. The equation (1) represents the law of Newton (F=ma) which gives the movement according
// to the forces applied, and the equation (2) expresses the conservation of volume.
//
// These two equations are nonlinear (nonconstant coefficients) but if the speed of the liquid is small and if the depth varies slowly,
// we can take the following approximation (we are unaware of the terms multiplied by u(x, t) and p does not depend any more of time t):
//
// [3] du(x,t)/dt + g*dh(x,t)/dx = 0
// [4] dh(x,t)/dt + p(x)*du(x,t)/dx = 0
//
// By differentiating the first equation with respect to x and the second equation with respect to t, we obtain:
//
// [5] d^2u(x,t)/dxdt + g*d^2h(x,t)/dx^2 = 0
// [6] d^2h(x,t)/dt^2 + p(x)*d^2u(x,t)/dtdx = 0
//
// As u is a "suitable" function (its second derivatives are continuous), its second partial derivatives are equals and we can deduce
// the following single equation where u is not present:
//
// [7] d^2h(x,t)/dt^2 = g*p(x)*d2h(x,t)/dx^2
//
// The differential equation of the second member (7) is an equation of wave and expresses the heights' variation as a function of time and the x-coordinate.
//
// In 3 dimensions, the equation (7) has the following form:
//
// [8] d^2h(x,y,t)/dt^2 = g*p(x,y)*(d^2h(x,y,t)/dx^2+d^2h(x,y,t)/dy^2)
//
// To be able to work with our heights' grid, we must have a discrete formula, but this one is continuous.
// Moreover this equation is always nonlinear by the presence of p(x, y). To simplify, we will
// consider p constant, i.e. a speed of constant wave whatever the depth (we will consider a bottom of flat container,
// which will limit the "variability" of the depth). We obtain the equation then (9) to discretize:
//
// [9] d^2h(x,y,t)/dt^2 = g*p(d^2h(x,y,t)/dx^2 + d^2h(x,y,t)/dy^2)
//
// We can discretize the terms of this equation in the following way (using the method of central-differences):
//
// [10] d^2h(x,y,t)/dt^2 => (Dh(x,y,t+1) - Dh(x,y,t))/Dt^2 = (h(x,y,t+1) - h(x,y,t) - h(x,y,t) + h(x,y,t-1)) / Dt^2
//
// [11] d^2h(x,y,t)/dx2 => (Dh(x+1,y,t) - Dh(x,y,t))/Dx2 = (h(x+1,y,t) - 2h(x,y,t) + h(x-1,y,t))/Dx^2
//
// [12] d^2h(x,y,t)/dy2 => (Dh(x,y+1,t) - Dh(x,y,t))/Dy2 = (h(x,y+1,t) - 2h(x,y,t) + h(x,y-1,t))/Dy^2
//
// By posing Dr = Dx = Dy = resolution of the grid, we obtain the discrete analogue of the equation (9):
//
// [13] (h(x,y,t+1) + h(x,y,t-1) - 2h(x,y,t))/Dt^2 = g*p/Dr^2 * (h(x+1,y,t)+h(x-1,y,t)+h(x,y+1,t)+h(x,y-1,t)-4h(x,y,t))
//
// where Dt is the variation of time. We can use a more compact notation for this relation of recurrence:
//
// [14] 1/Dt^2 * [1  -2  1] h(x,y) = g*p/Dr^2 * h(t)
//
// We can then have h(x, y, t+1):
//
// [15] h(x,y,t+1) = g*p*Dt^2/Dr^2 * h(t) -h(x,y,t-1) + 2h(x,y,t)
//
// [16] h(x,y,t+1) = g*p*Dt^2/Dr^2 * h(t) -h(x,y,t-1) + 2h(x,y,t)
//
// [17] h(x,y,t+1) = g*p*Dt^2/Dr^2 * h(t) -h(x,y,t-1) + 2h(x,y,t) - 4*g*p*Dt^2/Dr^2 * h(x,y,t)
//
// [18] h(x,y,t+1) = g*p*Dt^2/Dr^2 * h(t) -h(x,y,t-1) + (2 - 4*g*p*Dt^2/Dr^2) * h(x,y,t)
//
// While setting g*p*Dt^2/Dr^2 = 1/2, we eliminate the last term, and the relation is simplified while giving us:
//
// [19] h(x,y,t+1) = 1/2 * ( h(x+1,y,t) + h(x-1,y,t) + h(x,y+1,t) + h(x,y-1,t) - h(x,y,t-1) )
//
// This relation of recurrence has a step of 2: the height in t+1 is related to heights in T and in T-1.
// We can implement that using 2 heights' matrices H1 and H2: H2[x, y] = 1/2(H1[x+1, y] + H1[x-1, y] + H1[x, y+1] + H1[x, y-1]) - H2[x, y]
//
// We can then swap the 2 matrices with each step.
//
// To calculate the new height of a point of surface costs only 4 additions, 1 subtraction and a shift-right of 1 (for division by 2).
//
// From the result obtained, we subtract 1/2n of the result (i.e. this same result right shifted of N) to have a certain damping
// (n=4 gives a rather aesthetic result, n < 4 gives more viscosity, n > 4 gives more fluidity).
//
// Let us notice that the heights of these points are signed, 0 being the neutral level of rest.
//
// From the heights' matrix, we can easily build a polygonal representation by considering each box of the grid as a quadrilateral
// (or 2 triangles) of which the heights of the 4 vertices are given by the heights of 4 adjacent boxes of the matrix.
//
// In our example, we tessellate our model with GL_TRIANGLE_STRIP and we use some multipass effects to get it realistic.
//
// First we perturb a first set of texture coordinates proportionally to vertices normals (the logo's texture coordinates).
// It looks like refraction.
//
// Code:
// ---------------------------------------------------------------------------
// -- calculate ourself normalization
//
// for(x=0; x < FLOTSIZE*2; x++)
// {
//    for(y=0; y < FLOTSIZE*2; y++)
//    {
//       sqroot = sqrt(_snormal[x][y][0]*_snormal[x][y][0] + _snormal[x][y][1]*_snormal[x][y][1] + 0.0016);
//
//       _snormaln[x][y][0] = _snormal[x][y][0]/sqroot;
//       _snormaln[x][y][1] = _snormal[x][y][1]/sqroot;
//       _snormaln[x][y][2] = 0.04 / sqroot;
//
//       -- perturbate coordinates of background
//       -- mapping with the components X,Y of normals...
//       -- simulate refraction
//
//       _newuvmap[x][y][0] = _uvmap[x][y][0] + 0.05 * _snormaln[x][y][0];
//       _newuvmap[x][y][1] = _uvmap[x][y][1] + 0.05 * _snormaln[x][y][1];
//
//    }
// }
//
// Then we calculate a second set of texture coordinates using a fake environment mapping formula
// (invariant with observer eye's position, just project normals to the xy plane)
// (the sky's texture coordinates).
//
// Code
// ---------------------------------------------------------------------------
// really simple version of a fake envmap generator
// for(x=0; x < FLOTSIZE; x++)
// {
//     for(y=0; y < FLOTSIZE; y++)
//     {
//         -- trick : xy projection of normals  ->
//         -- assume reflection in direction of the normals
//         -- looks ok for non-plane surfaces
//         _envmap[x][y][0] = 0.5 + _snormaln[x][y][0] * 0.45;
//         _envmap[x][y][1] = 0.5 + _snormaln[x][y][1] * 0.45;
//
//     }
// }
// Then mix the textures together using multitexturing and blending.
//
// Code:
// ---------------------------------------------------------------------------
// glEnable(GL_BLEND);
//
// use texture alpha-channel for blending
//
// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
// glActiveTextureARB(GL_TEXTURE0_ARB);
// glBindTexture(GL_TEXTURE_2D, 2); -- 1st texture -> background ..
//
// glActiveTextureARB(GL_TEXTURE1_ARB);
// glBindTexture(GL_TEXTURE_2D, 1); -- 2nd texture -> envmap
// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
//
// -- enable texture mapping and specify ourself texcoords
//
// glDisable(GL_TEXTURE_GEN_S);
// glDisable(GL_TEXTURE_GEN_T);
//
// And to finish the stuff, tessellate using TRIANGLE_STRIPs per matrix scanline.
//
// Code:
// ---------------------------------------------------------------------------
//  for(x=0; x < strip_width; x++)
//  {
//      glBegin(GL_TRIANGLE_STRIP);
//
//      glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, _newuvmap[x+1][1]);
//      glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, _envmap[x+1][1]);
//      glVertex3fv(_sommet[x+1][1]); -- otherwise everything is scrolled !!!
//
//      for(y=1; y < strip_width; y++)
//      {
//          glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, _newuvmap[x][y]);
//          glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, _envmap[x][y]);
//          glVertex3fv(_sommet[x][y]);
//
//          glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, _newuvmap[x+1][y+1]);
//          glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, _envmap[x+1][y+1]);
//          glVertex3fv(_sommet[x+1][y+1]);
//      }
//
//      glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, _newuvmap[x][y]);
//      glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, _envmap[x][y]);
//      glVertex3fv(_sommet[x][y]);
//
//      glEnd();
// }
