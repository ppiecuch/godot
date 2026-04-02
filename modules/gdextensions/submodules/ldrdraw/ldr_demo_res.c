/**************************************************************************/
/*  ldr_demo_res.c                                                        */
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

/* Embedded LDraw demo models for editor/tools builds */

#ifdef TOOLS_ENABLED

#undef INCBIN_PREFIX
#define INCBIN_PREFIX
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_SILENCE_BITCODE_WARNING
#include "misc/incbin.h"

INCBIN(ldr_demo_car, "submodules/ldrdraw/demo/Car.dat");
INCBIN(ldr_demo_ship, "submodules/ldrdraw/demo/Ship.mpd");
INCBIN(ldr_demo_viper, "submodules/ldrdraw/demo/Viper.mpd");
INCBIN(ldr_demo_millennium_falcon, "submodules/ldrdraw/demo/4488 - Millennium Falcon - Mini.mpd");
INCBIN(ldr_demo_at_at, "submodules/ldrdraw/demo/4489 - AT-AT - Mini.mpd");
INCBIN(ldr_demo_at_st, "submodules/ldrdraw/demo/30054 - AT-ST - Mini.mpd");
INCBIN(ldr_demo_vulture_droid, "submodules/ldrdraw/demo/30055 - Vulture Droid - Mini.mpd");
INCBIN(ldr_demo_attack_cruiser, "submodules/ldrdraw/demo/30053 - Republic Attack Cruiser - Mini.mpd");

#endif /* TOOLS_ENABLED */
