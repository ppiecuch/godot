/**************************************************************************/
/*  geom_font_2d.cpp                                                      */
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

#include "fbdigitalfont/fb_font_symbol.h"
#include "fbdigitalfont/fb_font_view.h"

#include "geom_font_2d.h"

#include "hershey/hershey_render.h"

// Wrap font source includes in anonymous namespace to avoid duplicate symbols
// with gd_geomfonts.cpp (which includes the same files).
#include "leonsans/leon_render.h"

// Wrap font source includes in anonymous namespace to avoid duplicate symbols
// with gd_geomfonts.cpp (which includes the same files).
namespace {
#include "bob3d/bob_font.cpp"
#include "easyfont/stb_easy_font.h"
#include "lowpolysymbols/lowpoly_font.h"
#include "simplevector/asteroids.c"
#include "simplevector/hp1345.c"
} // namespace

// Hershey font data includes — wrapped in anonymous namespace to avoid
// duplicate symbol conflicts with gd_geomfonts.cpp which also includes them.
namespace {
#include "hershey/inc/astrology.h"
#include "hershey/inc/cursive.h"
#include "hershey/inc/cyrilc_1.h"
#include "hershey/inc/cyrillic.h"
#include "hershey/inc/futural.h"
#include "hershey/inc/futuram.h"
#include "hershey/inc/gothgbt.h"
#include "hershey/inc/gothgrt.h"
#include "hershey/inc/gothiceng.h"
#include "hershey/inc/gothicger.h"
#include "hershey/inc/gothicita.h"
#include "hershey/inc/gothitt.h"
#include "hershey/inc/greek.h"
#include "hershey/inc/greekc.h"
#include "hershey/inc/greeks.h"
#include "hershey/inc/japanese.h"
#include "hershey/inc/markers.h"
#include "hershey/inc/mathlow.h"
#include "hershey/inc/mathupp.h"
#include "hershey/inc/meteorology.h"
#include "hershey/inc/music.h"
#include "hershey/inc/rowmand.h"
#include "hershey/inc/rowmans.h"
#include "hershey/inc/rowmant.h"
#include "hershey/inc/scriptc.h"
#include "hershey/inc/scripts.h"
#include "hershey/inc/symbolic.h"
#include "hershey/inc/timesg.h"
#include "hershey/inc/timesi.h"
#include "hershey/inc/timesib.h"
#include "hershey/inc/timesr.h"
#include "hershey/inc/timesrb.h"
} // namespace

#include "common/gd_core.h"
#include "servers/visual_server.h"

#ifdef TOOLS_ENABLED
bool GeomFont2D::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	return _edit_get_rect().has_point(p_point);
}

Rect2 GeomFont2D::_edit_get_rect() const {
	return Rect2(Point2(0, 0), _text_rect_size);
}

bool GeomFont2D::_edit_use_rect() const {
	return true;
}
#endif

// --- BBCode Parser ---

