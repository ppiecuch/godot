/*************************************************************************/
/*  benchmark.cpp                                                        */
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

// Reference:
// ----------
// https://github.com/raysan5/raylib/issues/65
// http://www.codinghorror.com/blog/2011/12/fast-approximate-anti-aliasing-fxaa.html

#include "common/gd_core.h"
#include "common/gd_history_plot.h"
#include "core/error_macros.h"
#include "core/image.h"
#include "core/io/resource_saver.h"
#include "core/math/math_funcs.h"
#include "core/math/transform.h"
#include "core/os/keyboard.h"
#include "main/performance.h"
#include "scene/gui/label.h"
#include "scene/main/viewport.h"
#include "scene/resources/font.h"
#include "scene/resources/texture.h"

#include "benchmark.h"

const static char *postproShaderText[] = {
	"GRAYSCALE",
	"POSTERIZATION",
	"DREAM_VISION",
	"PIXELIZER",
	"CROSS_HATCHING",
	"CROSS_STITCHING",
	"PREDATOR_VIEW",
	"SCANLINES",
	"FISHEYE",
	"SOBEL",
	"BLOOM",
	"BLUR",
	"FXAA",
	"NONE"
};

const static char *shaderNames[] = {
	"PER-PIXEL LIGHTING",
	"PER-VERTEX LIGHTING",
	"PER-PIXEL UNTEXTURED",
	"PER-VERTEX UNTEXTURED",
	"FLAT COLORED"
};

const String _BULLET1 = String::chr(96);
const String _BULLET2 = String::chr(126);
const String _BULLET3 = String::chr(127);
const String _BULLET4 = String::chr(148);
const String _BULLET5 = String::chr(123);
const String _BULLET6 = String::chr(159);
const String _LT = String::chr(145);
const String _RT = String::chr(133);
const String _LB = String::chr(154);
const String _RB = String::chr(131);
const String _VL = String::chr(124);
const String _HL = String::chr(146);

_FORCE_INLINE_ static PoolByteArray create_poolarray(uint8_t *buf_ptr, int buf_size) {
	PoolByteArray data;
	data.resize(buf_size);
	memcpy(data.write().ptr(), buf_ptr, buf_size);
	return data;
}

static Ref<BitmapFont> make_font_from_grid(const String &p_characters, int p_grid_width, int p_grid_height, const uint8_t *p_img, size_t p_img_size) {
	Ref<BitmapFont> font(memnew(BitmapFont));
	Ref<Image> image = memnew(Image(p_img, p_img_size));
	if (!image->empty()) {
		Ref<ImageTexture> tex = memnew(ImageTexture);
		tex->create_from_image(image, 0);

		font->add_texture(tex);

		const Size2i cell_size = image->get_size() / Size2(p_grid_width, p_grid_height);
		for (int x = 0; x < p_grid_width; x++) {
			for (int y = 0; y < p_grid_height; y++) {
				const int index = x + y * p_grid_width;
				if (index < p_characters.length()) {
					const int chr = p_characters[index];
					Rect2 frect(Point2(x * cell_size.width, y * cell_size.height), cell_size);
					font->add_char(chr, 0, frect, Point2(), cell_size.width);
				} else {
					break;
				}
			}
		}
		font->set_height(cell_size.height);
	}
	return font;
}

static Ref<Texture> make_texture_from_data(const uint8_t *p_data, size_t p_data_len, const String &p_name) {
	String fn = "user://__benchmark_" + p_name; // cached texture
	if (ResourceLoader::exists(fn + ".tex")) {
		Ref<Texture> texture = ResourceLoader::load(fn + ".tex", "Texture");
		return texture;
	} else {
		// build texture atlas from resources
		Vector<Ref<Image>> images;
		Vector<String> names;

		Ref<Image> image = memnew(Image(p_data, p_data_len));
		Ref<ImageTexture> texture = memnew(ImageTexture);
		texture->create_from_image(image);

		ResourceSaver::save(fn + ".tex", texture);

		return texture;
	}
}

extern "C" const uint8_t model_cube_dat_data[];
extern "C" const uint8_t texture_cube_png_data[];
extern "C" const size_t texture_cube_png_size;

extern "C" const uint8_t model_frog_dat_data[];
extern "C" const uint8_t texture_frog_png_data[];
extern "C" const size_t texture_frog_png_size;

extern "C" const uint8_t model_kid_dat_data[];
extern "C" const uint8_t texture_kid_png_data[];
extern "C" const size_t texture_kid_png_size;

