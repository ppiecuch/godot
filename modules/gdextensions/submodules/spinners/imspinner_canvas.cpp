/**************************************************************************/
/*  imspinner_draw.cpp                                                    */
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

#include "gdimspinner.h"

#include "imspinner_imgui.h"

ImGuiWindowCanvas::ImGuiWindowCanvas(CanvasItem *p_canvas) :
		wnd(memnew(ImGuiWindow(p_canvas))) {}
ImGuiWindowCanvas::~ImGuiWindowCanvas() { memdelete(wnd); }

struct SpinnerCanvas::SpinnerCanvasData {
	real_t velocity = 1;
	int hue = 0;
};

void SpinnerCanvas::draw_spinners(ImGuiWindow *p_imgui, int p_spinner) {
	static real_t nextdot1 = 0, nextdot2;
	static ImColor spinner_filling_meb_bg;

	nextdot1 -= 0.07;
	nextdot2 -= 0.2 * data->velocity;

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

	using ImSpinner::red;
	using ImSpinner::white;

	using ImSpinner::PI_2;

	ERR_FAIL_COND_MSG(ImGui::GetCurrentWindow() != p_imgui, "IMGUI window already set");

	ImGui::SetCurrentWindow(p_imgui);

	switch (p_spinner) {
		/* 0 */ case SPINNER : {
			ImSpinner::Spinner<ImSpinner::e_st_rainbow>("Spinner", Radius{ 16 }, Thickness{ 2 }, Color{ ImColor::HSV(++data->hue * 0.005, 0.8, 0.8) }, Speed{ 8 * data->velocity });
		} break;
		case SPINNERANG: {
			ImSpinner::Spinner<ImSpinner::e_st_angle>("SpinnerAng", Radius{ 16 }, Thickness{ 2 }, Color{ white }, BgColor{ ImColor(255, 255, 255, 128) }, Speed{ 8 * data->velocity }, Angle{ IM_PI });
		} break;
		case SPINNERDOTS: {
			ImSpinner::Spinner<ImSpinner::e_st_dots>("SpinnerDots", Radius{ 16 }, Thickness{ 4 }, Color{ white }, FloatPtr{ &nextdot1 }, Speed{ 1 * data->velocity }, Dots{ 12 }, MiddleDots{ 6 }, MinThickness{ -1 });
		} break;
		case SPINNERANGNOBG: {
			ImSpinner::Spinner<ImSpinner::e_st_ang>("SpinnerAngNoBg", Radius{ 16 }, Thickness{ 2 }, Color{ white }, BgColor{ ImColor(255, 255, 255, 0) }, Speed{ 6 * data->velocity }, Angle{ IM_PI });
		} break;
		case SPINNERANG270: {
			ImSpinner::Spinner<ImSpinner::e_st_ang>("SpinnerAng270", Radius{ 16 }, Thickness{ 2 }, Color{ white }, BgColor{ ImColor(255, 255, 255, 128) }, Speed{ 6 * data->velocity }, Angle{ 270 / 360 * 2 * IM_PI });
		} break;
		/* 5 */ case SPINNERANG270NOBG : {
			ImSpinner::Spinner<ImSpinner::e_st_ang>("SpinnerAng270NoBg", Radius{ 16 }, Thickness{ 2 }, Color{ white }, BgColor{ ImColor(255, 255, 255, 0) }, Speed{ 6 * data->velocity }, Angle{ 270 / 360 * 2 * IM_PI });
		} break;
		case SPINNERVDOTS: {
			ImSpinner::Spinner<ImSpinner::e_st_vdots>("SpinnerVDots", Radius{ 16 }, Thickness{ 4 }, Color{ ImColor::HSV(data->hue * 0.001, 0.8, 0.8) }, BgColor{ ImColor::HSV(data->hue * 0.0011, 0.8, 0.8) }, Speed{ real_t(2.7 * data->velocity) }, Dots{ 12 }, MiddleDots{ 6 });
		} break;
		case SPINNERBOUNCEBALL: {
			ImSpinner::SpinnerBounceBall("SpinnerBounceBall", 16, 6, white, 4 * data->velocity);
		} break;
		case SPINNERANGECLIPSE: {
			ImSpinner::SpinnerAngEclipse("SpinnerAngEclipse", 16, 5, white, 6 * data->velocity);
		} break;
		case SPINNERINGYANG: {
			ImSpinner::SpinnerIngYang("SpinnerIngYang", 16, 5, false, 0, white, ImColor(255, 0, 0), 4 * data->velocity, IM_PI * 0.8);
		} break;
		/* 10 */ case SPINNERBARCHARTSINE : {
			ImSpinner::SpinnerBarChartSine("SpinnerBarChartSine", 16, 4, white, 6.8 * data->velocity, 4, 0);
		} break;
		case SPINNERBOUNCEDOTS: {
			ImSpinner::SpinnerBounceDots("SpinnerBounceDots", 6, white, 6 * data->velocity, 3);
		} break;
		case SPINNERFADEDOTS: {
			ImSpinner::SpinnerFadeDots("SpinnerFadeDots", 6, white, 8 * data->velocity, 3);
		} break;
		case SPINNERSCALEDOTS: {
			ImSpinner::SpinnerScaleDots("SpinnerMovingDots", 6, white, 7 * data->velocity, 3);
		} break;
		case SPINNERMOVINGDOTS: {
			ImSpinner::SpinnerMovingDots("SpinnerMovingDots", 6, white, 30 * data->velocity, 3);
		} break;
		/* 15 */ case SPINNERROTATEDOTS : {
			ImSpinner::SpinnerRotateDots("SpinnerRotateDots", 16, 6, white, 4 * data->velocity, 2);
		} break;
		case SPINNERTWINANG: {
			ImSpinner::SpinnerTwinAng("SpinnerTwinAng", 16, 16, 6, white, ImColor(255, 0, 0), 4 * data->velocity);
		} break;
		case SPINNERCLOCK: {
			ImSpinner::SpinnerClock("SpinnerClock", 16, 2, ImColor(255, 0, 0), white, 4 * data->velocity);
		} break;
		case SPINNERINGYANGR: {
			ImSpinner::SpinnerIngYang("SpinnerIngYangR", 16, 5, true, 0.1, white, ImColor(255, 0, 0), 4 * data->velocity, IM_PI * 0.8);
		} break;
		case SPINNERBARCHARTSINE2: {
			ImSpinner::SpinnerBarChartSine("SpinnerBarChartSine2", 16, 4, ImColor::HSV(data->hue * 0.005, 0.8, 0.8), 4.8 * data->velocity, 4, 1);
		} break;
		/* 20 */ case SPINNERTWINANG180 : {
			ImSpinner::SpinnerTwinAng180("SpinnerTwinAng180", 16, 12, 4, white, ImColor(255, 0, 0), 4 * data->velocity);
		} break;
		case SPINNERTWINANG360: {
			ImSpinner::SpinnerTwinAng360("SpinnerTwinAng360", 16, 11, 4, white, ImColor(255, 0, 0), 4 * data->velocity);
		} break;
		case SPINNERINCDOTS: {
			ImSpinner::SpinnerIncDots("SpinnerIncDots", 16, 4, white, 5.6, 6);
		} break;
		case SPINNERDOTSWOBG: {
			ImSpinner::SpinnerDots("SpinnerDotsWoBg", &nextdot2, 16, 4, white, 0.3 * data->velocity, 12, 0);
		} break;
		case SPINNERINCSCALEDOTS: {
			ImSpinner::SpinnerIncScaleDots("SpinnerIncScaleDots", 16, 4, white, 6.6, 6);
		} break;
		/* 25 */ case SPINNERANG90BG : {
			ImSpinner::SpinnerAng("SpinnerAng90Bg", 16, 6, white, ImColor(255, 255, 255, 128), 8 * data->velocity, IM_PI / 2);
		} break;
		case SPINNERANG90: {
			ImSpinner::SpinnerAng("SpinnerAng90", 16, 6, white, ImColor(255, 255, 255, 0), 8.5 * data->velocity, IM_PI / 2);
		} break;
		case SPINNERFADEBARS: {
			ImSpinner::SpinnerFadeBars("SpinnerFadeBars", 10, white, 4.8 * data->velocity, 3);
		} break;
		case SPINNERPULSARSEQ: {
			ImSpinner::SpinnerPulsar("SpinnerPulsar", 16, 2, white, 1 * data->velocity);
		} break;
		case SPINNERINGYANGR2: {
			ImSpinner::SpinnerIngYang("SpinnerIngYangR2", 16, 5, true, 3, white, ImColor(255, 0, 0), 4 * data->velocity, IM_PI * 0.8);
		} break;
		/* 30 */ case SPINNERBARCHARTRAINBOW : {
			ImSpinner::SpinnerBarChartRainbow("SpinnerBarChartRainbow", 16, 4, ImColor::HSV(data->hue * 0.005, 0.8, 0.8), 6.8 * data->velocity, 4);
		} break;
		case SPINNERBARSROTATEFADE: {
			ImSpinner::SpinnerBarsRotateFade("SpinnerBarsRotateFade", 8, 18, 4, white, 7.6, 6);
		} break;
		case SPINNERFADESCALEBARS: {
			ImSpinner::SpinnerFadeBars("SpinnerFadeScaleBars", 10, white, 6.8, 3, true);
		} break;
		case SPINNERBARSSCALEMIDDLE: {
			ImSpinner::SpinnerBarsScaleMiddle("SpinnerBarsScaleMiddle", 6, white, 8.8, 3);
		} break;
		case SPINNERANGTWIN1: {
			ImSpinner::SpinnerAngTwin("SpinnerAngTwin1", 16, 13, 2, ImColor(255, 0, 0), white, 6 * data->velocity, IM_PI / 2);
		} break;
		/* 35 */ case SPINNERANGTWIN2 : {
			ImSpinner::SpinnerAngTwin("SpinnerAngTwin2", 13, 16, 2, ImColor(255, 0, 0), white, 6 * data->velocity, IM_PI / 2);
		} break;
		case SPINNERANGTWIN3: {
			ImSpinner::SpinnerAngTwin("SpinnerAngTwin3", 13, 16, 2, ImColor(255, 0, 0), white, 6 * data->velocity, IM_PI / 2, 2);
		} break;
		case SPINNERANGTWIN4: {
			ImSpinner::SpinnerAngTwin("SpinnerAngTwin4", 16, 13, 2, ImColor(255, 0, 0), white, 6 * data->velocity, IM_PI / 2, 2);
		} break;
		case SPINNERTWINPULSAR: {
			ImSpinner::SpinnerTwinPulsar("SpinnerTwinPulsar", 16, 2, white, 0.5 * data->velocity, 2);
		} break;
		case SPINNERANGTWIN5: {
			ImSpinner::SpinnerAngTwin("SpinnerAngTwin4", 14, 13, 3, ImColor(255, 0, 0), ImColor(0, 0, 0, 0), 5 * data->velocity, IM_PI / 1.5, 2);
		} break;
		/* 40 */ case SPINNERBLOCKS : {
			ImSpinner::SpinnerBlocks("SpinnerBlocks", 16, 7, ImColor(255, 255, 255, 30), ImColor::HSV(data->hue * 0.005, 0.8, 0.8), 5 * data->velocity);
		} break;
		case SPINNERTWINBALL: {
			ImSpinner::SpinnerTwinBall("SpinnerTwinBall", 16, 11, 2, 2.5, ImColor(255, 0, 0), white, 6 * data->velocity, 2);
		} break;
		case SPINNERTWINBALL2: {
			ImSpinner::SpinnerTwinBall("SpinnerTwinBall2", 15, 19, 2, 2, ImColor(255, 0, 0), white, 6 * data->velocity, 3);
		} break;
		case SPINNERTWINBALL3: {
			ImSpinner::SpinnerTwinBall("SpinnerTwinBall2", 16, 16, 2, 5, ImColor(255, 0, 0), white, 5 * data->velocity, 1);
		} break;
		case SPINNERANGTRIPLE: {
			ImSpinner::SpinnerAngTriple("SpinnerAngTriple", 16, 13, 10, 1.3, white, ImColor(255, 0, 0), white, 5 * data->velocity, 1.5 * IM_PI);
		} break;
		/* 45 */ case SPINNERINCFULLDOTS : {
			ImSpinner::SpinnerIncFullDots("SpinnerIncFullDots", 16, 4, white, 5.6, 4);
		} break;
		case SPINNERGOOEYBALLS: {
			ImSpinner::SpinnerGooeyBalls("SpinnerGooeyBalls", 16, white, 2);
		} break;
		case SPINNERROTATEGOOEYBALLS2: {
			ImSpinner::SpinnerRotateGooeyBalls("SpinnerRotateGooeyBalls2", 16, 5, white, 6, 2);
		} break;
		case SPINNERROTATEGOOEYBALLS3: {
			ImSpinner::SpinnerRotateGooeyBalls("SpinnerRotateGooeyBalls3", 16, 5, white, 6, 3);
		} break;
		case SPINNERMOONLINE: {
			ImSpinner::SpinnerMoonLine("SpinnerMoonLine", 16, 3, ImColor(200, 80, 0), ImColor(80, 80, 80), 5 * data->velocity);
		} break;
		/* 50 */ case SPINNERARCROTATION : {
			ImSpinner::SpinnerArcRotation("SpinnerArcRotation", 13, 5, white, 3 * data->velocity, 4);
		} break;
		case SPINNERFLUID: {
			ImSpinner::SpinnerFluid("SpinnerFluid", 16, ImColor(0, 0, 255), 3.8 * data->velocity, 4);
		} break;
		case SPINNERARCFADE: {
			ImSpinner::SpinnerArcFade("SpinnerArcFade", 13, 5, white, 3 * data->velocity, 4);
		} break;
		case SPINNERFILLING: {
			ImSpinner::SpinnerFilling("SpinnerFilling", 16, 6, white, ImColor(255, 0, 0), 4 * data->velocity);
		} break;
		case SPINNERTOPUP: {
			ImSpinner::SpinnerTopup("SpinnerTopup", 16, 12, ImColor(255, 0, 0), ImColor(80, 80, 80), white, 1 * data->velocity);
		} break;
		/* 55 */ case SPINNERFADEPULSAR : {
			ImSpinner::SpinnerFadePulsar("SpinnerFadePulsar", 16, white, 1.5 * data->velocity, 1);
		} break;
		case SPINNERFADEPULSAR2: {
			ImSpinner::SpinnerFadePulsar("SpinnerFadePulsar2", 16, white, 0.9 * data->velocity, 2);
		} break;
		case SPINNERPULSAR: {
			ImSpinner::SpinnerPulsar("SpinnerPulsar", 16, 2, white, 1 * data->velocity, false);
		} break;
		case SPINNERDOUBLEFADEPULSAR: {
			ImSpinner::SpinnerDoubleFadePulsar("SpinnerDoubleFadePulsar", 16, 2, white, 2 * data->velocity);
		} break;
		case SPINNERFILLEDARCFADE: {
			ImSpinner::SpinnerFilledArcFade("SpinnerFilledArcFade", 16, white, 4 * data->velocity, 4);
		} break;
		/* 60 */ case SPINNERFILLEDARCFADE6 : {
			ImSpinner::SpinnerFilledArcFade("SpinnerFilledArcFade6", 16, white, 6 * data->velocity, 6);
		} break;
		case SPINNERFILLEDARCFADE8: {
			ImSpinner::SpinnerFilledArcFade("SpinnerFilledArcFade6", 16, white, 8 * data->velocity, 12);
		} break;
		case SPINNERFILLEDARCCOLOR: {
			ImSpinner::SpinnerFilledArcColor("SpinnerFilledArcColor", 16, ImColor(255, 0, 0), white, 2.8 * data->velocity, 4);
		} break;
		case SPINNERCIRCLEDROP: {
			ImSpinner::SpinnerCircleDrop("SpinnerCircleDrop", 16, 1.5, 4, ImColor(255, 0, 0), white, 2.8 * data->velocity, IM_PI);
		} break;
		case SPINNERSURROUNDEDINDICATOR: {
			ImSpinner::SpinnerSurroundedIndicator("SpinnerSurroundedIndicator", 16, 5, ImColor(0, 0, 0), white, 7.8 * data->velocity);
		} break;
		/* 65 */ case SPINNERTRIANGLESSELETOR : {
			ImSpinner::SpinnerTrianglesSelector("SpinnerTrianglesSeletor", 16, 8, ImColor(0, 0, 0), white, 4.8 * data->velocity, 8);
		} break;
		case SPINNERFLOWINGFRADIENT: {
			ImSpinner::SpinnerFlowingGradient("SpinnerFlowingFradient", 16, 6, ImColor(200, 80, 0), ImColor(80, 80, 80), 5 * data->velocity, IM_PI * 2);
		} break;
		case SPINNERROTATESEGMENTS: {
			ImSpinner::SpinnerRotateSegments("SpinnerRotateSegments", 16, 4, white, 3 * data->velocity, 4);
		} break;
		case SPINNERROTATESEGMENTS2: {
			ImSpinner::SpinnerRotateSegments("SpinnerRotateSegments2", 16, 3, white, 2.4 * data->velocity, 4, 2);
		} break;
		case SPINNERROTATESEGMENTS3: {
			ImSpinner::SpinnerRotateSegments("SpinnerRotateSegments3", 16, 2, white, 2.1 * data->velocity, 4, 3);
		} break;
		/* 70 */ case SPINNERLEMNISCATE : {
			ImSpinner::SpinnerLemniscate("SpinnerLemniscate", 20, 3, white, 2.1 * data->velocity, 3);
		} break;
		case SPINNERROTATEGEAR: {
			ImSpinner::SpinnerRotateGear("SpinnerRotateGear", 16, 6, white, 2.1 * data->velocity, 8);
		} break;
		case SPINNERROTATEDATOM: {
			ImSpinner::SpinnerAtom("SpinnerAtom", 16, 2, white, 4.1 * data->velocity, 3);
		} break;
		case SPINNERATOM: {
			ImSpinner::SpinnerRotatedAtom("SpinnerRotatedAtom", 16, 2, white, 2.1 * data->velocity, 3);
		} break;
		case SPINNERRAINBOWBALLS: {
			ImSpinner::SpinnerRainbowBalls("SpinnerRainbowBalls", 16, 4, ImColor(0, 0, 0, 0), 1.5 * data->velocity, 5);
		} break;
		/* 75 */ case SPINNERCAMERA : {
			ImSpinner::SpinnerCamera(
					"SpinnerCamera", 16, 8, [](int i) { return ImColor::HSV(i * 0.25, 0.8, 0.8); }, 4.8 * data->velocity, 8);
		} break;
		case SPINNERARCPOLARFADE: {
			ImSpinner::SpinnerArcPolarFade("SpinnerArcPolarFade", 16, white, 6 * data->velocity, 6);
		} break;
		case SPINNERARCPOLARRADIUS: {
			ImSpinner::SpinnerArcPolarRadius("SpinnerArcPolarRadius", 16, ImColor::HSV(0.25, 0.8, 0.8), 6 * data->velocity, 6);
		} break;
		case SPINNERARCPOLARPIES: {
			ImSpinner::SpinnerCaleidoscope("SpinnerArcPolarPies", 16, 4, ImColor::HSV(0.25, 0.8, 0.8), 2.6 * data->velocity, 10, 0);
		} break;
		case SPINNERARCPOLARPIES2: {
			ImSpinner::SpinnerCaleidoscope("SpinnerArcPolarPies2", 16, 4, ImColor::HSV(0.35, 0.8, 0.8), 3.2 * data->velocity, 10, 1);
		} break;
		/* 80 */ case SPINNERSCALEBLOCKS : {
			ImSpinner::SpinnerScaleBlocks("SpinnerScaleBlocks", 16, 8, ImColor::HSV(data->hue * 0.005, 0.8, 0.8), 5 * data->velocity);
		} break;
		case SPINNERROTATETRIANGLES: {
			ImSpinner::SpinnerRotateTriangles("SpinnerRotateTriangles", 16, 2, white, 6 * data->velocity, 3);
		} break;
		case SPINNERARCWEDGES: {
			ImSpinner::SpinnerArcWedges("SpinnerArcWedges", 16, ImColor::HSV(0.3, 0.8, 0.8), 2.8 * data->velocity, 4);
		} break;
		case SPINNERSCALESQUARES: {
			ImSpinner::SpinnerScaleSquares("SpinnerScaleSquares", 16, 8, ImColor::HSV(data->hue * 0.005, 0.8, 0.8), 5 * data->velocity);
		} break;
		case SPINNERMOVINGHBODOTS: {
			ImSpinner::SpinnerHboDots("SpinnerMovingDots", 16, 4, white, 0, 0, 1.1 * data->velocity, 6);
		} break;
		/* 85 */ case SPINNERMOVINGHBODOTS2 : {
			ImSpinner::SpinnerHboDots("SpinnerMovingDots2", 16, 4, white, 0.1, 0.5, 1.1 * data->velocity, 6);
		} break;
		case SPINNERBOUNCEBALL3: {
			ImSpinner::Spinner<ImSpinner::e_st_bounce_ball>("SpinnerBounceBall3", Radius{ 16 }, Thickness{ 4 }, Color{ white }, Speed{ 3.2f * data->velocity }, Dots{ 5 });
		} break;
		case SPINNERBOUNCEBALLSHADOW: {
			ImSpinner::SpinnerBounceBall("SpinnerBounceBallShadow", 16, 4, white, 2.2 * data->velocity, 1, true);
		} break;
		case SPINNERBOUNCEBALL5SHADOW: {
			ImSpinner::SpinnerBounceBall("SpinnerBounceBall5Shadow", 16, 4, white, 3.6 * data->velocity, 5, true);
		} break;
		case SPINNERSQUARESTROKEFADE: {
			ImSpinner::SpinnerSquareStrokeFade("SpinnerSquareStrokeFade", 13, 5, white, 3 * data->velocity);
		} break;
			/* 90 */ ImSpinner::SpinnerSquareStrokeFill("SpinnerSquareStrokeFill", 13, 5, white, 3 * data->velocity);
			break;
		case SPINNERSWINGDOTS: {
			ImSpinner::SpinnerSwingDots("SpinnerSwingDots", 16, 6, ImColor(255, 0, 0), 4.1 * data->velocity);
		} break;
		case SPINNERROTATEWHEEL: {
			ImSpinner::SpinnerRotateWheel("SpinnerRotateWheel", 16, 10, ImColor(255, 255, 0), white, 2.1 * data->velocity, 8);
		} break;
		case SPINNERWAVEDOTS: {
			ImSpinner::SpinnerWaveDots("SpinnerWaveDots", 16, 3, white, 6 * data->velocity, 8);
		} break;
		case SPINNERROTATESHAPES: {
			ImSpinner::SpinnerRotateShapes("SpinnerRotateShapes", 16, 2, white, 6 * data->velocity, 4, 4);
		} break;
		/* 95 */ case SPINNERSQUARESTROKELOANDING : {
			ImSpinner::SpinnerSquareStrokeLoading("SpinnerSquareStrokeLoanding", 13, 5, white, 3 * data->velocity);
		} break;
		case SPINNERSINSQUARES: {
			ImSpinner::SpinnerSinSquares("SpinnerSinSquares", 16, 2, white, 1 * data->velocity);
		} break;
		case SPINNERZIPDOTS: {
			ImSpinner::SpinnerZipDots("SpinnerZipDots", 16, 3, white, 6 * data->velocity, 5);
		} break;
		case SPINNERDOTSTOBAR: {
			ImSpinner::SpinnerDotsToBar("SpinnerDotsToBar", 16, 3, 0.5, ImColor::HSV(0.31, 0.8, 0.8), 5 * data->velocity, 5);
		} break;
		case SPINNERSINEARCS: {
			ImSpinner::SpinnerSineArcs("SpinnerSineArcs", 16, 1, white, 3 * data->velocity);
		} break;
		/* 100 */ case SPINNERTRIANGLESSHIFT : {
			ImSpinner::SpinnerTrianglesShift("SpinnerTrianglesShift", 16, 8, ImColor(0, 0, 0), white, 1.8 * data->velocity, 8);
		} break;
		case SPINNERCIRCULARLINES: {
			ImSpinner::SpinnerCircularLines("SpinnerCircularLines", 16, white, 1.5 * data->velocity, 8);
		} break;
		case SPINNERLOADINGRING: {
			ImSpinner::SpinnerLoadingRing("SpinnerLoadingRing", 16, 6, red, ImColor(255, 255, 255, 128), 1 * data->velocity, 5);
		} break;
		case SPINNERPATTERNRINGS: {
			ImSpinner::SpinnerPatternRings("SpinnerPatternRings", 16, 2, white, 4.1 * data->velocity, 3);
		} break;
		case SPINNERPATTERNSPHERE: {
			ImSpinner::SpinnerPatternSphere("SpinnerPatternSphere", 16, 2, white, 2.1 * data->velocity, 6);
		} break;
		/* 105 */ case SPINNERRINGSNCHRONOUS : {
			ImSpinner::SpinnerRingSynchronous("SpinnerRingSnchronous", 16, 2, white, 2.1 * data->velocity, 3);
		} break;
		case SPINNERRINGWATERMARKS: {
			ImSpinner::SpinnerRingWatermarks("SpinnerRingWatermarks", 16, 2, white, 2.1 * data->velocity, 3);
		} break;
		case SPINNERFILLEDARCRING: {
			ImSpinner::SpinnerFilledArcRing("SpinnerFilledArcRing", 16, 6, red, white, 2.8 * data->velocity, 8);
		} break;
		case SPINNERPOINTSSHIFT: {
			ImSpinner::SpinnerPointsShift("SpinnerPointsShift", 16, 3, ImColor(0, 0, 0), white, 1.8 * data->velocity, 10);
		} break;
		case SPINNERCIRCULARPOINTS: {
			ImSpinner::SpinnerCircularPoints("SpinnerCircularPoints", 16, 1.2, white, 10 * data->velocity, 7);
		} break;
		/* 110 */ case SPINNERCURVEDCIRCLE : {
			ImSpinner::SpinnerCurvedCircle("SpinnerCurvedCircle", 16, 1.2, white, 1 * data->velocity, 3);
		} break;
		case SPINNERMODCIRCLRE: {
			ImSpinner::SpinnerModCircle("SpinnerModCirclre", 16, 1.2, white, 1, 2, 3 * data->velocity);
		} break;
		case SPINNERMODCIRCLRE2: {
			ImSpinner::SpinnerModCircle("SpinnerModCirclre2", 16, 1.2, white, 1.11, 3.33, 3 * data->velocity);
		} break;
		case SPINNERPATTERNECLIPSE: {
			ImSpinner::SpinnerPatternEclipse("SpinnerPatternEclipse", 16, 2, white, 4.1 * data->velocity, 5, 2, 0);
		} break;
		case SPINNERPATTERNECLIPSE2: {
			ImSpinner::SpinnerPatternEclipse("SpinnerPatternEclipse2", 16, 2, white, 4.1 * data->velocity, 9, 4, 1);
		} break;
		/* 115 */ case SPINNERMULTIFADEDOTS : {
			ImSpinner::SpinnerMultiFadeDots("SpinnerMultiFadeDots", 16, 2, white, 8 * data->velocity, 8);
		} break;
		case SPINNERRAINBOWSHOT: {
			ImSpinner::SpinnerRainbowShot("SpinnerRainbowShot", 16, 4, ImColor::HSV(0.25, 0.8, 0.8, 0), 1.5 * data->velocity, 5);
		} break;
		case SPINNERSPIRAL: {
			ImSpinner::SpinnerSpiral("SpinnerSpiral", 16, 2, white, 6 * data->velocity, 5);
		} break;
		case SPINNERSPIRALEYE: {
			ImSpinner::SpinnerSpiralEye("SpinnerSpiralEye", 16, 1, white, 3 * data->velocity);
		} break;
		case SPINNERWIFIINDICATOR: {
			ImSpinner::SpinnerWifiIndicator("SpinnerWifiIndicator", 16, 1.5, ImColor(0, 0, 0), white, 7.8 * data->velocity, 5.52, 3);
		} break;
		/* 120 */ case SPINNERMOVINGDOTS3 : {
			ImSpinner::SpinnerHboDots("SpinnerMovingDots3", 16, 2, white, 0, 0, 1.1 * data->velocity, 10);
		} break;
		case SPINNERMOVINGDOTS4: {
			ImSpinner::SpinnerHboDots("SpinnerMovingDots4", 16, 4, white, 0.1, 0.5, 1.1 * data->velocity, 2);
		} break;
		case SPINNERMOVINGDOTS5: {
			ImSpinner::SpinnerHboDots("SpinnerMovingDots5", 16, 4, white, 0.1, 0.5, 1.1 * data->velocity, 3);
		} break;
		case SPINNERDNADOTSH: {
			ImSpinner::SpinnerDnaDots("SpinnerDnaDotsH", 16, 3, white, 2 * data->velocity, 8, 0.25);
		} break;
		case SPINNERDNADOTSV: {
			ImSpinner::SpinnerDnaDots("SpinnerDnaDotsV", 16, 3, white, 2 * data->velocity, 8, 0.25, true);
		} break;
		/* 125 */ case SPINNERROTATEDOTS2 : {
			ImSpinner::SpinnerRotateDots("SpinnerRotateDots2", 16, 6, white, 4 * data->velocity, ImMax<int>(int(ImSin((float)ImGui::GetTime() * 0.5) * 8), 3));
		} break;
		case SPINNERSEVENSEGMENTS: {
			ImSpinner::SpinnerSevenSegments("SpinnerSevenSegments", "012345679ABCDEF", 16, 2, white, 4 * data->velocity);
		} break;
		case SPINNERSOLARBALLS: {
			ImSpinner::SpinnerSolarBalls("SpinnerSolarBalls", 16, 4, red, white, 5 * data->velocity, 4);
		} break;
		case SPINNERSOLARARCS: {
			ImSpinner::SpinnerSolarArcs("SpinnerSolarArcs", 16, 4, red, white, 5 * data->velocity, 4);
		} break;
		case SPINNERRAINBOW: {
			ImSpinner::SpinnerRainbow("Spinner", 16, 2, ImColor::HSV(++data->hue * 0.005, 0.8, 0.8), 8 * data->velocity, 0, PI_2, 3);
		} break;
		/* 130 */ case SPINNERROTATINGHEART : {
			ImSpinner::SpinnerRotatingHeart("SpinnerRotatedHeart", 16, 2, red, 8 * data->velocity, 0);
		} break;
		case SPINNERSOLARSCALEBALLS: {
			ImSpinner::SpinnerSolarScaleBalls("SpinnerSolarScaleBalls", 16, 1.3, red, 1 * data->velocity, 36);
		} break;
		case SPINNERORIONDOTS: {
			ImSpinner::SpinnerOrionDots("SpinnerOrionDots", 16, 1.3, white, 4 * data->velocity, 12);
		} break;
		case SPINNERGALAXYDOTS: {
			ImSpinner::SpinnerGalaxyDots("SpinnerGalaxyDots", 16, 1.3, white, 0.2 * data->velocity, 6);
		} break;
		case SPINNERASCIISYMBOLPOINTS: {
			ImSpinner::SpinnerAsciiSymbolPoints("SpinnerAsciiSymbolPoints", "012345679ABCDEF", 16, 2, white, 4 * data->velocity);
		} break;
		/* 135 */ case SPINNERRAINBOWCIRCLE : {
			ImSpinner::SpinnerRainbowCircle("SpinnerRainbowCircle", 16, 4, ImColor::HSV(0.25, 0.8, 0.8), 1 * data->velocity, 4);
		} break;
		case SPINNERRAINBOWCIRCLE2: {
			ImSpinner::SpinnerRainbowCircle("SpinnerRainbowCircle2", 16, 2, ImColor::HSV(data->hue * 0.001, 0.8, 0.8), 2 * data->velocity, 8, 0);
		} break;
		case SPINNERVDOTS2: {
			constexpr real_t speed = 2.1;
			ImSpinner::Spinner<ImSpinner::e_st_vdots>("SpinnerVDots2", Radius{ 16 }, Thickness{ 4 }, Color{ white }, BgColor{ ImColor::HSV(data->hue * 0.0011, 0.8, 0.8) }, Speed{ speed * data->velocity }, Dots{ 2 }, MiddleDots{ 6 });
		} break;
		case SPINNERVDOTS3: {
			constexpr real_t speed = 2.9;
			ImSpinner::Spinner<ImSpinner::e_st_vdots>("SpinnerVDots3", Radius{ 16 }, Thickness{ 4 }, Color{ white }, BgColor{ ImColor::HSV(data->hue * 0.0011, 0.8, 0.8) }, Speed{ speed * data->velocity }, Dots{ 3 }, MiddleDots{ 6 });
		} break;
		case SPINNERSQUARERANDOMDOTS: {
			ImSpinner::SpinnerSquareRandomDots("SpinnerSquareRandomDots", 16, 2.8, ImColor(255, 255, 255, 30), ImColor::HSV(data->hue * 0.005, 0.8, 0.8), 5 * data->velocity);
		} break;
		/* 140 */ case SPINNERFLUIDPOINTS : {
			ImSpinner::SpinnerFluidPoints("SpinnerFluidPoints", 16, 2.8, ImColor(0, 0, 255), 3.8 * data->velocity, Dots{ 4 }, 0.45);
		} break;
		case SPINNERDOTSLOADING: {
			ImSpinner::SpinnerDotsLoading("SpinnerDotsLoading", 16, 4, white, white, 2 * data->velocity);
		} break;
		case SPINNERDOTSTOPOINTS: {
			ImSpinner::SpinnerDotsToPoints("SpinnerDotsToPoints", 16, 3, 0.5, ImColor::HSV(0.31, 0.8, 0.8), 1.8 * data->velocity, 5);
		} break;
		case SPINNERTHREEDOTS: {
			ImSpinner::SpinnerThreeDots("SpinnerThreeDots", 16, 6, white, 4 * data->velocity, 8);
		} break;
		case SPINNER4CALEIDOSPCOPE: {
			ImSpinner::Spinner4Caleidospcope("Spinner4Caleidospcope", 16, 6, ImColor::HSV(data->hue * 0.0031, 0.8, 0.8), 4 * data->velocity, 8);
		} break;
		/* 145 */ case SPINNERSIXDOTS : {
			ImSpinner::SpinnerFiveDots("SpinnerSixDots", 16, 6, white, 4 * data->velocity, 8);
		} break;
		case SPINNERFILLINGMEM: {
			ImSpinner::SpinnerFillingMem("SpinnerFillingMem", 16, 6, ImColor::HSV(data->hue * 0.001, 0.8, 0.8), spinner_filling_meb_bg, 4 * data->velocity);
		} break;
	}
	ImGui::SetCurrentWindow(nullptr);
}

SpinnerCanvas::SpinnerCanvas() :
		data(memnew(SpinnerCanvasData)) {}
SpinnerCanvas::~SpinnerCanvas() { memdelete(data); }
