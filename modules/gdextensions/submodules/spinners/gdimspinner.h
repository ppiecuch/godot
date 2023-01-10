/*************************************************************************/
/*  gdimspinner.h                                                        */
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

#include "scene/2d/node_2d.h"

class SpinnerCanvas;

enum {
	SPINNER,
	SPINNERANG,
	SPINNERDOTS,
	SPINNERANGNOBG,
	SPINNERANG270,
	SPINNERANG270NOBG,
	SPINNERVDOTS,
	SPINNERBOUNCEBALL,
	SPINNERANGECLIPSE,
	SPINNERINGYANG,
	SPINNERBARCHARTSINE,
	SPINNERBOUNCEDOTS,
	SPINNERFADEDOTS,
	SPINNERSCALEDOTS,
	SPINNERMOVINGDOTS,
	SPINNERROTATEDOTS,
	SPINNERTWINANG,
	SPINNERCLOCK,
	SPINNERINGYANGR,
	SPINNERBARCHARTSINE2,
	SPINNERTWINANG180,
	SPINNERTWINANG360,
	SPINNERINCDOTS,
	SPINNERDOTSWOBG,
	SPINNERINCSCALEDOTS,
	SPINNERANG90BG,
	SPINNERANG90,
	SPINNERFADEBARS,
	SPINNERPULSARSEQ,
	SPINNERINGYANGR2,
	SPINNERBARCHARTRAINBOW,
	SPINNERBARSROTATEFADE,
	SPINNERFADESCALEBARS,
	SPINNERBARSSCALEMIDDLE,
	SPINNERANGTWIN1,
	SPINNERANGTWIN2,
	SPINNERANGTWIN3,
	SPINNERANGTWIN4,
	SPINNERTWINPULSAR,
	SPINNERANGTWIN5,
	SPINNERBLOCKS,
	SPINNERTWINBALL,
	SPINNERTWINBALL2,
	SPINNERTWINBALL3,
	SPINNERANGTRIPLE,
	SPINNERINCFULLDOTS,
	SPINNERGOOEYBALLS,
	SPINNERROTATEGOOEYBALLS2,
	SPINNERROTATEGOOEYBALLS3,
	SPINNERMOONLINE,
	SPINNERARCROTATION,
	SPINNERARCFADE,
	SPINNERFILLING,
	SPINNERTOPUP,
	SPINNERFADEPULSAR,
	SPINNERFADEPULSAR2,
	SPINNERPULSAR,
	SPINNERDOUBLEFADEPULSAR,
	SPINNERFILLEDARCFADE,
	SPINNERFILLEDARCFADE6,
	SPINNERFILLEDARCFADE8,
	SPINNERFILLEDARCCOLOR,
	SPINNERCIRCLEDROP,
	SPINNERSURROUNDEDINDICATOR,
	SPINNERTRIANGLESSELETOR,
	SPINNERFLOWINGFRADIENT,
	SPINNERROTATESEGMENTS,
	SPINNERROTATESEGMENTS2,
	SPINNERROTATESEGMENTS3,
	SPINNERLEMNISCATE,
	SPINNERROTATEGEAR,
	SPINNERROTATEDATOM,
	SPINNERATOM,
	SPINNERRAINBOWBALLS,
	SPINNERCAMERA,
	SPINNERARCPOLARFADE,
	SPINNERARCPOLARRADIUS,
};

class SpinnerCanvas : public Reference {
	real_t velocity;
	int hue;

public:
	void draw_spinners(int p_spinner);

	SpinnerCanvas();
};

class Spinner : public Node2D {
	GDCLASS(Spinner, Node2D);

	Ref<SpinnerCanvas> canvas;

	int spinner_variant;
	void draw_spinners(int p_spinner);

protected:
	static void _bind_methods();
	void _notification(int p_notification);

public:
	Spinner();
};
