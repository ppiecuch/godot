/**************************************************************************/
/*  imspinner_imgui.h                                                     */
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

// This file should be use internally only
// and propably it should be use only once.

#ifndef IMSPINNER_IMGUI_H
#define IMSPINNER_IMGUI_H

#include "gdimspinner.h"

#include "common/gd_core.h"
#include "core/os/os.h"
#include "core/reference.h"
#include "scene/2d/canvas_item.h"
#include "scene/resources/font.h"

#include "misc/default_bitmap_font.auto.h"

// https://github.com/dalerank/imspinner
// commit 5896bfa0de0bef45b280e8a6fa5b08c7210727fb

typedef uint32_t ImU32;
typedef Vector2 ImVec2;
typedef Vector2i ImVec2ih;
typedef Rect2 ImRect;
typedef uint64_t ImGuiID;
typedef uint64_t ImDrawListFlags;

template <typename T>
static inline T ImMin(T lhs, T rhs) { return MIN(lhs, rhs); }
template <typename T>
static inline T ImMax(T lhs, T rhs) { return MAX(lhs, rhs); }
template <typename T>
static inline T ImClamp(T v, T mn, T mx) { return CLAMP(v, mn, mx); }

#define IM_PI real_t(Math_PI)
#define IM_COL32_A_MASK 0xff000000
#define IM_ROUNDUP_TO_EVEN(_V) ((((_V) + 1) / 2) * 2)
#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN 4
#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX 512
#define IM_DRAWLIST_CIRCLE_SEGMENT_MAX_ERROR 1.60f
#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(_RAD, _MAXERROR) CLAMP(IM_ROUNDUP_TO_EVEN((int)Math::ceil(IM_PI / Math::acos(1 - MIN((_MAXERROR), (_RAD)) / (_RAD)))), IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MIN, IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX)
#define IM_DRAWLIST_ARCFAST_TABLE_SIZE 48 // number of samples in lookup table.
#define IM_DRAWLIST_ARCFAST_SAMPLE_MAX IM_DRAWLIST_ARCFAST_TABLE_SIZE // sample index _PathArcToFastEx() for 360 angle.
#define IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_R(_N, _MAXERROR) ((_MAXERROR) / (1 - Math::cos(IM_PI / MAX((float)(_N), IM_PI))))

enum {
	ImDrawListFlags_AntiAliasedFill,
};

struct ImColor {
	union {
		struct {
			union {
				float r, x;
			};
			union {
				float g, y;
			};
			union {
				float b, z;
			};
			union {
				float a, w;
			};
		} Value;
		float Components[4];
	};
	ImColor &operator=(const ImColor &c) {
		memcpy(&Value, &c.Value, sizeof(Value));
		return *this;
	}
	_FORCE_INLINE_ operator Color() const { return Color(Components); }
	_FORCE_INLINE_ operator ImU32() const { return Color(Components).to_abgr32(); }
	static ImColor HSV(float h, float s, float v, float a = 1.0) {
		Color c;
		c.set_hsv(h, s, v, a);
		return ImColor(c);
	}
	constexpr ImColor() :
			Value{ { 0.f }, { 0.f }, { 0.f }, { 1.f } } {}
	constexpr ImColor(float rc, float gc, float bc, float ac) :
			Value{ { rc }, { gc }, { bc }, { ac } } {}
	ImColor(ImU32 rgba) {
		const Color &c = Color::from_abgr(rgba);
		Value.r = c.r;
		Value.g = c.g;
		Value.b = c.b;
		Value.a = c.a;
	}
	ImColor(int rc, int gc, int bc, int ac = 255) {
		const float sc = 1.0 / 255.0;
		Value.r = rc * sc;
		Value.g = gc * sc;
		Value.b = bc * sc;
		Value.a = ac * sc;
	}
	ImColor(const Color &c) {
		Value.r = c.r;
		Value.g = c.g;
		Value.b = c.b;
		Value.a = c.a;
	}
};

struct ImFontGlyph {
	real_t U0, U1;
	real_t V0, V1;
};

