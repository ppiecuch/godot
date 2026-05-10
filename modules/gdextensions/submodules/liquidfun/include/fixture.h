/**************************************************************************/
/*  fixture.h                                                             */
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

#ifndef BOX2D_FIXTURE_H
#define BOX2D_FIXTURE_H

#include "core/reference.h"

class FixtureB2 : public Object {
	GDCLASS(FixtureB2, Object);
	BOX2D_CLASS(Fixture);

protected:
	static void _bind_methods();

public:
	/** Box2D methods */
	Ref<class ShapeB2> get_shape() const;

	bool is_sensor() const;
	void set_sensor(bool);

	int get_filter_category() const;
	void set_filter_category(int);

	int get_filter_mask() const;
	void set_filter_mask(int);

	int get_filter_group() const;
	void set_filter_group(int);

	void refilter();

	class BodyB2 *get_body() const;

	Variant get_metadata() const;
	void set_metadata(const Variant &);

	bool test_point(const Vector2 &) const;
	Dictionary ray_cast(const Vector2 &a, const Vector2 &b, int child = 0) const;

	Dictionary get_mass_data() const;

	float get_density() const;
	void set_density(float);

	float get_friction() const;
	void set_friction(float);

	float get_restitution() const;
	void set_restitution(float);

	Rect2 get_aabb(int child = 0) const;
};

class FixtureDefB2 : public Reference {
	GDCLASS(FixtureDefB2, Reference);

protected:
	static void _bind_methods();

	/** Internal definition */
	struct b2FixtureDef *def;
	Ref<class ShapeB2> shape;
	Variant metadata;

public:
	/** Lifecycle */
	FixtureDefB2();
	~FixtureDefB2();

	/** Getters/setters */
	BOX2D_GET_SET_DATA(Ref<class ShapeB2>, shape);
	BOX2D_GET_SET(float, friction);
	BOX2D_GET_SET(float, restitution);
	BOX2D_GET_SET(float, density);
	BOX2D_GET_SET(bool, sensor);
	BOX2D_GET_SET(int, filter_category);
	BOX2D_GET_SET(int, filter_mask);
	BOX2D_GET_SET(int, filter_group);
	BOX2D_GET_SET_DATA(Variant, metadata);

	/** Create fixture */
	class FixtureB2 *instance(class BodyB2 *);
};

#endif // BOX2D_FIXTURE_H
