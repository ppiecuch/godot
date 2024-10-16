/**************************************************************************/
/*  round_progress.h                                                      */
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

/*************************************************************************/
/* Authors and contributors:                                             */
/*                                                                       */
/* Copyright (c) 2015-2017 Gauvain "GovanifY" Roussel-Tarbouriech        */
/* Copyright (c) 2016-2017 António "Keyaku" Sarmento                     */
/* Copyright (c) 2015-2017 SKYNISM                                       */
/*************************************************************************/

#ifndef ROUND_PROGRESS_BAR_H
#define ROUND_PROGRESS_BAR_H

#include "scene/gui/range.h"

class RoundProgress : public Range {
	GDCLASS(RoundProgress, Range);

	bool value_visible;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_value_visible(bool p_visible);
	bool is_value_visible() const;

	Size2 get_minimum_size() const;
	RoundProgress();
};

#endif // ROUND_PROGRESS_BAR_H
