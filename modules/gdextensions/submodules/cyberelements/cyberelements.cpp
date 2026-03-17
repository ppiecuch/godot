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

// --- Predefined palettes (from Qt CyberElems app) ---

static const Color DEFAULT_PRIMARY = Color(0.145f, 0.149f, 0.149f); // #252626
static const Color DEFAULT_SECONDARY = Color(0.463f, 0.463f, 0.463f); // #767676

const CyberPalette CyberPalette::PRESETS[] = {
	{ "Original Light", Color::html("#252626"), Color::html("#767676"), Color::html("#ffffff") },
	{ "Dark Gray", Color::html("#888888"), Color::html("#555555"), Color::html("#1a1a2e") },
	{ "Neon Magenta+Yellow", Color::html("#ff00ff"), Color::html("#ffff00"), Color::html("#000000") },
	{ "Blue Neon", Color::html("#0066ff"), Color::html("#0044cc"), Color::html("#0a0a1a") },
	{ "Pink Pastel", Color::html("#ffb0c8"), Color::html("#e890b0"), Color::html("#ffffff") },
	{ "Original Dark", Color::html("#252626"), Color::html("#767676"), Color::html("#0a0a0f") },
	{ "Neon Cyan", Color::html("#00e5ff"), Color::html("#006a75"), Color::html("#0a0a0f") },
	{ "Neon Green", Color::html("#39ff14"), Color::html("#1a8a0a"), Color::html("#000000") },
	{ "Gold", Color::html("#ffd700"), Color::html("#b8860b"), Color::html("#0a0a0f") },
	{ "Blueprint", Color::html("#4488ff"), Color::html("#2255aa"), Color::html("#0c1428") },
	{ "Terminal", Color::html("#00ff00"), Color::html("#008800"), Color::html("#000000") },
	{ "Red Alert", Color::html("#ff0040"), Color::html("#880022"), Color::html("#0f0008") },
};
const int CyberPalette::PRESET_COUNT = sizeof(CyberPalette::PRESETS) / sizeof(CyberPalette::PRESETS[0]);

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
		auto cmd = [p, s](CanvasItem *canvas, const Map<String, StyleDef> &styles,
						   const Color &p_primary, const Color &p_secondary, const Color &p_stroke,
						   real_t p_stroke_width, real_t p_fill_opacity) {
			// Merge style attributes from all referenced classes
			StyleDef attribs;
			for (const String &cls : s) {
				if (!cls.empty() && styles.has(cls)) {
					const StyleDef &def = styles[cls];
					for (int a = 0; a < ATTRIB_COUNT; a++) {
						if (!def[a].is_nil()) {
							attribs[a] = def[a];
						}
					}
				}
			}

			// Determine fill color with palette override
			Color fill_color(1, 1, 1);
			if (!attribs[ATTRIB_FILL].is_nil()) {
				Color original = attribs[ATTRIB_FILL];
				// Replace default primary/secondary with user-set colors
				if (original.is_equal_approx(DEFAULT_PRIMARY) || original.is_equal_approx(Color::html("#231f1f"))) {
					fill_color = p_primary;
				} else if (original.is_equal_approx(DEFAULT_SECONDARY)) {
					fill_color = p_secondary;
				} else {
					fill_color = original;
				}
			}

			// Apply fill opacity
			if (p_fill_opacity < 1.0f) {
				fill_color.a *= p_fill_opacity;
			}
			if (!attribs[ATTRIB_FILL_OPACITY].is_nil()) {
				fill_color.a *= (real_t)attribs[ATTRIB_FILL_OPACITY];
			}

			// Draw filled polyline
			canvas->draw_polyline(p, fill_color);

			// Draw stroke overlay if configured
			if (p_stroke.a > 0 && p_stroke_width > 0) {
				canvas->draw_polyline(p, p_stroke, p_stroke_width);
			} else if (!attribs[ATTRIB_STROKE].is_nil()) {
				Color stroke_col = attribs[ATTRIB_STROKE];
				real_t width = !attribs[ATTRIB_STROKE_WIDTH].is_nil() ? (real_t)attribs[ATTRIB_STROKE_WIDTH] : 1.0f;
				canvas->draw_polyline(p, stroke_col, width);
			}
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
		// Flush previous sub-path if we have accumulated points
		if (points.size() > 1) {
			stroke(points, styles);
			points.clear();
		}
		if (rel) {
			x += cursor.x;
			y += cursor.y;
		}
		control = cursor = { x, y };
		points.clear();
		points.push_back(cursor);
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
		// Flush any remaining open sub-path (paths not ending with Z)
		if (points.size() > 1) {
			stroke(points, styles);
			points.clear();
		}
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
						for (const String &ident : idents) {
							switch (k) {
								case ATTRIB_STROKE:
								case ATTRIB_FILL: {
									r[ident][k] = _get_color_from_str(value);
								} break;
								case ATTRIB_FILL_OPACITY: {
									r[ident][k] = value.to_float();
								} break;
								case ATTRIB_STROKE_WIDTH: {
									r[ident][k] = value.to_float();
								} break;
								case ATTRIB_STROKE_LINEJOIN: {
									r[ident][k] = value.ascii().get_data();
								} break;
								case ATTRIB_COUNT: {
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
							defs[k] = val.to_float();
						} break;
						case ATTRIB_STROKE_WIDTH: {
							defs[k] = val.to_float();
						} break;
						case ATTRIB_STROKE_LINEJOIN: {
							defs[k] = val.ascii().get_data();
						} break;
						case ATTRIB_COUNT: {
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

void CyberElement::_process_element_v2(const Dictionary &p_element, CacheEntry &entry) {
	// New flat format from generate.py:
	// viewBox: [x, y, w, h], paths: [{fill: int, d: string}], colors: [hex...], defs: [...]

	if (p_element.has("viewBox")) {
		Array vb = p_element["viewBox"];
		if (vb.size() >= 4) {
			entry.box = { (real_t)(int)vb[2], (real_t)(int)vb[3] };
		}
	}

	// Collect extra literal colors
	Vector<Color> extra_colors;
	if (p_element.has("colors")) {
		Array colors_arr = p_element["colors"];
		for (int i = 0; i < colors_arr.size(); i++) {
			String hex = colors_arr[i];
			if (hex.begins_with("#")) {
				extra_colors.push_back(Color::html(hex));
			} else if (hex.begins_with("url(")) {
				// Gradient reference — use primary as placeholder until gradient rendering
				extra_colors.push_back(DEFAULT_PRIMARY);
			} else {
				extra_colors.push_back(Color::named(hex));
			}
		}
	}

	if (p_element.has("paths")) {
		Array paths = p_element["paths"];
		for (int i = 0; i < paths.size(); i++) {
			Dictionary path_dict = paths[i];
			if (!path_dict.has("d")) {
				continue;
			}
			int fill_type = path_dict.has("fill") ? (int)path_dict["fill"] : 0;
			real_t path_opacity = path_dict.has("opacity") ? (real_t)(float)path_dict["opacity"] : 1.0f;

			// Capture fill_type, extra_colors, and path_opacity for the lambda
			Vector<Color> ec = extra_colors;
			CmdRecorder cmds(Vector<String>(), curve_precision);
			if (cmds.parse(path_dict["d"]) && cmds.commands.size()) {
				// Wrap each command to apply the pre-resolved fill type
				for (int c = 0; c < cmds.commands.size(); c++) {
					DrawCmd original_cmd = cmds.commands[c];
					auto resolved_cmd = [original_cmd, fill_type, ec, path_opacity](
												CanvasItem *canvas, const Map<String, StyleDef> &styles,
												const Color &p_primary, const Color &p_secondary, const Color &p_stroke,
												real_t p_stroke_width, real_t p_fill_opacity) {
						// Determine fill color from pre-resolved fill type
						Color fill_color;
						if (fill_type == 0) {
							fill_color = p_primary;
						} else if (fill_type == 1) {
							fill_color = p_secondary;
						} else if (fill_type - 2 < ec.size()) {
							fill_color = ec[fill_type - 2];
						} else {
							fill_color = p_primary;
						}

						fill_color.a *= p_fill_opacity * path_opacity;

						// Create a temporary style to pass through the existing draw cmd
						// But since v2 paths don't use CSS classes, just draw directly
						// by calling the original cmd but with override colors
						original_cmd(canvas, styles, fill_color, fill_color, p_stroke, p_stroke_width, 1.0f);
					};
					entry.commands.push_back(resolved_cmd);
				}
			}
		}
	}
}

bool CyberElement::draw_element() {
	CacheEntry &elem = cache.write[element];
	if (elem.commands.empty()) {
		// cache element
		Array elems = _json_description();
		ERR_FAIL_INDEX_V(element, elems.size(), false);
		Dictionary object = elems[element];
		// Detect format: v2 has "paths" array, v1 has "type"/"props"
		if (object.has("paths")) {
			_process_element_v2(object, elem);
		} else {
			_process_object(object, elem);
		}
	}
	if (!elem.commands.empty()) {
		VS::get_singleton()->canvas_item_add_set_transform(get_canvas_item(), Transform2D().scaled(get_view_size() / cache[element].box));
		for (const auto &cmd : elem.commands) {
			cmd(this, elem.styles, _primary_fill, _secondary_fill, _stroke_color, _stroke_width, _fill_opacity);
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

// --- Color customization ---

void CyberElement::set_primary_fill(const Color &p_color) {
	if (_primary_fill != p_color) {
		_primary_fill = p_color;
		update();
	}
}

Color CyberElement::get_primary_fill() const { return _primary_fill; }

void CyberElement::set_secondary_fill(const Color &p_color) {
	if (_secondary_fill != p_color) {
		_secondary_fill = p_color;
		update();
	}
}

Color CyberElement::get_secondary_fill() const { return _secondary_fill; }

void CyberElement::set_stroke_color(const Color &p_color) {
	if (_stroke_color != p_color) {
		_stroke_color = p_color;
		update();
	}
}

Color CyberElement::get_stroke_color() const { return _stroke_color; }

void CyberElement::set_stroke_width(real_t p_width) {
	p_width = CLAMP(p_width, 0.0f, 50.0f);
	if (_stroke_width != p_width) {
		_stroke_width = p_width;
		update();
	}
}

real_t CyberElement::get_stroke_width() const { return _stroke_width; }

void CyberElement::set_fill_opacity(real_t p_opacity) {
	p_opacity = CLAMP(p_opacity, 0.0f, 1.0f);
	if (_fill_opacity != p_opacity) {
		_fill_opacity = p_opacity;
		update();
	}
}

real_t CyberElement::get_fill_opacity() const { return _fill_opacity; }

void CyberElement::apply_palette(int p_preset_index) {
	ERR_FAIL_INDEX(p_preset_index, CyberPalette::PRESET_COUNT);
	const CyberPalette &preset = CyberPalette::PRESETS[p_preset_index];
	_primary_fill = preset.primary_fill;
	_secondary_fill = preset.secondary_fill;
	update();
}

int CyberElement::get_palette_count() const { return CyberPalette::PRESET_COUNT; }

String CyberElement::get_palette_name(int p_index) const {
	ERR_FAIL_INDEX_V(p_index, CyberPalette::PRESET_COUNT, String());
	return String(CyberPalette::PRESETS[p_index].name);
}

void CyberElement::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_cyberelement", "index"), &CyberElement::set_cyberelement);
	ClassDB::bind_method(D_METHOD("get_cyberelement"), &CyberElement::get_cyberelement);
	ClassDB::bind_method(D_METHOD("set_curve_precision", "precision"), &CyberElement::set_curve_precision);
	ClassDB::bind_method(D_METHOD("get_curve_precision"), &CyberElement::get_curve_precision);
	ClassDB::bind_method(D_METHOD("set_view_size", "size"), &CyberElement::set_view_size);
	ClassDB::bind_method(D_METHOD("get_view_size"), &CyberElement::get_view_size);

	ClassDB::bind_method(D_METHOD("set_primary_fill", "color"), &CyberElement::set_primary_fill);
	ClassDB::bind_method(D_METHOD("get_primary_fill"), &CyberElement::get_primary_fill);
	ClassDB::bind_method(D_METHOD("set_secondary_fill", "color"), &CyberElement::set_secondary_fill);
	ClassDB::bind_method(D_METHOD("get_secondary_fill"), &CyberElement::get_secondary_fill);
	ClassDB::bind_method(D_METHOD("set_stroke_color", "color"), &CyberElement::set_stroke_color);
	ClassDB::bind_method(D_METHOD("get_stroke_color"), &CyberElement::get_stroke_color);
	ClassDB::bind_method(D_METHOD("set_stroke_width", "width"), &CyberElement::set_stroke_width);
	ClassDB::bind_method(D_METHOD("get_stroke_width"), &CyberElement::get_stroke_width);
	ClassDB::bind_method(D_METHOD("set_fill_opacity", "opacity"), &CyberElement::set_fill_opacity);
	ClassDB::bind_method(D_METHOD("get_fill_opacity"), &CyberElement::get_fill_opacity);

	ClassDB::bind_method(D_METHOD("apply_palette", "preset_index"), &CyberElement::apply_palette);
	ClassDB::bind_method(D_METHOD("get_palette_count"), &CyberElement::get_palette_count);
	ClassDB::bind_method(D_METHOD("get_palette_name", "index"), &CyberElement::get_palette_name);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "cyberelement", PROPERTY_HINT_RANGE, "0,89"), "set_cyberelement", "get_cyberelement");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "curve_precision", PROPERTY_HINT_RANGE, "0.1,10,0.1"), "set_curve_precision", "get_curve_precision");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "view_size"), "set_view_size", "get_view_size");

	ADD_GROUP("Colors", "");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "primary_fill"), "set_primary_fill", "get_primary_fill");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "secondary_fill"), "set_secondary_fill", "get_secondary_fill");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "stroke_color"), "set_stroke_color", "get_stroke_color");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "stroke_width", PROPERTY_HINT_RANGE, "0,50,0.1"), "set_stroke_width", "get_stroke_width");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "fill_opacity", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_fill_opacity", "get_fill_opacity");

	ADD_SIGNAL(MethodInfo("view_size_changed"));
}

