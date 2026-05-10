/**************************************************************************/
/*  vat_animation_track.h                                                 */
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

#ifndef VAT_ANIMATION_TRACK_H
#define VAT_ANIMATION_TRACK_H

#include "core/reference.h"

class VATAnimationTrack : public Reference {
	GDCLASS(VATAnimationTrack, Reference);

	String name;
	int start_frame;
	int end_frame;
	int framerate;
	bool is_looping;

protected:
	static void _bind_methods();

public:
	void set_track(const String &p_name, int p_start, int p_end, int p_framerate, bool p_loop);

	void set_track_name(const String &p_name);
	String get_track_name() const;

	void set_start_frame(int p_frame);
	int get_start_frame() const;

	void set_end_frame(int p_frame);
	int get_end_frame() const;

	void set_framerate(int p_framerate);
	int get_framerate() const;

	void set_is_looping(bool p_loop);
	bool get_is_looping() const;

	String to_string();

	VATAnimationTrack();
};

#endif // VAT_ANIMATION_TRACK_H