Vector<GeomFont2D::TextSpan> GeomFont2D::_parse_bbcode(const String &p_text) const {
	Vector<TextSpan> spans;

	if (!_bbcode_enabled || p_text.find("[") == -1) {
		TextSpan span;
		span.text = p_text;
		span.color = _font_color;
		span.scale = _font_scale;
		spans.push_back(span);
		return spans;
	}

	// Stack for nested tags
	struct TagState {
		Color color;
		Vector2 scale;
	};
	Vector<TagState> stack;
	TagState current;
	current.color = _font_color;
	current.scale = _font_scale;

	String accum;
	int i = 0;
	while (i < p_text.length()) {
		if (p_text[i] == '[') {
			int close = p_text.find("]", i);
			if (close == -1) {
				accum += p_text[i];
				i++;
				continue;
			}
			String tag = p_text.substr(i + 1, close - i - 1);

			if (tag.begins_with("color=")) {
				// Flush accumulated text
				if (!accum.empty()) {
					TextSpan span;
					span.text = accum;
					span.color = current.color;
					span.scale = current.scale;
					spans.push_back(span);
					accum = "";
				}
				stack.push_back(current);
				String color_str = tag.substr(6).strip_edges();
				if (color_str.begins_with("#")) {
					current.color = Color::html(color_str);
				} else {
					current.color = Color::named(color_str);
				}
				i = close + 1;
			} else if (tag == "/color") {
				if (!accum.empty()) {
					TextSpan span;
					span.text = accum;
					span.color = current.color;
					span.scale = current.scale;
					spans.push_back(span);
					accum = "";
				}
				if (stack.size() > 0) {
					current = stack[stack.size() - 1];
					stack.resize(stack.size() - 1);
				}
				i = close + 1;
			} else if (tag.begins_with("scale=")) {
				if (!accum.empty()) {
					TextSpan span;
					span.text = accum;
					span.color = current.color;
					span.scale = current.scale;
					spans.push_back(span);
					accum = "";
				}
				stack.push_back(current);
				String scale_str = tag.substr(6).strip_edges();
				if (scale_str.find(",") != -1) {
					Vector<String> parts = scale_str.split(",");
					if (parts.size() >= 2) {
						current.scale = Vector2(parts[0].to_float(), parts[1].to_float());
					}
				} else {
					real_t s = scale_str.to_float();
					current.scale = Vector2(s, s);
				}
				i = close + 1;
			} else if (tag == "/scale") {
				if (!accum.empty()) {
					TextSpan span;
					span.text = accum;
					span.color = current.color;
					span.scale = current.scale;
					spans.push_back(span);
					accum = "";
				}
				if (stack.size() > 0) {
					current = stack[stack.size() - 1];
					stack.resize(stack.size() - 1);
				}
				i = close + 1;
			} else {
				// Unknown tag, treat as literal
				accum += p_text[i];
				i++;
			}
		} else {
			accum += p_text[i];
			i++;
		}
	}

	if (!accum.empty()) {
		TextSpan span;
		span.text = accum;
		span.color = current.color;
		span.scale = current.scale;
		spans.push_back(span);
	}

	if (spans.empty()) {
		TextSpan span;
		span.text = "";
		span.color = _font_color;
		span.scale = _font_scale;
		spans.push_back(span);
	}

	return spans;
}

// --- Hershey Data Lookup ---

#define HERSHEY_ENTRY(name)                                           \
	{                                                                 \
		(const char **)name, name##_size, name##_width, name##_height \
	}

GeomFont2D::HersheyData GeomFont2D::_get_hershey_data() const {
	switch (_hershey_font) {
		case HERSHEY_FUTURAL:
			return HERSHEY_ENTRY(futural);
		case HERSHEY_FUTURAM:
			return HERSHEY_ENTRY(futuram);
		case HERSHEY_ROWMANS:
			return HERSHEY_ENTRY(rowmans);
		case HERSHEY_ROWMAND:
			return HERSHEY_ENTRY(rowmand);
		case HERSHEY_ROWMANT:
			return HERSHEY_ENTRY(rowmant);
		case HERSHEY_SCRIPTS:
			return HERSHEY_ENTRY(scripts);
		case HERSHEY_SCRIPTC:
			return HERSHEY_ENTRY(scriptc);
		case HERSHEY_CURSIVE:
			return HERSHEY_ENTRY(cursive);
		case HERSHEY_GOTHICENG:
			return HERSHEY_ENTRY(gothiceng);
		case HERSHEY_GOTHICGER:
			return HERSHEY_ENTRY(gothicger);
		case HERSHEY_GOTHICITA:
			return HERSHEY_ENTRY(gothicita);
		case HERSHEY_GOTHGBT:
			return HERSHEY_ENTRY(gothgbt);
		case HERSHEY_GOTHGRT:
			return HERSHEY_ENTRY(gothgrt);
		case HERSHEY_GOTHITT:
			return HERSHEY_ENTRY(gothitt);
		case HERSHEY_TIMESI:
			return HERSHEY_ENTRY(timesi);
		case HERSHEY_TIMESR:
			return HERSHEY_ENTRY(timesr);
		case HERSHEY_TIMESIB:
			return HERSHEY_ENTRY(timesib);
		case HERSHEY_TIMESRB:
			return HERSHEY_ENTRY(timesrb);
		case HERSHEY_TIMESG:
			return HERSHEY_ENTRY(timesg);
		case HERSHEY_CYRILLIC:
			return HERSHEY_ENTRY(cyrillic);
		case HERSHEY_CYRILC_1:
			return HERSHEY_ENTRY(cyrilc_1);
		case HERSHEY_GREEK:
			return HERSHEY_ENTRY(greek);
		case HERSHEY_GREEKC:
			return HERSHEY_ENTRY(greekc);
		case HERSHEY_GREEKS:
			return HERSHEY_ENTRY(greeks);
		case HERSHEY_JAPANESE:
			return HERSHEY_ENTRY(japanese);
		case HERSHEY_SYMBOLIC:
			return HERSHEY_ENTRY(symbolic);
		case HERSHEY_MUSIC:
			return HERSHEY_ENTRY(music);
		case HERSHEY_MATHLOW:
			return HERSHEY_ENTRY(mathlow);
		case HERSHEY_MATHUPP:
			return HERSHEY_ENTRY(mathupp);
		case HERSHEY_ASTROLOGY:
			return HERSHEY_ENTRY(astrology);
		case HERSHEY_METEOROLOGY:
			return HERSHEY_ENTRY(meteorology);
		case HERSHEY_MARKERS:
			return HERSHEY_ENTRY(markers);
		default:
			return HERSHEY_ENTRY(futural);
	}
}