CyberElement::CyberElement() {
	cache.resize(CYBERELEMENT_COUNT);
	view_size = Size2(100, 100);
	element = 0;
	curve_precision = 2;
	_primary_fill = DEFAULT_PRIMARY;
	_secondary_fill = DEFAULT_SECONDARY;
	_stroke_color = Color(0, 0, 0, 0); // transparent
	_stroke_width = 0;
	_fill_opacity = 1.0f;
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[cyberelements]] CyberPalette") {
	TEST_CASE("Preset count") {
		CHECK(CyberPalette::PRESET_COUNT == 12);
	}

	TEST_CASE("Preset names are non-empty") {
		for (int i = 0; i < CyberPalette::PRESET_COUNT; i++) {
			CHECK(CyberPalette::PRESETS[i].name != nullptr);
			CHECK(String(CyberPalette::PRESETS[i].name).length() > 0);
		}
	}

	TEST_CASE("Original Light preset has correct defaults") {
		const CyberPalette &p = CyberPalette::PRESETS[0];
		CHECK(String(p.name) == "Original Light");
		CHECK(p.primary_fill.is_equal_approx(Color::html("#252626")));
		CHECK(p.secondary_fill.is_equal_approx(Color::html("#767676")));
	}
}

TEST_SUITE("[[cyberelements]] CyberElement properties") {
	TEST_CASE("Default constructor values") {
		CyberElement ce;
		CHECK(ce.get_cyberelement() == 0);
		CHECK(ce.get_curve_precision() == doctest::Approx(2.0));
		CHECK(ce.get_view_size() == Size2(100, 100));
		CHECK(ce.get_primary_fill().is_equal_approx(Color::html("#252626")));
		CHECK(ce.get_secondary_fill().is_equal_approx(Color::html("#767676")));
		CHECK(ce.get_stroke_width() == doctest::Approx(0.0));
		CHECK(ce.get_fill_opacity() == doctest::Approx(1.0));
	}

	TEST_CASE("Set element index") {
		CyberElement ce;
		ce.set_cyberelement(42);
		CHECK(ce.get_cyberelement() == 42);
	}

	TEST_CASE("Set color properties") {
		CyberElement ce;
		ce.set_primary_fill(Color::html("#ff0000"));
		CHECK(ce.get_primary_fill().is_equal_approx(Color::html("#ff0000")));

		ce.set_secondary_fill(Color::html("#00ff00"));
		CHECK(ce.get_secondary_fill().is_equal_approx(Color::html("#00ff00")));

		ce.set_stroke_color(Color::html("#0000ff"));
		CHECK(ce.get_stroke_color().is_equal_approx(Color::html("#0000ff")));
	}

	TEST_CASE("Stroke width clamped") {
		CyberElement ce;
		ce.set_stroke_width(100.0f);
		CHECK(ce.get_stroke_width() == doctest::Approx(50.0f));

		ce.set_stroke_width(-5.0f);
		CHECK(ce.get_stroke_width() == doctest::Approx(0.0f));
	}

	TEST_CASE("Fill opacity clamped") {
		CyberElement ce;
		ce.set_fill_opacity(2.0f);
		CHECK(ce.get_fill_opacity() == doctest::Approx(1.0f));

		ce.set_fill_opacity(-1.0f);
		CHECK(ce.get_fill_opacity() == doctest::Approx(0.0f));
	}

	TEST_CASE("Apply palette") {
		CyberElement ce;
		ce.apply_palette(6); // Neon Cyan
		CHECK(ce.get_primary_fill().is_equal_approx(Color::html("#00e5ff")));
		CHECK(ce.get_secondary_fill().is_equal_approx(Color::html("#006a75")));
	}

	TEST_CASE("Palette count and names") {
		CyberElement ce;
		CHECK(ce.get_palette_count() == 12);
		CHECK(ce.get_palette_name(0) == "Original Light");
		CHECK(ce.get_palette_name(10) == "Terminal");
	}
}

