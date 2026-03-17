/**************************************************************************/
/*  iso_screen_solver.h                                                   */
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

#ifndef ISO_SCREEN_SOLVER_H
#define ISO_SCREEN_SOLVER_H

#include "iso_assoc_list.h"
#include "iso_object.h"
#include "iso_quad_tree.h"
#include "iso_utils.h"

#include "scene/main/viewport.h"

class IsoWorld;

class IsoScreenSolver {
	Vector2 _min_iso_xy;

	IsoAssocList<IsoObject *> _old_visibles;
	IsoAssocList<IsoObject *> _cur_visibles;

	IsoQuadTree<IsoObject *> _quad_tree;

	// Parent tracking
	struct ParentInfo {
		Node *parent;
		Vector2 last_position;
		IsoAssocList<IsoObject *> iso_objects;

		ParentInfo() :
				parent(nullptr) {}
	};

	Vector<ParentInfo *> _parent_info_pool;
	IsoAssocList<ParentInfo *> _parent_info_list;

	// Simple association storage (no HashMap needed for typical object counts)
	struct ObjectParentPair {
		IsoObject *object;
		Node *parent;
	};
	Vector<ObjectParentPair> _object_parent_pairs;

	struct ParentInfoEntry {
		Node *parent;
		ParentInfo *info;
	};
	Vector<ParentInfoEntry> _parent_info_entries;

	int _find_object_parent(IsoObject *p_object) const {
		for (int i = 0; i < _object_parent_pairs.size(); i++) {
			if (_object_parent_pairs[i].object == p_object) {
				return i;
			}
		}
		return -1;
	}

	ParentInfo *_find_parent_info(Node *p_parent) const {
		for (int i = 0; i < _parent_info_entries.size(); i++) {
			if (_parent_info_entries[i].parent == p_parent) {
				return _parent_info_entries[i].info;
			}
		}
		return nullptr;
	}

	int _find_parent_info_index(Node *p_parent) const {
		for (int i = 0; i < _parent_info_entries.size(); i++) {
			if (_parent_info_entries[i].parent == p_parent) {
				return i;
			}
		}
		return -1;
	}

	void _register_iso_object_parent(IsoObject *p_object);
	void _unregister_iso_object_parent(IsoObject *p_object);
	void _process_parents();
	void _process_visibles(IsoWorld *p_world);
	void _swap_current_visibles();

	ParentInfo *_take_parent_info() {
		if (_parent_info_pool.size() > 0) {
			ParentInfo *info = _parent_info_pool[_parent_info_pool.size() - 1];
			_parent_info_pool.resize(_parent_info_pool.size() - 1);
			return info;
		}
		return memnew(ParentInfo);
	}

public:
	Vector2 get_min_iso_xy() const { return _min_iso_xy; }
	IsoAssocList<IsoObject *> &get_old_visibles() { return _old_visibles; }
	IsoAssocList<IsoObject *> &get_cur_visibles() { return _cur_visibles; }
	const IsoAssocList<IsoObject *> &get_cur_visibles() const { return _cur_visibles; }
	IsoQuadTree<IsoObject *> &get_quad_tree() { return _quad_tree; }

	void on_add_iso_object(IsoObject *p_object) {
		p_object->internal.qt_item = _quad_tree.add_item(
				p_object->internal.qt_bounds, p_object);
		_min_iso_xy = IsoUtils::vec2_min(_min_iso_xy, Vector2(p_object->get_iso_position().x, p_object->get_iso_position().y));
		_register_iso_object_parent(p_object);
	}

	void on_remove_iso_object(IsoObject *p_object) {
		_old_visibles.remove(p_object);
		_cur_visibles.remove(p_object);
		if (p_object->internal.qt_item) {
			_quad_tree.remove_item(p_object->internal.qt_item);
			p_object->internal.qt_item = nullptr;
		}
		clear_iso_object_depends(p_object);
		_unregister_iso_object_parent(p_object);
	}

	bool on_mark_dirty_iso_object(IsoObject *p_object) {
		if (p_object->internal.qt_item) {
			p_object->internal.qt_item = _quad_tree.move_item(
					p_object->internal.qt_bounds,
					p_object->internal.qt_item);
		} else {
			p_object->internal.qt_item = _quad_tree.add_item(
					p_object->internal.qt_bounds, p_object);
		}
		_min_iso_xy = IsoUtils::vec2_min(_min_iso_xy, Vector2(p_object->get_iso_position().x, p_object->get_iso_position().y));
		if (!p_object->internal.dirty) {
			p_object->internal.dirty = true;
			return true;
		}
		return false;
	}

