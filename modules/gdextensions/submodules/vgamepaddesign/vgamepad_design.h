/**************************************************************************/
/*  vgamepad_design.h                                                     */
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

#ifndef GD_VGAMEPAD_DESIGN_H
#define GD_VGAMEPAD_DESIGN_H

#include "scene/resources/texture.h"

// VGamePadDesign: built-in texture provider for VirtualJoystick.
//
// Loads joystick base/knob textures from INCBIN-embedded PNGs
// (compiled from vgamepaddesign/res/set1/) and exposes them as
// ready-to-use Ref<Texture> for VirtualJoystick::setup().
//
// Usage from GDScript:
//
//   var design = VGamePadDesign.new()
//   $VirtualJoystick.setup(
//       design.get_joystick_back(),   # base ring normal
//       null,                          # base ring pressed (fallback to normal)
//       design.get_joystick_knob(),   # tip/knob normal
//       null                           # tip pressed (fallback to normal)
//   )
//
// Or one-liner:
//
//   VGamePadDesign.apply_to($VirtualJoystick)
//

class VGamePadDesign : public Reference {
	GDCLASS(VGamePadDesign, Reference);

public:
	enum DesignSet {
		DESIGN_SET1,
	};

private:
	DesignSet current_set;

	// Cached textures (created on first access).
	mutable Ref<ImageTexture> joystick_back;
	mutable Ref<ImageTexture> joystick_knob;

	static Ref<ImageTexture> _load_embedded_png(const uint8_t *p_data, size_t p_size, const String &p_name);

protected:
	static void _bind_methods();

public:
	Ref<Texture> get_joystick_back() const;
	Ref<Texture> get_joystick_knob() const;

	void set_design_set(DesignSet p_set);
	DesignSet get_design_set() const;

	// Convenience: apply this design to a VirtualJoystick node.
	void apply_to(Node *p_joystick, const Vector2 &p_sizes = Vector2(-1, -1));

	VGamePadDesign();
};

VARIANT_ENUM_CAST(VGamePadDesign::DesignSet);

#endif // GD_VGAMEPAD_DESIGN_H