extern "C" const uint8_t model_trex_dat_data[];
extern "C" const uint8_t texture_trex_png_data[];
extern "C" const size_t texture_trex_png_size;

extern "C" const uint8_t model_robot_dat_data[];
extern "C" const uint8_t texture_robot_png_data[];
extern "C" const size_t texture_robot_png_size;

extern "C" const uint8_t bitmap_font_png_data[];
extern "C" const size_t bitmap_font_png_size;

String Benchmark::_get_stats() {
	Vector<String> out;
	int max_line = 0;

	auto add_line = [&out, &max_line](const String &line) {
		out.push_back(line);
		max_line = MAX(line.size(), max_line);
	};
	add_line(vformat("%d VERTS/FRAME", 3 * num_objects * models[curr_model].tris));
	add_line(vformat("+/-: OBJECT COUNT [%d]", num_objects));
	add_line(vformat("X: OBJECT COMPLEXITY [%d TRIS]", models[curr_model].tris));
	add_line(vformat("F: VIEWPORT [%d X %d]", int(get_viewport()->get_size().width), int(get_viewport()->get_size().height)));
	add_line(vformat(" " + _BULLET3 + " WINDOW [%d X %d]", int(OS::get_singleton()->get_window_size().width), int(OS::get_singleton()->get_window_size().height)));
	add_line(vformat(" " + _BULLET3 + " PROJECT [%d X %d]", int(ProjectSettings::get_singleton()->get("display/window/size/width")), int(ProjectSettings::get_singleton()->get("display/window/size/height"))));
	add_line(vformat("P: MULTISAMPLING [%s]", opts[OPTION_MULTISAMPLING] ? "ON" : "OFF"));
	add_line(vformat("H: LIGHT MODEL [%s]", "?"));
	add_line(vformat("I: TEXTURE FILTER [%s]", opts[OPTION_LINEAR_FILTERING] ? "LINEAR" : "NEAREST"));
	add_line(vformat("M: MIPMAPS [%s]", opts[OPTION_MIPMAPING] ? "ON" : "OFF"));
	add_line(vformat("C: BACKFACE CULLING [%s]", opts[OPTION_CULLING] ? "ON" : "OFF"));
	add_line(vformat("D: DEPTH TEST [%s]", opts[OPTION_DEPTHTEST] ? "ON" : "OFF"));
	add_line(vformat("W: WIREFRAME [%s]", opts[OPTION_WIREFRAME] ? "ON" : "OFF"));
	add_line(vformat("Z: CAMERA DISTANCE [%d]", int(camera_distance)));
	add_line(vformat("Y: CAMERA YAW [%d]", int(camera_yaw)));
	add_line(vformat("L: CAMERA LOOK [%s]", opts[OPTION_CAMERA_LOOKAWAY] ? "AWAY" : "TOWARDS"));
	add_line(vformat("A: POSTPROCESS [%s]", "?"));
	add_line("S: STATS DISPLAY [ON]");

	String ret = _LT + _HL.repeat(max_line) + _RT + "\n";
	for (const String &s : out) {
		ret += _VL + s.rpad(max_line) + _VL + "\n";
	}
	ret += _LB + _HL.repeat(max_line) + _RB;

	_stats_text_size = Size2(max_line + 2, ret.size());

	return ret;
}

RID Benchmark::_create_instance_from_model(const ModelInfo &p_model) {
	const int MESH_BASE_LAYER = 16;
	const uint32_t layer = 1 << MESH_BASE_LAYER;
	RID mesh_instance = RID_PRIME(VS::get_singleton()->instance_create());
	VS::get_singleton()->instance_set_base(mesh_instance, p_model.mesh->get_rid());
	VS::get_singleton()->instance_set_surface_material(mesh_instance, 0, model_mat->get_rid());
	VS::get_singleton()->instance_set_scenario(mesh_instance, get_tree()->get_root()->get_world()->get_scenario());
	VS::get_singleton()->instance_set_visible(mesh_instance, true);
	VS::get_singleton()->instance_geometry_set_cast_shadows_setting(mesh_instance, VS::SHADOW_CASTING_SETTING_OFF);
	VS::get_singleton()->instance_set_layer_mask(mesh_instance, layer);
	VS::get_singleton()->instance_set_portal_mode(mesh_instance, VisualServer::INSTANCE_PORTAL_MODE_GLOBAL);

	return mesh_instance;
}

