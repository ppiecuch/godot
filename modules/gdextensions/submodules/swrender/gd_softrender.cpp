/**************************************************************************/
/*  gd_softrender.cpp                                                     */
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

#include "gd_softrender.h"
#include "swr_backend.h"

#include "doctest/doctest.h"

#include <string.h>

// ============================================================
// SoftRender Implementation
// ============================================================

SoftRender::SoftRender() {
	backend = nullptr;
	fb_width = 0;
	fb_height = 0;
	current_matrix_mode = MATRIX_MODELVIEW;
	in_begin = false;
	current_primitive = PRIM_TRIANGLES;
	current_color = Color(1, 1, 1, 1);
	current_normal = Vector3(0, 0, 1);
	current_texcoord = Vector2(0, 0);
	has_colors = false;
	has_normals = false;
	has_texcoords = false;
	vertex_count = 0;
	dirty = false;

	modelview.set_identity();
	projection.set_identity();
}

SoftRender::~SoftRender() {
	if (backend) {
		backend->destroy();
		delete backend;
		backend = nullptr;
	}
}

void SoftRender::_ensure_backend() {
	if (!backend) {
		initialize(256, 256, BACKEND_PORTABLEGL);
	}
}

CameraMatrix &SoftRender::_current_matrix() {
	if (current_matrix_mode == MATRIX_PROJECTION) {
		return projection;
	}
	return modelview;
}

// ---- Lifecycle ----

void SoftRender::initialize(int p_width, int p_height, int p_backend) {
	ERR_FAIL_COND(p_width < 1 || p_width > 8192);
	ERR_FAIL_COND(p_height < 1 || p_height > 8192);

	if (backend) {
		backend->destroy();
		delete backend;
		backend = nullptr;
	}

	switch (p_backend) {
		case BACKEND_PORTABLEGL:
			backend = SWRBackend::create_portablegl();
			break;
		case BACKEND_FUSION2X:
			backend = SWRBackend::create_fusion2x();
			break;
		default:
			ERR_FAIL_MSG("Unknown backend type");
			return;
	}

	ERR_FAIL_COND_MSG(!backend, "Failed to create backend (not compiled in)");

	if (!backend->initialize(p_width, p_height)) {
		delete backend;
		backend = nullptr;
		ERR_FAIL_MSG("Backend initialization failed");
		return;
	}

	fb_width = p_width;
	fb_height = p_height;
	dirty = true;
	cached_image.unref();
	cached_texture.unref();

	// Reset matrix stacks
	modelview.set_identity();
	projection.set_identity();
	modelview_stack.clear();
	projection_stack.clear();
	current_matrix_mode = MATRIX_MODELVIEW;
}

int SoftRender::get_width() const {
	return fb_width;
}

int SoftRender::get_height() const {
	return fb_height;
}

String SoftRender::get_backend_name() const {
	if (!backend)
		return "None";
	return backend->get_name();
}

Ref<Image> SoftRender::get_image() {
	ERR_FAIL_COND_V(!backend, Ref<Image>());

	PoolByteArray data;
	int size = fb_width * fb_height * 4;
	data.resize(size);
	{
		PoolByteArray::Write wd = data.write();
		backend->read_pixels(wd.ptr());
	}
	cached_image.instance();
	cached_image->create(fb_width, fb_height, false, Image::FORMAT_RGBA8, data);
	dirty = false;
	return cached_image;
}

Ref<ImageTexture> SoftRender::get_texture() {
	ERR_FAIL_COND_V(!backend, Ref<ImageTexture>());
	Ref<Image> img = get_image();
	ERR_FAIL_COND_V(img.is_null(), Ref<ImageTexture>());
	if (cached_texture.is_null()) {
		cached_texture.instance();
	}
	cached_texture->create_from_image(img, 0);
	return cached_texture;
}

// ---- Clear ----

void SoftRender::clear_color(const Color &p_color) {
	_ensure_backend();
	backend->clear_color(p_color.r, p_color.g, p_color.b, p_color.a);
}