struct ImFont : public Reference {
	ImFontGlyph *FindGlyph(int glyph) { return nullptr; }
};

struct ImFontAtlas : public Reference {
	void GetTexDataAsAlpha8(const unsigned char **out_pixels, int *out_width, int *out_height, int *out_bytes_per_pixel = nullptr) {
		static Ref<BitmapFont> bfont;
		if (bfont.is_null()) {
			bfont.instance();
			Ref<BitmapFont> _font = memnew(BitmapFont);
			if (_font->create_from_fnt_ptr(_default_bitmap_fnt, strlen(_default_bitmap_fnt), _default_bitmap_png) != OK) {
				// this should not happen, since all data are embedded
				// if so, we are rather running out of resources/crashing
				WARN_PRINT("Failed to load default bitmap font.");
			}
		}
		if (Ref<ImageTexture> texture = bfont->get_texture(0)) {
			*out_pixels = texture->get_data()->get_raw_cptr();
		}
		if (out_width)
			*out_width = bfont->get_texture(0)->get_width();
		if (out_height)
			*out_height = bfont->get_texture(0)->get_height();
		if (out_bytes_per_pixel)
			*out_bytes_per_pixel = 1;
	}
};

struct ImGuiStyle {
	ImVec2 FramePadding;
	float Alpha;
};

struct ImGuiIO {
	ImFontAtlas *Fonts;
};

struct ImGuiStorage : public Reference {
	real_t GetFloat(ImGuiID id, real_t def_value) const { return get_meta(itos(id), def_value); }
	void SetFloat(ImGuiID id, real_t value) { set_meta(itos(id), value); }
	int64_t GetInt(ImGuiID id, int64_t def_value) const { return get_meta(itos(id), def_value); }
	void SetInt(ImGuiID id, int64_t value) { set_meta(itos(id), value); }
};