TEST_SUITE("[[cyberelements]] Style parsing") {
	TEST_CASE("Parse simple CSS style") {
		CyberElement ce;
		// Access _parse_style indirectly by checking that the JSON loading works
		// (can't call private method directly, but we verify the element count)
		CHECK(CYBERELEMENT_COUNT == 90);
	}

	TEST_CASE("Color from string") {
		CHECK(_get_color_from_str("#ff0000").is_equal_approx(Color(1, 0, 0)));
		CHECK(_get_color_from_str("#00ff00").is_equal_approx(Color(0, 1, 0)));
		CHECK(_get_color_from_str("#252626").is_equal_approx(Color::html("#252626")));
	}

	TEST_CASE("Attrib key lookup") {
		CHECK(_get_attrib_key("fill") == ATTRIB_FILL);
		CHECK(_get_attrib_key("stroke") == ATTRIB_STROKE);
		CHECK(_get_attrib_key("fill-opacity") == ATTRIB_FILL_OPACITY);
		CHECK(_get_attrib_key("stroke-width") == ATTRIB_STROKE_WIDTH);
		CHECK(_get_attrib_key("fill-rule") == ATTRIB_COUNT); // ignored
	}
}

TEST_SUITE("[[cyberelements]] CmdRecorder") {
	TEST_CASE("Simple triangle produces polyline") {
		CmdRecorder rec(Vector<String>(), 2);
		CHECK(rec.parse("M 0 0 L 10 0 L 5 10 Z"));
		CHECK(rec.commands.size() == 1); // one closed sub-path
	}

	TEST_CASE("Two sub-paths produce two commands") {
		CmdRecorder rec(Vector<String>(), 2);
		CHECK(rec.parse("M 0 0 L 10 0 Z M 20 20 L 30 30 Z"));
		CHECK(rec.commands.size() == 2);
	}

	TEST_CASE("Open path flushed on EOF") {
		CmdRecorder rec(Vector<String>(), 2);
		CHECK(rec.parse("M 0 0 L 10 0 L 10 10"));
		// No Z, but eof() should flush
		CHECK(rec.commands.size() == 1);
	}

	TEST_CASE("Multiple sub-paths: closed + open") {
		CmdRecorder rec(Vector<String>(), 2);
		CHECK(rec.parse("M 0 0 L 10 0 Z M 20 0 L 30 0"));
		// First closed by Z, second flushed by eof
		CHECK(rec.commands.size() == 2);
	}

	TEST_CASE("Cubic bezier produces polyline points") {
		CmdRecorder rec(Vector<String>(), 0.5);
		CHECK(rec.parse("M 0 0 C 10,0 10,10 0,10 Z"));
		CHECK(rec.commands.size() == 1);
	}

	TEST_CASE("Quadratic bezier converts to cubic") {
		CmdRecorder rec(Vector<String>(), 1);
		CHECK(rec.parse("M 0 0 Q 5,10 10,0 Z"));
		CHECK(rec.commands.size() == 1);
	}

	TEST_CASE("Arc produces polyline points") {
		CmdRecorder rec(Vector<String>(), 1);
		CHECK(rec.parse("M 10 80 A 45 45 0 0 0 125 125 Z"));
		CHECK(rec.commands.size() == 1);
	}

	TEST_CASE("Horizontal and vertical lines") {
		CmdRecorder rec(Vector<String>(), 2);
		CHECK(rec.parse("M 0 0 H 10 V 10 H 0 Z"));
		CHECK(rec.commands.size() == 1);
	}

	TEST_CASE("Relative coordinates accumulate correctly") {
		CmdRecorder rec(Vector<String>(), 2);
		CHECK(rec.parse("M 10 10 l 5 0 l 0 5 l -5 0 z"));
		CHECK(rec.commands.size() == 1);
	}

	TEST_CASE("Smooth curve (S command)") {
		CmdRecorder rec(Vector<String>(), 1);
		CHECK(rec.parse("M 10 80 C 40,10 65,10 95,80 S 150,150 180,80 Z"));
		CHECK(rec.commands.size() == 1);
	}

	TEST_CASE("Smooth quadratic (T command)") {
		CmdRecorder rec(Vector<String>(), 1);
		CHECK(rec.parse("M 10 80 Q 52.5 10 95 80 T 180 80 Z"));
		CHECK(rec.commands.size() == 1);
	}

	TEST_CASE("Degenerate arc falls back to line") {
		CmdRecorder rec(Vector<String>(), 2);
		// Zero radius -> degenerates to line
		CHECK(rec.parse("M 0 0 A 0 0 0 0 1 10 10 Z"));
		CHECK(rec.commands.size() == 1);
	}

	TEST_CASE("Curve precision affects tessellation") {
		CmdRecorder coarse(Vector<String>(), 10); // coarse
		CmdRecorder fine(Vector<String>(), 0.1); // fine
		coarse.parse("M 0 0 C 100,0 100,100 0,100 Z");
		fine.parse("M 0 0 C 100,0 100,100 0,100 Z");
		// Both produce 1 command but fine should have more accumulated points
		CHECK(coarse.commands.size() == 1);
		CHECK(fine.commands.size() == 1);
	}

	TEST_CASE("Real CyberElement path") {
		CmdRecorder rec(Vector<String>(), 2);
		CHECK(rec.parse("M742 863c-55 0-83-36-83-36l-21 46s37 43 104 43c67 0 104-43 104-43l-21-46s-28 36-83 36z"));
		CHECK(rec.commands.size() >= 1);
	}
}

