/**************************************************************************/
/*  gd_water_graphics.c                                                   */
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

#undef INCBIN_PREFIX
#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_SILENCE_BITCODE_WARNING

#include "misc/incbin.h"

INCBIN(envmap_png, "submodules/environment/water_2d/res/env.png");

INCBIN(caust0, "submodules/environment/water_2d/res/caust/caust01.png");
INCBIN(caust1, "submodules/environment/water_2d/res/caust/caust02.png");
INCBIN(caust2, "submodules/environment/water_2d/res/caust/caust03.png");
INCBIN(caust3, "submodules/environment/water_2d/res/caust/caust04.png");
INCBIN(caust4, "submodules/environment/water_2d/res/caust/caust05.png");
INCBIN(caust5, "submodules/environment/water_2d/res/caust/caust06.png");
INCBIN(caust6, "submodules/environment/water_2d/res/caust/caust07.png");
INCBIN(caust7, "submodules/environment/water_2d/res/caust/caust08.png");
INCBIN(caust8, "submodules/environment/water_2d/res/caust/caust09.png");
INCBIN(caust9, "submodules/environment/water_2d/res/caust/caust10.png");
INCBIN(caust10, "submodules/environment/water_2d/res/caust/caust11.png");
INCBIN(caust11, "submodules/environment/water_2d/res/caust/caust12.png");
INCBIN(caust12, "submodules/environment/water_2d/res/caust/caust13.png");
INCBIN(caust13, "submodules/environment/water_2d/res/caust/caust14.png");
INCBIN(caust14, "submodules/environment/water_2d/res/caust/caust15.png");
INCBIN(caust15, "submodules/environment/water_2d/res/caust/caust16.png");
INCBIN(caust16, "submodules/environment/water_2d/res/caust/caust17.png");
INCBIN(caust17, "submodules/environment/water_2d/res/caust/caust18.png");
INCBIN(caust18, "submodules/environment/water_2d/res/caust/caust19.png");
INCBIN(caust19, "submodules/environment/water_2d/res/caust/caust20.png");
INCBIN(caust20, "submodules/environment/water_2d/res/caust/caust21.png");
INCBIN(caust21, "submodules/environment/water_2d/res/caust/caust22.png");
INCBIN(caust22, "submodules/environment/water_2d/res/caust/caust23.png");
INCBIN(caust23, "submodules/environment/water_2d/res/caust/caust24.png");
INCBIN(caust24, "submodules/environment/water_2d/res/caust/caust25.png");
INCBIN(caust25, "submodules/environment/water_2d/res/caust/caust26.png");
INCBIN(caust26, "submodules/environment/water_2d/res/caust/caust27.png");
INCBIN(caust27, "submodules/environment/water_2d/res/caust/caust28.png");
INCBIN(caust28, "submodules/environment/water_2d/res/caust/caust29.png");
INCBIN(caust29, "submodules/environment/water_2d/res/caust/caust30.png");
INCBIN(caust30, "submodules/environment/water_2d/res/caust/caust31.png");
INCBIN(caust31, "submodules/environment/water_2d/res/caust/caust32.png");

