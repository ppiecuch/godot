/**************************************************************************/
/*  gd_waterfall.cpp                                                      */
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

#include "gd_waterfall.h"

#include "common/gd_core.h"
#include "common/gd_pack.h"
#include "core/bind/core_bind.h"
#include "core/variant.h"
#include "scene/resources/texture.h"

#include "particles.h"

#include <vector>

enum WaterfallLayers {
	LAYER0_DROPS = 0,
	LAYER1_SMALL_CLOUDS = 1,
	LAYER2_CLOUDS = 2,
	LAYER3_BOTTOM = 3,
	LAYERS_COUNT,
};

#ifdef TOOLS_ENABLED
bool GdWaterfall::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	return view_rect.has_point(p_point);
}

Rect2 GdWaterfall::_edit_get_rect() const {
	return view_rect;
}

bool GdWaterfall::_edit_use_rect() const {
	return true;
}
#endif

static const real_t _density_factor[] = { 0.25, 0.5, 0.75, 1 };
static const real_t _stretch_amount_factor[] = { 0, 0.1, 0.15, 0.2 };
static const real_t _stretch_from_factor[] = { 0, 0.25, 0.5, 0.75 };
static const real_t _accel_factor[] = { 0, 0.05, 0.1, 0.15, 0.2, 0.25 };
static const real_t _fade_amount_factor[] = { 0, 0.1, 0.25, 0.5 };
static const real_t _fade_from_factor[] = { 0, 0.5, 0.75, 0.9 };
static const real_t _splash_spread_factor[] = { 1, 1.5, 2 };
static const real_t _splash_height_factor[] = { 0.1, 0.2, 0.3, 0.4 };

