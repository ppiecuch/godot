/*************************************************************************/
/*  noise_texture.cpp                                                    */
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

#include "fastnoise_texture.h"

#include "core/core_string_names.h"
#include "noise.h"

FastNoiseTexture::FastNoiseTexture() {
	noise = Ref<Noise>();

	_queue_update();
}

FastNoiseTexture::~FastNoiseTexture() {
	if (texture.is_valid()) {
		RS::get_singleton()->free(texture);
	}
	noise_thread.wait_to_finish();
}

void FastNoiseTexture::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_width", "width"), &FastNoiseTexture::set_width);
	ClassDB::bind_method(D_METHOD("set_height", "height"), &FastNoiseTexture::set_height);

	ClassDB::bind_method(D_METHOD("set_noise", "noise"), &FastNoiseTexture::set_noise);
	ClassDB::bind_method(D_METHOD("get_noise"), &FastNoiseTexture::get_noise);

	ClassDB::bind_method(D_METHOD("set_invert", "invert"), &FastNoiseTexture::set_invert);
	ClassDB::bind_method(D_METHOD("get_invert"), &FastNoiseTexture::get_invert);

	ClassDB::bind_method(D_METHOD("set_seamless", "seamless"), &FastNoiseTexture::set_seamless);
	ClassDB::bind_method(D_METHOD("get_seamless"), &FastNoiseTexture::get_seamless);

	ClassDB::bind_method(D_METHOD("set_seamless_blend_skirt", "seamless_blend_skirt"), &FastNoiseTexture::set_seamless_blend_skirt);
	ClassDB::bind_method(D_METHOD("get_seamless_blend_skirt"), &FastNoiseTexture::get_seamless_blend_skirt);

	ClassDB::bind_method(D_METHOD("set_as_normal_map", "as_normal_map"), &FastNoiseTexture::set_as_normal_map);
	ClassDB::bind_method(D_METHOD("is_normal_map"), &FastNoiseTexture::is_normal_map);

	ClassDB::bind_method(D_METHOD("set_bump_strength", "bump_strength"), &FastNoiseTexture::set_bump_strength);
	ClassDB::bind_method(D_METHOD("get_bump_strength"), &FastNoiseTexture::get_bump_strength);

	ClassDB::bind_method(D_METHOD("_update_texture"), &FastNoiseTexture::_update_texture);
	ClassDB::bind_method(D_METHOD("_generate_texture"), &FastNoiseTexture::_generate_texture);
	ClassDB::bind_method(D_METHOD("_thread_done", "image"), &FastNoiseTexture::_thread_done);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "width", PROPERTY_HINT_RANGE, "1,2048,1,or_greater"), "set_width", "get_width");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "height", PROPERTY_HINT_RANGE, "1,2048,1,or_greater"), "set_height", "get_height");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "invert"), "set_invert", "get_invert");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "seamless"), "set_seamless", "get_seamless");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "seamless_blend_skirt", PROPERTY_HINT_RANGE, "0.05,1,0.001"), "set_seamless_blend_skirt", "get_seamless_blend_skirt");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "as_normal_map"), "set_as_normal_map", "is_normal_map");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bump_strength", PROPERTY_HINT_RANGE, "0,32,0.1,or_greater"), "set_bump_strength", "get_bump_strength");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "noise", PROPERTY_HINT_RESOURCE_TYPE, "Noise"), "set_noise", "get_noise");
}

void FastNoiseTexture::_validate_property(PropertyInfo &property) const {
	if (property.name == "bump_strength") {
		if (!as_normal_map) {
			property.usage = PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_INTERNAL;
		}
	}

	if (property.name == "seamless_blend_skirt") {
		if (!seamless) {
			property.usage = PROPERTY_USAGE_NO_EDITOR;
		}
	}
}

void FastNoiseTexture::_set_texture_image(const Ref<Image> &p_image) {
	image = p_image;
	if (image.is_valid()) {
		if (texture.is_valid()) {
			RID new_texture = RS::get_singleton()->texture_2d_create(p_image);
			RS::get_singleton()->texture_replace(texture, new_texture);
		} else {
			texture = RS::get_singleton()->texture_2d_create(p_image);
		}
	}
	emit_changed();
}

void FastNoiseTexture::_thread_done(const Ref<Image> &p_image) {
	_set_texture_image(p_image);
	noise_thread.wait_to_finish();
	if (regen_queued) {
		noise_thread.start(_thread_function, this);
		regen_queued = false;
	}
}

