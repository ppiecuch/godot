/**************************************************************************/
/*  contact.h                                                             */
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

#ifndef BOX2D_CONTACT_H
#define BOX2D_CONTACT_H

#include "core/reference.h"

class ContactB2 : public Reference {
	GDCLASS(ContactB2, Reference);

protected:
	static void _bind_methods();

	class b2Contact *entity;

public:
	/** Box2D methods */
	Dictionary get_world_manifold() const;

	bool is_touching() const;

	bool is_enabled() const;
	void set_enabled(bool);

	class FixtureB2 *get_fixture_a() const;
	int get_child_index_a() const;

	class FixtureB2 *get_fixture_b() const;
	int get_child_index_b() const;

	float get_friction() const;
	void set_friction(float);
	void reset_friction();

	float get_restitution() const;
	void set_restitution(float);
	void reset_restitution();

	float get_tangent_speed() const;
	void set_tangent_speed(float);

	ContactB2(class b2Contact *);
	~ContactB2();
};

#endif // BOX2D_CONTACT_H