// https://blog.demofox.org/2017/05/29/when-random-numbers-are-too-random-low-discrepancy-sequences/
static std::vector<Point2> _get_samples(int num, size_t basex, size_t basey, const Rect2 &view) {
	// calculate the sample points
	std::vector<Point2> samples;
	samples.resize(num);
	for (size_t i = 0; i < num; ++i) {
		// x axis
		samples[i].x = 0;
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
		// y axis
		samples[i].y = 0;
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
	return samples;
}

#define _default_alpha_0 Math::random(0.25, 0.6)
#define _default_alpha_1 Math::random(0.1, 0.25)
#define _default_alpha_2 Math::random(0.1, 0.25)

#define _default_stretch_0 Math::random(1.5, 3.0)

#define _default_velocity_0 Math::random(0.5, 1.0)
#define _default_velocity_1 (_default_velocity_0 * 0.15)
#define _default_velocity_2 (_default_velocity_0 * 0.1)

void GdWaterfall::_build_particles() {
	_p.clear();
	_particles.clear();
	_check_textures_cache();
	print_verbose("Rebuilding waterfall:");
	{ // drops on layer 0
		const real_t dim = particle_radius;
		const int num = view_rect.size.width / dim * view_rect.size.height / dim * _density_factor[waterfall_density];
		print_verbose("  layer 0: " + String::num(num) + " particles.");
		_particles.reserve(num * 3);
		auto points = _get_samples(num, 2, 3, view_rect.grow_individual(-dim, 0, -dim, 0));
		for (const Point2 &p : points) {
			const Vector2 pos(p.x, p.y);
			const Vector2 velocity(0, _default_velocity_0);
			const Vector2 stretch(1, _default_stretch_0);
			const real_t damping = 1;
			const real_t alpha = _default_alpha_0;
			const Ref<Texture> texture = textures_quality == NO_TEXTURES ? nullptr : _cache[textures_quality][Math::random(0, 4)];
			flex_particle_options opts(LAYER0_DROPS, pos, velocity, dim, damping, alpha, stretch, texture);
			_particles.push_back(flex_particle(opts));
		}
	}
	{ // stretch clouds on layer 1
		const real_t dim = particle_radius * 1.5;
		const int num = view_rect.size.width / dim * view_rect.size.height / dim;
		print_verbose("  layer 1: " + String::num(num) + " particles.");
		auto points = _get_samples(num, 2, 3, view_rect.grow_individual(-dim / 2, 0, -dim / 2, 0));
		for (const Point2 &p : points) {
			const Vector2 pos(p.x, p.y);
			const Vector2 velocity(0, _default_velocity_1);
			const real_t damping = 1;
			const real_t alpha = _default_alpha_1;
			const Ref<Texture> texture = textures_quality == NO_TEXTURES ? nullptr : _cache[textures_quality][WATERFALL_PARTICLE_CLOUD1];
			flex_particle_options opts(LAYER1_SMALL_CLOUDS, pos, velocity, dim, damping, alpha, texture);
			_particles.push_back(flex_particle(opts));
		}
	}
	{ // clouds on layer 2
		const real_t dim = particle_radius * 2;
		const int num = view_rect.size.width / dim * view_rect.size.height / dim;
		print_verbose("  layer 2: " + String::num(num) + " particles.");
		auto points = _get_samples(num, 2, 3, view_rect.grow_individual(-dim / 2, 0, -dim / 2, 0));
		for (const Point2 &p : points) {
			const Vector2 pos(p.x, p.y);
			const Vector2 velocity(0, _default_velocity_2);
			const real_t damping = 1;
			const real_t alpha = _default_alpha_2;
			const Ref<Texture> texture = textures_quality == NO_TEXTURES ? nullptr : _cache[textures_quality][WATERFALL_PARTICLE_CLOUD2];
			flex_particle_options opts(LAYER2_CLOUDS, pos, velocity, dim, damping, alpha, texture);
			_particles.push_back(flex_particle(opts));
		}
	}
	{ // TODO: bottom splash on layer 3
		const real_t dim = particle_radius * 2;
		const int num = _splash_spread_factor[waterfall_splash_spread] * view_rect.size.width / dim * _splash_height_factor[waterfall_splash_height] * view_rect.size.height / dim;
		print_verbose("  layer 3: " + String::num(num) + " particles.");
		auto points = _get_samples(num, 2, 3, view_rect.grow_individual(-dim / 2, 0, -dim / 2, 0));
		for (const Point2 &p : points) {
			const Vector2 pos(p.x, p.y);
			const Vector2 velocity(0, _default_velocity_2);
			const real_t damping = -1;
			const real_t alpha = _default_alpha_2;
			const Ref<Texture> texture = textures_quality == NO_TEXTURES ? nullptr : _cache[textures_quality][WATERFALL_PARTICLE_CLOUD2];
			flex_particle_options opts(LAYER3_BOTTOM, pos, velocity, dim, damping, alpha, texture);
			_particles.push_back(flex_particle(opts));
		}
	}
	for (int p = 0; p < _particles.size(); p++) {
		_p.add_particle(&_particles[p]);
	}
}

void GdWaterfall::_cache_textures(const unsigned char *particles_images[], WaterfallTexturesQuality quality) {
	String fn = vformat("user://__waterfall_q%d", quality); // cached texture
	if (ResourceLoader::exists(fn + ".tex") && FileAccess::exists(fn + ".dat")) {
		Ref<Texture> texture = ResourceLoader::load(fn + ".tex", "Texture");
	}
	// build texture atlas from resources
	Vector<Ref<Image>> images;
	Vector<String> names;
	for (int p = 0; p < WATERFALL_PARTICLE_COUNT; p++) {
		Ref<Image> image = memnew(Image(particles_images[p]));
		images.push_back(image);
		names.push_back(EnumString<WaterfallParticles>::From(WaterfallParticles(p)).c_str());
	}
	Dictionary atlas_info = merge_images(images);

	ERR_FAIL_COND(atlas_info.empty());

	Array pages = atlas_info["_generated_images"];
	if (pages.size() > 1) {
		WARN_PRINT("Too many texture pages - using only first one for Waterfall.");
	}
	Ref<Image> atlas_image = pages[0];

	print_verbose(vformat("Atlas generated: %s pixels", atlas_image->get_size()));

	Vector2 atlas_size(1, 1);
	if (atlas_image.is_valid()) {
		Ref<ImageTexture> texture = newref(ImageTexture);
		texture->create_from_image(atlas_image);
		atlas_size = texture->get_size();

		_cache[quality].clear();

		Array _rects;
		Array atlas_rects = atlas_info["_rects"];
		for (int j = 0; j < atlas_rects.size(); ++j) {
			Dictionary entry = atlas_rects[j];

			ERR_CONTINUE_MSG(entry.empty(), "Empty atlas entry, Skipping!");

			Ref<AtlasTexture> at = newref(AtlasTexture);
			at->set_atlas(texture);
			at->set_region(entry["rect"]);
#ifdef DEBUG_ENABLED
			at->set_name(names[j]);
#endif
			_rects.push_back(entry["rect"]);
			_cache[quality].push_back(at);
		}
		// save texture and atlas info
		ResourceSaver::save(fn + ".tex", texture);
		Ref<_File> file = memnew(_File);
		if (file->open(fn + ".dat", _File::WRITE) == OK) {
			file->store_var(_rects, true);
		}
	} else {
		WARN_PRINT("Atlas image is not valid, Skipping!");
	}
}

void GdWaterfall::_check_textures_cache() {
	if (textures_quality != NO_TEXTURES) {
		switch (textures_quality) {
			case TEXTURES_LOW: {
				ERR_FAIL_COND_MSG(!_available_particles[WATERFALL_PARTICLE_SIZE_S], "Low quality size is not available");
				if (_cache[TEXTURES_LOW].empty()) {
					_cache_textures(particles_size_s, TEXTURES_LOW);
				}
				ERR_FAIL_COND_MSG(_cache[TEXTURES_LOW].empty(), "Missing texture cache");
			}; break;
			case TEXTURES_MEDIUM: {
				ERR_FAIL_COND_MSG(!_available_particles[WATERFALL_PARTICLE_SIZE_M], "Medium quality size is not available");
				if (_cache[TEXTURES_MEDIUM].empty()) {
					_cache_textures(particles_size_m, TEXTURES_MEDIUM);
				}
				ERR_FAIL_COND_MSG(_cache[TEXTURES_MEDIUM].empty(), "Missing texture cache");
			}; break;
			case TEXTURES_HIGH: {
				ERR_FAIL_COND_MSG(!_available_particles[WATERFALL_PARTICLE_SIZE_L], "High quality size is not available");
				if (_cache[TEXTURES_HIGH].empty()) {
					_cache_textures(particles_size_l, TEXTURES_HIGH);
				}
				ERR_FAIL_COND_MSG(_cache[TEXTURES_HIGH].empty(), "Missing texture cache");
			}; break;
			default: {
				WARN_PRINT("Unknown texture quality level");
			}
		};
	}
}

void GdWaterfall::_update_particles() {
	_p.setup_world(view_rect.size);
	_p.set_alpha_change(LAYER0_DROPS, { 1, _fade_from_factor[particle_fade_from] }, _fade_amount_factor[particle_fade_amount]);
	_p.set_scale_change(LAYER0_DROPS, { 1, _stretch_from_factor[particle_stretch_from] }, { 1, _stretch_amount_factor[particle_fade_amount] });
}

void GdWaterfall::_reload_textures() {
	if (textures_quality == NO_TEXTURES) {
		for (auto &p : _particles) {
			p.texture.set(Ref<Texture>());
		}
	} else {
		_check_textures_cache();
		for (auto &p : _particles) {
			switch (p.layer) {
				case 0:
					p.texture.set(_cache[textures_quality][Math::random(0, 4)]);
					break; // drops
				case 1:
					p.texture.set(_cache[textures_quality][WATERFALL_PARTICLE_CLOUD1]);
					break; // stretch clouds
				case 2:
				case 3:
					p.texture.set(_cache[textures_quality][WATERFALL_PARTICLE_CLOUD2]);
					break; // clouds
			}
		}
	}
}

void GdWaterfall::set_active(bool p_state) {
	active = p_state;
	set_process(active);
}

bool GdWaterfall::is_active() const {
	return active;
}

void GdWaterfall::set_view_rect(const Rect2 &p_rect) {
	view_rect = p_rect;
	_rebuild = true;
	update();
}

Rect2 GdWaterfall::get_view_rect() const {
	return view_rect;
}

void GdWaterfall::set_canvas_clipping(bool p_state) {
	clip_view = p_state;
	update();
}

bool GdWaterfall::is_canvas_clipping() const {
	return clip_view;
}

void GdWaterfall::set_waterfall_density(int p_density) {
	ERR_FAIL_INDEX(p_density, sizeof(_density_factor) / sizeof(real_t));
	waterfall_density = p_density;
	_rebuild = true;
	update();
}

int GdWaterfall::get_waterfall_density() const {
	return waterfall_density;
}

void GdWaterfall::set_waterfall_splash_spread(int p_splash_spread) {
	waterfall_splash_spread = p_splash_spread;
	_update = true;
	update();
}

int GdWaterfall::get_waterfall_splash_spread() const {
	return waterfall_splash_spread;
}

void GdWaterfall::set_waterfall_splash_height(int p_splash_height) {
	waterfall_splash_height = p_splash_height;
	_update = true;
	update();
}

int GdWaterfall::get_waterfall_splash_height() const {
	return waterfall_splash_height;
}

void GdWaterfall::set_waterfall_speed(real_t p_speed) {
	waterfall_speed = p_speed;
}

real_t GdWaterfall::get_waterfall_speed() const {
	return waterfall_speed;
}

void GdWaterfall::set_particle_radius(int p_radius) {
	ERR_FAIL_COND(p_radius < 2);
	particle_radius = p_radius;
	_rebuild = true;
	update();
}

int GdWaterfall::get_particle_radius() const {
	return particle_radius;
}

void GdWaterfall::set_particle_acceleration(int p_accel) {
	ERR_FAIL_INDEX(p_accel, sizeof(_accel_factor) / sizeof(real_t));
	particle_accel = p_accel;
}

int GdWaterfall::get_particle_acceleration() const {
	return particle_accel;
}

void GdWaterfall::set_particle_stretch_from(int p_stretch_from) {
	ERR_FAIL_INDEX(p_stretch_from, sizeof(_stretch_from_factor) / sizeof(real_t));
	particle_stretch_from = p_stretch_from;
	_update = true;
	update();
}

int GdWaterfall::get_particle_stretch_from() const {
	return particle_stretch_from;
}

void GdWaterfall::set_particle_stretch_amount(int p_stretch_amount) {
	ERR_FAIL_INDEX(p_stretch_amount, sizeof(_stretch_amount_factor) / sizeof(real_t));
	particle_stretch_amount = p_stretch_amount;
	_update = true;
	update();
}

int GdWaterfall::get_particle_stretch_amount() const {
	return particle_stretch_amount;
}

void GdWaterfall::set_particle_fade_from(int p_fade_from) {
	ERR_FAIL_INDEX(p_fade_from, sizeof(_fade_from_factor) / sizeof(real_t));
	particle_fade_from = p_fade_from;
	_update = true;
	update();
}

int GdWaterfall::get_particle_fade_from() const {
	return particle_fade_from;
}

void GdWaterfall::set_particle_fade_amount(int p_fade_amount) {
	ERR_FAIL_INDEX(p_fade_amount, sizeof(_fade_amount_factor) / sizeof(real_t));
	particle_fade_amount = p_fade_amount;
	_update = true;
	update();
}

int GdWaterfall::get_particle_fade_amount() const {
	return particle_fade_amount;
}

void GdWaterfall::set_textures_quality(int p_quality) {
	if (textures_quality != p_quality) {
		ERR_FAIL_INDEX(p_quality, TEXTURES_QUALITY_NUM);
		textures_quality = (WaterfallTexturesQuality)p_quality;
		_reload = true;
		update();
	}
}

int GdWaterfall::get_textures_quality() const {
	return textures_quality;
}

#ifdef TOOLS_ENABLED
void GdWaterfall::set_layers_preview(int p_layers) {
	if (layers_preview != p_layers) {
		ERR_FAIL_INDEX(p_layers, 6);
		switch (p_layers) {
			case 0: {
				for (int l = 0; l < LAYERS_COUNT; l++) {
					_p.get_layer_conf(l).visible = true;
				}
			} break;
			default: {
				for (int l = 0; l < LAYERS_COUNT; l++) {
					_p.get_layer_conf(l).visible = (l == p_layers - 1);
				}
			} break;
		};
		layers_preview = p_layers;
		update();
	}
}

int GdWaterfall::get_layers_preview() const {
	return layers_preview;
}

void GdWaterfall::set_particle_texture_debug(bool p_debug) {
	textures_debug = p_debug;
	update();
}

bool GdWaterfall::get_particle_texture_debug() const {
	return textures_debug;
}
#endif

void GdWaterfall::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
		} break;
		case NOTIFICATION_DRAW: {
			if (!get_tree()) {
				return;
			}

			ERR_FAIL_COND(!is_visible_in_tree());

			if (_rebuild) {
				_build_particles();
				_rebuild = false;
				_reload = false;
			}
			if (_reload) {
				_reload_textures();
				_reload = false;
			}
			if (_update) {
				_update_particles();
				_update = false;
			}
			const Rect2 clip_rect = view_rect.grow_individual(particle_radius * 2, 0, particle_radius * 2, 0);
			VisualServer::get_singleton()->canvas_item_set_custom_rect(get_canvas_item(), true, clip_rect);
			VisualServer::get_singleton()->canvas_item_set_clip(get_canvas_item(), clip_view);
#ifdef TOOLS_ENABLED
			_p.draw(view_rect.position, textures_debug);
#else
			_p.draw(view_rect.position);
#endif
		} break;
		case NOTIFICATION_PROCESS: {
			_p.update(Vector2(0, waterfall_speed), Vector2(0, _accel_factor[particle_accel]));
			update();
		} break;
	}
}