#undef HERSHEY_ENTRY

// --- Mesh Builders ---

void GeomFont2D::_build_easy_font_mesh() {
	Vector<TextSpan> spans = _parse_bbcode(_text);

	PoolVector3Array all_verts;
	PoolColorArray all_colors;
	PoolIntArray all_indexes;
	int vert_offset = 0;

	for (int s = 0; s < spans.size(); s++) {
		const TextSpan &span = spans[s];
		if (span.text.empty()) {
			continue;
		}

		stb_easy_font_spacing(_letter_spacing);
		PoolVector2Array verts_2d;
		const int nquads = stb_easy_font_print(0, 0, span.text.ascii().c_str(), verts_2d);

		for (int i = 0; i < verts_2d.size(); i++) {
			const Vector2 &v = verts_2d[i];
			all_verts.push_back(Vector3(v.x * span.scale.x, v.y * span.scale.y, 0));
			all_colors.push_back(span.color);
		}

		for (int q = 0; q < nquads; q++) {
			const int v = vert_offset + q * 4;
			all_indexes.append_array(parray(v + 0, v + 1, v + 2, v + 0, v + 2, v + 3));
		}
		vert_offset += verts_2d.size();
	}

	if (all_verts.size() == 0) {
		return;
	}

	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);
	mesh_array[VS::ARRAY_VERTEX] = all_verts;
	mesh_array[VS::ARRAY_COLOR] = all_colors;
	mesh_array[VS::ARRAY_INDEX] = all_indexes;
	_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array);

	// Calculate text size
	const char *ptr = _text.ascii().c_str();
	_text_rect_size = Size2(stb_easy_font_width(ptr) * _font_scale.x, stb_easy_font_height(ptr) * _font_scale.y);
}

void GeomFont2D::_build_asteroids_mesh() {
	Vector<TextSpan> spans = _parse_bbcode(_text);

	PoolVector3Array all_verts;
	PoolColorArray all_colors;

	for (int s = 0; s < spans.size(); s++) {
		const TextSpan &span = spans[s];
		if (span.text.empty()) {
			continue;
		}

		PoolVector2Array data;
		Point2 pos(0, 0);
		const char *text = span.text.ascii().c_str();
		while (*text) {
			const Size2 adv = _draw_asteroid_glyph(data, *text, pos, span.scale);
			pos.x += adv.x;
			text++;
		}

		for (int i = 0; i < data.size(); i++) {
			const Vector2 &v = data[i];
			all_verts.push_back(Vector3(v.x, v.y, 0));
			all_colors.push_back(span.color);
		}
	}

	if (all_verts.size() == 0) {
		return;
	}

	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);
	mesh_array[VS::ARRAY_VERTEX] = all_verts;
	mesh_array[VS::ARRAY_COLOR] = all_colors;
	_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, mesh_array);

	_text_rect_size = Size2(_text.length() * 12 * _font_scale.x, 12 * _font_scale.y);
}

