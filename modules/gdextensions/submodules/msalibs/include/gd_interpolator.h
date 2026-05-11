/**************************************************************************/
/*  gd_interpolator.h                                                     */
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

#ifndef GD_INTERPOLATOR_H
#define GD_INTERPOLATOR_H

#include "core/math/vector2.h"
#include "core/object.h"
#include "core/pool_vector.h"
#include "core/reference.h"

#include "msainterpolator.h"

// 2D Catmull-Rom / linear spline. Control points are Vector2 values.
// Usage:
//   var s = MSASpline2D.new()
//   s.push(Vector2(0, 0)); s.push(Vector2(100, 50)); s.push(Vector2(200, 0))
//   var pt = s.sample(0.5)        # midpoint on spline
//   var pts = s.sample_array(32)  # 32 evenly-spaced PoolVector2Array
class MSASpline2D : public Reference {
	GDCLASS(MSASpline2D, Reference);

	mutable msa::InterpolatorT<Vector2> _interp;

protected:
	static void _bind_methods();

public:
	void push(Vector2 p);
	void clear();
	void reserve(int n);
	int get_size() const;
	Vector2 get_control_point(int i) const;

	void set_cubic(bool b);
	bool get_cubic() const;

	void set_use_length(bool b);
	bool get_use_length() const;

	void set_length_subdivisions(int n);
	int get_length_subdivisions() const;

	float get_length() const;

	Vector2 sample(float t);
	PoolVector2Array sample_array(int count);

	MSASpline2D();
};

#endif // GD_INTERPOLATOR_H