void GdWaterfall::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_active"), &GdWaterfall::set_active);
	ClassDB::bind_method(D_METHOD("is_active"), &GdWaterfall::is_active);
	ClassDB::bind_method(D_METHOD("set_view_rect"), &GdWaterfall::set_view_rect);
	ClassDB::bind_method(D_METHOD("get_view_rect"), &GdWaterfall::get_view_rect);
	ClassDB::bind_method(D_METHOD("set_canvas_clipping"), &GdWaterfall::set_canvas_clipping);
	ClassDB::bind_method(D_METHOD("is_canvas_clipping"), &GdWaterfall::is_canvas_clipping);
	ClassDB::bind_method(D_METHOD("set_waterfall_density"), &GdWaterfall::set_waterfall_density);
	ClassDB::bind_method(D_METHOD("get_waterfall_density"), &GdWaterfall::get_waterfall_density);
	ClassDB::bind_method(D_METHOD("set_waterfall_speed"), &GdWaterfall::set_waterfall_speed);
	ClassDB::bind_method(D_METHOD("get_waterfall_speed"), &GdWaterfall::get_waterfall_speed);
	ClassDB::bind_method(D_METHOD("set_waterfall_splash_spread"), &GdWaterfall::set_waterfall_splash_spread);
	ClassDB::bind_method(D_METHOD("get_waterfall_splash_spread"), &GdWaterfall::get_waterfall_splash_spread);
	ClassDB::bind_method(D_METHOD("set_waterfall_splash_height"), &GdWaterfall::set_waterfall_splash_height);
	ClassDB::bind_method(D_METHOD("get_waterfall_splash_height"), &GdWaterfall::get_waterfall_splash_height);
	ClassDB::bind_method(D_METHOD("set_particle_radius"), &GdWaterfall::set_particle_radius);
	ClassDB::bind_method(D_METHOD("get_particle_radius"), &GdWaterfall::get_particle_radius);
	ClassDB::bind_method(D_METHOD("set_particle_stretch_from"), &GdWaterfall::set_particle_stretch_from);
	ClassDB::bind_method(D_METHOD("get_particle_stretch_from"), &GdWaterfall::get_particle_stretch_from);
	ClassDB::bind_method(D_METHOD("set_particle_stretch_amount"), &GdWaterfall::set_particle_stretch_amount);
	ClassDB::bind_method(D_METHOD("get_particle_stretch_amount"), &GdWaterfall::get_particle_stretch_amount);
	ClassDB::bind_method(D_METHOD("set_particle_acceleration"), &GdWaterfall::set_particle_acceleration);
	ClassDB::bind_method(D_METHOD("get_particle_acceleration"), &GdWaterfall::get_particle_acceleration);
	ClassDB::bind_method(D_METHOD("set_particle_fade_from"), &GdWaterfall::set_particle_fade_from);
	ClassDB::bind_method(D_METHOD("get_particle_fade_from"), &GdWaterfall::get_particle_fade_from);
	ClassDB::bind_method(D_METHOD("set_particle_fade_amount"), &GdWaterfall::set_particle_fade_amount);
	ClassDB::bind_method(D_METHOD("get_particle_fade_amount"), &GdWaterfall::get_particle_fade_amount);
	ClassDB::bind_method(D_METHOD("set_textures_quality"), &GdWaterfall::set_textures_quality);
	ClassDB::bind_method(D_METHOD("get_textures_quality"), &GdWaterfall::get_textures_quality);
