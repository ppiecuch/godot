/*************************************************************************/
/*  imspinner.cpp                                                        */
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

#include "gdimspinner.h"

#include "common/gd_core.h"
#include "core/os/os.h"
#include "core/reference.h"
#include "scene/2d/canvas_item.h"

// https://github.com/dalerank/imspinner
// commit 89610eaceb0066b52d5a7c066871232f2f83aece

typedef uint32_t ImU32;
typedef Vector2 ImVec2;
typedef Vector2i ImVec2ih;
typedef Rect2 ImRect;
typedef uint64_t ImGuiID;

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

struct ImColor : public Color {
	struct {
		float &x, &y, &z, &w;
	} Value;
	ImColor &operator=(const ImColor &c) {
		memcpy(components, c.components, sizeof(components));
		return *this;
	}
	_FORCE_INLINE_ operator ImU32() const { return to_abgr32(); }
	static ImColor HSV(float h, float s, float v, float a = 1.0) {
		Color c;
		c.set_hsv(h, s, v, a);
		return ImColor(c);
	}
	ImColor() :
			Value({ r, g, b, a }) {}
	ImColor(ImU32 rgba) :
			Value({ r, g, b, a }) {
		const Color &c = Color::from_abgr(rgba);
		memcpy(components, c.components, sizeof(components));
	}
	ImColor(int rc, int gc, int bc, int ac = 255) :
			Value({ r, g, b, a }) {
		const float sc = 1.0 / 255.0;
		r = rc * sc;
		g = gc * sc;
		b = bc * sc;
		a = ac * sc;
	}
	ImColor(const Color &c) :
			Value({ r, g, b, a }) { memcpy(components, c.components, sizeof(components)); }
};

struct ImGuiStyle {
	ImVec2 FramePadding;
};

struct ImGuiStorage : public Reference {
	real_t GetFloat(ImGuiID id, real_t def_value) const { return get_meta(itos(id)); }
	void SetFloat(ImGuiID id, real_t value) { set_meta(itos(id), value); }
};

