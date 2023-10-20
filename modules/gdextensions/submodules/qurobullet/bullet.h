/**************************************************************************/
/*  bullet.h                                                              */
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

#ifndef BULLET_H
#define BULLET_H

#include "bullet_type.h"
#include "core/math/rect2.h"
#include "core/math/transform_2d.h"
#include "core/object.h"
#include "core/reference.h"
#include "scene/resources/texture.h"

class Bullet : public Object {
	GDCLASS(Bullet, Object);

	float time;
	bool _popped;

	Ref<BulletType> type;

	Vector2 direction;
	Vector2 position;
	float rotation;
	Vector2 _offset;

	RID ci_rid;

	void _update_offset();
	void _update_appearance(const Ref<BulletType> &p_type = NULL);

protected:
	static void _bind_methods();

public:
	void spawn(const Ref<BulletType> &p_type, const Vector2 &p_position, const Vector2 &p_direction);

	void update(float delta);

	void pop();
	bool is_popped();

	bool can_collide();

	void set_time(float p_time);
	float get_time() const;

	void set_type(const Ref<BulletType> &p_type);
	Ref<BulletType> get_type() const;

	void set_direction(const Vector2 &p_direction);
	Vector2 get_direction() const;

	void set_position(const Vector2 &p_position);
	Vector2 get_position() const;

	void set_rotation(float p_radians);
	float get_rotation() const;

	Transform2D get_transform();

	void set_ci_rid(const RID &p_rid);
	RID get_ci_rid() const;

	Bullet();
	~Bullet();
};

#endif