void GeomFont2D::_build_hp1345_mesh() {
	Vector<TextSpan> spans = _parse_bbcode(_text);

	PoolVector3Array all_verts;
	PoolColorArray all_colors;

	for (int s = 0; s < spans.size(); s++) {
		const TextSpan &span = spans[s];
		if (span.text.empty()) {
			continue;
		}

		PoolVector2Array data;
		Point2 pos(0, 0);
		for (int i = 0; i < span.text.length(); i++) {
			const CharType code = span.text[i];
			if (_prep_hp1345_glyph(data, code, pos, span.scale) || code == ' ') {
				const real_t adv = MAX(2, hp1345_meta[code].max_x - hp1345_meta[code].min_x) * span.scale.x;
				pos += Point2(adv + 2, 0);
			}
		}

		for (int i = 0; i < data.size(); i++) {
			const Vector2 &v = data[i];
			all_verts.push_back(Vector3(v.x, v.y, 0));
			all_colors.push_back(span.color);
		}
	}

	if (all_verts.size() == 0) {
		return;
	}

	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);
	mesh_array[VS::ARRAY_VERTEX] = all_verts;
	mesh_array[VS::ARRAY_COLOR] = all_colors;
	_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, mesh_array);

	_text_rect_size = _size_hp1345_text(_text, _font_scale);
}

void GeomFont2D::_build_bob3d_mesh() {
	// Bob3D uses its own color scheme, BBCode color not applicable (it has hardcoded face colors)
	bob_font_draw_string(_mesh, _text.ascii().c_str(), Point3(0, 0, 0), _font_scale.x, _bob3d_wireframe);
	_text_rect_size = Size2(_text.length() * (_font_scale.x * (BOBS_X + 1)), _font_scale.x * BOBS_Y);
}

void GeomFont2D::_build_hershey_mesh() {
	Vector<TextSpan> spans = _parse_bbcode(_text);
	HersheyData hd = _get_hershey_data();

	PoolVector3Array all_verts;
	PoolColorArray all_colors;

	for (int s = 0; s < spans.size(); s++) {
		const TextSpan &span = spans[s];
		if (span.text.empty()) {
			continue;
		}

		PoolVector2Array data;
		int width = 0;
		hershey_make(0, 0, span.text.ascii().c_str(), hd.font_data, hd.font_data_size, hd.font_width, data, &width);

		for (int i = 0; i < data.size(); i++) {
			const Vector2 &v = data[i];
			all_verts.push_back(Vector3(v.x * span.scale.x, -v.y * span.scale.y, 0));
			all_colors.push_back(span.color);
		}
	}

	if (all_verts.size() == 0) {
		return;
	}

	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);
	mesh_array[VS::ARRAY_VERTEX] = all_verts;
	mesh_array[VS::ARRAY_COLOR] = all_colors;
	_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, mesh_array);

	int total_width = hershey_string_width(_text.ascii().c_str(), hd.font_data, hd.font_data_size, hd.font_width);
	_text_rect_size = Size2(total_width * _font_scale.x, hd.font_height * _font_scale.y);
}

void GeomFont2D::_build_leon_mesh() {
	Vector<TextSpan> spans = _parse_bbcode(_text);

	PoolVector3Array all_verts;
	PoolColorArray all_colors;

	for (int s = 0; s < spans.size(); s++) {
		const TextSpan &span = spans[s];
		if (span.text.empty()) {
			continue;
		}

		PoolVector2Array data;
		leon_make_lines(span.text, FONT_HEIGHT * _font_scale.y, _leon_weight, data);

		for (int i = 0; i < data.size(); i++) {
			const Vector2 &v = data[i];
			all_verts.push_back(Vector3(v.x * span.scale.x, -v.y * span.scale.y, 0));
			all_colors.push_back(span.color);
		}
	}

	if (all_verts.size() == 0) {
		return;
	}

	Array mesh_array;
	mesh_array.resize(VS::ARRAY_MAX);
	mesh_array[VS::ARRAY_VERTEX] = all_verts;
	mesh_array[VS::ARRAY_COLOR] = all_colors;
	_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, mesh_array);

	_text_rect_size = Size2(
			leon_text_width(_text, FONT_HEIGHT * _font_scale.y) * _font_scale.x,
			FONT_HEIGHT * _font_scale.y);
}

void GeomFont2D::_build_lowpoly_mesh() {
	lowpoly_font_draw_string(_mesh, _text.ascii().c_str(), Point3(0, 0, 0), _font_scale.x);
	_text_rect_size = lowpoly_font_text_size(_text.ascii().c_str(), _font_scale.x);
}

// --- FB Font Builders (canvas-item based) ---

void GeomFont2D::_build_bitmap_dot(RID p_canvas) {
	FBBitmapFontView fnt(p_canvas);
	fnt.set_style(FBFontDotStyle(_bitmap_dot_style));
	fnt.set_text(_text);
	fnt.set_on_color(_font_color);
	fnt.draw(Point2());
	_text_rect_size = fnt.size_of_contents();
}