Benchmark::ModelInfo Benchmark::_make_model_from_data(const uint8_t *p_data, const uint8_t *p_tex_img, size_t p_tex_img_size, const String &p_name) {
	const uint32_t verts_num = *(uint32_t *)p_data;
	struct _vert_t {
		struct {
			float x, y, z;
		} pos;
		struct {
			float x, y, z;
		} norm;
		struct {
			float u, v;
		} uv;
	} *geom = (_vert_t *)(p_data + 4);
	const uint32_t verts_size = verts_num * sizeof(_vert_t);
	const uint32_t indexes_num = *(uint32_t *)(p_data + 4 + verts_size);
	const uint16_t *indexes = (uint16_t *)(p_data + 4 + verts_size + 4);

	PoolVector3Array verts, norm;
	PoolVector2Array uv;

	verts.resize(verts_num);
	norm.resize(verts_num);
	uv.resize(verts_num);

	auto _verts = verts.write();
	auto _norm = norm.write();
	auto _uv = uv.write();

	for (int n = 0; n < verts_num; n++) {
		_verts[n] = Vector3(geom[n].pos.x, geom[n].pos.y, geom[n].pos.z);
		_norm[n] = Vector3(geom[n].norm.x, geom[n].norm.y, geom[n].norm.z);
		_uv[n] = Vector2(geom[n].uv.u, geom[n].uv.v);
	}

	_uv.release();
	_norm.release();
	_verts.release();

	print_verbose(vformat("Loading model data: %d vertexes, %d indexes, %d bytes of data.", verts_num, indexes_num, verts_size));

	PoolIntArray index;
	index.resize(indexes_num);

	auto _index = index.write();

	for (int i = 0; i < indexes_num; i++) {
		_index[i] = indexes[i];
	}

	_index.release();

	Array d;
	d.resize(VS::ARRAY_MAX);
	d[VisualServer::ARRAY_VERTEX] = verts;
	d[VisualServer::ARRAY_NORMAL] = norm;
	d[VisualServer::ARRAY_TEX_UV] = uv;
	d[VisualServer::ARRAY_INDEX] = index;

	Ref<ArrayMesh> mesh = newref(ArrayMesh);
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, d);

	String fn = "user://__benchmark_" + p_name; // cache mesh
	ResourceSaver::save(fn + ".mesh", mesh);

	return ModelInfo{ mesh, make_texture_from_data(p_tex_img, p_tex_img_size, p_name), indexes_num / 3 };
}

void Benchmark::_load_resources() {
	static wchar_t _chars[] = {
		' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
		'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
		'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
		96, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
		'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 123, 124, 125, 126, 127,
		128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
		144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159
	};
	screen_font = make_font_from_grid(_chars, 16, 8, bitmap_font_png_data, bitmap_font_png_size);
	models[CUBE_MODEL] = _make_model_from_data(model_cube_dat_data, texture_cube_png_data, texture_cube_png_size, "cube");
	models[FROG_MODEL] = _make_model_from_data(model_frog_dat_data, texture_frog_png_data, texture_frog_png_size, "frog");
	models[KID_MODEL] = _make_model_from_data(model_kid_dat_data, texture_kid_png_data, texture_kid_png_size, "kid");
	models[TREX_MODEL] = _make_model_from_data(model_trex_dat_data, texture_trex_png_data, texture_trex_png_size, "trex");
	models[ROBOT_MODEL] = _make_model_from_data(model_robot_dat_data, texture_robot_png_data, texture_robot_png_size, "robot");
	if (_stats_label) {
		_stats_label->set("custom_fonts/font", screen_font);
	}
}

void Benchmark::_finalize() {
	// remove all objects
	for (auto &instance : instances) {
		if (instance.first.is_valid()) {
			VS::get_singleton()->free(instance.first);
		}
	}
	instances.clear();
}

void Benchmark::_check_instances() {
	// check and update instances
	while (num_objects > instances.size()) {
		instances.push_back({ _create_instance_from_model(models[curr_model]), curr_model });
	}
	int objects = 0;
	for (auto &instance : instances) {
		if (instance.second != curr_model && objects < num_objects) {
			VS::get_singleton()->instance_set_base(instance.first, models[curr_model].mesh->get_rid());
			VS::get_singleton()->instance_set_surface_material(instance.first, 0, model_mat->get_rid());
			instance.second = curr_model;
		}
		VS::get_singleton()->instance_set_visible(instance.first, objects < num_objects);
		objects++;
	}
}

