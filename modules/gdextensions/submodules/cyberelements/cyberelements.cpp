/**************************************************************************/
/*  cyberelements.cpp                                                     */
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

#include "cyberelements.h"

#include "core/io/json.h"
#include "core/os/os.h"
#include "svg_path/path.h"

#undef INCBIN_PREFIX
#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_SILENCE_BITCODE_WARNING

#include "misc/incbin.h"

INCBIN(cb_json, "submodules/cyberelements/cyberelements.json");

using namespace svg::types::parsers::path;

struct CmdRecorder : public parser {
	String style;
	Vector<DrawCmd> commands;
	Point2 cursor;
	Vector2 control;
	Vector<Point2> points;
	real_t tol;

	void stroke(const Vector<Point2> &p, const String &s) {
		auto cmd = [p, s](CanvasItem *canvas, const Map<String, Dictionary> &styles) {
			Color color(1, 1, 1);
			print_line("style:" + s);
			if (!s.empty() && styles.has(s)) {
				Dictionary defs = styles[s];
				if (defs.has("fill")) {
					color = defs["fill"];
				}
			}
			canvas->draw_polyline(p, color);
		};
		commands.push_back(cmd);
	}

	float dist_pt_seg(float x, float y, float px, float py, float qx, float qy) {
		const float pqx = qx - px;
		const float pqy = qy - py;
		const float d = pqx * pqx + pqy * pqy;
		float dx = x - px;
		float dy = y - py;
		float t = pqx * dx + pqy * dy;
		if (d > 0)
			t /= d;
		if (t < 0)
			t = 0;
		else if (t > 1)
			t = 1;
		dx = px + t * pqx - x;
		dy = py + t * pqy - y;
		return (dx * dx + dy * dy);
	}

	void cubic_bez(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int level) {
		if (level > 12) {
			return;
		}
		const float x12 = (x1 + x2) * 0.5;
		const float y12 = (y1 + y2) * 0.5;
		const float x23 = (x2 + x3) * 0.5;
		const float y23 = (y2 + y3) * 0.5;
		const float x34 = (x3 + x4) * 0.5;
		const float y34 = (y3 + y4) * 0.5;
		const float x123 = (x12 + x23) * 0.5;
		const float y123 = (y12 + y23) * 0.5;
		const float x234 = (x23 + x34) * 0.5;
		const float y234 = (y23 + y34) * 0.5;
		const float x1234 = (x123 + x234) * 0.5;
		const float y1234 = (y123 + y234) * 0.5;

		const float d = dist_pt_seg(x1234, y1234, x1, y1, x4, y4);
		if (d > tol * tol) {
			cubic_bez(x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1);
			cubic_bez(x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1);
		} else {
			points.push_back({ x4, y4 });
		}
	}

	/// path interface

	void move_to(bool rel, float x, float y) {
		if (rel) {
			x += cursor.x;
			y += cursor.y;
		}
		control = cursor = { x, y };
		if (points.empty()) {
			points.push_back(cursor);
		}
	}

	void line_to(bool rel, float x, float y) {
		move_to(rel, x, y);
		points.push_back(cursor);
	}

	void horizontal_line_to(bool rel, float x) {
		if (rel) {
			x += cursor.x;
		}
		cursor.x = x;
		points.push_back(cursor);
	}
	void vertical_line_to(bool rel, float y) {
		if (rel) {
			y += cursor.y;
		}
		cursor.y = y;
		points.push_back(cursor);
	}
	void curve_to(bool rel, float x1, float y1, float x2, float y2, float x, float y) {
		if (rel) {
			x1 += cursor.x;
			y1 += cursor.y;
			x2 += cursor.x;
			y2 += cursor.y;
			x += cursor.x;
			y += cursor.y;
		}
		cubic_bez(cursor.x, cursor.y, x1, y1, x2, y2, x, y, 0);
		cursor = { x, y };
		control = { x2, y2 };
	}
	void smooth_curve_to(bool rel, float x2, float y2, float x, float y) {
		if (rel) {
			x2 += cursor.x;
			y2 += cursor.y;
			x += cursor.x;
			y += cursor.y;
		}
		cubic_bez(cursor.x, cursor.y, control.x, control.y, x2, y2, x, y, 0);
		cursor = { x, y };
		control = { x2, y2 };
	}