	void step_sorting_action(IsoWorld *p_world) {
		_process_parents();
		_process_visibles(p_world);
	}

	void setup_iso_object_depends(IsoObject *p_object) {
		clear_iso_object_depends(p_object);
		if (p_object->internal.qt_item) {
			_quad_tree.visit_items_by_item(p_object->internal.qt_item,
					[this, p_object](IsoObject *p_other) {
						_lookup_cell_for_l_depends(p_object, p_other);
						_lookup_cell_for_r_depends(p_object, p_other);
					});
		}
	}

	void clear_iso_object_depends(IsoObject *p_object) {
		IsoAssocList<IsoObject *> &their_depends = p_object->internal.their_depends;
		for (int i = 0; i < their_depends.count(); i++) {
			IsoObject *their_obj = their_depends[i];
			if (!their_obj->internal.dirty) {
				their_obj->internal.self_depends.remove(p_object);
			}
		}
		p_object->internal.self_depends.clear();
		p_object->internal.their_depends.clear();
	}

	void clear() {
		_old_visibles.clear();
		_cur_visibles.clear();
		_quad_tree.clear();
	}

	// Dependency test (pure math on AABB overlaps)
	static bool is_iso_object_depends(const Vector3 &a_min, const Vector3 &a_size,
			const Vector3 &b_min, const Vector3 &b_size) {
		real_t a_min_x = a_min.x, a_min_y = a_min.y, a_min_z = a_min.z;
		real_t a_size_x = a_size.x, a_size_y = a_size.y, a_size_z = a_size.z;
		real_t b_min_x = b_min.x, b_min_y = b_min.y, b_min_z = b_min.z;
		real_t b_size_x = b_size.x, b_size_y = b_size.y, b_size_z = b_size.z;

		real_t a_max_x = a_min_x + a_size_x;
		real_t a_max_y = a_min_y + a_size_y;
		real_t a_max_z = a_min_z + a_size_z;
		real_t b_max_x = b_min_x + b_size_x;
		real_t b_max_y = b_min_y + b_size_y;
		real_t b_max_z = b_min_z + b_size_z;

		bool a_yesno = a_max_x > b_min_x && a_max_y > b_min_y && b_max_z > a_min_z;
		if (a_yesno) {
			bool b_yesno = b_max_x > a_min_x && b_max_y > a_min_y && a_max_z > b_min_z;
			if (b_yesno) {
				real_t dA_x = a_max_x - b_min_x;
				real_t dA_y = a_max_y - b_min_y;
				real_t dA_z = b_max_z - a_min_z;

				real_t dB_x = b_max_x - a_min_x;
				real_t dB_y = b_max_y - a_min_y;
				real_t dB_z = a_max_z - b_min_z;

				real_t dD_x = dB_x - dA_x;
				real_t dD_y = dB_y - dA_y;
				real_t dD_z = dB_z - dA_z;

				real_t dP_x = a_size_x + b_size_x - Math::abs(dD_x);
				real_t dP_y = a_size_y + b_size_y - Math::abs(dD_y);
				real_t dP_z = a_size_z + b_size_z - Math::abs(dD_z);

				if (dP_x <= dP_y && dP_x <= dP_z) {
					return dA_x > dB_x;
				} else if (dP_y <= dP_x && dP_y <= dP_z) {
					return dA_y > dB_y;
				} else {
					return dA_z > dB_z;
				}
			}
		}
		return a_yesno;
	}

	IsoScreenSolver() {}

	~IsoScreenSolver() {
		for (int i = 0; i < _parent_info_pool.size(); i++) {
			memdelete(_parent_info_pool[i]);
		}
		// Clean active parent infos
		for (int i = 0; i < _parent_info_list.count(); i++) {
			memdelete(_parent_info_list[i]);
		}
	}

private:
	void _lookup_cell_for_l_depends(IsoObject *p_a, IsoObject *p_b) {
		if (!p_b->internal.dirty && p_a != p_b &&
				is_iso_object_depends(p_a->get_iso_position(), p_a->get_iso_size(),
						p_b->get_iso_position(), p_b->get_iso_size())) {
			p_a->internal.self_depends.add(p_b);
			p_b->internal.their_depends.add(p_a);
		}
	}

