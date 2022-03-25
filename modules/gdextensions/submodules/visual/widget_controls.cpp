/*************************************************************************/
/*  widget_controls.cpp                                                  */
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
//  - https://stackoverflow.com/questions/7687148/

#include "common/gd_core.h"
#include "core/color.h"
#include "core/math/math_funcs.h"
#include "core/os/input_event.h"
#include "core/variant.h"
#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"

#include "widget_controls.h"

/// Rotation ball

enum { xcv_rotate_stacks = 16 }; // 32

static void _rotate_sphere(Ref<ArrayMesh> mesh, real_t r, int slices, int stacks, Transform transform = Transform()) {
	PoolVector3Array verts, norms;
	PoolVector2Array texs;
	PoolIntArray indexes;

	// There's a backward facing disc on one end visible in LINE mode.
	// Adjusting s0 and s1 would work also.
	for (int i = 1, v = 0; i <= stacks; i++) {
		real_t s0 = (real_t)(i - 1) / stacks;

		const real_t lat0 = Math_PI * (-0.5 + s0);
		const real_t z0 = Math::sin(lat0);
		const real_t zr0 = Math::cos(lat0);

		real_t s1 = (real_t)i / stacks;

		const real_t lat1 = Math_PI * (-0.5 + s1);
		const real_t z1 = Math::sin(lat1);
		const real_t zr1 = Math::cos(lat1);

		// I'm having to concoct UVs for this code.
		// This just mirrors the UVs for the rotate
		// widget's checkerboard. It might be wrong
		// for a non symmetrical texture.
		// Note, in theory whether the checkerboard
		// colors are switched doesn't matter but it
		// might because of its shadow and it's the
		// way it's always been.
		s0 = 1 - s0;
		s1 = 1 - s1;

		// NOTE: Quads look best in wireframe.
		for (int j = slices + 1; j-- > 0; v += 2) {
			const real_t t = (real_t)j / slices;
			const real_t lng = 2 * Math_PI * t;
			const real_t x = Math::cos(lng);
			const real_t y = Math::sin(lng);

			texs.push_back({ s0, t });
			norms.push_back({ x * zr0, y * zr0, z0 });
			verts.push_back({ r * x * zr0, r * y * zr0, r * z0 });

			texs.push_back({ s1, t });
			norms.push_back({ x * zr1, y * zr1, z1 });
			verts.push_back({ r * x * zr1, r * y * zr1, r * z1 });

			if (j == slices) {
				continue;
			}

			indexes.append_array(parray(v - 2, v - 1, v + 0, v - 1, v + 0, v + 1)); // 0,1,2 1,2,3
		}
	}

	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);
	mesh_array[VS::ARRAY_VERTEX] = verts;
	mesh_array[VS::ARRAY_TEX_UV] = texs;
	mesh_array[VS::ARRAY_NORMAL] = norms;
	mesh_array[VS::ARRAY_INDEX] = indexes;
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array);
}

static void _draw_ball(Ref<ArrayMesh> mesh, real_t radius) {
	if (radius) {
		_rotate_sphere(mesh, radius, xcv_rotate_stacks * 3 / 2, xcv_rotate_stacks);
	}
}

/// Translation

/* orientation: 0=up, 1=left, 2=down, 3=right (note: not diagram notation)  */
/*                                                                          */
/*                                                                          */
/*                           0, y2                                          */
/*                      /            \                                      */
/*                     /              \                                     */
/*                    /                \                                    */
/*                   /                  \                                   */
/*                  /                    \                                  */
/*                 /                      \                                 */
/*                /                        \                                */
/*               /                          \                               */
/*            -x2,y1   -x1b,y1   x1b,y1     x2,y1                           */
/*                        |        |                                        */
/*                        |        |                                        */
/*                        |        |                                        */
/*                        |        |                                        */
/*                        |        |                                        */
/*                    -x1a,y0    x1a,y0                                     */
/*                                                                          */

enum TranslateType {
	TRANS_X,
	TRANS_XY,
	TRANS_Y,
	TRANS_Z,
};