void GeomFont2D::_build_lcd(RID p_canvas) {
	FBLCDFontView fnt(p_canvas);
	fnt.set_text(_text);
	fnt.draw(Point2());
	_text_rect_size = fnt.size_of_contents();
}

void GeomFont2D::_build_square(RID p_canvas) {
	FBSquareFontView fnt(p_canvas);
	fnt.set_text(_text);
	fnt.draw(Point2());
	_text_rect_size = fnt.size_of_contents();
}

// --- Rebuild ---

void GeomFont2D::_rebuild() {
	_dirty = false;

	if (_text.empty()) {
		_mesh = Ref<ArrayMesh>();
		_text_rect_size = Size2();
		return;
	}

	if (_font_type <= GEOM_FONT_LOWPOLY) {
		// Mesh-based fonts
		if (_mesh.is_valid()) {
			_mesh->clear_mesh();
		} else {
			_mesh = Ref<ArrayMesh>(memnew(ArrayMesh));
		}

		switch (_font_type) {
			case GEOM_FONT_EASY:
				_build_easy_font_mesh();
				break;
			case GEOM_FONT_ASTEROIDS:
				_build_asteroids_mesh();
				break;
			case GEOM_FONT_HP1345:
				_build_hp1345_mesh();
				break;
			case GEOM_FONT_BOB3D:
				_build_bob3d_mesh();
				break;
			case GEOM_FONT_HERSHEY:
				_build_hershey_mesh();
				break;
			case GEOM_FONT_LEON:
				_build_leon_mesh();
				break;
			case GEOM_FONT_LOWPOLY:
				_build_lowpoly_mesh();
				break;
			default:
				break;
		}
	}
	// FB fonts are drawn directly in NOTIFICATION_DRAW

	item_rect_changed();
}

void GeomFont2D::_mark_dirty() {
	_dirty = true;
	update();
}

// --- Notification ---

void GeomFont2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			if (_text.empty()) {
				break;
			}
			if (_dirty) {
				_rebuild();
			}
			if (_font_type <= GEOM_FONT_LOWPOLY) {
				// Mesh-based fonts — use draw_mesh_3d for 3D transform support
				if (_mesh.is_valid() && _mesh->get_surface_count() > 0) {
					draw_mesh_3d(_mesh, Ref<Texture>(), Ref<Texture>(), Ref<Texture>(),
							_font_transform, _font_color);
				}
			} else {
				// Canvas-item-based (FB fonts)
				RID ci = get_canvas_item();
				switch (_font_type) {
					case GEOM_FONT_BITMAP_DOT:
						_build_bitmap_dot(ci);
						break;
					case GEOM_FONT_LCD:
						_build_lcd(ci);
						break;
					case GEOM_FONT_SQUARE:
						_build_square(ci);
						break;
					default:
						break;
				}
			}
		} break;
	}
}

// --- Property Validation ---

void GeomFont2D::_validate_property(PropertyInfo &property) const {
	if (property.name == "hershey_font" && _font_type != GEOM_FONT_HERSHEY) {
		property.usage = PROPERTY_USAGE_NOEDITOR;
	}
	if (property.name == "bob3d_wireframe" && _font_type != GEOM_FONT_BOB3D) {
		property.usage = PROPERTY_USAGE_NOEDITOR;
	}
	if (property.name == "leon_weight" && _font_type != GEOM_FONT_LEON) {
		property.usage = PROPERTY_USAGE_NOEDITOR;
	}
	if (property.name == "bitmap_dot_style" && _font_type != GEOM_FONT_BITMAP_DOT) {
		property.usage = PROPERTY_USAGE_NOEDITOR;
	}
	if (property.name == "line_width") {
		if (_font_type != GEOM_FONT_ASTEROIDS && _font_type != GEOM_FONT_HP1345 && _font_type != GEOM_FONT_HERSHEY && _font_type != GEOM_FONT_LEON) {
			property.usage = PROPERTY_USAGE_NOEDITOR;
		}
	}
	if (property.name == "letter_spacing") {
		if (_font_type != GEOM_FONT_EASY && _font_type != GEOM_FONT_HERSHEY) {
			property.usage = PROPERTY_USAGE_NOEDITOR;
		}
	}
	if (property.name == "font_transform") {
		if (_font_type > GEOM_FONT_LOWPOLY) {
			property.usage = PROPERTY_USAGE_NOEDITOR;
		}
	}
}