INCBIN(drop0, "submodules/environment/water_2d/res/drops/drop000.png");
INCBIN(drop1, "submodules/environment/water_2d/res/drops/drop001.png");
INCBIN(drop2, "submodules/environment/water_2d/res/drops/drop002.png");
INCBIN(drop3, "submodules/environment/water_2d/res/drops/drop003.png");
INCBIN(drop4, "submodules/environment/water_2d/res/drops/drop004.png");
INCBIN(drop5, "submodules/environment/water_2d/res/drops/drop005.png");
INCBIN(drop6, "submodules/environment/water_2d/res/drops/drop006.png");
INCBIN(drop7, "submodules/environment/water_2d/res/drops/drop007.png");
INCBIN(drop8, "submodules/environment/water_2d/res/drops/drop008.png");
INCBIN(drop9, "submodules/environment/water_2d/res/drops/drop009.png");
INCBIN(drop10, "submodules/environment/water_2d/res/drops/drop010.png");
INCBIN(drop11, "submodules/environment/water_2d/res/drops/drop011.png");
INCBIN(drop12, "submodules/environment/water_2d/res/drops/drop012.png");
INCBIN(drop13, "submodules/environment/water_2d/res/drops/drop013.png");
INCBIN(drop14, "submodules/environment/water_2d/res/drops/drop014.png");
INCBIN(drop15, "submodules/environment/water_2d/res/drops/drop015.png");
INCBIN(drop16, "submodules/environment/water_2d/res/drops/drop016.png");
INCBIN(drop17, "submodules/environment/water_2d/res/drops/drop017.png");
INCBIN(drop18, "submodules/environment/water_2d/res/drops/drop018.png");
INCBIN(drop19, "submodules/environment/water_2d/res/drops/drop019.png");
INCBIN(drop20, "submodules/environment/water_2d/res/drops/drop020.png");
INCBIN(drop21, "submodules/environment/water_2d/res/drops/drop021.png");
INCBIN(drop22, "submodules/environment/water_2d/res/drops/drop022.png");
INCBIN(drop23, "submodules/environment/water_2d/res/drops/drop023.png");
INCBIN(drop24, "submodules/environment/water_2d/res/drops/drop024.png");
INCBIN(drop25, "submodules/environment/water_2d/res/drops/drop025.png");
INCBIN(drop26, "submodules/environment/water_2d/res/drops/drop026.png");
INCBIN(drop27, "submodules/environment/water_2d/res/drops/drop027.png");
INCBIN(drop28, "submodules/environment/water_2d/res/drops/drop028.png");
INCBIN(drop29, "submodules/environment/water_2d/res/drops/drop029.png");
INCBIN(drop30, "submodules/environment/water_2d/res/drops/drop030.png");
INCBIN(drop31, "submodules/environment/water_2d/res/drops/drop031.png");

INCBIN(noise0, "submodules/environment/water_2d/res/noise/normal01.png");
INCBIN(noise1, "submodules/environment/water_2d/res/noise/normal02.png");
INCBIN(noise2, "submodules/environment/water_2d/res/noise/normal03.png");
INCBIN(noise3, "submodules/environment/water_2d/res/noise/normal04.png");
INCBIN(noise4, "submodules/environment/water_2d/res/noise/normal05.png");
INCBIN(noise5, "submodules/environment/water_2d/res/noise/normal06.png");
INCBIN(noise6, "submodules/environment/water_2d/res/noise/normal07.png");
INCBIN(noise7, "submodules/environment/water_2d/res/noise/normal08.png");
INCBIN(noise8, "submodules/environment/water_2d/res/noise/normal09.png");
INCBIN(noise9, "submodules/environment/water_2d/res/noise/normal10.png");
INCBIN(noise10, "submodules/environment/water_2d/res/noise/normal11.png");
INCBIN(noise11, "submodules/environment/water_2d/res/noise/normal12.png");
INCBIN(noise12, "submodules/environment/water_2d/res/noise/normal13.png");
INCBIN(noise13, "submodules/environment/water_2d/res/noise/normal14.png");
INCBIN(noise14, "submodules/environment/water_2d/res/noise/normal15.png");
INCBIN(noise15, "submodules/environment/water_2d/res/noise/normal16.png");
INCBIN(noise16, "submodules/environment/water_2d/res/noise/normal17.png");
INCBIN(noise17, "submodules/environment/water_2d/res/noise/normal18.png");
INCBIN(noise18, "submodules/environment/water_2d/res/noise/normal19.png");
INCBIN(noise19, "submodules/environment/water_2d/res/noise/normal20.png");
INCBIN(noise20, "submodules/environment/water_2d/res/noise/normal21.png");
INCBIN(noise21, "submodules/environment/water_2d/res/noise/normal22.png");
INCBIN(noise22, "submodules/environment/water_2d/res/noise/normal23.png");
INCBIN(noise23, "submodules/environment/water_2d/res/noise/normal24.png");
INCBIN(noise24, "submodules/environment/water_2d/res/noise/normal25.png");
INCBIN(noise25, "submodules/environment/water_2d/res/noise/normal26.png");
INCBIN(noise26, "submodules/environment/water_2d/res/noise/normal27.png");
INCBIN(noise27, "submodules/environment/water_2d/res/noise/normal28.png");
INCBIN(noise28, "submodules/environment/water_2d/res/noise/normal29.png");
INCBIN(noise29, "submodules/environment/water_2d/res/noise/normal30.png");
INCBIN(noise30, "submodules/environment/water_2d/res/noise/normal31.png");
INCBIN(noise31, "submodules/environment/water_2d/res/noise/normal32.png");
INCBIN(noise32, "submodules/environment/water_2d/res/noise/normal33.png");
INCBIN(noise33, "submodules/environment/water_2d/res/noise/normal34.png");
INCBIN(noise34, "submodules/environment/water_2d/res/noise/normal35.png");
INCBIN(noise35, "submodules/environment/water_2d/res/noise/normal36.png");
INCBIN(noise36, "submodules/environment/water_2d/res/noise/normal37.png");
INCBIN(noise37, "submodules/environment/water_2d/res/noise/normal38.png");
INCBIN(noise38, "submodules/environment/water_2d/res/noise/normal39.png");
INCBIN(noise39, "submodules/environment/water_2d/res/noise/normal40.png");
INCBIN(noise40, "submodules/environment/water_2d/res/noise/normal41.png");
INCBIN(noise41, "submodules/environment/water_2d/res/noise/normal42.png");
INCBIN(noise42, "submodules/environment/water_2d/res/noise/normal43.png");
INCBIN(noise43, "submodules/environment/water_2d/res/noise/normal44.png");
INCBIN(noise44, "submodules/environment/water_2d/res/noise/normal45.png");
INCBIN(noise45, "submodules/environment/water_2d/res/noise/normal46.png");
INCBIN(noise46, "submodules/environment/water_2d/res/noise/normal47.png");
INCBIN(noise47, "submodules/environment/water_2d/res/noise/normal48.png");
INCBIN(noise48, "submodules/environment/water_2d/res/noise/normal49.png");
INCBIN(noise49, "submodules/environment/water_2d/res/noise/normal50.png");
INCBIN(noise50, "submodules/environment/water_2d/res/noise/normal51.png");
INCBIN(noise51, "submodules/environment/water_2d/res/noise/normal52.png");
INCBIN(noise52, "submodules/environment/water_2d/res/noise/normal53.png");
INCBIN(noise53, "submodules/environment/water_2d/res/noise/normal54.png");
INCBIN(noise54, "submodules/environment/water_2d/res/noise/normal55.png");
INCBIN(noise55, "submodules/environment/water_2d/res/noise/normal56.png");
INCBIN(noise56, "submodules/environment/water_2d/res/noise/normal57.png");
INCBIN(noise57, "submodules/environment/water_2d/res/noise/normal58.png");
INCBIN(noise58, "submodules/environment/water_2d/res/noise/normal59.png");
INCBIN(noise59, "submodules/environment/water_2d/res/noise/normal60.png");

