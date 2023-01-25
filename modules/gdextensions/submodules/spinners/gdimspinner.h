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

enum SpinnerVariant {
	/*   0 */ SPINNER,
	/*   1 */ SPINNERANG,
	/*   2 */ SPINNERDOTS,
	/*   3 */ SPINNERANGNOBG,
	/*   4 */ SPINNERANG270,
	/*   5 */ SPINNERANG270NOBG,
	/*   6 */ SPINNERVDOTS,
	/*   7 */ SPINNERBOUNCEBALL,
	/*   8 */ SPINNERANGECLIPSE,
	/*   9 */ SPINNERINGYANG,
	/*  10 */ SPINNERBARCHARTSINE,
	/*  11 */ SPINNERBOUNCEDOTS,
	/*  12 */ SPINNERFADEDOTS,
	/*  13 */ SPINNERSCALEDOTS,
	/*  14 */ SPINNERMOVINGDOTS,
	/*  15 */ SPINNERROTATEDOTS,
	/*  16 */ SPINNERTWINANG,
	/*  17 */ SPINNERCLOCK,
	/*  18 */ SPINNERINGYANGR,
	/*  19 */ SPINNERBARCHARTSINE2,
	/*  20 */ SPINNERTWINANG180,
	/*  21 */ SPINNERTWINANG360,
	/*  22 */ SPINNERINCDOTS,
	/*  23 */ SPINNERDOTSWOBG,
	/*  24 */ SPINNERINCSCALEDOTS,
	/*  25 */ SPINNERANG90BG,
	/*  26 */ SPINNERANG90,
	/*  27 */ SPINNERFADEBARS,
	/*  28 */ SPINNERPULSARSEQ,
	/*  29 */ SPINNERINGYANGR2,
	/*  30 */ SPINNERBARCHARTRAINBOW,
	/*  31 */ SPINNERBARSROTATEFADE,
	/*  32 */ SPINNERFADESCALEBARS,
	/*  33 */ SPINNERBARSSCALEMIDDLE,
	/*  34 */ SPINNERANGTWIN1,
	/*  35 */ SPINNERANGTWIN2,
	/*  36 */ SPINNERANGTWIN3,
	/*  37 */ SPINNERANGTWIN4,
	/*  38 */ SPINNERTWINPULSAR,
	/*  39 */ SPINNERANGTWIN5,
	/*  40 */ SPINNERBLOCKS,
	/*  41 */ SPINNERTWINBALL,
	/*  42 */ SPINNERTWINBALL2,
	/*  43 */ SPINNERTWINBALL3,
	/*  44 */ SPINNERANGTRIPLE,
	/*  45 */ SPINNERINCFULLDOTS,
	/*  46 */ SPINNERGOOEYBALLS,
	/*  47 */ SPINNERROTATEGOOEYBALLS2,
	/*  48 */ SPINNERROTATEGOOEYBALLS3,
	/*  49 */ SPINNERMOONLINE,
	/*  50 */ SPINNERARCROTATION,
	/*  51 */ SPINNERFLUID,
	/*  52 */ SPINNERARCFADE,
	/*  53 */ SPINNERFILLING,
	/*  54 */ SPINNERTOPUP,
	/*  55 */ SPINNERFADEPULSAR,
	/*  56 */ SPINNERFADEPULSAR2,
	/*  57 */ SPINNERPULSAR,
	/*  58 */ SPINNERDOUBLEFADEPULSAR,
	/*  59 */ SPINNERFILLEDARCFADE,
	/*  60 */ SPINNERFILLEDARCFADE6,
	/*  61 */ SPINNERFILLEDARCFADE8,
	/*  62 */ SPINNERFILLEDARCCOLOR,
	/*  63 */ SPINNERCIRCLEDROP,
	/*  64 */ SPINNERSURROUNDEDINDICATOR,
	/*  65 */ SPINNERTRIANGLESSELETOR,
	/*  66 */ SPINNERFLOWINGFRADIENT,
	/*  67 */ SPINNERROTATESEGMENTS,
	/*  68 */ SPINNERROTATESEGMENTS2,
	/*  69 */ SPINNERROTATESEGMENTS3,
	/*  70 */ SPINNERLEMNISCATE,
	/*  71 */ SPINNERROTATEGEAR,
	/*  72 */ SPINNERROTATEDATOM,
	/*  73 */ SPINNERATOM,
	/*  74 */ SPINNERRAINBOWBALLS,
	/*  75 */ SPINNERCAMERA,
	/*  76 */ SPINNERARCPOLARFADE,
	/*  77 */ SPINNERARCPOLARRADIUS,
	/*  78 */ SPINNERARCPOLARPIES,
	/*  79 */ SPINNERARCPOLARPIES2,
	/*  80 */ SPINNERSCALEBLOCKS,
	/*  81 */ SPINNERROTATETRIANGLES,
	/*  82 */ SPINNERARCWEDGES,
	/*  83 */ SPINNERSCALESQUARES,
	/*  84 */ SPINNERMOVINGDHBOOTS,
	/*  85 */ SPINNERMOVINGHBODOTS2,
	/*  86 */ SPINNERBOUNCEBALL3,
	/*  87 */ SPINNERBOUNCEBALLSHADOW,
	/*  88 */ SPINNERBOUNCEBALL5SHADOW,
	/*  89 */ SPINNERSQUARESTROKEFADE,
	/*  90 */ SPINNERSQUARESTROKEFILL,
	/*  91 */ SPINNERSWINGDOTS,
	/*  92 */ SPINNERROTATEWHEEL,
	/*  93 */ SPINNERWAVEDOTS,
	/*  94 */ SPINNERROTATESHAPES,
	/*  95 */ SPINNERSQUARESTROKELOANDING,
	/*  96 */ SPINNERSINSQUARES,
	/*  97 */ SPINNERZIPDOTS,
	/*  98 */ SPINNERDOTSTOBAR,
	/*  99 */ SPINNERSINEARCS,
	/* 100 */ SPINNERTRIANGLESSHIFT,
	/* 101 */ SPINNERCIRCULARLINES,
	/* 102 */ SPINNERLOADINGRING,
	/* 103 */ SPINNERPATTERNRINGS,
	/* 104 */ SPINNERPATTERNSPHERE,
	/* 105 */ SPINNERRINGSNCHRONOUS,
	/* 106 */ SPINNERRINGWATERMARKS,
	/* 107 */ SPINNERFILLEDARCRING,
	/* 108 */ SPINNERPOINTSSHIFT,
	/* 109 */ SPINNERCIRCULARPOINTS,
	/* 110 */ SPINNERCURVEDCIRCLE,
	/* 111 */ SPINNERMODCIRCLRE,
	/* 112 */ SPINNERMODCIRCLRE2,
	/* 113 */ SPINNERPATTERNECLIPSE,
	/* 114 */ SPINNERPATTERNECLIPSE2,
	/* 115 */ SPINNERMULTIFADEDOTS,
	/* 116 */ SPINNERRAINBOWSHOT,
	/* 117 */ SPINNERSPIRAL,
	/* 118 */ SPINNERSPIRALEYE,
	/* 119 */ SPINNERWIFIINDICATOR,
	/* 120 */ SPINNERMOVINGDOTS3,
	/* 121 */ SPINNERMOVINGDOTS4,
	/* 122 */ SPINNERMOVINGDOTS5,
	/* 123 */ SPINNERDNADOTSH,
	/* 124 */ SPINNERDNADOTSV,
	/* 125 */ SPINNERROTATEDOTS2,
	/* 126 */ SPINNERSEVENSEGMENTS,
	/* 127 */ SPINNERSOLARBALLS,
	/* 128 */ SPINNERSOLARARCS,
	/* 129 */ SPINNERRAINBOW,
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
	bool spinner_active;
	void draw_spinners(int p_spinner);

protected:
	static void _bind_methods();
	void _notification(int p_notification);

public:
	void set_spinner_active(bool p_active);
	bool get_spinner_active() const;

	void set_spinner_variant(int p_variant);
	int get_spinner_variant() const;

	Spinner();
};

VARIANT_ENUM_CAST(SpinnerVariant);