void FastNoiseTexture::_thread_function(void *p_ud) {
	FastNoiseTexture *tex = (FastNoiseTexture *)p_ud;
	tex->call_deferred(SNAME("_thread_done"), tex->_generate_texture());
}

void FastNoiseTexture::_queue_update() {
	if (update_queued) {
		return;
	}

	update_queued = true;
	call_deferred(SNAME("_update_texture"));
}

Ref<Image> FastNoiseTexture::_generate_texture() {
	// Prevent memdelete due to unref() on other thread.
	Ref<Noise> ref_noise = noise;

	if (ref_noise.is_null()) {
		return Ref<Image>();
	}

	Ref<Image> image;

	if (seamless) {
		image = ref_noise->get_seamless_image(size.x, size.y, invert, seamless_blend_skirt);
	} else {
		image = ref_noise->get_image(size.x, size.y, invert);
	}

	if (as_normal_map) {
		image->bump_map_to_normal_map(bump_strength);
	}

	return image;
}

void FastNoiseTexture::_update_texture() {
	bool use_thread = true;
	if (first_time) {
		use_thread = false;
		first_time = false;
	}
#ifdef NO_THREADS
	use_thread = false;
#endif
	if (use_thread) {
		if (!noise_thread.is_started()) {
			noise_thread.start(_thread_function, this);
			regen_queued = false;
		} else {
			regen_queued = true;
		}

	} else {
		Ref<Image> image = _generate_texture();
		_set_texture_image(image);
	}
	update_queued = false;
}

void FastNoiseTexture::set_noise(Ref<Noise> p_noise) {
	if (p_noise == noise) {
		return;
	}
	if (noise.is_valid()) {
		noise->disconnect(CoreStringNames::get_singleton()->changed, callable_mp(this, &FastNoiseTexture::_queue_update));
	}
	noise = p_noise;
	if (noise.is_valid()) {
		noise->connect(CoreStringNames::get_singleton()->changed, callable_mp(this, &FastNoiseTexture::_queue_update));
	}
	_queue_update();
}

Ref<Noise> FastNoiseTexture::get_noise() {
	return noise;
}

void FastNoiseTexture::set_width(int p_width) {
	ERR_FAIL_COND(p_width <= 0);
	if (p_width == size.x) {
		return;
	}
	size.x = p_width;
	_queue_update();
}

void FastNoiseTexture::set_height(int p_height) {
	ERR_FAIL_COND(p_height <= 0);
	if (p_height == size.y) {
		return;
	}
	size.y = p_height;
	_queue_update();
}

void FastNoiseTexture::set_invert(bool p_invert) {
	if (p_invert == invert) {
		return;
	}
	invert = p_invert;
	_queue_update();
}

bool FastNoiseTexture::get_invert() const {
	return invert;
}

void FastNoiseTexture::set_seamless(bool p_seamless) {
	if (p_seamless == seamless) {
		return;
	}
	seamless = p_seamless;
	_queue_update();
	notify_property_list_changed();
}

bool FastNoiseTexture::get_seamless() {
	return seamless;
}

void FastNoiseTexture::set_seamless_blend_skirt(real_t p_blend_skirt) {
	if (p_blend_skirt == seamless_blend_skirt) {
		return;
	}
	seamless_blend_skirt = p_blend_skirt;
	_queue_update();
}
real_t FastNoiseTexture::get_seamless_blend_skirt() {
	return seamless_blend_skirt;
}

void FastNoiseTexture::set_as_normal_map(bool p_as_normal_map) {
	if (p_as_normal_map == as_normal_map) {
		return;
	}
	as_normal_map = p_as_normal_map;
	_queue_update();
	notify_property_list_changed();
}

bool FastNoiseTexture::is_normal_map() {
	return as_normal_map;
}

void FastNoiseTexture::set_bump_strength(float p_bump_strength) {
	if (p_bump_strength == bump_strength) {
		return;
	}
	bump_strength = p_bump_strength;
	if (as_normal_map) {
		_queue_update();
	}
}

float FastNoiseTexture::get_bump_strength() {
	return bump_strength;
}

int FastNoiseTexture::get_width() const {
	return size.x;
}

int FastNoiseTexture::get_height() const {
	return size.y;
}

RID FastNoiseTexture::get_rid() const {
	if (!texture.is_valid()) {
		texture = RS::get_singleton()->texture_2d_placeholder_create();
	}

	return texture;
}

Ref<Image> FastNoiseTexture::get_image() const {
	return image;
}