struct ImGuiWindow {
	bool SkipItems = false;
	ImGuiID GetID(const char *label) { return 1; }
	Ref<ImGuiStorage> Storage = memnew(ImGuiStorage);
	struct {
		ImVec2 CursorPos;
		ImGuiStorage *StateStorage = nullptr;
	} DC;
	struct _DrawList : public Reference {
		CanvasItem *_Canvas;
		Vector<Vector2> _Path;
		int _CalcCircleAutoSegmentCount(real_t radius) {
			return IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(radius, IM_DRAWLIST_CIRCLE_SEGMENT_MAX_ERROR);
		}
		// Path drawing
		void PathArcTo(const ImVec2 &center, real_t radius, real_t a_min, real_t a_max, int num_segments = 0) {
		}
		void PathFillConvex(ImU32 col) {
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

		// Shape drawing
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
			const float a_max = (IM_PI * 2) * (float(num_segments - 1)) / float(num_segments);
			const float a_min = radius - 0.5;
			// Note that we are adding a point at both a_min and a_max.
			// If you are trying to draw a full closed circle you don't want the overlapping points!
			for (int i = 0; i <= num_segments; i++) {
				const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
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
		void AddConvexPolyFilled(const ImVec2 *points, int num_points, ImU32 col) {
			if ((col & IM_COL32_A_MASK) == 0) {
				return;
			}
		}
	};
	Ref<_DrawList> DrawList = memnew(_DrawList);
};

struct ImGuiContext {
	ImGuiStyle Style;
};

namespace ImGui {
ImGuiWindow *GetCurrentWindow() { return nullptr; }
ImVec2 ItemSize() { return Size2(); }
real_t GetTime() { return OS::get_singleton()->get_ticks_msec() / 1000.0; }
bool ItemAdd(const ImRect &bb, ImGuiID id) { return false; }
void ItemSize(const ImVec2 &size, real_t text_baseline_y = -1.0) {}
_FORCE_INLINE_ void ItemSize(const ImRect &bb, real_t text_baseline_y = -1.0) { ItemSize(bb.get_size(), text_baseline_y); }
_FORCE_INLINE_ void ColorConvertRGBtoHSV(float r, float g, float b, float &out_h, float &out_s, float &out_v) {
	Color c(r, g, b);
	out_h = c.get_h();
	out_s = c.get_s();
	out_v = c.get_v();
}
} //namespace ImGui

static ImGuiContext *GImGui;

// -----
#define GetCenter get_center
#define ImSin(x) Math::sin(real_t(x))
#define ImCos(x) Math::cos(real_t(x))
#define ImAbs(x) Math::abs(x)
#define ImFmod(x, y) Math::fmod(real_t(x), real_t(y))
#define powf(x, y) Math::pow(x, y)
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

void SpinnerCanvas::draw_spinners(int p_spinner) {
	static real_t nextdot1 = 0, nextdot2;

	nextdot1 -= 0.07;
	nextdot2 -= 0.2 * velocity;

	typedef ImSpinner::Radius Radius;
	typedef ImSpinner::Thickness Thickness;
	typedef ImSpinner::MinThickness MinThickness;
	typedef ImSpinner::Speed Speed;
	typedef ImSpinner::Angle Angle;
	typedef ImSpinner::Color Color;
	typedef ImSpinner::Dots Dots;
	typedef ImSpinner::MiddleDots MiddleDots;
	typedef ImSpinner::BgColor BgColor;
	typedef ImSpinner::FloatPtr FloatPtr;

	switch (p_spinner) {
		/* 0 */ case SPINNER : {
			ImSpinner::Spinner<ImSpinner::e_st_rainbow>("Spinner", Radius{ 16 }, Thickness{ 2 }, Color{ ImColor::HSV(++hue * 0.005, 0.8, 0.8) }, Speed{ 8 * velocity });
		} break;
		case SPINNERANG: {
			ImSpinner::Spinner<ImSpinner::e_st_angle>("SpinnerAng", Radius{ 16 }, Thickness{ 2 }, Color{ ImColor(255, 255, 255) }, BgColor{ ImColor(255, 255, 255, 128) }, Speed{ 8 * velocity }, Angle{ IM_PI });
		} break;
		case SPINNERDOTS: {
			ImSpinner::Spinner<ImSpinner::e_st_dots>("SpinnerDots", Radius{ 16 }, Thickness{ 4 }, Color{ ImColor(255, 255, 255) }, FloatPtr{ &nextdot1 }, Speed{ 1 * velocity }, Dots{ 12 }, MiddleDots{ 6 }, MinThickness{ -1 });
		} break;
		case SPINNERANGNOBG: {
			ImSpinner::Spinner<ImSpinner::e_st_ang>("SpinnerAngNoBg", Radius{ 16 }, Thickness{ 2 }, Color{ ImColor(255, 255, 255) }, BgColor{ ImColor(255, 255, 255, 0) }, Speed{ 6 * velocity }, Angle{ IM_PI });
		} break;
		case SPINNERANG270: {
			ImSpinner::Spinner<ImSpinner::e_st_ang>("SpinnerAng270", Radius{ 16 }, Thickness{ 2 }, Color{ ImColor(255, 255, 255) }, BgColor{ ImColor(255, 255, 255, 128) }, Speed{ 6 * velocity }, Angle{ 270 / 360 * 2 * IM_PI });
		} break;
		/* 5 */ case SPINNERANG270NOBG : {
			ImSpinner::Spinner<ImSpinner::e_st_ang>("SpinnerAng270NoBg", Radius{ 16 }, Thickness{ 2 }, Color{ ImColor(255, 255, 255) }, BgColor{ ImColor(255, 255, 255, 0) }, Speed{ 6 * velocity }, Angle{ 270 / 360 * 2 * IM_PI });
		} break;
		case SPINNERVDOTS: {
			ImSpinner::Spinner<ImSpinner::e_st_vdots>("SpinnerVDots", Radius{ 16 }, Thickness{ 4 }, Color{ ImColor::HSV(hue * 0.001, 0.8, 0.8) }, BgColor{ ImColor::HSV(hue * 0.0011, 0.8, 0.8) }, Speed{ real_t(2.7 * velocity) }, Dots{ 12 }, MiddleDots{ 6 });
		} break;
		case SPINNERBOUNCEBALL: {
			ImSpinner::SpinnerBounceBall("SpinnerBounceBall", 16, 6, ImColor(255, 255, 255), 4 * velocity);
		} break;
		case SPINNERANGECLIPSE: {
			ImSpinner::SpinnerAngEclipse("SpinnerAngEclipse", 16, 5, ImColor(255, 255, 255), 6 * velocity);
		} break;
		case SPINNERINGYANG: {
			ImSpinner::SpinnerIngYang("SpinnerIngYang", 16, 5, false, 0, ImColor(255, 255, 255), ImColor(255, 0, 0), 4 * velocity, IM_PI * 0.8);
		} break;
		/* 10 */ case SPINNERBARCHARTSINE : {
			ImSpinner::SpinnerBarChartSine("SpinnerBarChartSine", 16, 4, ImColor(255, 255, 255), 6.8 * velocity, 4, 0);
		} break;
		case SPINNERBOUNCEDOTS: {
			ImSpinner::SpinnerBounceDots("SpinnerBounceDots", 6, ImColor(255, 255, 255), 6 * velocity, 3);
		} break;
		case SPINNERFADEDOTS: {
			ImSpinner::SpinnerFadeDots("SpinnerFadeDots", 6, ImColor(255, 255, 255), 8 * velocity, 3);
		} break;
		case SPINNERSCALEDOTS: {
			ImSpinner::SpinnerScaleDots("SpinnerMovingDots", 6, ImColor(255, 255, 255), 7 * velocity, 3);
		} break;
		case SPINNERMOVINGDOTS: {
			ImSpinner::SpinnerMovingDots("SpinnerMovingDots", 6, ImColor(255, 255, 255), 30 * velocity, 3);
		} break;
		/* 15 */ case SPINNERROTATEDOTS : {
			ImSpinner::SpinnerRotateDots("SpinnerRotateDots", 16, 6, ImColor(255, 255, 255), 4 * velocity, 2);
		} break;
		case SPINNERTWINANG: {
			ImSpinner::SpinnerTwinAng("SpinnerTwinAng", 16, 16, 6, ImColor(255, 255, 255), ImColor(255, 0, 0), 4 * velocity);
		} break;
		case SPINNERCLOCK: {
			ImSpinner::SpinnerClock("SpinnerClock", 16, 2, ImColor(255, 0, 0), ImColor(255, 255, 255), 4 * velocity);
		} break;
		case SPINNERINGYANGR: {
			ImSpinner::SpinnerIngYang("SpinnerIngYangR", 16, 5, true, 0.1, ImColor(255, 255, 255), ImColor(255, 0, 0), 4 * velocity, IM_PI * 0.8f);
		} break;
		case SPINNERBARCHARTSINE2: {
			ImSpinner::SpinnerBarChartSine("SpinnerBarChartSine2", 16, 4, ImColor::HSV(hue * 0.005f, 0.8, 0.8), 4.8 * velocity, 4, 1);
		} break;
		/* 20 */ case SPINNERTWINANG180 : {
			ImSpinner::SpinnerTwinAng180("SpinnerTwinAng180", 16, 12, 4, ImColor(255, 255, 255), ImColor(255, 0, 0), 4 * velocity);
		} break;
		case SPINNERTWINANG360: {
			ImSpinner::SpinnerTwinAng360("SpinnerTwinAng360", 16, 11, 4, ImColor(255, 255, 255), ImColor(255, 0, 0), 4 * velocity);
		} break;
		case SPINNERINCDOTS: {
			ImSpinner::SpinnerIncDots("SpinnerIncDots", 16, 4, ImColor(255, 255, 255), 5.6, 6);
		} break;
		case SPINNERDOTSWOBG: {
			ImSpinner::SpinnerDots("SpinnerDotsWoBg", &nextdot2, 16, 4, ImColor(255, 255, 255), 0.3 * velocity, 12, 0);
		} break;
		case SPINNERINCSCALEDOTS: {
			ImSpinner::SpinnerIncScaleDots("SpinnerIncScaleDots", 16, 4, ImColor(255, 255, 255), 6.6, 6);
		} break;
		/* 25 */ case SPINNERANG90BG : {
			ImSpinner::SpinnerAng("SpinnerAng90Bg", 16, 6, ImColor(255, 255, 255), ImColor(255, 255, 255, 128), 8 * velocity, IM_PI / 2);
		} break;
		case SPINNERANG90: {
			ImSpinner::SpinnerAng("SpinnerAng90", 16, 6, ImColor(255, 255, 255), ImColor(255, 255, 255, 0), 8.5 * velocity, IM_PI / 2);
		} break;
		case SPINNERFADEBARS: {
			ImSpinner::SpinnerFadeBars("SpinnerFadeBars", 10, ImColor(255, 255, 255), 4.8 * velocity, 3);
		} break;
		case SPINNERPULSARSEQ: {
			ImSpinner::SpinnerPulsar("SpinnerPulsar", 16, 2, ImColor(255, 255, 255), 1 * velocity);
		} break;
		case SPINNERINGYANGR2: {
			ImSpinner::SpinnerIngYang("SpinnerIngYangR2", 16, 5, true, 3, ImColor(255, 255, 255), ImColor(255, 0, 0), 4 * velocity, IM_PI * 0.8);
		} break;
		/* 30 */ case SPINNERBARCHARTRAINBOW : {
			ImSpinner::SpinnerBarChartRainbow("SpinnerBarChartRainbow", 16, 4, ImColor::HSV(hue * 0.005, 0.8, 0.8), 6.8 * velocity, 4);
		} break;
		case SPINNERBARSROTATEFADE: {
			ImSpinner::SpinnerBarsRotateFade("SpinnerBarsRotateFade", 8, 18, 4, ImColor(255, 255, 255), 7.6, 6);
		} break;
		case SPINNERFADESCALEBARS: {
			ImSpinner::SpinnerFadeBars("SpinnerFadeScaleBars", 10, ImColor(255, 255, 255), 6.8, 3, true);
		} break;
		case SPINNERBARSSCALEMIDDLE: {
			ImSpinner::SpinnerBarsScaleMiddle("SpinnerBarsScaleMiddle", 6, ImColor(255, 255, 255), 8.8, 3);
		} break;
		case SPINNERANGTWIN1: {
			ImSpinner::SpinnerAngTwin("SpinnerAngTwin1", 16, 13, 2, ImColor(255, 0, 0), ImColor(255, 255, 255), 6 * velocity, IM_PI / 2);
		} break;
		/* 35 */ case SPINNERANGTWIN2 : {
			ImSpinner::SpinnerAngTwin("SpinnerAngTwin2", 13, 16, 2, ImColor(255, 0, 0), ImColor(255, 255, 255), 6 * velocity, IM_PI / 2);
		} break;
		case SPINNERANGTWIN3: {
			ImSpinner::SpinnerAngTwin("SpinnerAngTwin3", 13, 16, 2, ImColor(255, 0, 0), ImColor(255, 255, 255), 6 * velocity, IM_PI / 2, 2);
		} break;
		case SPINNERANGTWIN4: {
			ImSpinner::SpinnerAngTwin("SpinnerAngTwin4", 16, 13, 2, ImColor(255, 0, 0), ImColor(255, 255, 255), 6 * velocity, IM_PI / 2, 2);
		} break;
		case SPINNERTWINPULSAR: {
			ImSpinner::SpinnerTwinPulsar("SpinnerTwinPulsar", 16, 2, ImColor(255, 255, 255), 0.5 * velocity, 2);
		} break;
		case SPINNERANGTWIN5: {
			ImSpinner::SpinnerAngTwin("SpinnerAngTwin4", 14, 13, 3, ImColor(255, 0, 0), ImColor(0, 0, 0, 0), 5 * velocity, IM_PI / 1.5, 2);
		} break;
		/* 40 */ case SPINNERBLOCKS : {
			ImSpinner::SpinnerBlocks("SpinnerBlocks", 16, 7, ImColor(255, 255, 255, 30), ImColor::HSV(hue * 0.005, 0.8, 0.8), 5 * velocity);
		} break;
		case SPINNERTWINBALL: {
			ImSpinner::SpinnerTwinBall("SpinnerTwinBall", 16, 11, 2, 2.5, ImColor(255, 0, 0), ImColor(255, 255, 255), 6 * velocity, 2);
		} break;
		case SPINNERTWINBALL2: {
			ImSpinner::SpinnerTwinBall("SpinnerTwinBall2", 15, 19, 2, 2, ImColor(255, 0, 0), ImColor(255, 255, 255), 6 * velocity, 3);
		} break;
		case SPINNERTWINBALL3: {
			ImSpinner::SpinnerTwinBall("SpinnerTwinBall2", 16, 16, 2, 5, ImColor(255, 0, 0), ImColor(255, 255, 255), 5 * velocity, 1);
		} break;
		case SPINNERANGTRIPLE: {
			ImSpinner::SpinnerAngTriple("SpinnerAngTriple", 16, 13, 10, 1.3, ImColor(255, 255, 255), ImColor(255, 0, 0), ImColor(255, 255, 255), 5 * velocity, 1.5 * IM_PI);
		} break;
		/* 45 */ case SPINNERINCFULLDOTS : {
			ImSpinner::SpinnerIncFullDots("SpinnerIncFullDots", 16, 4, ImColor(255, 255, 255), 5.6, 4);
		} break;
		case SPINNERGOOEYBALLS: {
			ImSpinner::SpinnerGooeyBalls("SpinnerGooeyBalls", 16, ImColor(255, 255, 255), 2);
		} break;
		case SPINNERROTATEGOOEYBALLS2: {
			ImSpinner::SpinnerRotateGooeyBalls("SpinnerRotateGooeyBalls2", 16, 5, ImColor(255, 255, 255), 6, 2);
		} break;
		case SPINNERROTATEGOOEYBALLS3: {
			ImSpinner::SpinnerRotateGooeyBalls("SpinnerRotateGooeyBalls3", 16, 5, ImColor(255, 255, 255), 6, 3);
		} break;
		case SPINNERMOONLINE: {
			ImSpinner::SpinnerMoonLine("SpinnerMoonLine", 16, 3, ImColor(200, 80, 0), ImColor(80, 80, 80), 5 * velocity);
		} break;
		/* 50 */ case SPINNERARCROTATION : {
			ImSpinner::SpinnerArcRotation("SpinnerArcRotation", 13, 5, ImColor(255, 255, 255), 3 * velocity, 4);
		} break;
		case SPINNERFLUID: {
			ImSpinner::SpinnerFluid("SpinnerFluid", 16, ImColor(0, 0, 255), 3.8 * velocity, 4);
		} break;
		case SPINNERARCFADE: {
			ImSpinner::SpinnerArcFade("SpinnerArcFade", 13, 5, ImColor(255, 255, 255), 3 * velocity, 4);
		} break;
		case SPINNERFILLING: {
			ImSpinner::SpinnerFilling("SpinnerFilling", 16, 6, ImColor(255, 255, 255), ImColor(255, 0, 0), 4 * velocity);
		} break;
		case SPINNERTOPUP: {
			ImSpinner::SpinnerTopup("SpinnerTopup", 16, 12, ImColor(255, 0, 0), ImColor(80, 80, 80), ImColor(255, 255, 255), 1 * velocity);
		} break;
		/* 55 */ case SPINNERFADEPULSAR : {
			ImSpinner::SpinnerFadePulsar("SpinnerFadePulsar", 16, ImColor(255, 255, 255), 1.5 * velocity, 1);
		} break;
		case SPINNERFADEPULSAR2: {
			ImSpinner::SpinnerFadePulsar("SpinnerFadePulsar2", 16, ImColor(255, 255, 255), 0.9 * velocity, 2);
		} break;
		case SPINNERPULSAR: {
			ImSpinner::SpinnerPulsar("SpinnerPulsar", 16, 2, ImColor(255, 255, 255), 1 * velocity, false);
		} break;
		case SPINNERDOUBLEFADEPULSAR: {
			ImSpinner::SpinnerDoubleFadePulsar("SpinnerDoubleFadePulsar", 16, 2, ImColor(255, 255, 255), 2 * velocity);
		} break;
		case SPINNERFILLEDARCFADE: {
			ImSpinner::SpinnerFilledArcFade("SpinnerFilledArcFade", 16, ImColor(255, 255, 255), 4 * velocity, 4);
		} break;
		/* 60 */ case SPINNERFILLEDARCFADE6 : {
			ImSpinner::SpinnerFilledArcFade("SpinnerFilledArcFade6", 16, ImColor(255, 255, 255), 6 * velocity, 6);
		} break;
		case SPINNERFILLEDARCFADE8: {
			ImSpinner::SpinnerFilledArcFade("SpinnerFilledArcFade6", 16, ImColor(255, 255, 255), 8 * velocity, 12);
		} break;
		case SPINNERFILLEDARCCOLOR: {
			ImSpinner::SpinnerFilledArcColor("SpinnerFilledArcColor", 16, ImColor(255, 0, 0), ImColor(255, 255, 255), 2.8 * velocity, 4);
		} break;
		case SPINNERCIRCLEDROP: {
			ImSpinner::SpinnerCircleDrop("SpinnerCircleDrop", 16, 1.5, 4, ImColor(255, 0, 0), ImColor(255, 255, 255), 2.8 * velocity, IM_PI);
		} break;
		case SPINNERSURROUNDEDINDICATOR: {
			ImSpinner::SpinnerSurroundedIndicator("SpinnerSurroundedIndicator", 16, 5, ImColor(0, 0, 0), ImColor(255, 255, 255), 7.8 * velocity);
		} break;
		/* 65 */ case SPINNERTRIANGLESSELETOR : {
			ImSpinner::SpinnerTrianglesSelector("SpinnerTrianglesSeletor", 16, 8, ImColor(0, 0, 0), ImColor(255, 255, 255), 4.8 * velocity, 8);
		} break;
		case SPINNERFLOWINGFRADIENT: {
			ImSpinner::SpinnerFlowingGradient("SpinnerFlowingFradient", 16, 6, ImColor(200, 80, 0), ImColor(80, 80, 80), 5 * velocity, IM_PI * 2);
		} break;
		case SPINNERROTATESEGMENTS: {
			ImSpinner::SpinnerRotateSegments("SpinnerRotateSegments", 16, 4, ImColor(255, 255, 255), 3 * velocity, 4);
		} break;
		case SPINNERROTATESEGMENTS2: {
			ImSpinner::SpinnerRotateSegments("SpinnerRotateSegments2", 16, 3, ImColor(255, 255, 255), 2.4 * velocity, 4, 2);
		} break;
		case SPINNERROTATESEGMENTS3: {
			ImSpinner::SpinnerRotateSegments("SpinnerRotateSegments3", 16, 2, ImColor(255, 255, 255), 2.1 * velocity, 4, 3);
		} break;
		/* 70 */ case SPINNERLEMNISCATE : {
			ImSpinner::SpinnerLemniscate("SpinnerLemniscate", 20, 3, ImColor(255, 255, 255), 2.1 * velocity, 3);
		} break;
		case SPINNERROTATEGEAR: {
			ImSpinner::SpinnerRotateGear("SpinnerRotateGear", 16, 6, ImColor(255, 255, 255), 2.1 * velocity, 8);
		} break;
		case SPINNERROTATEDATOM: {
			ImSpinner::SpinnerAtom("SpinnerAtom", 16, 2, ImColor(255, 255, 255), 4.1 * velocity, 3);
		} break;
		case SPINNERATOM: {
			ImSpinner::SpinnerRotatedAtom("SpinnerRotatedAtom", 16, 2, ImColor(255, 255, 255), 2.1 * velocity, 3);
		} break;
		case SPINNERRAINBOWBALLS: {
			ImSpinner::SpinnerRainbowBalls("SpinnerRainbowBalls", 16, 4, ImColor(0, 0, 0, 0), 1.5 * velocity, 5);
		} break;
		/* 75 */ case SPINNERCAMERA : {
			ImSpinner::SpinnerCamera(
					"SpinnerCamera", 16, 8, [](int i) { return ImColor::HSV(i * 0.25, 0.8, 0.8); }, 4.8 * velocity, 8);
		} break;
		case SPINNERARCPOLARFADE: {
			ImSpinner::SpinnerArcPolarFade("SpinnerArcPolarFade", 16, ImColor(255, 255, 255), 6 * velocity, 6);
		} break;
		case SPINNERARCPOLARRADIUS: {
			ImSpinner::SpinnerArcPolarRadius("SpinnerArcPolarRadius", 16, ImColor::HSV(0.25, 0.8, 0.8), 6 * velocity, 6);
		} break;
	}
}