void SoftRender::clear(int p_mask) {
	_ensure_backend();
	backend->clear((uint32_t)p_mask);
	dirty = true;
}

// ---- Matrix operations ----

void SoftRender::matrix_mode(int p_mode) {
	current_matrix_mode = p_mode;
}

void SoftRender::load_identity() {
	_current_matrix().set_identity();
}

void SoftRender::push_matrix() {
	if (current_matrix_mode == MATRIX_PROJECTION) {
		projection_stack.push_back(projection);
	} else {
		modelview_stack.push_back(modelview);
	}
}

void SoftRender::pop_matrix() {
	if (current_matrix_mode == MATRIX_PROJECTION) {
		ERR_FAIL_COND(projection_stack.size() == 0);
		projection = projection_stack[projection_stack.size() - 1];
		projection_stack.resize(projection_stack.size() - 1);
	} else {
		ERR_FAIL_COND(modelview_stack.size() == 0);
		modelview = modelview_stack[modelview_stack.size() - 1];
		modelview_stack.resize(modelview_stack.size() - 1);
	}
}

void SoftRender::perspective(float p_fov_degrees, float p_aspect, float p_z_near, float p_z_far) {
	CameraMatrix persp;
	persp.set_perspective(p_fov_degrees, p_aspect, p_z_near, p_z_far);
	_current_matrix() = _current_matrix() * persp;
}

void SoftRender::ortho(float p_size, float p_aspect, float p_z_near, float p_z_far) {
	CameraMatrix orth;
	orth.set_orthogonal(p_size, p_aspect, p_z_near, p_z_far);
	_current_matrix() = _current_matrix() * orth;
}

void SoftRender::ortho_bounds(float p_left, float p_right, float p_bottom, float p_top, float p_z_near, float p_z_far) {
	CameraMatrix orth;
	orth.set_orthogonal(p_left, p_right, p_bottom, p_top, p_z_near, p_z_far);
	_current_matrix() = _current_matrix() * orth;
}

void SoftRender::frustum(float p_size, float p_aspect, float p_z_near, float p_z_far) {
	CameraMatrix frust;
	frust.set_frustum(p_size, p_aspect, Vector2(), p_z_near, p_z_far);
	_current_matrix() = _current_matrix() * frust;
}

void SoftRender::translate(const Vector3 &p_v) {
	CameraMatrix t;
	t.set_identity();
	t.matrix[3][0] = p_v.x;
	t.matrix[3][1] = p_v.y;
	t.matrix[3][2] = p_v.z;
	_current_matrix() = _current_matrix() * t;
}

void SoftRender::rotate(float p_angle_degrees, const Vector3 &p_axis) {
	float rad = Math::deg2rad(p_angle_degrees);
	Vector3 axis = p_axis.normalized();
	float c = Math::cos(rad);
	float s = Math::sin(rad);
	float t = 1.0f - c;

	CameraMatrix r;
	r.set_identity();
	r.matrix[0][0] = t * axis.x * axis.x + c;
	r.matrix[0][1] = t * axis.x * axis.y + s * axis.z;
	r.matrix[0][2] = t * axis.x * axis.z - s * axis.y;

	r.matrix[1][0] = t * axis.x * axis.y - s * axis.z;
	r.matrix[1][1] = t * axis.y * axis.y + c;
	r.matrix[1][2] = t * axis.y * axis.z + s * axis.x;

	r.matrix[2][0] = t * axis.x * axis.z + s * axis.y;
	r.matrix[2][1] = t * axis.y * axis.z - s * axis.x;
	r.matrix[2][2] = t * axis.z * axis.z + c;

	_current_matrix() = _current_matrix() * r;
}

void SoftRender::scale_matrix(const Vector3 &p_v) {
	CameraMatrix s;
	s.set_identity();
	s.matrix[0][0] = p_v.x;
	s.matrix[1][1] = p_v.y;
	s.matrix[2][2] = p_v.z;
	_current_matrix() = _current_matrix() * s;
}