	void bezier_curve_to(bool rel, float x1, float y1, float x, float y) {
		if (rel) {
			x1 += cursor.x;
			y1 += cursor.y;
			x += cursor.x;
			y += cursor.y;
		}
		// convert to cubic bezier
		const float cx1 = cursor.x + 2.0 / 3.0 * (x1 - cursor.x);
		const float cy1 = cursor.y + 2.0 / 3.0 * (y1 - cursor.y);
		const float cx2 = x + 2.0 / 3.0 * (x1 - x);
		const float cy2 = y + 2.0 / 3.0 * (y1 - y);
		cubic_bez(cursor.x, cursor.y, cx1, cy1, cx2, cy2, x, y, 0);
		cursor = { x, y };
		control = { x1, y1 };
	}
	void smooth_bezier_curve_to(bool rel, float x, float y) {
		if (rel) {
			x += cursor.x;
			y += cursor.y;
		}
		const float x1 = 2 * cursor.x - control.x;
		const float y1 = 2 * cursor.y - control.y;
		// convert to cubix bezier
		const float cx1 = cursor.x + 2.0 / 3.0 * (x1 - cursor.x);
		const float cy1 = cursor.y + 2.0 / 3.0 * (y1 - cursor.y);
		const float cx2 = x + 2.0 / 3.0 * (x1 - x);
		const float cy2 = y + 2.0 / 3.0 * (y1 - y);
		cubic_bez(cursor.x, cursor.y, cx1, cy1, cx2, cy2, x, y, 0);
		cursor = { x, y };
		control = { x1, y1 };
	}

	static _FORCE_INLINE_ float __vecrat(float ux, float uy, float vx, float vy) {
		return (ux * vx + uy * vy) / (Math::sqrt(ux * ux + uy * uy) * Math::sqrt(vx * vx + vy * vy));
	}

	static _FORCE_INLINE_ float __vecang(float ux, float uy, float vx, float vy) {
		float r = __vecrat(ux, uy, vx, vy);
		if (r < -1)
			r = -1;
		if (r > 1)
			r = 1;
		return ((ux * vy < uy * vx) ? -1 : 1) * Math::acos(r);
	}

	static _FORCE_INLINE_ void __xformpoint(float *dx, float *dy, float x, float y, const float t[6]) {
		*dx = x * t[0] + y * t[2] + t[4];
		*dy = x * t[1] + y * t[3] + t[5];
	}

	static _FORCE_INLINE_ void __xformvec(float *dx, float *dy, float x, float y, const float t[6]) {
		*dx = x * t[0] + y * t[2];
		*dy = x * t[1] + y * t[3];
	}

	void elliptical_arc_to(bool rel, float rx, float ry, float x_rotation, bool large_arc, bool sweep, float x, float y) {
		// ported from canvg (https://code.google.com/p/canvg/)
		if (rel) {
			x += cursor.x;
			y += cursor.y;
		}
		const float dx = cursor.x - x;
		const float dy = cursor.y - y;
		const float d = Math::sqrt(dx * dx + dy * dy);
		if (d < 1e-6 || rx < 1e-6 || ry < 1e-6) {
			points.push_back({ x, y }); // the arc degenerates to a line
		} else {
			x_rotation = Math::deg2rad(x_rotation);
			const float sinrx = Math::sin(x_rotation);
			const float cosrx = Math::cos(x_rotation);
			// convert to center point parameterization.
			// http://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes

			// 1) compute x1', y1'
			const float x1p = cosrx * dx / 2 + sinrx * dy / 2;
			const float y1p = -sinrx * dx / 2 + cosrx * dy / 2;
			float d = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry);
			if (d > 1) {
				d = Math::sqrt(d);
				rx *= d;
				ry *= d;
			}
			// 2) compute cx', cy'
			float s = 0.;
			float sa = (rx * rx) * (ry * ry) - (rx * rx) * (y1p * y1p) - (ry * ry) * (x1p * x1p);
			const float sb = (rx * rx) * (y1p * y1p) + (ry * ry) * (x1p * x1p);
			if (sa < 0)
				sa = 0;
			if (sb > 0)
				s = Math::sqrt(sa / sb);
			if (large_arc == sweep)
				s = -s;
			const float cxp = s * rx * y1p / ry;
			const float cyp = s * -ry * x1p / rx;
			// 3) compute cx,cy from cx',cy'
			const float cx = (cursor.x + x) / 2 + cosrx * cxp - sinrx * cyp;
			const float cy = (cursor.y + y) / 2 + sinrx * cxp + cosrx * cyp;
			// 4) calculate theta1, and delta theta.
			const float ux = (x1p - cxp) / rx;
			const float uy = (y1p - cyp) / ry;
			const float vx = (-x1p - cxp) / rx;
			const float vy = (-y1p - cyp) / ry;
			const float a1 = __vecang(1, 0, ux, uy); // initial angle
			float da = __vecang(ux, uy, vx, vy); // delta angle
			if (!sweep && da > 0) {
				da -= 2 * Math_PI;
			} else if (sweep && da < 0) {
				da += 2 * Math_PI;
			}
			// approximate the arc using cubic spline segments.
			const float t[6] = { cosrx, sinrx, -sinrx, cosrx, cx, cy };
			// split arc into max 90 degree segments;
			// the loop assumes an iteration per end point (including start and end), this +1.
			const int ndivs = (int)(Math::abs(da) / (Math_PI * 0.5) + 1);
			float hda = (da / ndivs) * 0.5;
			// fix for division by 0: avoid cotangens around 0 (infinite)
			if ((hda < 1e-3) && (hda > -1e-3)) {
				hda *= 0.5;
			} else {
				hda = (1 - Math::cos(hda)) / Math::sin(hda);
			}
			float kappa = Math::abs(4.0 / 3.0 * hda);
			if (da < 0)
				kappa = -kappa;
			float px = cursor.x, py = cursor.y, tanx, tany, ptanx = 0, ptany = 0;
			for (int i = 0; i <= ndivs; i++) {
				const float a = a1 + da * (float(i) / float(ndivs));
				const float dx = Math::cos(a);
				const float dy = Math::sin(a);
				__xformpoint(&x, &y, dx * rx, dy * ry, t); // position
				__xformvec(&tanx, &tany, -dy * rx * kappa, dx * ry * kappa, t); // tangent
				if (i > 0) {
					cubic_bez(px, py, px + ptanx, py + ptany, x - tanx, y - tany, x, y, 0);
				}
				px = x;
				py = y;
				ptanx = tanx;
				ptany = tany;
			}
		}
		cursor = control = { x, y };
		points.push_back(cursor);
	}

	void close_path() {
		if (!points.empty()) {
			points.push_back(points[0]); // close path
			stroke(points, style);
			points.clear();
		}
	}
	void eof() {
	}

	CmdRecorder(const String &style = "", real_t tol = 2) :
			style(style), tol(tol) {}
};

