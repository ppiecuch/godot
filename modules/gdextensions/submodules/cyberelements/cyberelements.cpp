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

static AttribKey _get_attrib_key(const String &key) {
	if (key == "fill") {
		return ATTRIB_FILL;
	} else if (key == "fill-opacity") {
		return ATTRIB_FILL_OPACITY;
	} else if (key == "stroke") {
		return ATTRIB_STROKE;
	} else if (key == "stroke-width") {
		return ATTRIB_STROKE_WIDTH;
	} else if (key == "stroke-linejoin") {
		return ATTRIB_STROKE_LINEJOIN;
	} else if (key == "fill-rule" || key == "fillRule") {
		return ATTRIB_COUNT; // ignore
	}
	ERR_FAIL_V_MSG(ATTRIB_COUNT, "Unsupported attribute: " + key);
}

static _FORCE_INLINE_ Color _get_color_from_str(const String &val) {
	if (val[0] == '#') {
		return Color::html(val);
	} else {
		return Color::named(val);
	}
}

struct CmdRecorder : public parser {
	Vector<String> styles;
	Vector<DrawCmd> commands;
	Point2 cursor;
	Vector2 control;
	Vector<Point2> points;
	real_t tol;

	void stroke(const Vector<Point2> &p, const Vector<String> &s) {
		auto cmd = [p, s](CanvasItem *canvas, const Map<String, StyleDef> &styles) {
			Color color(1, 1, 1);
			// merge and prep. attributes
			StyleDef attribs;
			for (const String &c : s) {
				print_line("style:" + c);
				if (!c.empty() && styles.has(c)) {
					const StyleDef &def = styles[c];
					// merge attributes
					for (int a = 0; a < ATTRIB_COUNT; a++) {
						if (!def[a].is_nil()) {
							attribs[a] = def[a];
						}
					}
				}
			}
			if (!attribs[ATTRIB_FILL].is_nil()) {
				color = attribs[ATTRIB_FILL];
			} else if (!attribs[ATTRIB_FILL_OPACITY].is_nil()) {
			} else if (!attribs[ATTRIB_STROKE].is_nil()) {
			} else if (!attribs[ATTRIB_STROKE_WIDTH].is_nil()) {
			} else if (!attribs[ATTRIB_STROKE_LINEJOIN].is_nil()) {
			}
			canvas->draw_polyline(p, color);
		};
		commands.push_back(cmd);
	}

	real_t dist_pt_seg(real_t x, real_t y, real_t px, real_t py, real_t qx, real_t qy) {
		const real_t pqx = qx - px;
		const real_t pqy = qy - py;
		const real_t d = pqx * pqx + pqy * pqy;
		real_t dx = x - px;
		real_t dy = y - py;
		real_t t = pqx * dx + pqy * dy;
		if (d > 0) {
			t /= d;
		}
		if (t < 0) {
			t = 0;
		} else if (t > 1) {
			t = 1;
		}
		dx = px + t * pqx - x;
		dy = py + t * pqy - y;
		return (dx * dx + dy * dy);
	}

	void cubic_bez(real_t x1, real_t y1, real_t x2, real_t y2, real_t x3, real_t y3, real_t x4, real_t y4, int level) {
		if (level > 12) {
			return;
		}
		const real_t x12 = (x1 + x2) * 0.5;
		const real_t y12 = (y1 + y2) * 0.5;
		const real_t x23 = (x2 + x3) * 0.5;
		const real_t y23 = (y2 + y3) * 0.5;
		const real_t x34 = (x3 + x4) * 0.5;
		const real_t y34 = (y3 + y4) * 0.5;
		const real_t x123 = (x12 + x23) * 0.5;
		const real_t y123 = (y12 + y23) * 0.5;
		const real_t x234 = (x23 + x34) * 0.5;
		const real_t y234 = (y23 + y34) * 0.5;
		const real_t x1234 = (x123 + x234) * 0.5;
		const real_t y1234 = (y123 + y234) * 0.5;

		const real_t d = dist_pt_seg(x1234, y1234, x1, y1, x4, y4);
		if (d > tol * tol) {
			cubic_bez(x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1);
			cubic_bez(x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1);
		} else {
			points.push_back({ x4, y4 });
		}
	}

	/// path interface

	void move_to(bool rel, real_t x, real_t y) {
		if (rel) {
			x += cursor.x;
			y += cursor.y;
		}
		control = cursor = { x, y };
		if (points.empty()) {
			points.push_back(cursor);
		}
	}

	void line_to(bool rel, real_t x, real_t y) {
		move_to(rel, x, y);
		points.push_back(cursor);
	}