void SoftRender::look_at(const Vector3 &p_eye, const Vector3 &p_center, const Vector3 &p_up) {
	Vector3 f = (p_center - p_eye).normalized();
	Vector3 s = f.cross(p_up).normalized();
	Vector3 u = s.cross(f);

	CameraMatrix m;
	m.set_identity();
	m.matrix[0][0] = s.x;
	m.matrix[1][0] = s.y;
	m.matrix[2][0] = s.z;
	m.matrix[0][1] = u.x;
	m.matrix[1][1] = u.y;
	m.matrix[2][1] = u.z;
	m.matrix[0][2] = -f.x;
	m.matrix[1][2] = -f.y;
	m.matrix[2][2] = -f.z;
	m.matrix[3][0] = -s.dot(p_eye);
	m.matrix[3][1] = -u.dot(p_eye);
	m.matrix[3][2] = f.dot(p_eye);

	_current_matrix() = _current_matrix() * m;
}

// ---- Immediate mode geometry ----

void SoftRender::begin_mesh(int p_primitive_type) {
	ERR_FAIL_COND(in_begin);
	_ensure_backend();

	in_begin = true;
	current_primitive = p_primitive_type;
	has_colors = false;
	has_normals = false;
	has_texcoords = false;
	vertex_count = 0;

	acc_positions.clear();
	acc_colors.clear();
	acc_normals.clear();
	acc_texcoords.clear();

	current_color = Color(1, 1, 1, 1);
	current_normal = Vector3(0, 0, 1);
	current_texcoord = Vector2(0, 0);
}

void SoftRender::end_mesh() {
	ERR_FAIL_COND(!in_begin);
	in_begin = false;

	if (vertex_count == 0 || !backend) {
		return;
	}

	// Compute MVP
	CameraMatrix mvp = projection * modelview;
	float mvp_data[16];
	// CameraMatrix stores matrix[col][row], flatten to column-major
	for (int col = 0; col < 4; col++) {
		for (int row = 0; row < 4; row++) {
			mvp_data[col * 4 + row] = mvp.matrix[col][row];
		}
	}

	float uniform_color[4] = {
		current_color.r, current_color.g, current_color.b, current_color.a
	};

	backend->draw(
			(SWRBackend::PrimitiveType)current_primitive,
			acc_positions.ptr(), vertex_count,
			has_colors ? acc_colors.ptr() : nullptr,
			has_texcoords ? acc_texcoords.ptr() : nullptr,
			has_normals ? acc_normals.ptr() : nullptr,
			mvp_data,
			uniform_color);

	dirty = true;
}

void SoftRender::vertex3(const Vector3 &p_v) {
	ERR_FAIL_COND(!in_begin);

	acc_positions.push_back(p_v.x);
	acc_positions.push_back(p_v.y);
	acc_positions.push_back(p_v.z);

	// When vertex is submitted, store current attribute state
	if (has_colors) {
		acc_colors.push_back(current_color.r);
		acc_colors.push_back(current_color.g);
		acc_colors.push_back(current_color.b);
		acc_colors.push_back(current_color.a);
	}
	if (has_texcoords) {
		acc_texcoords.push_back(current_texcoord.x);
		acc_texcoords.push_back(current_texcoord.y);
	}
	if (has_normals) {
		acc_normals.push_back(current_normal.x);
		acc_normals.push_back(current_normal.y);
		acc_normals.push_back(current_normal.z);
	}

	vertex_count++;
}

void SoftRender::color4(const Color &p_c) {
	current_color = p_c;
	if (in_begin && !has_colors) {
		has_colors = true;
		// Backfill previous vertices with default color
		for (int i = 0; i < vertex_count; i++) {
			acc_colors.push_back(1.0f);
			acc_colors.push_back(1.0f);
			acc_colors.push_back(1.0f);
			acc_colors.push_back(1.0f);
		}
	}
}

void SoftRender::normal3(const Vector3 &p_n) {
	current_normal = p_n;
	if (in_begin && !has_normals) {
		has_normals = true;
		for (int i = 0; i < vertex_count; i++) {
			acc_normals.push_back(0.0f);
			acc_normals.push_back(0.0f);
			acc_normals.push_back(1.0f);
		}
	}
}