// --- Getters / Setters ---

void GeomFont2D::set_text(const String &p_text) {
	if (_text != p_text) {
		_text = p_text;
		_mark_dirty();
	}
}

String GeomFont2D::get_text() const {
	return _text;
}

void GeomFont2D::set_font_type(GeomFontType p_type) {
	if (_font_type != p_type) {
		_font_type = p_type;
		property_list_changed_notify();
		_mark_dirty();
	}
}

GeomFont2D::GeomFontType GeomFont2D::get_font_type() const {
	return _font_type;
}

void GeomFont2D::set_hershey_font(HersheyFont p_font) {
	if (_hershey_font != p_font) {
		_hershey_font = p_font;
		_mark_dirty();
	}
}

GeomFont2D::HersheyFont GeomFont2D::get_hershey_font() const {
	return _hershey_font;
}

void GeomFont2D::set_font_color(const Color &p_color) {
	if (_font_color != p_color) {
		_font_color = p_color;
		_mark_dirty();
	}
}

Color GeomFont2D::get_font_color() const {
	return _font_color;
}

void GeomFont2D::set_font_scale(const Vector2 &p_scale) {
	if (_font_scale != p_scale) {
		_font_scale = p_scale;
		_mark_dirty();
	}
}

Vector2 GeomFont2D::get_font_scale() const {
	return _font_scale;
}

void GeomFont2D::set_font_transform(const Transform &p_xform) {
	_font_transform = p_xform;
	update();
}

Transform GeomFont2D::get_font_transform() const {
	return _font_transform;
}

void GeomFont2D::set_leon_weight(real_t p_weight) {
	p_weight = CLAMP(p_weight, 1.0f, 900.0f);
	if (_leon_weight != p_weight) {
		_leon_weight = p_weight;
		_mark_dirty();
	}
}

real_t GeomFont2D::get_leon_weight() const {
	return _leon_weight;
}

void GeomFont2D::set_line_width(real_t p_width) {
	if (_line_width != p_width) {
		_line_width = p_width;
		_mark_dirty();
	}
}

real_t GeomFont2D::get_line_width() const {
	return _line_width;
}

void GeomFont2D::set_letter_spacing(real_t p_spacing) {
	if (_letter_spacing != p_spacing) {
		_letter_spacing = p_spacing;
		_mark_dirty();
	}
}

real_t GeomFont2D::get_letter_spacing() const {
	return _letter_spacing;
}

void GeomFont2D::set_bbcode_enabled(bool p_enabled) {
	if (_bbcode_enabled != p_enabled) {
		_bbcode_enabled = p_enabled;
		_mark_dirty();
	}
}

bool GeomFont2D::is_bbcode_enabled() const {
	return _bbcode_enabled;
}

void GeomFont2D::set_bob3d_wireframe(bool p_wire) {
	if (_bob3d_wireframe != p_wire) {
		_bob3d_wireframe = p_wire;
		_mark_dirty();
	}
}

bool GeomFont2D::is_bob3d_wireframe() const {
	return _bob3d_wireframe;
}

void GeomFont2D::set_bitmap_dot_style(int p_style) {
	if (_bitmap_dot_style != p_style) {
		_bitmap_dot_style = p_style;
		_mark_dirty();
	}
}

int GeomFont2D::get_bitmap_dot_style() const {
	return _bitmap_dot_style;
}

Size2 GeomFont2D::get_text_size() const {
	return _text_rect_size;
}

// --- Bind Methods ---