struct ImGuiWindow {
	struct {
		ImVec2 CursorPos;
		ImGuiStorage *StateStorage = nullptr;
	} DC;
	struct ImDrawListSharedData {
		ImVec2 ArcFastVtx[IM_DRAWLIST_ARCFAST_TABLE_SIZE]; // Sample points on the quarter of the circle.
		// Cutoff radius after which arc drawing will fallback to slower PathArcTo()
		const real_t ArcFastRadiusCutoff = IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC_R(IM_DRAWLIST_ARCFAST_SAMPLE_MAX, IM_DRAWLIST_CIRCLE_SEGMENT_MAX_ERROR);
		ImDrawListSharedData() {
			for (int i = 0; i < IM_DRAWLIST_ARCFAST_TABLE_SIZE; i++) {
				const real_t a = (real_t(i) * 2 * IM_PI) / real_t(IM_DRAWLIST_ARCFAST_TABLE_SIZE);
				ArcFastVtx[i] = Vector2(Math::cos(a), Math::sin(a));
			}
		}
	};
	struct _DrawList : public Reference {
		ImDrawListFlags Flags;
		CanvasItem *_Canvas = nullptr;
		ImDrawListSharedData *_Data = nullptr;
		Vector<Vector2> _Path;
		int _CalcCircleAutoSegmentCount(real_t radius) {
			return IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(radius, IM_DRAWLIST_CIRCLE_SEGMENT_MAX_ERROR);
		}
		void _PathArcToN(const ImVec2 &center, real_t radius, real_t a_min, real_t a_max, int num_segments) {
			if (radius < 0.5) {
				_Path.push_back(center);
				return;
			}
			// Note that we are adding a point at both a_min and a_max.
			// If you are trying to draw a full closed circle you don't want the overlapping points!
			for (int i = 0; i <= num_segments; i++) {
				const real_t a = a_min + (real_t(i) / real_t(num_segments)) * (a_max - a_min);
				_Path.push_back({ center.x + Math::cos(a) * radius, center.y + Math::sin(a) * radius });
			}
		}
		void _PathArcToFastEx(const ImVec2 &center, float radius, int a_min_sample, int a_max_sample, int a_step) {
			if (radius < 0.5) {
				_Path.push_back(center);
				return;
			}
			// Calculate arc auto segment step size
			if (a_step <= 0) {
				a_step = IM_DRAWLIST_ARCFAST_SAMPLE_MAX / _CalcCircleAutoSegmentCount(radius);
			}
			a_step = CLAMP(a_step, 1, IM_DRAWLIST_ARCFAST_TABLE_SIZE / 4); // Make sure we never do steps larger than one quarter of the circle

			const int sample_range = Math::abs(a_max_sample - a_min_sample);
			const int a_next_step = a_step;

			int samples = sample_range + 1;
			bool extra_max_sample = false;
			if (a_step > 1) {
				samples = sample_range / a_step + 1;
				const int overstep = sample_range % a_step;
				if (overstep > 0) {
					extra_max_sample = true;
					samples++;
					// When we have overstep to avoid awkwardly looking one long line and one tiny one at the end,
					// distribute first step range evenly between them by reducing first step size.
					if (sample_range > 0) {
						a_step -= (a_step - overstep) / 2;
					}
				}
			}

			_Path.resize(_Path.size() + samples);
			ImVec2 *out_ptr = _Path.ptrw() + (_Path.size() - samples);

			int sample_index = a_min_sample;
			if (sample_index < 0 || sample_index >= IM_DRAWLIST_ARCFAST_SAMPLE_MAX) {
				sample_index = sample_index % IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
				if (sample_index < 0) {
					sample_index += IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
				}
			}

			if (a_max_sample >= a_min_sample) {
				for (int a = a_min_sample; a <= a_max_sample; a += a_step, sample_index += a_step, a_step = a_next_step) {
					// a_step is clamped to IM_DRAWLIST_ARCFAST_SAMPLE_MAX, so we have guaranteed that it will not wrap over range twice or more
					if (sample_index >= IM_DRAWLIST_ARCFAST_SAMPLE_MAX) {
						sample_index -= IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
					}
					const ImVec2 s = _Data->ArcFastVtx[sample_index];
					out_ptr->x = center.x + s.x * radius;
					out_ptr->y = center.y + s.y * radius;
					out_ptr++;
				}
			} else {
				for (int a = a_min_sample; a >= a_max_sample; a -= a_step, sample_index -= a_step, a_step = a_next_step) {
					// a_step is clamped to IM_DRAWLIST_ARCFAST_SAMPLE_MAX, so we have guaranteed that it will not wrap over range twice or more
					if (sample_index < 0) {
						sample_index += IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
					}
					const ImVec2 s = _Data->ArcFastVtx[sample_index];
					out_ptr->x = center.x + s.x * radius;
					out_ptr->y = center.y + s.y * radius;
					out_ptr++;
				}
			}
			if (extra_max_sample) {
				int normalized_max_sample = a_max_sample % IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
				if (normalized_max_sample < 0) {
					normalized_max_sample += IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
				}
				const ImVec2 s = _Data->ArcFastVtx[normalized_max_sample];
				out_ptr->x = center.x + s.x * radius;
				out_ptr->y = center.y + s.y * radius;
				out_ptr++;
			}
		}
		// Path drawing
		void PathArcTo(const ImVec2 &center, real_t radius, real_t a_min, real_t a_max, int num_segments = 0) {
			if (radius < 0.5) {
				_Path.push_back(center);
				return;
			}
			if (num_segments > 0) {
				_PathArcToN(center, radius, a_min, a_max, num_segments);
				return;
			}
			// Automatic segment count
			if (radius <= _Data->ArcFastRadiusCutoff) {
				const bool a_is_reverse = a_max < a_min;
				// We are going to use precomputed values for mid samples.
				// Determine first and last sample in lookup table that belong to the arc.
				const real_t a_min_sample_f = IM_DRAWLIST_ARCFAST_SAMPLE_MAX * a_min / (IM_PI * 2);
				const real_t a_max_sample_f = IM_DRAWLIST_ARCFAST_SAMPLE_MAX * a_max / (IM_PI * 2);

				const int a_min_sample = a_is_reverse ? int(Math::floor(a_min_sample_f)) : int(Math::ceil(a_min_sample_f));
				const int a_max_sample = a_is_reverse ? int(Math::ceil(a_max_sample_f)) : int(Math::floor(a_max_sample_f));
				const int a_mid_samples = a_is_reverse ? MAX(a_min_sample - a_max_sample, 0) : MAX(a_max_sample - a_min_sample, 0);

				const real_t a_min_segment_angle = a_min_sample * IM_PI * 2 / IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
				const real_t a_max_segment_angle = a_max_sample * IM_PI * 2 / IM_DRAWLIST_ARCFAST_SAMPLE_MAX;
				const bool a_emit_start = Math::abs(a_min_segment_angle - a_min) >= 1e-5f;
				const bool a_emit_end = Math::abs(a_max - a_max_segment_angle) >= 1e-5f;

				if (a_emit_start) {
					_Path.push_back({ center.x + Math::cos(a_min) * radius, center.y + Math::sin(a_min) * radius });
				}
				if (a_mid_samples > 0) {
					_PathArcToFastEx(center, radius, a_min_sample, a_max_sample, 0);
				}
				if (a_emit_end) {
					_Path.push_back({ center.x + Math::cos(a_max) * radius, center.y + Math::sin(a_max) * radius });
				}
			} else {
				const real_t arc_length = Math::abs(a_max - a_min);
				const int circle_segment_count = _CalcCircleAutoSegmentCount(radius);
				const int arc_segment_count = MAX(int(Math::ceil(circle_segment_count * arc_length / (IM_PI * 2))), int(2 * IM_PI / arc_length));
				_PathArcToN(center, radius, a_min, a_max, arc_segment_count);
			}
		}
		void PathFillConvex(ImU32 col) {
			AddConvexPolyFilled(_Path, col);
			_Path.clear();
		}
		void PathStroke(ImU32 col, bool closed = false, real_t thickness = 1) {
			if ((col & IM_COL32_A_MASK) == 0) {
				return;
			}
			const int last = _Path.size() - 1;
			if (last > 0) {
				if (closed && !_Path[last].is_equal_approx(_Path[0])) {
					_Path.push_back(_Path[0]);
				}
				_Canvas->draw_polyline(_Path, ImColor(col), thickness);
			}
		}
		_FORCE_INLINE_ void PathLineTo(const ImVec2 &pos) { _Path.push_back(pos); }
		_FORCE_INLINE_ void PathClear() { _Path.clear(); }
		void AddLine(const ImVec2 &p1, const ImVec2 &p2, ImU32 col, real_t thickness = 1) {
			if ((col & IM_COL32_A_MASK) == 0) {
				return;
			}
			_Canvas->draw_line(p1 + ImVec2(0.5, 0.5), p2 + ImVec2(0.5, 0.5), ImColor(col), thickness);
		}
		void AddCircle(const ImVec2 &center, real_t radius, ImU32 col, int num_segments = 0, real_t thickness = 1) {
			if ((col & IM_COL32_A_MASK) == 0 || radius < 0.5) {
				return;
			}
			if (num_segments <= 0) {
				num_segments = _CalcCircleAutoSegmentCount(radius);
			}
			// Clamp to avoid drawing insanely tessellated shapes
			num_segments = ImClamp(num_segments, 3, IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_MAX);
			// Because we are filling a closed shape we remove 1 from the count of segments/points
			const real_t a_max = (IM_PI * 2) * (real_t(num_segments - 1)) / real_t(num_segments);
			const real_t a_min = radius - 0.5;
			// Note that we are adding a point at both a_min and a_max.
			// If you are trying to draw a full closed circle you don't want the overlapping points!
			for (int i = 0; i <= num_segments; i++) {
				const real_t a = a_min + ((real_t)i / (real_t)num_segments) * (a_max - a_min);
				_Path.push_back(ImVec2(center.x + Math::cos(a) * radius, center.y + Math::sin(a) * radius));
			}
			PathStroke(col, true, thickness);
		}
		void AddCircleFilled(const ImVec2 &center, real_t radius, ImU32 col, int num_segments = 0) {
			if ((col & IM_COL32_A_MASK) == 0 || radius < 0.5) {
				return;
			}
			_Canvas->draw_circle(center, radius, ImColor(col));
		}
		void AddRectFilled(const ImVec2 &p_min, const ImVec2 &p_max, ImU32 col, real_t rounding = 0) { // a: upper-left, b: lower-right (== upper-left + size)
			if ((col & IM_COL32_A_MASK) == 0) {
				return;
			}
			_Canvas->draw_rect(Rect2(p_min, p_max - p_min), ImColor(col), true);
		}
		void AddConvexPolyFilled(const Vector<ImVec2> &points, ImU32 col) {
			if ((col & IM_COL32_A_MASK) == 0) {
				return;
			}
			_Canvas->draw_polygon(points, make_vector<Color>(ImColor(col)));
		}
		void AddConvexPolyFilled(const ImVec2 *points, int num_points, ImU32 col) {
			if ((col & IM_COL32_A_MASK) == 0) {
				return;
			}
			_Canvas->draw_polygon(Vector<Vector2>(num_points, points), make_vector<Color>(ImColor(col)));
		}
	};

