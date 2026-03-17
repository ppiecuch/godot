/**************************************************************************/
/*  iso_object.h                                                          */
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

#ifndef ISO_OBJECT_H
#define ISO_OBJECT_H

#include "iso_assoc_list.h"
#include "iso_quad_tree.h"
#include "iso_utils.h"

#include "scene/2d/node_2d.h"

class IsoWorld;

class IsoObject : public Node2D {
	GDCLASS(IsoObject, Node2D);

public:
	enum RenderersMode {
		MODE_2D,
		MODE_3D,
	};

	// Internal state (accessible by solvers)
	struct Internal {
		bool dirty;
		bool placed;
		IsoQuadTree<IsoObject *>::Item *qt_item;
		IsoRect qt_bounds;
		IsoMinMax minmax_3d;
		real_t offset_3d;
		IsoAssocList<IsoObject *> self_depends;
		IsoAssocList<IsoObject *> their_depends;

		Internal() :
				dirty(true),
				placed(false),
				qt_item(nullptr),
				offset_3d(0) {}
	};

	Internal internal;

private:
	Vector3 _iso_position;
	Vector3 _iso_size;
	RenderersMode _renderers_mode;

	IsoWorld *_iso_world;

	void _find_iso_world();
	void _mark_dirty_iso_world();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_iso_position(const Vector3 &p_position);
	Vector3 get_iso_position() const;

	void set_iso_position_x(real_t p_value);
	real_t get_iso_position_x() const;
	void set_iso_position_y(real_t p_value);
	real_t get_iso_position_y() const;
	void set_iso_position_z(real_t p_value);
	real_t get_iso_position_z() const;

	void set_iso_size(const Vector3 &p_size);
	Vector3 get_iso_size() const;

	void set_iso_size_x(real_t p_value);
	real_t get_iso_size_x() const;
	void set_iso_size_y(real_t p_value);
	real_t get_iso_size_y() const;
	void set_iso_size_z(real_t p_value);
	real_t get_iso_size_z() const;

	void set_tile_position(const Vector3 &p_position);
	Vector3 get_tile_position() const;

	void set_renderers_mode(RenderersMode p_mode);
	RenderersMode get_renderers_mode() const;

	IsoWorld *get_iso_world() const;

	void fix_transform();
	void fix_iso_position();
	void fix_screen_bounds();

	IsoObject();
};

VARIANT_ENUM_CAST(IsoObject::RenderersMode);

#endif // ISO_OBJECT_H