void Benchmark::_update(float p_delta) {
	ERR_FAIL_COND(num_objects > instances.size());

	// calculate how many objects to draw per row
	int row_size = 1;
	while (row_size * row_size < num_objects) {
		row_size++;
	}
	const real_t row_scale = 25;
	const real_t row_step = num_objects == 1 ? row_scale : row_scale / (row_size - 1);
	// draw the objects in a grid layout
	int num_objects_drawn = 0;
	for (int x = 0; x < row_size && num_objects_drawn < num_objects; x++) {
		const float obj_x = (num_objects == 1) ? 0 : (1.5 * (-row_scale / 2.0 + x * row_step));

		for (int y = 0; y < row_size && num_objects_drawn < num_objects; y++) {
			const float obj_xs = obj_x + (y % 2) * row_step / 2.0;
			const float obj_y = (num_objects == 1) ? 0 : (-row_scale / 2.0 + y * row_step);

			// construct a yaw for this object, by combining the global yaw with the grid position
			// calculate the model matrix from the yaw
			const float yaw = glob_yaw + x * 40 + y * 50;
			Transform xform = Transform(Basis(Vector3(0, 1, 0), yaw), Vector3(obj_xs, obj_y, 0));
			// update
			VisualServer::get_singleton()->instance_set_transform(instances[num_objects_drawn].first, xform);
			num_objects_drawn++;
		}
	}
}

void Benchmark::_input(const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND(p_event.is_null());

	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	if (!get_tree()) {
		return;
	}

	ERR_FAIL_COND(!is_inside_tree());

	if (const InputEventKey *kb = cast_to<InputEventKey>(*p_event)) {
		if ((kb->get_scancode() == KEY_PLUS || kb->get_scancode() == KEY_EQUAL) and kb->is_pressed()) {
			set_num_objects(num_objects + 1);
			return;
		}
		if (kb->get_scancode() == KEY_MINUS and kb->is_pressed() && num_objects > 0) {
			set_num_objects(num_objects - 1);
			return;
		}
		if (kb->get_scancode() == KEY_S and kb->is_pressed()) {
			set_option(OPTION_STATS, !opts[OPTION_STATS]);
			return;
		}
		if (kb->get_scancode() == KEY_X and kb->is_pressed()) {
			if (curr_model + 1 == NUM_MODEL_TYPES) {
				set_display_model(FIRST_MODEL_TYPE);
			} else {
				set_display_model(curr_model + 1);
			}
			return;
		}
	}
}

void Benchmark::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_READY: {
			_load_resources();
		} break;
		case NOTIFICATION_ENTER_TREE: {
			if (!_stats_label) {
				_stats_label = memnew(Label);
				_stats_label->set_vertical_spacing(-3);
				add_child(_stats_label);
			}
			for (const Performance::Monitor &mon : helper::vector(Performance::TIME_FPS, Performance::MEMORY_STATIC)) {
				if (!_monitors[mon]) {
					_monitors[mon] = memnew(GdHistoryPlot);
					_monitors[mon]->set_title_label(Performance::get_singleton()->get_monitor_name(mon));
					add_child(_monitors[mon]);
				}
			}
			_monitors[Performance::MEMORY_STATIC]->set_humanize_value(true);
			if (!Engine::get_singleton()->is_editor_hint()) {
				set_process_input(true);
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_finalize();
			_stats_label = nullptr;
			_monitors[Performance::TIME_FPS] = nullptr;
			_monitors.clear();
			set_process_input(false);
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			static float _plot_update = 0;
			const float delta = get_process_delta_time();
			if (_dirty) {
				_check_instances();
				//model_mat->set_flag(SpatialMaterial::FLAG_UNSHADED, flag_unshaded);
				model_mat->set_texture(SpatialMaterial::TEXTURE_ALBEDO, models[curr_model].tex);
				if (_stats_label) {
					_stats_label->set_visible(opts[OPTION_STATS]);
				}
				_dirty = false;
			}
			glob_yaw += delta;
			_update(delta);
			if (_stats_label) {
				_stats_label->set_text(_get_stats());
			}
			const Size2 stats_size = _stats_text_size * Size2(16, 16);
			const int _monitor_height = 50, _monitor_space = 4;
			const Size2 _monitor_size = Size2(
					(Engine::get_singleton()->is_editor_hint() ? real_t(ProjectSettings::get_singleton()->get("display/window/size/width")) : get_viewport()->get_size().width) - stats_size.width - 2, _monitor_height);
			_plot_update += delta;
			if (_monitors[Performance::TIME_FPS]) {
				static float _accum = 0;
				static int _accum_count = 0;
				_monitors[Performance::TIME_FPS]->set_size(_monitor_size);
				_monitors[Performance::TIME_FPS]->set_position(Point2(stats_size.width, _monitor_space));
				_accum += Performance::get_singleton()->get_monitor(Performance::TIME_FPS);
				_accum_count++;
				if (_plot_update > 0.5) {
					_monitors[Performance::TIME_FPS]->add_sample(_accum / _accum_count);
					_accum = _accum_count = 0;
				}
			}
			if (_monitors[Performance::MEMORY_STATIC]) {
				static float _accum = 0;
				static int _accum_count = 0;
				_monitors[Performance::MEMORY_STATIC]->set_size(_monitor_size);
				_monitors[Performance::MEMORY_STATIC]->set_position(Point2(stats_size.width, _monitor_height + 2 * _monitor_space));
				_accum += Performance::get_singleton()->get_monitor(Performance::MEMORY_STATIC);
				_accum_count++;
				if (_plot_update > 0.5) {
					_monitors[Performance::MEMORY_STATIC]->add_sample(_accum / _accum_count);
					_accum = _accum_count = 0;
				}
			}
			if (_plot_update > 0.5) {
				_plot_update = 0;
			}
		} break;
	}
}