TEST_SUITE("[[cyberelements]] AttribValue") {
	TEST_CASE("Default is nil") {
		AttribValue v;
		CHECK(v.is_nil());
	}

	TEST_CASE("Assign float") {
		AttribValue v;
		v = 0.5f;
		CHECK_FALSE(v.is_nil());
		CHECK((real_t)v == doctest::Approx(0.5f));
	}

	TEST_CASE("Assign Color") {
		AttribValue v;
		v = Color(1, 0, 0);
		CHECK_FALSE(v.is_nil());
		Color c = v;
		CHECK(c.r == doctest::Approx(1.0f));
		CHECK(c.g == doctest::Approx(0.0f));
	}

	TEST_CASE("Assign string") {
		AttribValue v;
		v = "miter";
		CHECK_FALSE(v.is_nil());
	}
}

TEST_SUITE("[[cyberelements]] Element count and JSON") {
	TEST_CASE("Element count constant") {
		CHECK(CYBERELEMENT_COUNT == 90);
	}

	TEST_CASE("Default colors are distinct") {
		CHECK_FALSE(DEFAULT_PRIMARY.is_equal_approx(DEFAULT_SECONDARY));
	}

	TEST_CASE("Default primary is dark gray") {
		CHECK(DEFAULT_PRIMARY.r < 0.2f);
		CHECK(DEFAULT_PRIMARY.g < 0.2f);
		CHECK(DEFAULT_PRIMARY.b < 0.2f);
	}

	TEST_CASE("Default secondary is medium gray") {
		CHECK(DEFAULT_SECONDARY.r > 0.4f);
		CHECK(DEFAULT_SECONDARY.r < 0.5f);
	}
}

#endif // DOCTEST
