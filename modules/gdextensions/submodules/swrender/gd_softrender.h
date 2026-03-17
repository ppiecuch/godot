/**************************************************************************/
/*  gd_softrender.h                                                       */
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

#ifndef SOFTRENDER_H
#define SOFTRENDER_H

#include "core/image.h"
#include "core/math/camera_matrix.h"
#include "core/reference.h"
#include "core/vector.h"
#include "scene/resources/texture.h"

class SWRBackend;

class SoftRender : public Reference {
	GDCLASS(SoftRender, Reference);

public:
	enum Backend {
		BACKEND_PORTABLEGL = 0,
		BACKEND_FUSION2X = 1,
	};

	enum MatrixMode {
		MATRIX_MODELVIEW = 0,
		MATRIX_PROJECTION = 1,
	};

	enum ClearBit {
		COLOR_BUFFER_BIT = 1,
		DEPTH_BUFFER_BIT = 2,
	};

	enum Primitive {
		PRIM_POINTS = 0,
		PRIM_LINES = 1,
		PRIM_LINE_STRIP = 2,
		PRIM_LINE_LOOP = 3,
		PRIM_TRIANGLES = 4,
		PRIM_TRIANGLE_STRIP = 5,
		PRIM_TRIANGLE_FAN = 6,
		PRIM_QUADS = 7,
	};

private:
	SWRBackend *backend;
	int fb_width;
	int fb_height;

	// Matrix stacks
	int current_matrix_mode;
	CameraMatrix modelview;
	CameraMatrix projection;
	Vector<CameraMatrix> modelview_stack;
	Vector<CameraMatrix> projection_stack;

	// Immediate mode state
	bool in_begin;
	int current_primitive;
	Color current_color;
	Vector3 current_normal;
	Vector2 current_texcoord;
	bool has_colors;
	bool has_normals;
	bool has_texcoords;

	// Vertex accumulation buffers
	Vector<float> acc_positions; // 3 per vertex
	Vector<float> acc_colors; // 4 per vertex
	Vector<float> acc_normals; // 3 per vertex
	Vector<float> acc_texcoords; // 2 per vertex
	int vertex_count;

	// Cached output
	Ref<Image> cached_image;
	Ref<ImageTexture> cached_texture;
	bool dirty;

	void _ensure_backend();
	CameraMatrix &_current_matrix();

protected:
	static void _bind_methods();

public:
	// Lifecycle
	void initialize(int p_width, int p_height, int p_backend = BACKEND_PORTABLEGL);
	int get_width() const;
	int get_height() const;
	String get_backend_name() const;
	Ref<Image> get_image();
	Ref<ImageTexture> get_texture();

	// Clear
	void clear_color(const Color &p_color);
	void clear(int p_mask);

	// Matrix operations
	void matrix_mode(int p_mode);
	void load_identity();
	void push_matrix();
	void pop_matrix();
	void perspective(float p_fov_degrees, float p_aspect, float p_z_near, float p_z_far);
	void ortho(float p_size, float p_aspect, float p_z_near, float p_z_far);
	void ortho_bounds(float p_left, float p_right, float p_bottom, float p_top, float p_z_near, float p_z_far);
	void frustum(float p_size, float p_aspect, float p_z_near, float p_z_far);
	void translate(const Vector3 &p_v);
	void rotate(float p_angle_degrees, const Vector3 &p_axis);
	void scale_matrix(const Vector3 &p_v);
	void look_at(const Vector3 &p_eye, const Vector3 &p_center, const Vector3 &p_up);

	// Immediate mode geometry
	void begin_mesh(int p_primitive_type);
	void end_mesh();
	void vertex3(const Vector3 &p_v);
	void color4(const Color &p_c);
	void normal3(const Vector3 &p_n);
	void texcoord2(const Vector2 &p_t);

	// State
	void enable_depth_test(bool p_enabled);
	void enable_blend(bool p_enabled);
	void enable_cull_face(bool p_enabled);

	// Textures
	int create_texture(const Ref<Image> &p_image);
	void bind_texture(int p_tex_id);
	void delete_texture(int p_tex_id);

	SoftRender();
	~SoftRender();
};

VARIANT_ENUM_CAST(SoftRender::Backend);
VARIANT_ENUM_CAST(SoftRender::MatrixMode);
VARIANT_ENUM_CAST(SoftRender::ClearBit);
VARIANT_ENUM_CAST(SoftRender::Primitive);

#endif // SOFTRENDER_H
