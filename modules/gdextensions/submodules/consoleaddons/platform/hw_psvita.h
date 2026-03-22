/**************************************************************************/
/*  hw_psvita.h                                                           */
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

#ifndef HW_PSVITA_H
#define HW_PSVITA_H

#ifdef PSVITA
#include <psp2/power.h>

static int hw_psvita_get_battery_level() {
	return scePowerGetBatteryLifePercent();
}

static int hw_psvita_get_backlight() {
	// Vita doesn't expose backlight level directly through a simple API;
	// would need sceAVConfig which requires more setup
	return -1;
}

static void hw_psvita_set_backlight(int p_percent) {
	// Placeholder — requires sceAVConfigSetDisplayBrightness
}

#else // !PSVITA — stubs

static int hw_psvita_get_battery_level() { return -1; }
static int hw_psvita_get_backlight() { return -1; }
static void hw_psvita_set_backlight(int p_percent) {}

#endif // PSVITA

#endif // HW_PSVITA_H
