/**************************************************************************/
/*  vgamepad_design.cpp                                                   */
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

#include "vgamepad_design.h"

#include "core/image.h"
#include "scene/gui/virtual_joystick.h"

// INCBIN-embedded PNGs from set1/ (compiled in vgamepad_res.c).
extern "C" const uint8_t set1_Joystick_Back_png_data[];
extern "C" const size_t set1_Joystick_Back_png_size;
extern "C" const uint8_t set1_Joystick_Knob_png_data[];
extern "C" const size_t set1_Joystick_Knob_png_size;

Ref<ImageTexture> VGamePadDesign::_load_embedded_png(const uint8_t *p_data, size_t p_size, const String &p_name) {
	Ref<Image> image = memnew(Image(p_data, p_size));
	ERR_FAIL_COND_V_MSG(image->empty(), Ref<ImageTexture>(), "VGamePadDesign: failed to load embedded PNG: " + p_name);
	Ref<ImageTexture> texture = memnew(ImageTexture);
	texture->create_from_image(image, Texture::FLAG_FILTER);
	return texture;
}

Ref<Texture> VGamePadDesign::get_joystick_back() const {
	if (joystick_back.is_null()) {
		joystick_back = _load_embedded_png(set1_Joystick_Back_png_data, set1_Joystick_Back_png_size, "Joystick_Back");
	}
	return joystick_back;
}

Ref<Texture> VGamePadDesign::get_joystick_knob() const {
	if (joystick_knob.is_null()) {
		joystick_knob = _load_embedded_png(set1_Joystick_Knob_png_data, set1_Joystick_Knob_png_size, "Joystick_Knob");
	}
	return joystick_knob;
}

void VGamePadDesign::set_design_set(DesignSet p_set) {
	if (current_set == p_set) {
		return;
	}
	current_set = p_set;
	// Invalidate cached textures so they reload from the new set.
	joystick_back = Ref<ImageTexture>();
	joystick_knob = Ref<ImageTexture>();
}

VGamePadDesign::DesignSet VGamePadDesign::get_design_set() const {
	return current_set;
}

void VGamePadDesign::apply_to(Node *p_joystick, const Vector2 &p_sizes) {
	VirtualJoystick *vj = Object::cast_to<VirtualJoystick>(p_joystick);
	ERR_FAIL_NULL_MSG(vj, "VGamePadDesign::apply_to: node is not a VirtualJoystick.");
	vj->setup(get_joystick_back(), Ref<Texture>(), get_joystick_knob(), Ref<Texture>(), p_sizes);
}

void VGamePadDesign::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_joystick_back"), &VGamePadDesign::get_joystick_back);
	ClassDB::bind_method(D_METHOD("get_joystick_knob"), &VGamePadDesign::get_joystick_knob);

	ClassDB::bind_method(D_METHOD("set_design_set", "set"), &VGamePadDesign::set_design_set);
	ClassDB::bind_method(D_METHOD("get_design_set"), &VGamePadDesign::get_design_set);

	ClassDB::bind_method(D_METHOD("apply_to", "virtual_joystick", "sizes"), &VGamePadDesign::apply_to, DEFVAL(Vector2(-1, -1)));

	ADD_PROPERTY(PropertyInfo(Variant::INT, "design_set", PROPERTY_HINT_ENUM, "Set1"), "set_design_set", "get_design_set");

	BIND_ENUM_CONSTANT(DESIGN_SET1);
}

VGamePadDesign::VGamePadDesign() {
	current_set = DESIGN_SET1;
}
