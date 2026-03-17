/**************************************************************************/
/*  cyberelements.h                                                       */
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

#ifndef CYBERELEMENT_H
#define CYBERELEMENT_H

#include "core/color.h"
#include "core/map.h"
#include "core/vector.h"
#include "scene/2d/node_2d.h"

#include <array>

#define CYBERELEMENT_COUNT 90

// Predefined color palette presets (from Qt CyberElems app)
struct CyberPalette {
	const char *name;
	Color primary_fill;
	Color secondary_fill;
	Color background;

	static const CyberPalette PRESETS[];
	static const int PRESET_COUNT;
};

// Known attributes
enum AttribKey {
	ATTRIB_FILL = 0,
	ATTRIB_FILL_OPACITY,
	ATTRIB_STROKE,
	ATTRIB_STROKE_WIDTH,
	ATTRIB_STROKE_LINEJOIN,
	ATTRIB_COUNT,
};

class AttribValue {
	union {
		Color c;
		const char *s;
		real_t f;
	};
	bool nil;

public:
	_FORCE_INLINE_ bool is_nil() const { return nil; }
	AttribValue &operator=(const real_t &v) {
		f = v;
		nil = false;
		return *this;
	}
	AttribValue &operator=(const Color &v) {
		c = v;
		nil = false;
		return *this;
	}
	AttribValue &operator=(const char *v) {
		s = v;
		nil = false;
		return *this;
	}
	operator real_t() const { return f; }
	operator Color() const { return c; }
	operator const char *() const { return s; }
	AttribValue() :
			nil(true) {}
	~AttribValue() {}
};

typedef std::array<AttribValue, ATTRIB_COUNT> StyleDef;
typedef std::function<void(CanvasItem *canvas, const Map<String, StyleDef> &styles, const Color &p_primary, const Color &p_secondary, const Color &p_stroke, real_t p_stroke_width, real_t p_fill_opacity)> DrawCmd;

class CyberElement : public Node2D {
	GDCLASS(CyberElement, Node2D);

	int element;
	real_t curve_precision;

	// Color customization (mirrors Qt CyberElems app)
	Color _primary_fill;
	Color _secondary_fill;
	Color _stroke_color;
	real_t _stroke_width;
	real_t _fill_opacity;

	struct CacheEntry {
		Size2 box{ 1, 1 };
		Vector<DrawCmd> commands;
		Map<String, StyleDef> styles;
	};
	Vector<CacheEntry> cache;
	Size2 view_size;

	Map<String, StyleDef> _parse_style(const String &p_style);
	void _process_object(const Dictionary &object, CacheEntry &entry);
	void _process_element_v2(const Dictionary &p_element, CacheEntry &entry);
	Array _json_description();

	bool draw_element();

protected:
	static void _bind_methods();
	void _notification(int p_notification);

public:
#ifdef TOOLS_ENABLED
	Dictionary _edit_get_state() const;
	void _edit_set_state(const Dictionary &p_state);
	bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
	Rect2 _edit_get_rect() const;
	void _edit_set_rect(const Rect2 &p_rect);
	bool _edit_use_rect() const;
#endif // TOOLS_ENABLED

	void set_cyberelement(int p_index);
	int get_cyberelement() const;

	void set_curve_precision(real_t p_prec);
	real_t get_curve_precision() const;

	Size2 get_view_size() const;
	void set_view_size(const Size2 &p_size);

	// Color customization
	void set_primary_fill(const Color &p_color);
	Color get_primary_fill() const;

	void set_secondary_fill(const Color &p_color);
	Color get_secondary_fill() const;

	void set_stroke_color(const Color &p_color);
	Color get_stroke_color() const;

	void set_stroke_width(real_t p_width);
	real_t get_stroke_width() const;

	void set_fill_opacity(real_t p_opacity);
	real_t get_fill_opacity() const;

	// Palette presets
	void apply_palette(int p_preset_index);
	int get_palette_count() const;
	String get_palette_name(int p_index) const;

	CyberElement();
};

#endif // CYBERELEMENT_H