static void _draw_2d_arrow(Ref<ArrayMesh> mesh, int radius, bool filled, bool enabled, int orientation, TranslateType trans_type) {
	struct mesh_info_t {
		PoolVector2Array verts;
		PoolColorArray colors;

		Color curr_color;

		void set_color(const Color &color) { curr_color = color; }
		void set_color(real_t grey) { curr_color = { grey, grey, grey }; }
		void add_vert(const Vector2 &vert) {
			colors.push_back(curr_color);
			verts.push_back(vert);
		}
		void add_vert(const Transform2D &transform, const Vector2 &vert) {
			colors.push_back(curr_color);
			verts.push_back(transform.xform(vert));
		}
	};

	Transform2D curr_xform;

	const Color white{ 1, 1, 1 };
	const Color black{ 0, 0, 0 };
	const Color gray{ 0.45, 0.45, 0.45 };
	const Color bkgd{ 0.7, 0.7, 0.7 };

	Color bkgd2{ bkgd[0] + 0.07f, bkgd[1] + 0.07f, bkgd[2] + 0.07f };
	Color bkgd3 = enabled ? bkgd : bkgd2;

	// The following 8 colors define the shading of an octagon,
	// in clockwise order, starting from the upstroke on the left
	// This is for an outside and inside octagons
	Color colors_in[8] = { bkgd3, white, bkgd3, gray, gray, gray, gray, gray };
	Color colors_out[8] = { white, white, white, gray, black, black, black, gray };

	int c_rot = 0; // color index offset

	if (orientation == 2) {
		c_rot = 4;
	} else if (orientation == 0) {
		c_rot = 0;
		curr_xform.rotate(Math::deg2rad(180.0));
	} else if (orientation == 1) {
		c_rot = 2;
		curr_xform.rotate(Math::deg2rad(90.0));
	} else if (orientation == 3) {
		c_rot = 6;
		curr_xform.rotate(Math::deg2rad(-90.0));
	}

	real_t x1_ = radius * 0.2;
	real_t x2 = x1_ * 2;
	real_t y0 = (trans_type == TRANS_XY) ? x1_ : 0;
	real_t y1 = radius * 0.54;
	real_t y2 = y1 + x2;
	real_t x1a = x1_, x1b = x1_;
	if (trans_type == TRANS_Z) {
		real_t x = 0.75;

		if (orientation == 0) {
			// x1b -= 2; x1a += 2;
			x1b *= x * 0.8;
			x1a /= x;

			// y1 += 2; y2 += 0;
			x = y1;
			y1 *= 1.2; // 45 degrees

			// x2 -= 2;
			x2 -= int(y1 + 0.5) - x; // 45
		} else if (orientation == 2) {
			// x1a += 2;
			x1a /= x;

			// y1 -= 6;
			x = y1;
			y1 /= 2; // 45 degrees

			// x2 += 6;
			x2 -= int(y1 + 0.5) - x; // 45

			// x1b += 4;
			x1b *= 1.7;
		}
	}
	bool z1 = false;
	if (true) { // EXPERIMENTAL
		// Pretty good foreshortening. This is pretty cool
		// but I think I did it because the thin neck part
		// looked either too thin or too fat with the hard
		// pixel offests.
		if (false) { // DISABLED: z1 = (TRANS_Z == trans_type)
			y2 *= orientation ? 1.2 : 1;

			real_t x = 0.5;
			y1 *= x;
			y2 *= x;

			x = 1.8;
			x1a *= x;
			if (orientation) {
				x2 *= x;
				x1b *= x;
			}
		}

		// Fixing to hard pixel boundaries before applying
		// 1px offsets below.
		// x1_ = int(x1 + 0.5);
		x2 = (real_t)(int)(x2 + 0.5);
		y0 = (real_t)(int)(y0 + 0.5);
		y1 = (real_t)(int)(y1 + 0.5);
		y2 = (real_t)(int)(y2 + 0.5);
		x1a = (real_t)(int)(x1a + 0.5);
		x1b = (real_t)(int)(x1b + 0.5);
	}

	{
		mesh_info_t mesh_info;

		if (!enabled) {
			// Check if control is enabled or not
			// Indents the shadows - goes from a raised look to embossed
			c_rot += 4;
			goto disabled; // set_to_bkgd_color();
		} else if (!filled) { // Fill in inside of arrow
			// Means button is up - control is not clicked
			mesh_info.set_color(bkgd2); // set_to_bkgd_color();
		} else { // button is down on control
			mesh_info.set_color(0.6);
			c_rot += 4; // Indents the shadows - goes from a raised look to embossed
		}

		mesh_info.add_vert(curr_xform, { 0, y2 });
		mesh_info.add_vert(curr_xform, { -x2, y1 });
		mesh_info.add_vert(curr_xform, { -x1b, y1 });
		mesh_info.add_vert(curr_xform, { -x1a, 0 }); // y0
		mesh_info.add_vert(curr_xform, { x1a, 0 }); // y0
		mesh_info.add_vert(curr_xform, { x1b, y1 });
		mesh_info.add_vert(curr_xform, { x2, y1 });

		Array mesh_array;
		mesh_array.resize(VS::ARRAY_MAX);
		mesh_array[VS::ARRAY_VERTEX] = mesh_info.verts;
		mesh_array[VS::ARRAY_COLOR] = mesh_info.colors;
		mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLE_FAN, mesh_array, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);
	}

