/**************************************************************************/
/*  godot_box2d.h                                                         */
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

#ifndef GODOT_BOX2D_H
#define GODOT_BOX2D_H

#include "core/reference.h"

/***********************************************************************
 * Helpers macros
 **********************************************************************/

#define BOX2D_CLASS(name)                             \
protected:                                            \
	class b2##name *entity;                           \
	Variant metadata;                                 \
                                                      \
public:                                               \
	name##B2(class b2##name *);                       \
	~name##B2();                                      \
	class b2##name *get_b2() const { return entity; } \
	static name##B2 *get(const b2##name *);

#define BOX2D_PROPERTY(cls, name, type)                                      \
	ClassDB::bind_method(D_METHOD("get_" #name), &cls::get_##name);          \
	ClassDB::bind_method(D_METHOD("set_" #name, "value"), &cls::set_##name); \
	ADD_PROPERTY(PropertyInfo(type, #name), "set_" #name, "get_" #name);

#define BOX2D_PROPERTY_ENUM(cls, name, enum)                                 \
	ClassDB::bind_method(D_METHOD("get_" #name), &cls::get_##name);          \
	ClassDB::bind_method(D_METHOD("set_" #name, "value"), &cls::set_##name); \
	ADD_PROPERTY(PropertyInfo(Variant::INT, #name, PROPERTY_HINT_ENUM, #enum), "set_" #name, "get_" #name);

#define BOX2D_PROPERTY_BOOL(cls, name)                                       \
	ClassDB::bind_method(D_METHOD("is_" #name), &cls::is_##name);            \
	ClassDB::bind_method(D_METHOD("set_" #name, "value"), &cls::set_##name); \
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, #name), "set_" #name, "is_" #name);

#define BOX2D_JOINT(name)                      \
private:                                       \
	name##B2(class b2Joint *o) : JointB2(o) {} \
	friend class name##DefB2;

#define BOX2D_GET_SET(type, name) \
	type get_##name() const;      \
	void set_##name(type);
#define BOX2D_GET_SET_DATA(type, name) \
	type get_##name() const;           \
	void set_##name(const type &);
#define BOX2D_IS_SET(name)  \
	bool is_##name() const; \
	void set_##name(bool);

/***********************************************************************
 * Enable conversion from Variant to any class derived from Object
 **********************************************************************/

template <class T>
struct VariantCaster<T *> {
	static T *cast(const Variant &p_variant) {
		return Object::cast_to<T>(p_variant);
	}
};

/***********************************************************************
 * Math interoperability between Box2D/Godot data structures and units
 **********************************************************************/

struct b2Vec2 B2(const struct Vector2 &);
struct Vector2 GD(const struct b2Vec2 &);

struct b2AABB B2(const struct Rect2 &);
struct Rect2 GD(const struct b2AABB &);

struct b2Transform B2(const struct Transform2D &);
struct Transform2D GD(const struct b2Transform &);

#define B2_TO_GD static_cast<float>(GLOBAL_GET("physics/2d/box2d_conversion_factor"))
#define GD_TO_B2 (1.0f / static_cast<float>(GLOBAL_GET("physics/2d/box2d_conversion_factor")))

/***********************************************************************
 * Include entities
 **********************************************************************/

#include "body.h"
#include "contact.h"
#include "fixture.h"
#include "particles.h"
#include "shape.h"
#include "world.h"

#include "distance_joint.h"
#include "friction_joint.h"
#include "gear_joint.h"
#include "joint.h"
#include "motor_joint.h"
#include "mouse_joint.h"
#include "prismatic_joint.h"
#include "pulley_joint.h"
#include "revolute_joint.h"
#include "rope_joint.h"
#include "weld_joint.h"
#include "wheel_joint.h"

/***********************************************************************
 * Type Factory
 **********************************************************************/
class Box2D : public Object {
	GDCLASS(Box2D, Object);

protected:
	static void _bind_methods();

	static Box2D *singleton;

public:
	static Box2D *get() { return singleton; }

	/** Creation methods */
	WorldB2 *world(const Vector2 &gravity);
	BodyB2 *body(WorldB2 *world, int type, const Transform2D &xf = Transform2D());
	FixtureB2 *fixture(BodyB2 *body, const ShapeB2 *shape, float density);

	Ref<ShapeB2> circle(const Vector2 &offset, float radius);
	Ref<ShapeB2> box(const Vector2 &extents, const Vector2 &offset = Vector2(), float angle = 0);
	Ref<ShapeB2> poly(const PoolVector2Array &vertices);
	Ref<ShapeB2> edge(const Vector2 &a, const Vector2 &b);
	Ref<ShapeB2> chain(const PoolVector2Array &vertices, bool loop = false);

	/** Query methods */
	bool overlap_fixtures(FixtureB2 *a, FixtureB2 *b);
	bool overlap_shapes(ShapeB2 *a, ShapeB2 *b, const Transform2D &xf_a, const Transform2D &xf_b);

	Box2D();
	~Box2D();
};

#endif // GODOT_BOX2D_H