#define CAUST(seq) caust##seq##_data

const unsigned char *causticsmaps[] = {
	CAUST(0),
	CAUST(1),
	CAUST(2),
	CAUST(3),
	CAUST(4),
	CAUST(5),
	CAUST(6),
	CAUST(7),
	CAUST(8),
	CAUST(9),
	CAUST(10),
	CAUST(11),
	CAUST(12),
	CAUST(13),
	CAUST(14),
	CAUST(15),
	CAUST(16),
	CAUST(17),
	CAUST(18),
	CAUST(19),
	CAUST(20),
	CAUST(21),
	CAUST(22),
	CAUST(23),
	CAUST(24),
	CAUST(25),
	CAUST(26),
	CAUST(27),
	CAUST(28),
	CAUST(29),
	CAUST(30),
	CAUST(31),
	0
};

#define NOISE(seq) noise##seq##_data

const unsigned char *noisemaps[] = {
	NOISE(0),
	NOISE(1),
	NOISE(2),
	NOISE(3),
	NOISE(4),
	NOISE(5),
	NOISE(6),
	NOISE(7),
	NOISE(8),
	NOISE(9),
	NOISE(10),
	NOISE(11),
	NOISE(12),
	NOISE(13),
	NOISE(14),
	NOISE(15),
	NOISE(16),
	NOISE(17),
	NOISE(18),
	NOISE(19),
	NOISE(20),
	NOISE(21),
	NOISE(22),
	NOISE(23),
	NOISE(24),
	NOISE(25),
	NOISE(26),
	NOISE(27),
	NOISE(28),
	NOISE(29),
	NOISE(30),
	NOISE(31),
	NOISE(32),
	NOISE(33),
	NOISE(34),
	NOISE(35),
	NOISE(36),
	NOISE(37),
	NOISE(38),
	NOISE(39),
	NOISE(40),
	NOISE(41),
	NOISE(42),
	NOISE(43),
	NOISE(44),
	NOISE(45),
	NOISE(46),
	NOISE(47),
	NOISE(48),
	NOISE(49),
	NOISE(50),
	NOISE(51),
	NOISE(52),
	NOISE(53),
	NOISE(54),
	NOISE(55),
	NOISE(56),
	NOISE(57),
	NOISE(58),
	NOISE(59),
	0
};