#ifdef TOOLS_ENABLED
Dictionary CyberElement::_edit_get_state() const {
	Dictionary state = Node2D::_edit_get_state();
	state["view_size"] = get_view_size();

	return state;
}

void CyberElement::_edit_set_state(const Dictionary &p_state) {
	Node2D::_edit_set_state(p_state);
	set_view_size(p_state["view_size"]);
}

bool CyberElement::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	return _edit_get_rect().has_point(p_point);
};

Rect2 CyberElement::_edit_get_rect() const {
	return Rect2(Point2(), get_view_size());
}

void CyberElement::_edit_set_rect(const Rect2 &p_rect) {
	set_view_size(p_rect.size);
	_change_notify();
}

bool CyberElement::_edit_use_rect() const {
	return true;
}
#endif // TOOLS_ENABLED

void CyberElement::set_cyberelement(int p_index) {
	ERR_FAIL_INDEX(p_index, CYBERELEMENT_COUNT);
	if (element != p_index) {
		element = p_index;
		update();
	}
}

int CyberElement::get_cyberelement() const {
	return element;
}

void CyberElement::set_curve_precision(real_t p_prec) {
	ERR_FAIL_COND(p_prec < 0.1 || p_prec > 10);
	if (curve_precision != p_prec) {
		curve_precision = p_prec;
		cache.clear();
		cache.resize(CYBERELEMENT_COUNT);
		update(); // clear and cache and rebuild curves with new precision
	}
}

real_t CyberElement::get_curve_precision() const {
	return curve_precision;
}

void CyberElement::set_view_size(const Size2 &p_size) {
	if (p_size != view_size) {
		view_size = p_size;
		item_rect_changed();
		emit_signal("view_size_changed");
		update();
	}
}

Size2 CyberElement::get_view_size() const {
	return view_size;
}

// extractig information from style string,
// eg. ".CyberEl1_svg__fil0,.CyberEl1_svg__fil1{fill:#252626;fill-rule:nonzero}.CyberEl1_svg__fil1{fill:#767676}"
Map<String, Dictionary> CyberElement::_parse_style(const String &p_style) {
	const CharString style = p_style.ascii();
	const char *s = style.c_str();
	const char *end = s + style.length();
	Map<String, Dictionary> r;
	Vector<String> idents;
	String attrib;
	const char *tok = s;
	while (s != end) {
		switch (*s) {
			case '.': { // ident
				tok = s + 1;
			} break;
			case ',': { // next ident
				idents.push_back(String(tok, s - tok));
				tok = s + 1;
			} break;
			case '{': { // ident{attrib
				idents.push_back(String(tok, s - tok));
				tok = s + 1;
			} break;
			case ':': { // attrib:value
				attrib = String(tok, s - tok);
				if (attrib.empty()) {
					WARN_PRINT("(SVG) Empty class attribute");
				}
				tok = s + 1;
			} break;
			case ';':
			case '}': {
				String raw = String(tok, s - tok);
				if (!raw.empty()) {
					Variant value;
					if (raw[0] == '#') {
						value = Color::html(raw);
					} else {
						value = raw;
					}
					for (const String &k : idents) {
						r[k][attrib] = value;
					}
				}
				if (*s == '}') {
					idents.clear();
				}
				tok = s + 1;
			} break;
		};
		s++;
	};
	return r;
}