void SoftRender::texcoord2(const Vector2 &p_t) {
	current_texcoord = p_t;
	if (in_begin && !has_texcoords) {
		has_texcoords = true;
		for (int i = 0; i < vertex_count; i++) {
			acc_texcoords.push_back(0.0f);
			acc_texcoords.push_back(0.0f);
		}
	}
}

// ---- State ----

void SoftRender::enable_depth_test(bool p_enabled) {
	_ensure_backend();
	backend->set_depth_test(p_enabled);
}

void SoftRender::enable_blend(bool p_enabled) {
	_ensure_backend();
	backend->set_blend(p_enabled);
}

void SoftRender::enable_cull_face(bool p_enabled) {
	_ensure_backend();
	backend->set_cull_face(p_enabled);
}

// ---- Textures ----

int SoftRender::create_texture(const Ref<Image> &p_image) {
	_ensure_backend();
	ERR_FAIL_COND_V(p_image.is_null(), 0);

	Ref<Image> img = p_image;
	if (img->get_format() != Image::FORMAT_RGBA8) {
		img = img->duplicate();
		img->convert(Image::FORMAT_RGBA8);
	}

	PoolByteArray data = img->get_data();
	PoolByteArray::Read rd = data.read();
	return (int)backend->upload_texture(img->get_width(), img->get_height(), rd.ptr());
}

void SoftRender::bind_texture(int p_tex_id) {
	_ensure_backend();
	backend->bind_texture((uint32_t)p_tex_id);
}

void SoftRender::delete_texture(int p_tex_id) {
	if (!backend)
		return;
	backend->delete_texture((uint32_t)p_tex_id);
}

// ============================================================
// GDScript Bindings
// ============================================================