void GeomFont2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_text", "text"), &GeomFont2D::set_text);
	ClassDB::bind_method(D_METHOD("get_text"), &GeomFont2D::get_text);
	ClassDB::bind_method(D_METHOD("set_font_type", "type"), &GeomFont2D::set_font_type);
	ClassDB::bind_method(D_METHOD("get_font_type"), &GeomFont2D::get_font_type);
	ClassDB::bind_method(D_METHOD("set_hershey_font", "font"), &GeomFont2D::set_hershey_font);
	ClassDB::bind_method(D_METHOD("get_hershey_font"), &GeomFont2D::get_hershey_font);
	ClassDB::bind_method(D_METHOD("set_font_color", "color"), &GeomFont2D::set_font_color);
	ClassDB::bind_method(D_METHOD("get_font_color"), &GeomFont2D::get_font_color);
	ClassDB::bind_method(D_METHOD("set_font_scale", "scale"), &GeomFont2D::set_font_scale);
	ClassDB::bind_method(D_METHOD("get_font_scale"), &GeomFont2D::get_font_scale);
	ClassDB::bind_method(D_METHOD("set_font_transform", "xform"), &GeomFont2D::set_font_transform);
	ClassDB::bind_method(D_METHOD("get_font_transform"), &GeomFont2D::get_font_transform);
	ClassDB::bind_method(D_METHOD("set_leon_weight", "weight"), &GeomFont2D::set_leon_weight);
	ClassDB::bind_method(D_METHOD("get_leon_weight"), &GeomFont2D::get_leon_weight);
	ClassDB::bind_method(D_METHOD("set_line_width", "width"), &GeomFont2D::set_line_width);
	ClassDB::bind_method(D_METHOD("get_line_width"), &GeomFont2D::get_line_width);
	ClassDB::bind_method(D_METHOD("set_letter_spacing", "spacing"), &GeomFont2D::set_letter_spacing);
	ClassDB::bind_method(D_METHOD("get_letter_spacing"), &GeomFont2D::get_letter_spacing);
	ClassDB::bind_method(D_METHOD("set_bbcode_enabled", "enabled"), &GeomFont2D::set_bbcode_enabled);
	ClassDB::bind_method(D_METHOD("is_bbcode_enabled"), &GeomFont2D::is_bbcode_enabled);
	ClassDB::bind_method(D_METHOD("set_bob3d_wireframe", "wire"), &GeomFont2D::set_bob3d_wireframe);
	ClassDB::bind_method(D_METHOD("is_bob3d_wireframe"), &GeomFont2D::is_bob3d_wireframe);
	ClassDB::bind_method(D_METHOD("set_bitmap_dot_style", "style"), &GeomFont2D::set_bitmap_dot_style);
	ClassDB::bind_method(D_METHOD("get_bitmap_dot_style"), &GeomFont2D::get_bitmap_dot_style);
	ClassDB::bind_method(D_METHOD("get_text_size"), &GeomFont2D::get_text_size);

	// Font type enum
	BIND_ENUM_CONSTANT(GEOM_FONT_EASY);
	BIND_ENUM_CONSTANT(GEOM_FONT_ASTEROIDS);
	BIND_ENUM_CONSTANT(GEOM_FONT_HP1345);
	BIND_ENUM_CONSTANT(GEOM_FONT_BOB3D);
	BIND_ENUM_CONSTANT(GEOM_FONT_HERSHEY);
	BIND_ENUM_CONSTANT(GEOM_FONT_LEON);
	BIND_ENUM_CONSTANT(GEOM_FONT_LOWPOLY);
	BIND_ENUM_CONSTANT(GEOM_FONT_BITMAP_DOT);
	BIND_ENUM_CONSTANT(GEOM_FONT_LCD);
	BIND_ENUM_CONSTANT(GEOM_FONT_SQUARE);

	// Hershey sub-type enum
	BIND_ENUM_CONSTANT(HERSHEY_FUTURAL);
	BIND_ENUM_CONSTANT(HERSHEY_FUTURAM);
	BIND_ENUM_CONSTANT(HERSHEY_ROWMANS);
	BIND_ENUM_CONSTANT(HERSHEY_ROWMAND);
	BIND_ENUM_CONSTANT(HERSHEY_ROWMANT);
	BIND_ENUM_CONSTANT(HERSHEY_SCRIPTS);
	BIND_ENUM_CONSTANT(HERSHEY_SCRIPTC);
	BIND_ENUM_CONSTANT(HERSHEY_CURSIVE);
	BIND_ENUM_CONSTANT(HERSHEY_GOTHICENG);
	BIND_ENUM_CONSTANT(HERSHEY_GOTHICGER);
	BIND_ENUM_CONSTANT(HERSHEY_GOTHICITA);
	BIND_ENUM_CONSTANT(HERSHEY_GOTHGBT);
	BIND_ENUM_CONSTANT(HERSHEY_GOTHGRT);
	BIND_ENUM_CONSTANT(HERSHEY_GOTHITT);
	BIND_ENUM_CONSTANT(HERSHEY_TIMESI);
	BIND_ENUM_CONSTANT(HERSHEY_TIMESR);
	BIND_ENUM_CONSTANT(HERSHEY_TIMESIB);
	BIND_ENUM_CONSTANT(HERSHEY_TIMESRB);
	BIND_ENUM_CONSTANT(HERSHEY_TIMESG);
	BIND_ENUM_CONSTANT(HERSHEY_CYRILLIC);
	BIND_ENUM_CONSTANT(HERSHEY_CYRILC_1);
	BIND_ENUM_CONSTANT(HERSHEY_GREEK);
	BIND_ENUM_CONSTANT(HERSHEY_GREEKC);
	BIND_ENUM_CONSTANT(HERSHEY_GREEKS);
	BIND_ENUM_CONSTANT(HERSHEY_JAPANESE);
	BIND_ENUM_CONSTANT(HERSHEY_SYMBOLIC);
	BIND_ENUM_CONSTANT(HERSHEY_MUSIC);
	BIND_ENUM_CONSTANT(HERSHEY_MATHLOW);
	BIND_ENUM_CONSTANT(HERSHEY_MATHUPP);
	BIND_ENUM_CONSTANT(HERSHEY_ASTROLOGY);
	BIND_ENUM_CONSTANT(HERSHEY_METEOROLOGY);
	BIND_ENUM_CONSTANT(HERSHEY_MARKERS);

	// Bitmap dot style enum
	BIND_ENUM_CONSTANT(DOT_FLAT_CIRCLE);
	BIND_ENUM_CONSTANT(DOT_FLAT_SQUARE);
	BIND_ENUM_CONSTANT(DOT_TEXTURE_CIRCLE);
	BIND_ENUM_CONSTANT(DOT_TEXTURE_SQUARE);
	BIND_ENUM_CONSTANT(DOT_TEXTURE_3D_1);
	BIND_ENUM_CONSTANT(DOT_TEXTURE_3D_2);

	// Properties
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_MULTILINE_TEXT), "set_text", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "font_type", PROPERTY_HINT_ENUM, "Easy,Asteroids,HP1345,Bob3D,Hershey,Leon,LowPoly,BitmapDot,LCD,Square"), "set_font_type", "get_font_type");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "hershey_font", PROPERTY_HINT_ENUM,
						 "Futural,Futuram,Roman Simplex,Roman Duplex,Roman Triplex,"
						 "Script Simplex,Script Complex,Cursive,"
						 "Gothic English,Gothic German,Gothic Italian,"
						 "Gothic GB Triplex,Gothic GR Triplex,Gothic IT Triplex,"
						 "Times Italic,Times Roman,Times Italic Bold,Times Roman Bold,Times Greek,"
						 "Cyrillic,Cyrillic Alt,Greek,Greek Complex,Greek Simplex,"
						 "Japanese,Symbolic,Music,Math Lower,Math Upper,"
						 "Astrology,Meteorology,Markers"),
			"set_hershey_font", "get_hershey_font");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "leon_weight", PROPERTY_HINT_RANGE, "1,900,1"), "set_leon_weight", "get_leon_weight");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "font_color"), "set_font_color", "get_font_color");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "font_scale"), "set_font_scale", "get_font_scale");
	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM, "font_transform"), "set_font_transform", "get_font_transform");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "line_width", PROPERTY_HINT_RANGE, "0.5,10,0.5"), "set_line_width", "get_line_width");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "letter_spacing", PROPERTY_HINT_RANGE, "-5,20,0.5"), "set_letter_spacing", "get_letter_spacing");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "bbcode_enabled"), "set_bbcode_enabled", "is_bbcode_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "bob3d_wireframe"), "set_bob3d_wireframe", "is_bob3d_wireframe");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "bitmap_dot_style", PROPERTY_HINT_ENUM, "Flat Circle,Flat Square,Texture Circle,Texture Square,Texture 3D 1,Texture 3D 2"), "set_bitmap_dot_style", "get_bitmap_dot_style");
}

// --- Constructor ---

GeomFont2D::GeomFont2D() {
	_text = "";
	_font_type = GEOM_FONT_EASY;
	_hershey_font = HERSHEY_FUTURAL;
	_font_color = Color(1, 1, 1, 1);
	_font_scale = Vector2(1, 1);
	_font_transform = Transform();
	_leon_weight = 200;
	_line_width = 1.0;
	_letter_spacing = 0.0;
	_bbcode_enabled = false;
	_bob3d_wireframe = true;
	_bitmap_dot_style = DOT_FLAT_CIRCLE;
	_dirty = false;
	_text_rect_size = Size2();
}
