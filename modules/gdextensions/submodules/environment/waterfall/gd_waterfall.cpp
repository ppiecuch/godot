/*************************************************************************/
/*  gd_waterfall.cpp                                                     */
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

#include "gd_waterfall.h"

#include "scene/resources/texture.h"
#include "common/gd_pack.h"
#include "common/gd_core.h"

#include <vector>

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

static const real_t _density_factor[] = { 0.5, 0.75, 1, 1.5, 1.75, 2 };
static const real_t _stretch_factor[] = { 0, 0.1, 0.15, 0.2 };
static const real_t _accel_factor[] = { 0, 0.05, 0.1, 0.15, 0.2, 0.25 };
static const real_t _fade_amount_factor[] = { 0, 0.1, 0.25, 0.5 };
static const real_t _fade_from_factor[] = { 0, 0.5, 0.75, 0.9 };

// https://blog.demofox.org/2017/05/29/when-random-numbers-are-too-random-low-discrepancy-sequences/
std::vector<Point2> _get_samples(int num, size_t basex, size_t basey, Rect2 view, real_t pad) {
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
				size_t multiplier = n % basex;
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
				size_t multiplier = n % basey;
				samples[i].y += multiplier / denominator;
				n = n / basey;
				denominator *= basey;
			}
		}
	}
	for (size_t i = 0; i < num; ++i) {
		samples[i].x = view.position.x + samples[i].x * view.size.width + pad;
		samples[i].y = view.position.y + samples[i].y * view.size.height + pad;
	}
	return samples;
}

#define _default_velocity Math::random(0.5, 1.0)

void GdWaterfall::_build_particles() {
	// make all particles
	_p.clear();
	_particles.clear();
	const real_t dim = 2 * particle_radius;
	const int num = (view_rect.size.width / dim + view_rect.size.height / dim) * _density_factor[waterfall_density];
	_particles.reserve(num);
	auto points = _get_samples(num, 2, 3, view_rect, particle_radius);
	for (const Point2 &p : points) {
		const Vector2 pos(p.x, p.y);
		const Vector2 velocity(0, _default_velocity);
		const Vector2 rotation(0, 0);
		const Vector2 rotation_velocity(0, 0);
		const real_t damping = 1;

		flex_particle_options opts(pos, velocity, rotation, rotation_velocity, particle_radius, damping);

		_particles.push_back(flex_particle(opts));
		// add particles
		_p.add_particle(&_particles.back());
	}
	print_verbose("Rebuiling waterfall with " + String::num(num) + " particles.");
}

void GdWaterfall::_cache_textures(const unsigned char *particles_images[], WaterfallParticlesQuality quality) {
	// build texture atlas from resources
	Vector<Ref<Image>> images;
	Vector<String> names;
	for (int p = 0; p < WATERFALL_PARTICLE_COUNT; p++) {
		Ref<Image> image = memnew(Image(particles_images[p]));
		images.push_back(image);
		names.push_back(EnumString<WaterfallParticles>::From(WaterfallParticles(p)).c_str());
	}
	Dictionary atlas_info = merge_images(images, names);

	ERR_FAIL_COND(atlas_info.empty());

	Array pages = atlas_info["_generated_images"];
	if (pages.size() > 1) {
		WARN_PRINT("Too many texture pages - using only first one for Waterfall.");
	}
	Ref<Image> atlas_image = pages[0];

	Vector2 atlas_size(1, 1);
	if (atlas_image.is_valid()) {
		Ref<ImageTexture> texture = newref(ImageTexture);
		texture->create_from_image(atlas_image);
		atlas_size = texture->get_size();

		_cache[quality].clear();

		Dictionary atlas_rects = atlas_info["_rects"];
		for (int j = 0; j < names.size(); ++j) {
			String name = names[j];
			Dictionary entry = atlas_rects[name];

			ERR_CONTINUE_MSG(entry.empty(), "Empty atlas entry, Skipping!");

			Ref<AtlasTexture> at = newref(AtlasTexture);
			at->set_atlas(texture);
			at->set_region(entry["rect"]);
	#ifdef DEBUG_ENABLED
			at->set_name(name);
	#endif
			_cache[quality].push_back(at);
		}
	} else {
		WARN_PRINT("Atlas image is not valid, Skipping!");
	}
}

