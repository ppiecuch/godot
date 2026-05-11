/**************************************************************************/
/*  gd_interpolator.h                                                     */
/**************************************************************************/
/* Godot Reference wrapper around msa::InterpolatorT<Vector2>.            */
/* Catmull-Rom / linear spline supporting optional arc-length param.      */
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
