/*************************************************************************/
/*  gdimspinner.cpp                                                      */
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
// commit 01c5e76c23d482a146e91fd3be96c590e2fadd51

// from imspinner.h
#include <array>
#include <functional>

class SpinnerCanvas : public Reference {
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
		real_t GetFloat(ImGuiID id, real_t def_value);
		void SetFloat(ImGuiID id, real_t value);
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
			PoolVector2Array _Path;
			int _CalcCircleAutoSegmentCount(real_t radius) {
				return IM_DRAWLIST_CIRCLE_AUTO_SEGMENT_CALC(radius, IM_DRAWLIST_CIRCLE_SEGMENT_MAX_ERROR);
			}
			_FORCE_INLINE_ void PathClear() { _Path.clear(); }
			void AddLine(const ImVec2 &p1, const ImVec2 &p2, ImU32 col, real_t thickness = 1.0) {
				if ((col & IM_COL32_A_MASK) == 0)
					return;
				PathLineTo(p1 + ImVec2(0.5, 0.5));
				PathLineTo(p2 + ImVec2(0.5, 0.5));
				PathStroke(col, 0, thickness);
			}
			void AddCircleFilled(const ImVec2 &center, real_t radius, ImU32 col, int num_segments = 0);
			void AddRectFilled(const ImVec2 &p_min, const ImVec2 &p_max, ImU32 col, real_t rounding = 0.0); // a: upper-left, b: lower-right (== upper-left + size)
			void AddConvexPolyFilled(const ImVec2 *points, int num_points, ImU32 col) {
			}
			_FORCE_INLINE_ void PathLineTo(const ImVec2 &pos) { _Path.push_back(pos); }
			void PathArcTo(const ImVec2 &center, real_t radius, real_t a_min, real_t a_max, int num_segments = 0) {
			}
			void PathFillConvex(ImU32 col) {
			}
			void PathStroke(ImU32 col, bool closed = false, real_t thickness = 1.0) {
			}
		};
		Ref<_DrawList> DrawList = memnew(_DrawList);
	};

	struct ImGuiContext {
		ImGuiStyle Style;
	};

	struct ImGui {
		static ImGuiWindow *GetCurrentWindow() { return nullptr; }
		static ImVec2 ItemSize() { return Size2(); }
		static real_t GetTime() { return OS::get_singleton()->get_ticks_msec() / 1000.0; }
		static bool ItemAdd(const ImRect &bb, ImGuiID id) { return false; }
		static void ItemSize(const ImVec2 &size, real_t text_baseline_y = -1.0) {}
		_FORCE_INLINE_ static void ItemSize(const ImRect &bb, real_t text_baseline_y = -1.0) { ItemSize(bb.get_size(), text_baseline_y); }
		_FORCE_INLINE_ static void ColorConvertRGBtoHSV(float r, float g, float b, float &out_h, float &out_s, float &out_v) {
			Color c(r, g, b);
			out_h = c.get_h();
			out_s = c.get_s();
			out_v = c.get_v();
		}
	};

	real_t velocity;
	int hue;

	static ImGuiContext *GImGui;

public:
	void draw_spinners(int p_spinner);

	// -----
#define GetCenter get_center
#define ImSin(x) Math::sin(x)
#define ImCos(x) Math::cos(x)
#define ImAbs(x) Math::abs(x)
#define ImFmod(x, y) Math::fmod(real_t(x), real_t(y))
#define powf(x, y) Math::pow(x, y)
#define float real_t
#define namespace struct
#include "imspinner.h"
	_spinner;
#undef namespace
	// -----

	SpinnerCanvas();
};