void SoftRender::_bind_methods() {
	// Lifecycle
	ClassDB::bind_method(D_METHOD("initialize", "width", "height", "backend"), &SoftRender::initialize, DEFVAL(BACKEND_PORTABLEGL));
	ClassDB::bind_method(D_METHOD("get_width"), &SoftRender::get_width);
	ClassDB::bind_method(D_METHOD("get_height"), &SoftRender::get_height);
	ClassDB::bind_method(D_METHOD("get_backend_name"), &SoftRender::get_backend_name);
	ClassDB::bind_method(D_METHOD("get_image"), &SoftRender::get_image);
	ClassDB::bind_method(D_METHOD("get_texture"), &SoftRender::get_texture);

	// Clear
	ClassDB::bind_method(D_METHOD("clear_color", "color"), &SoftRender::clear_color);
	ClassDB::bind_method(D_METHOD("clear", "mask"), &SoftRender::clear);

	// Matrix operations
	ClassDB::bind_method(D_METHOD("matrix_mode", "mode"), &SoftRender::matrix_mode);
	ClassDB::bind_method(D_METHOD("load_identity"), &SoftRender::load_identity);
	ClassDB::bind_method(D_METHOD("push_matrix"), &SoftRender::push_matrix);
	ClassDB::bind_method(D_METHOD("pop_matrix"), &SoftRender::pop_matrix);
	ClassDB::bind_method(D_METHOD("perspective", "fov_degrees", "aspect", "z_near", "z_far"), &SoftRender::perspective);
	ClassDB::bind_method(D_METHOD("ortho", "size", "aspect", "z_near", "z_far"), &SoftRender::ortho);
	ClassDB::bind_method(D_METHOD("frustum", "size", "aspect", "z_near", "z_far"), &SoftRender::frustum);
	ClassDB::bind_method(D_METHOD("translate", "offset"), &SoftRender::translate);
	ClassDB::bind_method(D_METHOD("rotate", "angle_degrees", "axis"), &SoftRender::rotate);
	ClassDB::bind_method(D_METHOD("scale_matrix", "scale"), &SoftRender::scale_matrix);
	ClassDB::bind_method(D_METHOD("look_at", "eye", "center", "up"), &SoftRender::look_at);

	// Immediate mode geometry
	ClassDB::bind_method(D_METHOD("begin_mesh", "primitive_type"), &SoftRender::begin_mesh);
	ClassDB::bind_method(D_METHOD("end_mesh"), &SoftRender::end_mesh);
	ClassDB::bind_method(D_METHOD("vertex3", "vertex"), &SoftRender::vertex3);
	ClassDB::bind_method(D_METHOD("color4", "color"), &SoftRender::color4);
	ClassDB::bind_method(D_METHOD("normal3", "normal"), &SoftRender::normal3);
	ClassDB::bind_method(D_METHOD("texcoord2", "texcoord"), &SoftRender::texcoord2);

	// State
	ClassDB::bind_method(D_METHOD("enable_depth_test", "enabled"), &SoftRender::enable_depth_test);
	ClassDB::bind_method(D_METHOD("enable_blend", "enabled"), &SoftRender::enable_blend);
	ClassDB::bind_method(D_METHOD("enable_cull_face", "enabled"), &SoftRender::enable_cull_face);

	// Textures
	ClassDB::bind_method(D_METHOD("create_texture", "image"), &SoftRender::create_texture);
	ClassDB::bind_method(D_METHOD("bind_texture", "tex_id"), &SoftRender::bind_texture);
	ClassDB::bind_method(D_METHOD("delete_texture", "tex_id"), &SoftRender::delete_texture);

	// Enums
	BIND_ENUM_CONSTANT(BACKEND_PORTABLEGL);
	BIND_ENUM_CONSTANT(BACKEND_FUSION2X);

	BIND_ENUM_CONSTANT(MATRIX_MODELVIEW);
	BIND_ENUM_CONSTANT(MATRIX_PROJECTION);

	BIND_ENUM_CONSTANT(COLOR_BUFFER_BIT);
	BIND_ENUM_CONSTANT(DEPTH_BUFFER_BIT);

	BIND_ENUM_CONSTANT(PRIM_POINTS);
	BIND_ENUM_CONSTANT(PRIM_LINES);
	BIND_ENUM_CONSTANT(PRIM_LINE_STRIP);
	BIND_ENUM_CONSTANT(PRIM_LINE_LOOP);
	BIND_ENUM_CONSTANT(PRIM_TRIANGLES);
	BIND_ENUM_CONSTANT(PRIM_TRIANGLE_STRIP);
	BIND_ENUM_CONSTANT(PRIM_TRIANGLE_FAN);
	BIND_ENUM_CONSTANT(PRIM_QUADS);
}

// ============================================================
// Doctests
// ============================================================

#ifdef SWRENDER_PORTABLEGL

