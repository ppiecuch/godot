/*************************************************************************/
/*  starfield_2d.cpp                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "starfield_2d.h"
#include "inc/gd_pack.h"
#include "starfield.h"
#include "starfield_res.h"

void Starfield2D::_update() {
	update();
}

void Starfield2D::_notification(int p_what) {

	switch (p_what) {

		case NOTIFICATION_READY: {

			// prepare starfield
			_starfield = Ref<Starfield>(memnew(Starfield));
			update();
		} break;

		case NOTIFICATION_PHYSICS_PROCESS: {
		} break;

		case NOTIFICATION_DRAW: {
		} break;
	}
}

void Starfield2D::set_virtual_size(const Size2 &p_size) {

	if (p_size != virtual_size) {
		virtual_size = p_size;
		update();
	}
}

Vector2 Starfield2D::get_virtual_size() const {

	return virtual_size;
}

void Starfield2D::set_movement_vector(const Vector2 &p_movement) {

	if (p_movement != movement_vector) {
		movement_vector = p_movement;
		update();
	}
}

Vector2 Starfield2D::get_movement_vector() const {

	return movement_vector;
}

void Starfield2D::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_virtual_size"), &Starfield2D::set_virtual_size);
	ClassDB::bind_method(D_METHOD("get_virtual_size"), &Starfield2D::get_virtual_size);
	ClassDB::bind_method(D_METHOD("set_movement_vector"), &Starfield2D::set_movement_vector);
	ClassDB::bind_method(D_METHOD("get_movement_vector"), &Starfield2D::get_movement_vector);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "virtual_size"), "set_virtual_size", "get_virtual_size");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "movement_vector"), "set_movement_vector", "get_movement_vector");
}

Starfield2D::Starfield2D() {
	// build texture atlas from resources
	Vector<Ref<Image> > images;
	Vector<String> names;
	std::vector<EmbedImageItem> embed(embed_starfield_res, embed_starfield_res + embed_starfield_res_count);
	for (const auto &r : embed) {
		ERR_CONTINUE_MSG(r.channels < 3, "Format is not supported, Skipping!");
		Ref<Image> image;
		image.instance();
		PoolByteArray data;
		data.resize(r.size);
		std::memcpy(data.write().ptr(), r.pixels, r.size);
		image->create(r.width, r.height, false, r.channels == 4 ? Image::FORMAT_RGBA8 : Image::FORMAT_RGB8, data);
		images.push_back(image);
		names.push_back(r.image);
	}
	_image_atlas = merge_images(images, names);
}

Starfield2D::~Starfield2D() {
}