	void horizontal_line_to(bool rel, real_t x) {
		if (rel) {
			x += cursor.x;
		}
		cursor.x = x;
		points.push_back(cursor);
	}
	void vertical_line_to(bool rel, real_t y) {
		if (rel) {
			y += cursor.y;
		}
		cursor.y = y;
		points.push_back(cursor);
	}
	void curve_to(bool rel, real_t x1, real_t y1, real_t x2, real_t y2, real_t x, real_t y) {
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
	void smooth_curve_to(bool rel, real_t x2, real_t y2, real_t x, real_t y) {
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

	void bezier_curve_to(bool rel, real_t x1, real_t y1, real_t x, real_t y) {
		if (rel) {
			x1 += cursor.x;
			y1 += cursor.y;
			x += cursor.x;
			y += cursor.y;
		}
		// convert to cubic bezier
		const real_t cx1 = cursor.x + 2.0 / 3.0 * (x1 - cursor.x);
		const real_t cy1 = cursor.y + 2.0 / 3.0 * (y1 - cursor.y);
		const real_t cx2 = x + 2.0 / 3.0 * (x1 - x);
		const real_t cy2 = y + 2.0 / 3.0 * (y1 - y);
		cubic_bez(cursor.x, cursor.y, cx1, cy1, cx2, cy2, x, y, 0);
		cursor = { x, y };
		control = { x1, y1 };
	}
	void smooth_bezier_curve_to(bool rel, real_t x, real_t y) {
		if (rel) {
			x += cursor.x;
			y += cursor.y;
		}
		const real_t x1 = 2 * cursor.x - control.x;
		const real_t y1 = 2 * cursor.y - control.y;
		// convert to cubix bezier
		const real_t cx1 = cursor.x + 2.0 / 3.0 * (x1 - cursor.x);
		const real_t cy1 = cursor.y + 2.0 / 3.0 * (y1 - cursor.y);
		const real_t cx2 = x + 2.0 / 3.0 * (x1 - x);
		const real_t cy2 = y + 2.0 / 3.0 * (y1 - y);
		cubic_bez(cursor.x, cursor.y, cx1, cy1, cx2, cy2, x, y, 0);
		cursor = { x, y };
		control = { x1, y1 };
	}

	static _FORCE_INLINE_ real_t __vecrat(real_t ux, real_t uy, real_t vx, real_t vy) {
		return (ux * vx + uy * vy) / (Math::sqrt(ux * ux + uy * uy) * Math::sqrt(vx * vx + vy * vy));
	}

	static _FORCE_INLINE_ real_t __vecang(real_t ux, real_t uy, real_t vx, real_t vy) {
		real_t r = __vecrat(ux, uy, vx, vy);
		if (r < -1) {
			r = -1;
		}
		if (r > 1) {
			r = 1;
		}
		return ((ux * vy < uy * vx) ? -1 : 1) * Math::acos(r);
	}

	static _FORCE_INLINE_ void __xformpoint(real_t *dx, real_t *dy, real_t x, real_t y, const real_t t[6]) {
		*dx = x * t[0] + y * t[2] + t[4];
		*dy = x * t[1] + y * t[3] + t[5];
	}

	static _FORCE_INLINE_ void __xformvec(real_t *dx, real_t *dy, real_t x, real_t y, const real_t t[6]) {
		*dx = x * t[0] + y * t[2];
		*dy = x * t[1] + y * t[3];
	}

	void elliptical_arc_to(bool rel, real_t rx, real_t ry, real_t x_rotation, bool large_arc, bool sweep, real_t x, real_t y) {
		// ported from canvg (https://code.google.com/p/canvg/)
		if (rel) {
			x += cursor.x;
			y += cursor.y;
		}
		const real_t dx = cursor.x - x;
		const real_t dy = cursor.y - y;
		const real_t d = Math::sqrt(dx * dx + dy * dy);
		if (d < 1e-6 || rx < 1e-6 || ry < 1e-6) {
			points.push_back({ x, y }); // the arc degenerates to a line
		} else {
			x_rotation = Math::deg2rad(x_rotation);
			const real_t sinrx = Math::sin(x_rotation);
			const real_t cosrx = Math::cos(x_rotation);
			// convert to center point parameterization.
			// http://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes

			// 1) compute x1', y1'
			const real_t x1p = cosrx * dx / 2 + sinrx * dy / 2;
			const real_t y1p = -sinrx * dx / 2 + cosrx * dy / 2;
			real_t d = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry);
			if (d > 1) {
				d = Math::sqrt(d);
				rx *= d;
				ry *= d;
			}
			// 2) compute cx', cy'
			real_t s = 0;
			real_t sa = (rx * rx) * (ry * ry) - (rx * rx) * (y1p * y1p) - (ry * ry) * (x1p * x1p);
			const real_t sb = (rx * rx) * (y1p * y1p) + (ry * ry) * (x1p * x1p);
			if (sa < 0) {
				sa = 0;
			}
			if (sb > 0) {
				s = Math::sqrt(sa / sb);
			}
			if (large_arc == sweep) {
				s = -s;
			}
			const real_t cxp = s * rx * y1p / ry;
			const real_t cyp = s * -ry * x1p / rx;
			// 3) compute cx,cy from cx',cy'
			const real_t cx = (cursor.x + x) / 2 + cosrx * cxp - sinrx * cyp;
			const real_t cy = (cursor.y + y) / 2 + sinrx * cxp + cosrx * cyp;
			// 4) calculate theta1, and delta theta.
			const real_t ux = (x1p - cxp) / rx;
			const real_t uy = (y1p - cyp) / ry;
			const real_t vx = (-x1p - cxp) / rx;
			const real_t vy = (-y1p - cyp) / ry;
			const real_t a1 = __vecang(1, 0, ux, uy); // initial angle
			real_t da = __vecang(ux, uy, vx, vy); // delta angle
			if (!sweep && da > 0) {
				da -= 2 * Math_PI;
			} else if (sweep && da < 0) {
				da += 2 * Math_PI;
			}
			// approximate the arc using cubic spline segments.
			const real_t t[6] = { cosrx, sinrx, -sinrx, cosrx, cx, cy };
			// split arc into max 90 degree segments;
			// the loop assumes an iteration per end point (including start and end), this +1.
			const int ndivs = (int)(Math::abs(da) / (Math_PI * 0.5) + 1);
			real_t hda = (da / ndivs) * 0.5;
			// fix for division by 0: avoid cotangens around 0 (infinite)
			if ((hda < 1e-3) && (hda > -1e-3)) {
				hda *= 0.5;
			} else {
				hda = (1 - Math::cos(hda)) / Math::sin(hda);
			}
			real_t kappa = Math::abs(4.0 / 3.0 * hda);
			if (da < 0) {
				kappa = -kappa;
			}
			real_t px = cursor.x, py = cursor.y, tanx, tany, ptanx = 0, ptany = 0;
			for (int i = 0; i <= ndivs; i++) {
				const real_t a = a1 + da * (real_t(i) / real_t(ndivs));
				const real_t dx = Math::cos(a);
				const real_t dy = Math::sin(a);
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
			stroke(points, styles);
			points.clear();
		}
	}
	void eof() {
	}

	CmdRecorder(const Vector<String> &styles = Vector<String>(), real_t tol = 2) :
			styles(styles), tol(tol) {}
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
// eg. ".CyberEl57_svg__fil1{fill:url(#CyberEl57_svg__id3)}"
Map<String, StyleDef> CyberElement::_parse_style(const String &p_style) {
	const CharString style = p_style.ascii();
	const char *s = style.c_str();
	const char *end = s + style.length();
	Map<String, StyleDef> r;
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
				String value = String(tok, s - tok);
				if (!value.empty()) {
					AttribKey k = _get_attrib_key(attrib);
					if (k < ATTRIB_COUNT) {
						for (const String &s : idents) {
							switch (k) {
								case ATTRIB_STROKE:
								case ATTRIB_FILL: {
									r[s][k] = _get_color_from_str(value);
								} break;
								case ATTRIB_FILL_OPACITY: {
								} break;
								case ATTRIB_STROKE_WIDTH: {
								} break;
								case ATTRIB_STROKE_LINEJOIN: {
								} break;
								case ATTRIB_COUNT: {
									// Unsupported
								} break;
							}
						}
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
		Vector<String> styles;
		if (object.has("className")) { // optional styles
			styles.append_array(String(object["className"]).split(" "));
		}
		if (object.has("style")) { // inline style
			String style_name = vformat("inline_%x_%x", OS::get_singleton()->get_ticks_msec(), Math::rand());
			StyleDef defs;
			Dictionary dict = object["style"];
			for (const auto &kv : dict) {
				const String val = kv.value;
				if (!val.empty()) {
					switch (AttribKey k = _get_attrib_key(kv.key)) {
						case ATTRIB_STROKE:
						case ATTRIB_FILL: {
							defs[k] = _get_color_from_str(val);
						} break;
						case ATTRIB_FILL_OPACITY: {
						} break;
						case ATTRIB_STROKE_WIDTH: {
						} break;
						case ATTRIB_STROKE_LINEJOIN: {
						} break;
						case ATTRIB_COUNT: {
							// Unsupported
						} break;
					}
				}
			}
			entry.styles[style_name] = defs;
			styles.push_back(style_name);
		}
		CmdRecorder cmds(styles, curve_precision);
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
				Map<String, StyleDef> defs = _parse_style(object["children"]);
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