TEST_SUITE("swrender") {
	TEST_CASE("[SoftRender] Lifecycle - initialize and dimensions") {
		Ref<SoftRender> sr;
		sr.instance();
		CHECK(sr->get_width() == 0);
		CHECK(sr->get_height() == 0);

		sr->initialize(320, 240);
		CHECK(sr->get_width() == 320);
		CHECK(sr->get_height() == 240);
	}

	TEST_CASE("[SoftRender] Lifecycle - get_image returns RGBA8") {
		Ref<SoftRender> sr;
		sr.instance();
		sr->initialize(64, 64);

		Ref<Image> img = sr->get_image();
		REQUIRE(img.is_valid());
		CHECK(img->get_width() == 64);
		CHECK(img->get_height() == 64);
		CHECK(img->get_format() == Image::FORMAT_RGBA8);
	}

	TEST_CASE("[SoftRender] Lifecycle - get_texture valid") {
		Ref<SoftRender> sr;
		sr.instance();
		sr->initialize(32, 32);

		Ref<ImageTexture> tex = sr->get_texture();
		REQUIRE(tex.is_valid());
		CHECK(tex->get_width() == 32);
		CHECK(tex->get_height() == 32);
	}

	TEST_CASE("[SoftRender] Backend name") {
		Ref<SoftRender> sr;
		sr.instance();
		sr->initialize(32, 32, SoftRender::BACKEND_PORTABLEGL);
		CHECK(sr->get_backend_name() == "PortableGL");
	}

	TEST_CASE("[SoftRender] Clear fills framebuffer with expected color") {
		Ref<SoftRender> sr;
		sr.instance();
		sr->initialize(64, 64);

		sr->clear_color(Color(1, 0, 0, 1));
		sr->clear(SoftRender::COLOR_BUFFER_BIT);

		Ref<Image> img = sr->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		Color pixel = img->get_pixel(32, 32);
		CHECK(pixel.r > 0.9f);
		CHECK(pixel.g < 0.1f);
		CHECK(pixel.b < 0.1f);
		CHECK(pixel.a > 0.9f);
		img->unlock();
	}

	TEST_CASE("[SoftRender] Clear with blue") {
		Ref<SoftRender> sr;
		sr.instance();
		sr->initialize(64, 64);

		sr->clear_color(Color(0, 0, 1, 1));
		sr->clear(SoftRender::COLOR_BUFFER_BIT);

		Ref<Image> img = sr->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		Color pixel = img->get_pixel(16, 16);
		CHECK(pixel.b > 0.9f);
		CHECK(pixel.r < 0.1f);
		img->unlock();
	}

	TEST_CASE("[SoftRender] Triangle rendering - flat color") {
		Ref<SoftRender> sr;
		sr.instance();
		sr->initialize(128, 128);

		sr->clear_color(Color(0, 0, 0, 1));
		sr->clear(SoftRender::COLOR_BUFFER_BIT | SoftRender::DEPTH_BUFFER_BIT);

		// Set up orthographic projection
		sr->matrix_mode(SoftRender::MATRIX_PROJECTION);
		sr->load_identity();
		sr->ortho_bounds(-1, 1, -1, 1, -1, 1);

		sr->matrix_mode(SoftRender::MATRIX_MODELVIEW);
		sr->load_identity();

		// Draw a white triangle covering center area
		sr->begin_mesh(SoftRender::PRIM_TRIANGLES);
		sr->vertex3(Vector3(0, 0.8, 0));
		sr->vertex3(Vector3(-0.8, -0.8, 0));
		sr->vertex3(Vector3(0.8, -0.8, 0));
		sr->end_mesh();

		Ref<Image> img = sr->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		// Center of triangle should be non-black
		Color center = img->get_pixel(64, 64);
		CHECK(center.r > 0.5f);
		CHECK(center.a > 0.5f);
		img->unlock();
	}

	TEST_CASE("[SoftRender] Vertex color interpolation") {
		Ref<SoftRender> sr;
		sr.instance();
		sr->initialize(128, 128);

		sr->clear_color(Color(0, 0, 0, 1));
		sr->clear(SoftRender::COLOR_BUFFER_BIT);

		sr->matrix_mode(SoftRender::MATRIX_PROJECTION);
		sr->load_identity();
		sr->ortho_bounds(-1, 1, -1, 1, -1, 1);
		sr->matrix_mode(SoftRender::MATRIX_MODELVIEW);
		sr->load_identity();

		sr->begin_mesh(SoftRender::PRIM_TRIANGLES);
		sr->color4(Color(1, 0, 0, 1));
		sr->vertex3(Vector3(0, 0.9, 0));
		sr->color4(Color(0, 1, 0, 1));
		sr->vertex3(Vector3(-0.9, -0.9, 0));
		sr->color4(Color(0, 0, 1, 1));
		sr->vertex3(Vector3(0.9, -0.9, 0));
		sr->end_mesh();

		Ref<Image> img = sr->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		// Center should be a mix of R, G, B
		Color center = img->get_pixel(64, 70);
		CHECK(center.a > 0.5f);
		// At the centroid, each channel should have some contribution
		CHECK(center.r > 0.05f);
		CHECK(center.g > 0.05f);
		CHECK(center.b > 0.05f);
		img->unlock();
	}

	TEST_CASE("[SoftRender] Depth test - front occludes back") {
		Ref<SoftRender> sr;
		sr.instance();
		sr->initialize(128, 128);
		sr->enable_depth_test(true);

		sr->clear_color(Color(0, 0, 0, 1));
		sr->clear(SoftRender::COLOR_BUFFER_BIT | SoftRender::DEPTH_BUFFER_BIT);

		sr->matrix_mode(SoftRender::MATRIX_PROJECTION);
		sr->load_identity();
		sr->ortho_bounds(-1, 1, -1, 1, -1, 1);
		sr->matrix_mode(SoftRender::MATRIX_MODELVIEW);
		sr->load_identity();

		// Draw blue triangle at z=-0.5 (far)
		sr->begin_mesh(SoftRender::PRIM_TRIANGLES);
		sr->color4(Color(0, 0, 1, 1));
		sr->vertex3(Vector3(-0.9, -0.9, -0.5));
		sr->vertex3(Vector3(0.9, -0.9, -0.5));
		sr->vertex3(Vector3(0, 0.9, -0.5));
		sr->end_mesh();

		// Draw red triangle at z=0 (front), smaller
		sr->begin_mesh(SoftRender::PRIM_TRIANGLES);
		sr->color4(Color(1, 0, 0, 1));
		sr->vertex3(Vector3(-0.5, -0.5, 0));
		sr->vertex3(Vector3(0.5, -0.5, 0));
		sr->vertex3(Vector3(0, 0.5, 0));
		sr->end_mesh();

		Ref<Image> img = sr->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		// Center should be red (front triangle)
		Color center = img->get_pixel(64, 64);
		CHECK(center.r > 0.5f);
		CHECK(center.b < 0.5f);
		img->unlock();

		sr->enable_depth_test(false);
	}

	TEST_CASE("[SoftRender] Matrix stack - push/pop preserves state") {
		Ref<SoftRender> sr;
		sr.instance();
		sr->initialize(128, 128);

		sr->clear_color(Color(0, 0, 0, 1));
		sr->clear(SoftRender::COLOR_BUFFER_BIT);

		sr->matrix_mode(SoftRender::MATRIX_PROJECTION);
		sr->load_identity();
		sr->ortho_bounds(-1, 1, -1, 1, -1, 1);

		sr->matrix_mode(SoftRender::MATRIX_MODELVIEW);
		sr->load_identity();

		// Draw first triangle at origin
		sr->push_matrix();
		sr->translate(Vector3(-0.5, 0, 0));

		sr->begin_mesh(SoftRender::PRIM_TRIANGLES);
		sr->color4(Color(1, 0, 0, 1));
		sr->vertex3(Vector3(-0.3, -0.3, 0));
		sr->vertex3(Vector3(0.3, -0.3, 0));
		sr->vertex3(Vector3(0, 0.3, 0));
		sr->end_mesh();

		sr->pop_matrix();

		// After pop, should be back at identity
		sr->push_matrix();
		sr->translate(Vector3(0.5, 0, 0));

		sr->begin_mesh(SoftRender::PRIM_TRIANGLES);
		sr->color4(Color(0, 0, 1, 1));
		sr->vertex3(Vector3(-0.3, -0.3, 0));
		sr->vertex3(Vector3(0.3, -0.3, 0));
		sr->vertex3(Vector3(0, 0.3, 0));
		sr->end_mesh();

		sr->pop_matrix();

		Ref<Image> img = sr->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		// Left side should have red
		Color left = img->get_pixel(32, 64);
		CHECK(left.r > 0.5f);
		// Right side should have blue
		Color right = img->get_pixel(96, 64);
		CHECK(right.b > 0.5f);
		img->unlock();
	}

	TEST_CASE("[SoftRender] _ensure_backend auto-initializes") {
		Ref<SoftRender> sr;
		sr.instance();
		sr->clear_color(Color(0.5, 0.5, 0.5, 1));
		CHECK(sr->get_width() == 256);
		CHECK(sr->get_height() == 256);
		CHECK(sr->get_backend_name() == "PortableGL");
	}

	TEST_CASE("[SoftRender] Reinitialize with new size") {
		Ref<SoftRender> sr;
		sr.instance();
		sr->initialize(64, 64);
		CHECK(sr->get_width() == 64);

		sr->initialize(128, 96);
		CHECK(sr->get_width() == 128);
		CHECK(sr->get_height() == 96);
	}

	TEST_CASE("[SoftRender] Quad rendering") {
		Ref<SoftRender> sr;
		sr.instance();
		sr->initialize(64, 64);

		sr->clear_color(Color(0, 0, 0, 1));
		sr->clear(SoftRender::COLOR_BUFFER_BIT);

		sr->matrix_mode(SoftRender::MATRIX_PROJECTION);
		sr->load_identity();
		sr->ortho_bounds(-1, 1, -1, 1, -1, 1);
		sr->matrix_mode(SoftRender::MATRIX_MODELVIEW);
		sr->load_identity();

		sr->begin_mesh(SoftRender::PRIM_QUADS);
		sr->color4(Color(0, 1, 0, 1));
		sr->vertex3(Vector3(-0.5, -0.5, 0));
		sr->vertex3(Vector3(0.5, -0.5, 0));
		sr->vertex3(Vector3(0.5, 0.5, 0));
		sr->vertex3(Vector3(-0.5, 0.5, 0));
		sr->end_mesh();

		Ref<Image> img = sr->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		Color center = img->get_pixel(32, 32);
		CHECK(center.g > 0.5f);
		CHECK(center.a > 0.5f);
		img->unlock();
	}

	TEST_CASE("[SoftRender] Perspective projection") {
		Ref<SoftRender> sr;
		sr.instance();
		sr->initialize(128, 128);

		sr->clear_color(Color(0, 0, 0, 1));
		sr->clear(SoftRender::COLOR_BUFFER_BIT | SoftRender::DEPTH_BUFFER_BIT);

		sr->matrix_mode(SoftRender::MATRIX_PROJECTION);
		sr->load_identity();
		sr->perspective(60.0f, 1.0f, 0.1f, 100.0f);

		sr->matrix_mode(SoftRender::MATRIX_MODELVIEW);
		sr->load_identity();
		sr->translate(Vector3(0, 0, -3));

		sr->begin_mesh(SoftRender::PRIM_TRIANGLES);
		sr->color4(Color(1, 1, 0, 1));
		sr->vertex3(Vector3(0, 1, 0));
		sr->vertex3(Vector3(-1, -1, 0));
		sr->vertex3(Vector3(1, -1, 0));
		sr->end_mesh();

		Ref<Image> img = sr->get_image();
		REQUIRE(img.is_valid());
		img->lock();
		// Something should be drawn
		bool found = false;
		for (int y = 0; y < 128 && !found; y++) {
			for (int x = 0; x < 128 && !found; x++) {
				Color px = img->get_pixel(x, y);
				if (px.r > 0.5f && px.g > 0.5f) {
					found = true;
				}
			}
		}
		CHECK(found);
		img->unlock();
	}

	TEST_CASE("[SoftRender] Texture creation and binding") {
		Ref<SoftRender> sr;
		sr.instance();
		sr->initialize(64, 64);

		// Create a checkerboard texture
		Ref<Image> checker;
		checker.instance();
		checker->create(4, 4, false, Image::FORMAT_RGBA8);
		checker->lock();
		for (int y = 0; y < 4; y++) {
			for (int x = 0; x < 4; x++) {
				if ((x + y) % 2 == 0) {
					checker->set_pixel(x, y, Color(1, 1, 1, 1));
				} else {
					checker->set_pixel(x, y, Color(0, 0, 0, 1));
				}
			}
		}
		checker->unlock();

		int tex_id = sr->create_texture(checker);
		CHECK(tex_id > 0);

		sr->bind_texture(tex_id);
		sr->delete_texture(tex_id);
	}
}

#endif // SWRENDER_PORTABLEGL
