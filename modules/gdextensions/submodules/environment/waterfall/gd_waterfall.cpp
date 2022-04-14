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

void GdWaterfall::set_active(bool p_state) {
	active = p_state;
}

bool GdWaterfall::is_active() const {
	return active;
}

void GdWaterfall::set_view_rect(const Rect2 &p_rect) {
	view_rect = p_rect;
	_dirty = true;
	update();
}

Rect2 GdWaterfall::get_view_rect() const {
	return view_rect;
}

void GdWaterfall::set_density(real_t p_density) {
	density = p_density;
	_dirty = true;
	update();
}

real_t GdWaterfall::get_density() const {
	return density;
}

void GdWaterfall::set_speed(real_t p_speed) {
	speed = p_speed;
}

real_t GdWaterfall::get_speed() const {
	return speed;
}

void GdWaterfall::set_textures_quality(int p_quality) {
	ERR_FAIL_INDEX(p_quality, ParticlesQualityCount);
	textures_quality = (WaterfallParticlesQuality)p_quality;
}

int GdWaterfall::get_textures_quality() const {
	return textures_quality;
}

void GdWaterfall::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			_p.draw(view_rect);
		} break;
		case NOTIFICATION_PROCESS: {
			if (_dirty) {
				_p.setup_quad(
					{view_rect.left(), view_rect.top()},
					{view_rect.left(), view_rect.bottom()},
					{view_rect.right(), view_rect.top()},
					{view_rect.right(), view_rect.bottom()}
				);
				_dirty = false;
			}
			_p.update();
		}
	}
}

void GdWaterfall::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_active"), &GdWaterfall::set_active);
	ClassDB::bind_method(D_METHOD("is_active"), &GdWaterfall::is_active);
	ClassDB::bind_method(D_METHOD("set_view_rect"), &GdWaterfall::set_view_rect);
	ClassDB::bind_method(D_METHOD("get_view_rect"), &GdWaterfall::get_view_rect);
	ClassDB::bind_method(D_METHOD("set_density"), &GdWaterfall::set_density);
	ClassDB::bind_method(D_METHOD("get_density"), &GdWaterfall::get_density);
	ClassDB::bind_method(D_METHOD("set_speed"), &GdWaterfall::set_speed);
	ClassDB::bind_method(D_METHOD("get_speed"), &GdWaterfall::get_speed);
	ClassDB::bind_method(D_METHOD("set_textures_quality"), &GdWaterfall::set_textures_quality);
	ClassDB::bind_method(D_METHOD("get_textures_quality"), &GdWaterfall::get_textures_quality);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "is_active");
	ADD_PROPERTY(PropertyInfo(Variant::RECT2, "view_rect"), "set_view_rect", "get_view_rect");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "density"), "set_density", "get_density");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "speed"), "set_speed", "get_speed");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "textures_quality", PROPERTY_HINT_ENUM, "Low,Medium,High"), "set_textures_quality", "get_textures_quality");
}

GdWaterfall::GdWaterfall() : _p(this) {
	_dirty = false;
	active = true;
	speed = 1.0;
	density = 10;
	textures_quality = ParticlesMedium;
	view_rect = Rect2(0, 0, 50, 150);

	_p.set_option(flex_particle_system::VERTICAL_WRAP, true);
}