#ifdef TOOLS_ENABLED
	ClassDB::bind_method(D_METHOD("set_layers_preview"), &GdWaterfall::set_layers_preview);
	ClassDB::bind_method(D_METHOD("get_layers_preview"), &GdWaterfall::get_layers_preview);
	ClassDB::bind_method(D_METHOD("set_particle_texture_debug"), &GdWaterfall::set_particle_texture_debug);
	ClassDB::bind_method(D_METHOD("get_particle_texture_debug"), &GdWaterfall::get_particle_texture_debug);
#endif

	BIND_ENUM_CONSTANT(NO_TEXTURES);
	BIND_ENUM_CONSTANT(TEXTURES_LOW);
	BIND_ENUM_CONSTANT(TEXTURES_MEDIUM);
	BIND_ENUM_CONSTANT(TEXTURES_HIGH);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "is_active");
	ADD_PROPERTY(PropertyInfo(Variant::RECT2, "view_rect"), "set_view_rect", "get_view_rect");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_canvas_clip"), "set_canvas_clipping", "is_canvas_clipping");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "waterfall_density", PROPERTY_HINT_ENUM, "25%,50%,75%,100%"), "set_waterfall_density", "get_waterfall_density");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "waterfall_speed"), "set_waterfall_speed", "get_waterfall_speed");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "waterfall_splash_spread", PROPERTY_HINT_ENUM, "100%,150%,200%"), "set_waterfall_splash_spread", "get_waterfall_splash_spread");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "waterfall_splash_height", PROPERTY_HINT_ENUM, "10%,20%,30%,40%"), "set_waterfall_splash_height", "get_waterfall_splash_height");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "particle_radius", PROPERTY_HINT_RANGE, "2,100,1"), "set_particle_radius", "get_particle_radius");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "particle_stretch_from", PROPERTY_HINT_ENUM, "None,25%,50%,75%"), "set_particle_stretch_from", "get_particle_stretch_from");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "particle_stretch_amount", PROPERTY_HINT_ENUM, "None,10%,15%,20%"), "set_particle_stretch_amount", "get_particle_stretch_amount");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "particle_acceleration", PROPERTY_HINT_ENUM, "None,5%,10%,15%,20%,25%"), "set_particle_acceleration", "get_particle_acceleration");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "particle_fade_from", PROPERTY_HINT_ENUM, "None,50%,75%,90%"), "set_particle_fade_from", "get_particle_fade_from");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "particle_fade_amount", PROPERTY_HINT_ENUM, "None,10%,25%,50%"), "set_particle_fade_amount", "get_particle_fade_amount");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "textures_quality", PROPERTY_HINT_ENUM, "None,Low,Medium,High"), "set_textures_quality", "get_textures_quality");
