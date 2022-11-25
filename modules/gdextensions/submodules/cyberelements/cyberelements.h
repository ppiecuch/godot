/*************************************************************************/
/*  cyberelements.h                                                      */
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

#ifndef CYBERELEMENT_H
#define CYBERELEMENT_H

#include "core/map.h"
#include "core/vector.h"
#include "scene/2d/node_2d.h"

#define CYBERELEMENT_COUNT 90

typedef std::function<void(CanvasItem *canvas, const Map<String, Dictionary> &styles)> DrawCmd;

class CyberElement : public Node2D {
	GDCLASS(CyberElement, Node2D);

	int element;
	real_t curve_precision;
	struct CacheEntry {
		Size2 box{ 1, 1 };
		Vector<DrawCmd> commands;
		Map<String, Dictionary> styles;
	};
	Vector<CacheEntry> cache;
	Size2 view_size;

	Map<String, Dictionary> _parse_style(const String &p_style);
	void _process_object(const Dictionary &object, CacheEntry &entry);
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

	CyberElement();
};

#endif // CYBERELEMENT_H