void GdWaterfall::_update_particles() {
	_p.setup_world(view_rect.size);
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

void GdWaterfall::set_waterfall_density(int p_density) {
	ERR_FAIL_INDEX(p_density, sizeof(_density_factor) / sizeof(real_t));
	waterfall_density = p_density;
	_rebuild = true;
	update();
}

int GdWaterfall::get_waterfall_density() const {
	return waterfall_density;
}

void GdWaterfall::set_particle_radius(real_t p_radius) {
	particle_radius = p_radius;
	_rebuild = true;
	update();
}

real_t GdWaterfall::get_particle_radius() const {
	return particle_radius;
}

void GdWaterfall::set_waterfall_speed(real_t p_speed) {
	waterfall_speed = p_speed;
}

real_t GdWaterfall::get_waterfall_speed() const {
	return waterfall_speed;
}

void GdWaterfall::set_particle_acceleration(int p_accel) {
	ERR_FAIL_INDEX(p_accel, sizeof(_accel_factor) / sizeof(real_t));
	particle_accel = p_accel;
}

int GdWaterfall::get_particle_acceleration() const {
	return particle_accel;
}

void GdWaterfall::set_particle_stretch(int p_stretch) {
	ERR_FAIL_INDEX(p_stretch, sizeof(_stretch_factor) / sizeof(real_t));
	particle_stretch = p_stretch;
}

int GdWaterfall::get_particle_stretch() const {
	return particle_stretch;
}

void GdWaterfall::set_particle_fade_from(int p_fade_from) {
	ERR_FAIL_INDEX(p_fade_from, sizeof(_fade_from_factor) / sizeof(real_t));
	particle_fade_from = p_fade_from;
}

int GdWaterfall::get_particle_fade_from() const {
	return particle_fade_from;
}

void GdWaterfall::set_particle_fade_amount(int p_fade_amount) {
	ERR_FAIL_INDEX(p_fade_amount, sizeof(_fade_amount_factor) / sizeof(real_t));
	particle_fade_amount = p_fade_amount;
}

int GdWaterfall::get_particle_fade_amount() const {
	return particle_fade_amount;
}

void GdWaterfall::set_textures_quality(int p_quality) {
	ERR_FAIL_INDEX(p_quality, ParticlesQualityCount);
	switch (p_quality) {
		case None: {
			for (auto &p : _particles) {
				p.texture = Ref<Texture>();
			}
		}; break;
		case ParticlesMedium: {
			ERR_FAIL_COND_MSG(!_available_particles[WATERFALL_PARTICLE_SIZE_M], "Medium size is not available");
			if (_cache[ParticlesMedium].empty()) {
				_cache_textures(particles_size_m, ParticlesMedium);
			}
			ERR_FAIL_COND_MSG(_cache[ParticlesMedium].empty(), "Missing texture cache");
			for (auto &p : _particles) {
				p.texture = _cache[ParticlesMedium][Math::random(0, 3)]; // drops
			}
		}; break;
	};
	textures_quality = (WaterfallParticlesQuality)p_quality;
	update();
}

int GdWaterfall::get_textures_quality() const {
	return textures_quality;
}

void GdWaterfall::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_build_particles();
		} break;
		case NOTIFICATION_DRAW: {
			_p.draw(view_rect, view_rect.position);
		} break;
		case NOTIFICATION_PROCESS: {
			if (_rebuild) {
				_build_particles();
				_rebuild = false;
			}
			if (_update) {
				_update_particles();
				_update = false;
			}
			_p.update(Vector2(0, waterfall_speed), Vector2(0, _accel_factor[particle_accel]));
			update();
		}
	}
}