disabled:
	bool z2 = z1;
	(orientation ? z1 : z2) = false;

	// Draw arrow outline
	{
		mesh_info_t mesh_info;
		Color *col, *sel = nullptr;

#define SET_COL(io, i)                                         \
	if (sel != (col = &colors_##io[(i + c_rot) % 8]) && sel) { \
		col = 0;                                               \
	}
#define DRAW_SEG(xa, ya, xb, yb)                    \
	if (col) {                                      \
		mesh_info.set_color(*col);                  \
		mesh_info.add_vert(curr_xform, { xa, ya }); \
		mesh_info.add_vert(curr_xform, { xb, yb }); \
		mesh_info.add_vert(curr_xform, { xb, yb }); \
		mesh_info.add_vert(curr_xform, { xa, ya }); \
	} // double ends?

		if (!z1) {
			SET_COL(in, 1);
			DRAW_SEG(0, y2 - 1, -x2 + 2, y1 + 1); // flipping
			SET_COL(in, 3);
			DRAW_SEG(0, y2 - 1, x2 - 2, y1 + 1);

			SET_COL(out, 1);
			DRAW_SEG(0, y2, -x2, y1);
			SET_COL(out, 3);
			DRAW_SEG(0, y2, x2, y1);
		}

		int l = (bkgd3 == colors_in[(6 + c_rot) % 8]) ? 3 : 1;
		int ll = (TRANS_XY == trans_type) ? 1 : 0;

		SET_COL(in, 0);
		DRAW_SEG(-x1b + 1, y1 + 1, -x1a + 1, y0 + ll); // flipping
		SET_COL(in, 6);
		DRAW_SEG(-x2 + l, y1 + 1, -x1b + 1, y1 + 1); // flipping
		SET_COL(in, 6);
		DRAW_SEG(x2 - l, y1 + 1, x1b - 1, y1 + 1);
		SET_COL(in, 4);
		DRAW_SEG(x1b - 1, y1 + 1, x1a - 1, y0 + ll);

		if (trans_type == TRANS_Z) { // Fill in gaps.
			const real_t hl = 0.5;
			SET_COL(in, 0);
			DRAW_SEG(-x1b + hl, y1, -x1a + 1, y0); // flipping
			SET_COL(in, 4);
			DRAW_SEG(x1b - hl, y1, x1a - 1, y0);
		}

		if (!z2) { // Saving for end?
			SET_COL(out, 0);
			DRAW_SEG(-x1b, y1, -x1a, y0); // flipping
			SET_COL(out, 6);
			DRAW_SEG(-x2, y1, -x1b, y1); // flipping
		} else { // Make point 2 pixels tall.
			SET_COL(out, 1);
			DRAW_SEG(0, y2 + 1, -x2, y1); // flipping
			SET_COL(out, 3);
			DRAW_SEG(0, y2 + 1, x2, y1);
		}
		// -1 is not connecting in this order??? But flipping
		// its line makes the other end break.
		SET_COL(out, 6);
		DRAW_SEG(x2, y1, x1b, y1); // doubling
		// SET_COL(out,6); DRAW_SEG(x1b, y1, x2, y1) // doubling flipped
		SET_COL(out, 4);
		DRAW_SEG(x1b, y1, x1a, y0); // doubling
		// SET_COL(out,4); DRAW_SEG(x1a, y0, x1b, y1); // doubling flipped

		if (z2) { // front of forshortened arrow
			// Dawing these last for forshortened Z to be drawn
			// in front of the frame.
			SET_COL(out, 0);
			DRAW_SEG(-x1b, y1, -x1a, y0); // flipping
			SET_COL(out, 6);
			DRAW_SEG(-x2, y1, -x1b, y1); // flipping
		}

		Array mesh_array;
		mesh_array.resize(VS::ARRAY_MAX);
		mesh_array[VS::ARRAY_VERTEX] = mesh_info.verts;
		mesh_array[VS::ARRAY_COLOR] = mesh_info.colors;
		mesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, mesh_array, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);
#undef SET_COL
#undef DRAW_SEG
	}
}