Array CyberElement::_json_description() {
	static String json_string((const char *)cb_json_data, cb_json_size);
	static Variant data;
	if (!data.is_nil()) {
		return data;
	}
	String error_string;
	int error_line;
	Error err = JSON::parse(json_string, data, error_string, error_line);
	ERR_FAIL_COND_V_MSG(err != OK, Array(), String("Can not parse JSON ") + error_string + " on line " + rtos(error_line) + ".");
	return data;
}

void CyberElement::_process_object(const Dictionary &object, CacheEntry &entry) {
	if (object.has("viewBox")) {
		String box = object["viewBox"];
		Vector<String> elems = box.split(" ");
		entry.box = { elems[2].to_float(), elems[3].to_float() };
	}
	if (object.has("d")) { // contour definition
		String style;
		if (object.has("className")) { // optional
			style = object["className"];
		}
		if (object.has("style")) { // inline style
			style = vformat("inline_%x_%x", OS::get_singleton()->get_ticks_msec(), Math::rand());
			Dictionary defs;
			Dictionary dict = object["style"];
			for (const auto &kv : dict) {
				String val = kv.value;
				if (!val.empty()) {
					if (val[0] == '#') {
						defs[kv.key] = Color::html(val);
					} else {
						defs[kv.key] = kv.value;
					}
				}
			}
			entry.styles[style] = defs;
		}
		CmdRecorder cmds(style, curve_precision);
		if (cmds.parse(object["d"]) && cmds.commands.size()) {
			entry.commands.append_array(cmds.commands);
		}
	} else if (object.has("props")) {
		_process_object(object["props"], entry);
	} else if (object.has("children")) {
		switch (object["children"].get_type()) {
			case Variant::ARRAY: {
				for (const Variant &o : Array(object["children"])) {
					_process_object(o, entry);
				}
			} break;
			case Variant::STRING: {
				Map<String, Dictionary> defs = _parse_style(object["children"]);
				entry.styles = defs;
			} break;
			default: {
				_process_object(object["children"], entry);
			} break;
		}
	}
}

bool CyberElement::draw_element() {
	CacheEntry &elem = cache.write[element];
	if (elem.commands.empty()) {
		// cache element
		Array elems = _json_description();
		Dictionary object = elems[element];
		_process_object(object, elem);
	}
	if (!elem.commands.empty()) {
		VS::get_singleton()->canvas_item_add_set_transform(get_canvas_item(), Transform2D().scaled(get_view_size() / cache[element].box));
		for (const auto &cmd : elem.commands) {
			cmd(this, elem.styles);
		}
	}
	return !elem.commands.empty();
}

void CyberElement::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_READY: {
		} break;
		case NOTIFICATION_DRAW: {
			draw_element();
		} break;
		case NOTIFICATION_TRANSFORM_CHANGED: {
		} break;
	}
}

void CyberElement::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_cyberelement"), &CyberElement::set_cyberelement);
	ClassDB::bind_method(D_METHOD("get_cyberelement"), &CyberElement::get_cyberelement);
	ClassDB::bind_method(D_METHOD("set_curve_precision"), &CyberElement::set_curve_precision);
	ClassDB::bind_method(D_METHOD("get_curve_precision"), &CyberElement::get_curve_precision);
	ClassDB::bind_method(D_METHOD("set_view_size"), &CyberElement::set_view_size);
	ClassDB::bind_method(D_METHOD("get_view_size"), &CyberElement::get_view_size);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "cyberelement", PROPERTY_HINT_RANGE, "0,89"), "set_cyberelement", "get_cyberelement");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "curve_precision", PROPERTY_HINT_RANGE, "0.1,10,0.1"), "set_curve_precision", "get_curve_precision");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "view_size"), "set_view_size", "get_view_size");

	ADD_SIGNAL(MethodInfo("view_size_changed"));
}

CyberElement::CyberElement() {
	cache.resize(CYBERELEMENT_COUNT);
	view_size = Size2(100, 100);
	element = 0;
	curve_precision = 2;
}
