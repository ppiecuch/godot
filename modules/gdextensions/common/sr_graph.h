/**************************************************************************/
/*  sr_graph.h                                                            */
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

// Base on sr_graph - public domain plot tracer; no warranties implied, use at your own risk.

#ifndef SR_GRAPH_H
#define SR_GRAPH_H

#include "scene/gui/control.h"

typedef int sr_graph_t;

class SRGraph : public Control {
	GDCLASS(SRGraph, Control);

	int plot_history_size;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	int get_plot_history_size() const;
	void set_plot_history_size(int p_size);

	sr_graph_t add_graph(const Point2 &p_min, const Point2 &p_max, real_t p_ratio, const Color &p_bg, const String &p_label = "");

	void set_grid(sr_graph_t p_graph, bool p_visible, const Color &p_color = Color());
	void set_axes(sr_graph_t p_graph, bool p_visible, const Color &p_color = Color());

	int add_curve(sr_graph_t p_graph, const Vector<real_t> &p_xs, const Vector<real_t> &p_ys, const Color &p_color);
	void update_curve(sr_graph_t p_graph, int p_curve, const Vector<real_t> &p_xs, const Vector<real_t> &p_ys);

	SRGraph();
	~SRGraph();
};

#endif // SR_GRAPH_H