static void draw_2d_arrows(Ref<ArrayMesh> mesh, TranslateType trans_type, int radius, bool press, bool enabled, char translation_locked /* 'X', 'Y' or 0 */) {
	int o1 = 0, o2 = 0;
	if (trans_type == TRANS_Z) {
		o1 = 2;
		o2 = 0; // draw_2d_z_arrows(radius);
	} else if (trans_type == TRANS_XY) { // draw_2d_xy_arrows(radius);
		char lock = press ? translation_locked : 0;

		bool filled = false;
		if (!lock) {
			filled = press;
			o1 = 0;
			o2 = 1;
		} else if (lock == 'Y') { // LOCK_X
			o1 = 0;
			o2 = 2; // Y
		} else if (lock == 'X') { // LOCK_Y
			o1 = 1;
			o2 = 3; // X
		}
		_draw_2d_arrow(mesh, radius, filled, enabled, o1, trans_type);
		_draw_2d_arrow(mesh, radius, filled, enabled, o2, trans_type);

		if (lock == 'Y')
			goto X; // LOCK_X
		if (lock == 'X')
			goto Y; // LOCK_Y

		o1 = 2;
		o2 = 3;
	} else if (trans_type == TRANS_X)
	X : {
		o1 = 1;
		o2 = 3; // draw_2d_x_arrows((int)radius-1);
	}
		else if (trans_type == TRANS_Y)
				Y : {
			o1 = 0;
			o2 = 2; // draw_2d_y_arrows((int)radius-1);
		}

	_draw_2d_arrow(mesh, radius, press, enabled, o1, trans_type);
	_draw_2d_arrow(mesh, radius, press, enabled, o2, trans_type);
}

/// Node control

#ifdef TOOLS_ENABLED
bool ControlWidget::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	return control_rect.has_point(p_point);
}

Rect2 ControlWidget::_edit_get_rect() const {
	return control_rect;
}

bool ControlWidget::_edit_use_rect() const {
	return true;
}
#endif

Rect2 ControlWidget::_get_global_rect() const {
	return Rect2(get_global_position() + control_rect.position, control_rect.size);
}

bool ControlWidget::_is_point_inside(const Point2 &point) {
	return _get_global_rect().has_point(point);
}