void GdWaterfall::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_active"), &GdWaterfall::set_active);
	ClassDB::bind_method(D_METHOD("is_active"), &GdWaterfall::is_active);
	ClassDB::bind_method(D_METHOD("set_view_rect"), &GdWaterfall::set_view_rect);
	ClassDB::bind_method(D_METHOD("get_view_rect"), &GdWaterfall::get_view_rect);
	ClassDB::bind_method(D_METHOD("set_waterfall_density"), &GdWaterfall::set_waterfall_density);
	ClassDB::bind_method(D_METHOD("get_waterfall_density"), &GdWaterfall::get_waterfall_density);
	ClassDB::bind_method(D_METHOD("set_waterfall_speed"), &GdWaterfall::set_waterfall_speed);
	ClassDB::bind_method(D_METHOD("get_waterfall_speed"), &GdWaterfall::get_waterfall_speed);
	ClassDB::bind_method(D_METHOD("set_particle_radius"), &GdWaterfall::set_particle_radius);
	ClassDB::bind_method(D_METHOD("get_particle_radius"), &GdWaterfall::get_particle_radius);
	ClassDB::bind_method(D_METHOD("set_particle_stretch"), &GdWaterfall::set_particle_stretch);
	ClassDB::bind_method(D_METHOD("get_particle_stretch"), &GdWaterfall::get_particle_stretch);
	ClassDB::bind_method(D_METHOD("set_particle_acceleration"), &GdWaterfall::set_particle_acceleration);
	ClassDB::bind_method(D_METHOD("get_particle_acceleration"), &GdWaterfall::get_particle_acceleration);
	ClassDB::bind_method(D_METHOD("set_particle_fade_from"), &GdWaterfall::set_particle_fade_from);
	ClassDB::bind_method(D_METHOD("get_particle_fade_from"), &GdWaterfall::get_particle_fade_from);
	ClassDB::bind_method(D_METHOD("set_particle_fade_amount"), &GdWaterfall::set_particle_fade_amount);
	ClassDB::bind_method(D_METHOD("get_particle_fade_amount"), &GdWaterfall::get_particle_fade_amount);
	ClassDB::bind_method(D_METHOD("set_textures_quality"), &GdWaterfall::set_textures_quality);
	ClassDB::bind_method(D_METHOD("get_textures_quality"), &GdWaterfall::get_textures_quality);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "is_active");
	ADD_PROPERTY(PropertyInfo(Variant::RECT2, "view_rect"), "set_view_rect", "get_view_rect");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "waterfall_density", PROPERTY_HINT_ENUM, "50%,75%,100%,150%,175%,200%"), "set_waterfall_density", "get_waterfall_density");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "waterfall_speed"), "set_waterfall_speed", "get_waterfall_speed");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "particle_radius"), "set_particle_radius", "get_particle_radius");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "particle_stretch", PROPERTY_HINT_ENUM, "None,10%,15%,20%"), "set_particle_stretch", "get_particle_stretch");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "particle_acceleration", PROPERTY_HINT_ENUM, "None,5%,10%,15%,20%,25%"), "set_particle_acceleration", "get_particle_acceleration");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "particle_fade_from", PROPERTY_HINT_ENUM, "None,50%,75%,90%"), "set_particle_fade_from", "get_particle_fade_from");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "particle_fade_amount", PROPERTY_HINT_ENUM, "None,10%,25%,50%"), "set_particle_fade_amount", "get_particle_fade_amount");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "textures_quality", PROPERTY_HINT_ENUM, "None,Low,Medium,High"), "set_textures_quality", "get_textures_quality");
}

GdWaterfall::GdWaterfall() :
		_p(this) {
	active = true;
	particle_radius = 4;
	particle_stretch = 1;
	particle_fade_from = 3;
	particle_fade_amount = 3;
	waterfall_speed = 0.05;
	particle_accel = 1;
	waterfall_density = 2;
	textures_quality = None;
	view_rect = Rect2(0, 0, 50, 150);

	_p.set_option(flex_particle_system::VERTICAL_WRAP, true);
    _p.set_wall_callback([](flex_particle *p) {
		p->velocity = Vector2(0, _default_velocity);
	}, flex_particle_system::BOTTOM_WALL);
	set_process(active);

	_build_particles();
	_rebuild = false;
	_update_particles();
	_update = false;
}