	static ImDrawListSharedData SharedData;

	bool SkipItems = false;
	CanvasItem *DrawCanvas = nullptr;
	Ref<ImGuiStorage> Storage = memnew(ImGuiStorage);
	Ref<_DrawList> DrawList = memnew(_DrawList);
	ImGuiID GetID(const char *label) { return ImGuiID(DrawCanvas->get_instance_id() + String(label).hash64()); }

	ImGuiWindow(CanvasItem *canvas) :
			DrawCanvas(canvas) {
		DrawList->_Canvas = canvas;
		DrawList->_Data = &SharedData;
	}
};

struct ImGuiContext {
	ImGuiStyle Style;
	Ref<ImFont> Font = memnew(ImFont);
};

ImGuiWindow::ImDrawListSharedData ImGuiWindow::SharedData;

static ImGuiContext CImGui, *GImGui = &CImGui;
static ImGuiIO GImGuiIO;
static ImGuiWindow *_CurrentImWindow = nullptr;

namespace ImGui {
void SetCurrentWindow(ImGuiWindow *wnd) { _CurrentImWindow = wnd; }
ImGuiWindow *GetCurrentWindow() { return _CurrentImWindow; }
ImGuiContext *GetCurrentContext() { return GImGui; }
ImGuiStyle &GetStyle() { return GImGui->Style; }
ImGuiIO &GetIO() { return GImGuiIO; }
ImVec2 ItemSize() { return Size2(); }
real_t GetTime() { return OS::get_singleton()->get_ticks_msec() / 1000.0; }
bool ItemAdd(const ImRect &bb, ImGuiID id) { return true; }
void ItemSize(const ImVec2 &size, real_t text_baseline_y = -1.0) {}
_FORCE_INLINE_ void ItemSize(const ImRect &bb, real_t text_baseline_y = -1.0) { ItemSize(bb.get_size(), text_baseline_y); }
_FORCE_INLINE_ void ColorConvertRGBtoHSV(float r, float g, float b, float &out_h, float &out_s, float &out_v) {
	Color c(r, g, b);
	out_h = c.get_h();
	out_s = c.get_s();
	out_v = c.get_v();
}
} //namespace ImGui

// -----
#define GetCenter get_center
#define ImSin(x) Math::sin(real_t(x))
#define ImCos(x) Math::cos(real_t(x))
#define ImAbs(x) Math::abs(x)
#define ImFmod(x, y) Math::fmod(real_t(x), real_t(y))
#define ImPow(x, y) Math::pow(x, y)

#define float real_t

typedef std::function<float(int)> grad_f;
typedef std::function<ImColor(int)> color_f;
typedef std::function<ImVec2(int)> point_f;

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include "imspinner/imspinner.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
// -----

#endif // IMSPINNER_IMGUI_H