void ControlWidget::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			if (!ball_texture && control_type == WIDGET_ROTATION) {
				enum {
					dark = 110, // Dark and light colors for _ball checkerboard
					light = 220,

					checkboard_size = 64 * 2, // pixels across whole texture
					checkboard_repeat = 32, // pixels across one black/white sector
				};

				unsigned char texture_image[checkboard_size][checkboard_size];
				for (int i = 0; i < checkboard_size; i++) {
					for (int j = 0; j < checkboard_size; j++) {
						unsigned char c = ((i / checkboard_repeat) & 1) ^ ((j / checkboard_repeat) & 1) ? light : dark;
						texture_image[i][j] = c;
					}
				}
				Ref<Image> image = newref(Image);
				PoolByteArray data;
				data.resize(sizeof(texture_image));
				memcpy(data.write().ptr(), texture_image, sizeof(texture_image));
				image->create(checkboard_size, checkboard_size, false, Image::FORMAT_L8, data);
				Ref<ImageTexture> texture = newref(ImageTexture);
				texture->create_from_image(image);
				ball_texture = texture;
			}
			mesh->clear_mesh();
			const int radius = control_rect.size.height / 2;
			switch (control_type) {
				case WIDGET_TRANSLATION_XY: {
					draw_2d_arrows(mesh, TRANS_XY, radius, _press, _enabled, _locked);
				} break;
				case WIDGET_TRANSLATION_Z: {
					draw_2d_arrows(mesh, TRANS_Z, radius, _press, _enabled, _locked);
				} break;
				case WIDGET_ROTATION: {
					_draw_ball(mesh, radius);
				} break;
			}
			if (control_themed) {
				if (!_style) {
					_style = newref(StyleBoxFlat);
				}
				_style->set_border_width_all(_style_info.width);
				_style->set_corner_radius_all(_style_info.radius);
				_style->set_bg_color(_style_info.bg_color);
				_style->set_shadow_size(_style_info.shadow_size);
				_style->set_shadow_color(_style_info.shadow_color);
				_style->set_shadow_offset(_style_info.shadow_offset);
				draw_style_box(_style, control_rect);
			}
			draw_mesh(mesh, control_type == WIDGET_ROTATION ? ball_texture : Ref<Texture>());
		} break;
	}
}

void ControlWidget::_input(const Ref<InputEvent> &p_event) {
	if (!p_event.is_valid()) {
		return;
	}
	if (!get_tree()) {
		return;
	}

	ERR_FAIL_COND(!is_visible_in_tree());

	if (!p_event->is_echo()) {
		if (_press != p_event->is_pressed()) {
			_press = p_event->is_pressed();
			update();
		}
	}

	Ref<InputEventScreenTouch> b = p_event;

	if (b.is_valid() && _is_point_inside(b->get_position())) {
	}
}

void ControlWidget::set_control_type(WidgetType p_type) {
	ERR_FAIL_INDEX(p_type, WIDGET_TYPES);
	control_type = p_type;
	update();
}
WidgetType ControlWidget::get_control_type() const {
	return control_type;
}

void ControlWidget::set_control_themed(bool p_themed) {
	control_themed = p_themed;
	update();
}
bool ControlWidget::get_control_themed() const {
	return control_themed;
}

void ControlWidget::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_control_type"), &ControlWidget::set_control_type);
	ClassDB::bind_method(D_METHOD("get_control_type"), &ControlWidget::get_control_type);
	ClassDB::bind_method(D_METHOD("set_control_themed"), &ControlWidget::set_control_themed);
	ClassDB::bind_method(D_METHOD("get_control_themed"), &ControlWidget::get_control_themed);

	ClassDB::bind_method(D_METHOD("_input"), &ControlWidget::_input);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "control_type", PROPERTY_HINT_ENUM, "XY,Z,BALL"), "set_control_type", "get_control_type");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "themed"), "set_control_themed", "get_control_themed");
}

ControlWidget::ControlWidget() {
	mesh = newref(ArrayMesh);
	control_type = WIDGET_TRANSLATION_XY;
	control_themed = true;
	control_rect = Rect2(-50, -50, 100, 100);
	_style_info = StyleInfo{ 2, 2, Color::named("lightgray"), 2, Color::named("black"), Vector2(1, 2) };
	_enabled = true;
	_press = false;
	_locked = 0;
}