SpinnerCanvas::ImGuiContext *SpinnerCanvas::GImGui = nullptr;

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
		case SPINNER:
			_spinner.Spinner<ImSpinner::e_st_rainbow>("Spinner", Radius{ 16 }, Thickness{ 2 }, Color{ ImColor::HSV(++hue * 0.005, 0.8, 0.8) }, Speed{ 8 * velocity });
			break;
		case SPINNERANG:
			_spinner.Spinner<ImSpinner::e_st_angle>("SpinnerAng", Radius{ 16 }, Thickness{ 2 }, Color{ ImColor(255, 255, 255) }, BgColor{ ImColor(255, 255, 255, 128) }, Speed{ 8 * velocity }, Angle{ IM_PI });
			break;
		case SPINNERDOTS:
			_spinner.Spinner<ImSpinner::e_st_dots>("SpinnerDots", Radius{ 16 }, Thickness{ 4 }, Color{ ImColor(255, 255, 255) }, FloatPtr{ &nextdot1 }, Speed{ 1 * velocity }, Dots{ 12 }, MiddleDots{ 6 }, MinThickness{ -1 });
			break;
		case SPINNERANGNOBG:
			_spinner.Spinner<ImSpinner::e_st_ang>("SpinnerAngNoBg", Radius{ 16 }, Thickness{ 2 }, Color{ ImColor(255, 255, 255) }, BgColor{ ImColor(255, 255, 255, 0) }, Speed{ 6 * velocity }, Angle{ IM_PI });
			break;
		case SPINNERANG270:
			_spinner.Spinner<ImSpinner::e_st_ang>("SpinnerAng270", Radius{ 16 }, Thickness{ 2 }, Color{ ImColor(255, 255, 255) }, BgColor{ ImColor(255, 255, 255, 128) }, Speed{ 6 * velocity }, Angle{ 270 / 360 * 2 * IM_PI });
			break;
		case SPINNERANG270NOBG:
			_spinner.Spinner<ImSpinner::e_st_ang>("SpinnerAng270NoBg", Radius{ 16 }, Thickness{ 2 }, Color{ ImColor(255, 255, 255) }, BgColor{ ImColor(255, 255, 255, 0) }, Speed{ 6 * velocity }, Angle{ 270 / 360 * 2 * IM_PI });
			break;
		case SPINNERVDOTS:
			_spinner.Spinner<ImSpinner::e_st_vdots>("SpinnerVDots", Radius{ 16 }, Thickness{ 4 }, Color{ ImColor::HSV(hue * 0.001, 0.8, 0.8) }, BgColor{ ImColor::HSV(hue * 0.0011, 0.8, 0.8) }, Speed{ real_t(2.7 * velocity) }, Dots{ 12 }, MiddleDots{ 6 });
			break;
		case SPINNERBOUNCEBALL:
			_spinner.SpinnerBounceBall("SpinnerBounceBall", 16, 6, ImColor(255, 255, 255), 4 * velocity);
			break;
		case SPINNERANGECLIPSE:
			_spinner.SpinnerAngEclipse("SpinnerAngEclipse", 16, 5, ImColor(255, 255, 255), 6 * velocity);
			break;
		case SPINNERINGYANG:
			_spinner.SpinnerIngYang("SpinnerIngYang", 16, 5, false, 0, ImColor(255, 255, 255), ImColor(255, 0, 0), 4 * velocity, IM_PI * 0.8);
			break;
		case SPINNERBARCHARTSINE:
			_spinner.SpinnerBarChartSine("SpinnerBarChartSine", 16, 4, ImColor(255, 255, 255), 6.8 * velocity, 4, 0);
			break;
		case SPINNERBOUNCEDOTS:
			_spinner.SpinnerBounceDots("SpinnerBounceDots", 6, ImColor(255, 255, 255), 6 * velocity, 3);
			break;
		case SPINNERFADEDOTS:
			_spinner.SpinnerFadeDots("SpinnerFadeDots", 6, ImColor(255, 255, 255), 8 * velocity, 3);
			break;
		case SPINNERSCALEDOTS:
			_spinner.SpinnerScaleDots("SpinnerMovingDots", 6, ImColor(255, 255, 255), 7 * velocity, 3);
			break;
		case SPINNERMOVINGDOTS:
			_spinner.SpinnerMovingDots("SpinnerMovingDots", 6, ImColor(255, 255, 255), 30 * velocity, 3);
			break;
		case SPINNERROTATEDOTS:
			_spinner.SpinnerRotateDots("SpinnerRotateDots", 16, 6, ImColor(255, 255, 255), 4 * velocity, 2);
			break;
		case SPINNERTWINANG:
			_spinner.SpinnerTwinAng("SpinnerTwinAng", 16, 16, 6, ImColor(255, 255, 255), ImColor(255, 0, 0), 4 * velocity);
			break;
		case SPINNERCLOCK:
			_spinner.SpinnerClock("SpinnerClock", 16, 2, ImColor(255, 0, 0), ImColor(255, 255, 255), 4 * velocity);
			break;
		case SPINNERINGYANGR:
			_spinner.SpinnerIngYang("SpinnerIngYangR", 16, 5, true, 0.1, ImColor(255, 255, 255), ImColor(255, 0, 0), 4 * velocity, IM_PI * 0.8f);
			break;
		case SPINNERBARCHARTSINE2:
			_spinner.SpinnerBarChartSine("SpinnerBarChartSine2", 16, 4, ImColor::HSV(hue * 0.005f, 0.8, 0.8), 4.8 * velocity, 4, 1);
			break;
		case SPINNERTWINANG180:
			_spinner.SpinnerTwinAng180("SpinnerTwinAng180", 16, 12, 4, ImColor(255, 255, 255), ImColor(255, 0, 0), 4 * velocity);
			break;
		case SPINNERTWINANG360:
			_spinner.SpinnerTwinAng360("SpinnerTwinAng360", 16, 11, 4, ImColor(255, 255, 255), ImColor(255, 0, 0), 4 * velocity);
			break;
		case SPINNERINCDOTS:
			_spinner.SpinnerIncDots("SpinnerIncDots", 16, 4, ImColor(255, 255, 255), 5.6, 6);
			break;
		case SPINNERDOTSWOBG:
			_spinner.SpinnerDots("SpinnerDotsWoBg", &nextdot2, 16, 4, ImColor(255, 255, 255), 0.3, 12, 6, 0);
			break;
		case SPINNERINCSCALEDOTS:
			_spinner.SpinnerIncScaleDots("SpinnerIncScaleDots", 16, 4, ImColor(255, 255, 255), 6.6, 6);
			break;
		case SPINNERANG90BG:
			_spinner.SpinnerAng("SpinnerAng90Bg", 16, 6, ImColor(255, 255, 255), ImColor(255, 255, 255, 128), 8 * velocity, IM_PI / 2);
			break;
		case SPINNERANG90:
			_spinner.SpinnerAng("SpinnerAng90", 16, 6, ImColor(255, 255, 255), ImColor(255, 255, 255, 0), 8.5 * velocity, IM_PI / 2);
			break;
		case SPINNERFADEBARS:
			_spinner.SpinnerFadeBars("SpinnerFadeBars", 10, ImColor(255, 255, 255), 4.8 * velocity, 3);
			break;
		case SPINNERPULSARSEQ:
			_spinner.SpinnerPulsar("SpinnerPulsar", 16, 2, ImColor(255, 255, 255), 1 * velocity);
			break;
		case SPINNERINGYANGR2:
			_spinner.SpinnerIngYang("SpinnerIngYangR2", 16, 5, true, 3, ImColor(255, 255, 255), ImColor(255, 0, 0), 4 * velocity, IM_PI * 0.8);
			break;
		case SPINNERBARCHARTRAINBOW:
			_spinner.SpinnerBarChartRainbow("SpinnerBarChartRainbow", 16, 4, ImColor::HSV(hue * 0.005, 0.8, 0.8), 6.8 * velocity, 4);
			break;
		case SPINNERBARSROTATEFADE:
			_spinner.SpinnerBarsRotateFade("SpinnerBarsRotateFade", 8, 18, 4, ImColor(255, 255, 255), 7.6, 6);
			break;
		case SPINNERFADESCALEBARS:
			_spinner.SpinnerFadeBars("SpinnerFadeScaleBars", 10, ImColor(255, 255, 255), 6.8, 3, true);
			break;
		case SPINNERBARSSCALEMIDDLE:
			_spinner.SpinnerBarsScaleMiddle("SpinnerBarsScaleMiddle", 6, ImColor(255, 255, 255), 8.8, 3);
			break;
		case SPINNERANGTWIN1:
			_spinner.SpinnerAngTwin("SpinnerAngTwin1", 16, 13, 2, ImColor(255, 0, 0), ImColor(255, 255, 255), 6 * velocity, IM_PI / 2);
			break;
		case SPINNERANGTWIN2:
			_spinner.SpinnerAngTwin("SpinnerAngTwin2", 13, 16, 2, ImColor(255, 0, 0), ImColor(255, 255, 255), 6 * velocity, IM_PI / 2);
			break;
		case SPINNERANGTWIN3:
			_spinner.SpinnerAngTwin("SpinnerAngTwin3", 13, 16, 2, ImColor(255, 0, 0), ImColor(255, 255, 255), 6 * velocity, IM_PI / 2, 2);
			break;
		case SPINNERANGTWIN4:
			_spinner.SpinnerAngTwin("SpinnerAngTwin4", 16, 13, 2, ImColor(255, 0, 0), ImColor(255, 255, 255), 6 * velocity, IM_PI / 2, 2);
			break;
		case SPINNERTWINPULSAR:
			_spinner.SpinnerTwinPulsar("SpinnerTwinPulsar", 16, 2, ImColor(255, 255, 255), 0.5 * velocity, 2);
			break;
		case SPINNERANGTWIN5:
			_spinner.SpinnerAngTwin("SpinnerAngTwin4", 14, 13, 3, ImColor(255, 0, 0), ImColor(0, 0, 0, 0), 5 * velocity, IM_PI / 1.5, 2);
			break;
		case SPINNERBLOCKS:
			_spinner.SpinnerBlocks("SpinnerBlocks", 16, 7, ImColor(255, 255, 255, 30), ImColor::HSV(hue * 0.005, 0.8, 0.8), 5 * velocity);
			break;
		case SPINNERTWINBALL:
			_spinner.SpinnerTwinBall("SpinnerTwinBall", 16, 11, 2, 2.5, ImColor(255, 0, 0), ImColor(255, 255, 255), 6 * velocity, 2);
			break;
		case SPINNERTWINBALL2:
			_spinner.SpinnerTwinBall("SpinnerTwinBall2", 15, 19, 2, 2, ImColor(255, 0, 0), ImColor(255, 255, 255), 6 * velocity, 3);
			break;
		case SPINNERTWINBALL3:
			_spinner.SpinnerTwinBall("SpinnerTwinBall2", 16, 16, 2, 5, ImColor(255, 0, 0), ImColor(255, 255, 255), 5 * velocity, 1);
			break;
		case SPINNERANGTRIPLE:
			_spinner.SpinnerAngTriple("SpinnerAngTriple", 16, 13, 10, 1.3, ImColor(255, 255, 255), ImColor(255, 0, 0), ImColor(255, 255, 255), 5 * velocity, 1.5 * IM_PI);
			break;
		case SPINNERINCFULLDOTS:
			_spinner.SpinnerIncFullDots("SpinnerIncFullDots", 16, 4, ImColor(255, 255, 255), 5.6, 4);
			break;
		case SPINNERGOOEYBALLS:
			_spinner.SpinnerGooeyBalls("SpinnerGooeyBalls", 16, ImColor(255, 255, 255), 2);
			break;
		case SPINNERROTATEGOOEYBALLS2:
			_spinner.SpinnerRotateGooeyBalls("SpinnerRotateGooeyBalls2", 16, 5, ImColor(255, 255, 255), 6, 2);
			break;
		case SPINNERROTATEGOOEYBALLS3:
			_spinner.SpinnerRotateGooeyBalls("SpinnerRotateGooeyBalls3", 16, 5, ImColor(255, 255, 255), 6, 3);
			break;
		case SPINNERMOONLINE:
			_spinner.SpinnerMoonLine("SpinnerMoonLine", 16, 3, ImColor(200, 80, 0), ImColor(80, 80, 80), 5 * velocity);
			break;
		case SPINNERARCROTATION:
			_spinner.SpinnerArcRotation("SpinnerArcRotation", 13, 5, ImColor(255, 255, 255), 3 * velocity, 4);
			break;
		case SPINNERARCFADE:
			_spinner.SpinnerArcFade("SpinnerArcFade", 13, 5, ImColor(255, 255, 255), 3 * velocity, 4);
			break;
		case SPINNERFILLING:
			_spinner.SpinnerFilling("SpinnerFilling", 16, 6, ImColor(255, 255, 255), ImColor(255, 0, 0), 4 * velocity);
			break;
		case SPINNERTOPUP:
			_spinner.SpinnerTopup("SpinnerTopup", 16, 12, ImColor(255, 0, 0), ImColor(80, 80, 80), ImColor(255, 255, 255), 1 * velocity);
			break;
		case SPINNERFADEPULSAR:
			_spinner.SpinnerFadePulsar("SpinnerFadePulsar", 16, ImColor(255, 255, 255), 1.5 * velocity, 1);
			break;
		case SPINNERFADEPULSAR2:
			_spinner.SpinnerFadePulsar("SpinnerFadePulsar2", 16, ImColor(255, 255, 255), 0.9 * velocity, 2);
			break;
		case SPINNERPULSAR:
			_spinner.SpinnerPulsar("SpinnerPulsar", 16, 2, ImColor(255, 255, 255), 1 * velocity, false);
			break;
		case SPINNERDOUBLEFADEPULSAR:
			_spinner.SpinnerDoubleFadePulsar("SpinnerDoubleFadePulsar", 16, 2, ImColor(255, 255, 255), 2 * velocity);
			break;
		case SPINNERFILLEDARCFADE:
			_spinner.SpinnerFilledArcFade("SpinnerFilledArcFade", 16, ImColor(255, 255, 255), 4 * velocity, 4);
			break;
		case SPINNERFILLEDARCFADE6:
			_spinner.SpinnerFilledArcFade("SpinnerFilledArcFade6", 16, ImColor(255, 255, 255), 6 * velocity, 6);
			break;
		case SPINNERFILLEDARCFADE8:
			_spinner.SpinnerFilledArcFade("SpinnerFilledArcFade6", 16, ImColor(255, 255, 255), 8 * velocity, 12);
			break;
		case SPINNERFILLEDARCCOLOR:
			_spinner.SpinnerFilledArcColor("SpinnerFilledArcColor", 16, ImColor(255, 0, 0), ImColor(255, 255, 255), 2.8 * velocity, 4);
			break;
		case SPINNERCIRCLEDROP:
			_spinner.SpinnerCircleDrop("SpinnerCircleDrop", 16, 1.5, 4, ImColor(255, 0, 0), ImColor(255, 255, 255), 2.8 * velocity, IM_PI);
			break;
		case SPINNERSURROUNDEDINDICATOR:
			_spinner.SpinnerSurroundedIndicator("SpinnerSurroundedIndicator", 16, 5, ImColor(0, 0, 0), ImColor(255, 255, 255), 7.8 * velocity);
			break;
		case SPINNERTRIANGLESSELETOR:
			_spinner.SpinnerTrianglesSeletor("SpinnerTrianglesSeletor", 16, 8, ImColor(0, 0, 0), ImColor(255, 255, 255), 4.8 * velocity, 8);
			break;
		case SPINNERFLOWINGFRADIENT:
			_spinner.SpinnerFlowingGradient("SpinnerFlowingFradient", 16, 6, ImColor(200, 80, 0), ImColor(80, 80, 80), 5 * velocity, IM_PI * 2);
			break;
		case SPINNERROTATESEGMENTS:
			_spinner.SpinnerRotateSegments("SpinnerRotateSegments", 16, 4, ImColor(255, 255, 255), 3 * velocity, 4);
			break;
		case SPINNERROTATESEGMENTS2:
			_spinner.SpinnerRotateSegments("SpinnerRotateSegments2", 16, 3, ImColor(255, 255, 255), 2.4 * velocity, 4, 2);
			break;
		case SPINNERROTATESEGMENTS3:
			_spinner.SpinnerRotateSegments("SpinnerRotateSegments3", 16, 2, ImColor(255, 255, 255), 2.1 * velocity, 4, 3);
			break;
		case SPINNERLEMNISCATE:
			_spinner.SpinnerLemniscate("SpinnerLemniscate", 20, 3, ImColor(255, 255, 255), 2.1 * velocity, 3);
			break;
		case SPINNERROTATEGEAR:
			_spinner.SpinnerRotateGear("SpinnerRotateGear", 16, 6, ImColor(255, 255, 255), 2.1 * velocity, 8);
			break;
		case SPINNERROTATEDATOM:
			_spinner.SpinnerAtom("SpinnerAtom", 16, 2, ImColor(255, 255, 255), 4.1 * velocity, 3);
			break;
		case SPINNERATOM:
			_spinner.SpinnerRotatedAtom("SpinnerRotatedAtom", 16, 2, ImColor(255, 255, 255), 2.1 * velocity, 3);
			break;
		case SPINNERRAINBOWBALLS:
			_spinner.SpinnerRainbowBalls("SpinnerRainbowBalls", 16, 4, ImColor(0, 0, 0, 0), 1.5 * velocity, 5);
			break;
		case SPINNERCAMERA:
			_spinner.SpinnerCamera(
					"SpinnerCamera", 16, 8, [](int i) { return ImColor::HSV(i * 0.25, 0.8, 0.8); }, 4.8 * velocity, 8);
			break;
		case SPINNERARCPOLARFADE:
			_spinner.SpinnerArcPolarFade("SpinnerArcPolarFade", 16, ImColor(255, 255, 255), 6 * velocity, 6);
			break;
		case SPINNERARCPOLARRADIUS:
			_spinner.SpinnerArcPolarRadius("SpinnerArcPolarRadius", 16, ImColor::HSV(0.25, 0.8, 0.8), 6 * velocity, 6);
			break;
	}
}

SpinnerCanvas::SpinnerCanvas() {
	velocity = 1;
	hue = 0;
}

// Node2D

void Spinner::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_DRAW: {
			canvas->draw_spinners(spinner_variant);
		}
	}
}

void Spinner::_bind_methods() {
}

Spinner::Spinner() {
	spinner_variant = 0;
	canvas = newref(SpinnerCanvas);
}