	void _lookup_cell_for_r_depends(IsoObject *p_a, IsoObject *p_b) {
		if (!p_b->internal.dirty && p_a != p_b &&
				is_iso_object_depends(p_b->get_iso_position(), p_b->get_iso_size(),
						p_a->get_iso_position(), p_a->get_iso_size())) {
			p_b->internal.self_depends.add(p_a);
			p_a->internal.their_depends.add(p_b);
		}
	}
};

// --- Inline implementations ---

inline void IsoScreenSolver::_register_iso_object_parent(IsoObject *p_object) {
	Node *parent = p_object->get_parent();
	if (!parent) {
		return;
	}
	ParentInfo *info = _find_parent_info(parent);
	if (info) {
		info->iso_objects.add(p_object);
	} else {
		info = _take_parent_info();
		info->parent = parent;
		Node2D *parent_2d = Object::cast_to<Node2D>(parent);
		info->last_position = parent_2d ? parent_2d->get_position() : Vector2();
		info->iso_objects.add(p_object);
		ParentInfoEntry entry;
		entry.parent = parent;
		entry.info = info;
		_parent_info_entries.push_back(entry);
		_parent_info_list.add(info);
	}
	ObjectParentPair pair;
	pair.object = p_object;
	pair.parent = parent;
	_object_parent_pairs.push_back(pair);
}

inline void IsoScreenSolver::_unregister_iso_object_parent(IsoObject *p_object) {
	int pair_idx = _find_object_parent(p_object);
	if (pair_idx == -1) {
		return;
	}
	Node *parent = _object_parent_pairs[pair_idx].parent;
	// Remove pair (swap-remove)
	int last_pair = _object_parent_pairs.size() - 1;
	if (pair_idx != last_pair) {
		_object_parent_pairs.write[pair_idx] = _object_parent_pairs[last_pair];
	}
	_object_parent_pairs.resize(last_pair);

	ParentInfo *info = _find_parent_info(parent);
	if (info) {
		info->iso_objects.remove(p_object);
		if (info->iso_objects.count() == 0) {
			int entry_idx = _find_parent_info_index(parent);
			if (entry_idx >= 0) {
				int last_entry = _parent_info_entries.size() - 1;
				if (entry_idx != last_entry) {
					_parent_info_entries.write[entry_idx] = _parent_info_entries[last_entry];
				}
				_parent_info_entries.resize(last_entry);
			}
			_parent_info_list.remove(info);
			info->iso_objects.clear();
			info->parent = nullptr;
			_parent_info_pool.push_back(info);
		}
	}
}

inline void IsoScreenSolver::_process_parents() {
	for (int i = 0; i < _parent_info_list.count(); i++) {
		ParentInfo *info = _parent_info_list[i];
		Node2D *parent_2d = Object::cast_to<Node2D>(info->parent);
		Vector2 parent_pos = parent_2d ? parent_2d->get_position() : Vector2();
		if (info->last_position != parent_pos) {
			info->last_position = parent_pos;
			for (int j = 0; j < info->iso_objects.count(); j++) {
				info->iso_objects[j]->fix_iso_position();
			}
		}
	}
}

inline void IsoScreenSolver::_process_visibles(IsoWorld *p_world) {
	// Swap old/new
	_old_visibles.clear();

	// Query viewport bounds for visible objects
	IsoObject *obj_ptr = nullptr;
	(void)obj_ptr;

	// Get viewport transform to find visible area
	// For simplicity: visit all items in the quadtree and mark as visible
	// A full implementation would use camera viewport bounds
	// For now, visit all and let objects be visible
	_quad_tree.visit_items_by_bounds(
			IsoRect(-1e6f, -1e6f, 1e6f, 1e6f),
			[this](IsoObject *p_object) {
				p_object->internal.placed = false;
				_old_visibles.add(p_object);
			});

	_swap_current_visibles();
}

inline void IsoScreenSolver::_swap_current_visibles() {
	// Swap old and cur
	// We need to copy because IsoAssocList doesn't have a swap.
	// Instead, we reuse old_visibles as the new cur_visibles.
	IsoAssocList<IsoObject *> tmp;

	// Move cur -> tmp
	for (int i = 0; i < _cur_visibles.count(); i++) {
		tmp.add(_cur_visibles[i]);
	}
	// Move old -> cur
	_cur_visibles.clear();
	for (int i = 0; i < _old_visibles.count(); i++) {
		_cur_visibles.add(_old_visibles[i]);
	}
	// Move tmp -> old
	_old_visibles.clear();
	for (int i = 0; i < tmp.count(); i++) {
		_old_visibles.add(tmp[i]);
	}
}

#endif // ISO_SCREEN_SOLVER_H
