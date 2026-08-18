/**************************************************************************/
/*  secondary_display.h                                                   */
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

#ifndef SECONDARY_DISPLAY_H
#define SECONDARY_DISPLAY_H

#include "core/math/vector2.h"
#include "core/os/mutex.h"
#include "core/vector.h"

#include <stdint.h>

/* Optional auxiliary screen a platform can expose (see CONSOLE.md sections 3 and 11).
 *
 * The only client so far is the debug console, which blits its character grid there with
 * console_blit() instead of going through the VisualServer: the panel is text at ~5 Hz, so
 * a second GL context would cost far more than it is worth.
 *
 * Platforms without such a screen simply never instantiate a subclass, and
 * get_singleton() keeps returning null -- every caller must handle that.
 *
 * Threading: the surface may appear and disappear from a platform UI thread at any time.
 * Implementations are responsible for their own locking; lock_buffer() ... unlock_buffer()
 * is expected to keep the surface alive for the duration of the blit.
 */
class SecondaryDisplay {
	static SecondaryDisplay *singleton;

public:
	enum TouchType {
		TOUCH_DOWN,
		TOUCH_UP,
		TOUCH_MOVE,
	};

	struct TouchEvent {
		Vector2 position; // pixels, in secondary display space
		TouchType type;
		int index; // platform pointer id, so multi-touch streams stay separable
		uint64_t time_msec; // monotonic, stamped when the platform reported the event

		bool is_pressed() const { return type == TOUCH_DOWN; }
	};

	static SecondaryDisplay *get_singleton() { return singleton; }

	// True once a surface is attached and large enough to draw on.
	virtual bool is_available() const = 0;
	virtual Size2i get_size() const = 0;
	virtual int get_dpi() const = 0;

	// Locks the surface for writing. Returns the pixel buffer (32 bpp, the layout reported
	// by get_pixel_format()) or null when the surface went away; r_stride_px is the row
	// stride in *pixels*. Every successful call must be paired with unlock_and_post().
	virtual uint32_t *lock_buffer(int &r_stride_px, Size2i &r_size) = 0;
	virtual void unlock_and_post() = 0;

	// Touches collected from the platform UI thread, drained by the console on the main
	// thread. Returns false when the queue is empty.
	virtual bool pop_touch(TouchEvent &r_event) = 0;

	SecondaryDisplay();
	virtual ~SecondaryDisplay();
};

#endif // SECONDARY_DISPLAY_H