void Benchmark::set_active(bool p_state) {
	set_process_internal(p_state);
}

bool Benchmark::is_active() const {
	return is_processing_internal();
}

void Benchmark::set_num_objects(int p_num) {
	ERR_FAIL_COND(p_num < 1);
	num_objects = p_num;
	_dirty = true;
}

int Benchmark::get_num_objects() const {
	return num_objects;
}

void Benchmark::set_display_model(int p_model) {
	ERR_FAIL_INDEX(p_model, NUM_MODEL_TYPES);
	curr_model = p_model;
	_dirty = true;
}

int Benchmark::get_display_model() const {
	return curr_model;
}

void Benchmark::set_option(int p_opt, bool p_state) {
	ERR_FAIL_INDEX(p_opt, NUM_OPTIONS);
	if (opts[p_opt] != p_state) {
		opts[p_opt] = p_state;
		_dirty = true;
	}
}

bool Benchmark::get_option(int p_opt) const {
	ERR_FAIL_INDEX_V(p_opt, NUM_OPTIONS, false);
	return opts[p_opt];
}

void Benchmark::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_active"), &Benchmark::set_active);
	ClassDB::bind_method(D_METHOD("is_active"), &Benchmark::is_active);
	ClassDB::bind_method(D_METHOD("set_num_objects"), &Benchmark::set_num_objects);
	ClassDB::bind_method(D_METHOD("get_num_objects"), &Benchmark::get_num_objects);
	ClassDB::bind_method(D_METHOD("set_display_model"), &Benchmark::set_display_model);
	ClassDB::bind_method(D_METHOD("get_display_model"), &Benchmark::get_display_model);
	ClassDB::bind_method(D_METHOD("get_option"), &Benchmark::get_option);
	ClassDB::bind_method(D_METHOD("set_option"), &Benchmark::set_option);

	ClassDB::bind_method(D_METHOD("_input"), &Benchmark::_input);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "is_active");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "num_objects"), "set_num_objects", "get_num_objects");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "current_model", PROPERTY_HINT_ENUM, "Cube,Frog,Kid,TRex,Robot"), "set_display_model", "get_display_model");
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "option_unshaded"), "set_option", "get_option", OPTION_UNSHADED);
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "option_depthtest"), "set_option", "get_option", OPTION_DEPTHTEST);
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "option_culling"), "set_option", "get_option", OPTION_CULLING);
	ADD_PROPERTYI(PropertyInfo(Variant::BOOL, "option_stats"), "set_option", "get_option", OPTION_STATS);

	BIND_ENUM_CONSTANT(CUBE_MODEL);
	BIND_ENUM_CONSTANT(FROG_MODEL);
	BIND_ENUM_CONSTANT(KID_MODEL);
	BIND_ENUM_CONSTANT(TREX_MODEL);
	BIND_ENUM_CONSTANT(ROBOT_MODEL);
}

Benchmark::Benchmark() {
	_dirty = true;
	_stats_label = nullptr;
	_monitors[Performance::TIME_FPS] = nullptr;
	_stats_text_size = Size2(0, 0);
	plot_hist_size = 100;
	plot_buf_index = 0;
	num_objects = 20;
	curr_model = 0;
	glob_yaw = 0;

	camera_distance = 0, camera_yaw = 0;

	opts[OPTION_STATS] = true;

	model_mat = newref(SpatialMaterial);
	model_mat->set_cull_mode(SpatialMaterial::CULL_DISABLED);

	set_process_internal(true);
}

Benchmark::~Benchmark() {
	_finalize(); // should not be needed
}