#ifdef TOOLS_ENABLED
	ADD_PROPERTY(PropertyInfo(Variant::INT, "layers_preview", PROPERTY_HINT_ENUM, "All,Drops,Stretch Clouds,Clouds,Bottom", PROPERTY_USAGE_EDITOR), "set_layers_preview", "get_layers_preview");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "textures_debug", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "set_particle_texture_debug", "get_particle_texture_debug");
#endif
}

GdWaterfall::GdWaterfall() :
		_p(this) {
	active = true;
	particle_radius = 4;
	particle_stretch_from = 3;
	particle_stretch_amount = 1;
	particle_fade_from = 2;
	particle_fade_amount = 3;
	particle_accel = 1;
	waterfall_splash_spread = 0;
	waterfall_splash_height = 1;
	waterfall_speed = 0.05;
	waterfall_density = 1;
	textures_quality = NO_TEXTURES;
	view_rect = Rect2(0, 0, 50, 150);
#ifdef TOOLS_ENABLED
	layers_preview = 0;
	textures_debug = false;
#endif

	_p.set_option(flex_particle_system::VERTICAL_WRAP, true);
	_p.set_wall_callback([](flex_particle *p) {
		// restore initial values
		if (p->layer == LAYER0_DROPS) {
			p->velocity = Vector2(0, _default_velocity_0);
		} else if (p->layer == LAYER1_SMALL_CLOUDS) {
			p->velocity = Vector2(0, _default_velocity_1);
		} else if (p->layer == LAYER2_CLOUDS) {
			p->velocity = Vector2(0, _default_velocity_2);
		}
	},
			flex_particle_system::BOTTOM_WALL);
	_p.set_layer_option(LAYER0_DROPS, flex_particle_system::PROGRESS_ALPHA, true);
	_p.set_layer_option(LAYER0_DROPS, flex_particle_system::PROGRESS_SCALE, true);
	set_process(active);

	_rebuild = true;
	_reload = false;
	_update = true;
}
