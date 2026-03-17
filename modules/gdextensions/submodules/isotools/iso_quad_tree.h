/**************************************************************************/
/*  iso_quad_tree.h                                                       */
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

#ifndef ISO_QUAD_TREE_H
#define ISO_QUAD_TREE_H

#include "iso_assoc_list.h"
#include "iso_utils.h"

#include "core/error_macros.h"
#include "core/vector.h"

template <typename T>
class IsoQuadTree {
public:
	static const int MIN_CHILD_COUNT_PER_NODE = 3;

	class Node;

	// --- Item ---

	struct Item {
		int qt_id;
		Node *owner;
		IsoRect bounds;
		T content;

		Item() :
				qt_id(0), owner(nullptr) {}
		explicit Item(int p_qt_id) :
				qt_id(p_qt_id), owner(nullptr) {}

		Item *init(Node *p_owner, const IsoRect &p_bounds, const T &p_content) {
			owner = p_owner;
			bounds = p_bounds;
			content = p_content;
			return this;
		}

		Item *clear() {
			owner = nullptr;
			bounds = IsoRect();
			content = T();
			return this;
		}
	};

	// --- Node ---

	class Node {
	public:
		Node *nodes[4];
		Vector<Item *> items;
		Node *parent;
		IsoRect self_bounds;
		IsoRect node_bounds[4];

		Node() :
				parent(nullptr) {
			for (int i = 0; i < 4; i++) {
				nodes[i] = nullptr;
			}
		}

		Node *init(Node *p_parent, const IsoRect &p_bounds) {
			parent = p_parent;
			self_bounds = p_bounds;
			fill_node_bounds();
			return this;
		}

		Node *clear(Vector<Node *> &p_node_pool, Vector<Item *> &p_item_pool) {
			clear_nodes(p_node_pool, p_item_pool);
			clear_items(p_item_pool);
			parent = nullptr;
			self_bounds = IsoRect();
			fill_node_bounds();
			return this;
		}

		bool cleanup_nodes(Vector<Node *> &p_node_pool, Vector<Item *> &p_item_pool) {
			bool has_any_busy_nodes = false;
			for (int i = 0; i < 4; i++) {
				Node *node = nodes[i];
				if (node) {
					if (node->cleanup_nodes(p_node_pool, p_item_pool)) {
						p_node_pool.push_back(node->clear(p_node_pool, p_item_pool));
						nodes[i] = nullptr;
					} else {
						has_any_busy_nodes = true;
					}
				}
			}
			return !has_any_busy_nodes && items.size() == 0;
		}

		bool add_item(const IsoRect &p_bounds, const T &p_content, Item *&r_item,
				Vector<Node *> &p_node_pool, Vector<Item *> &p_item_pool) {
			if (!self_bounds.contains(p_bounds)) {
				r_item = nullptr;
				return false;
			}
			for (int i = 0; i < 4; i++) {
				Node *node = nodes[i];
				if (node) {
					if (node->add_item(p_bounds, p_content, r_item, p_node_pool, p_item_pool)) {
						return true;
					}
				} else if (items.size() >= MIN_CHILD_COUNT_PER_NODE && node_bounds[i].contains(p_bounds)) {
					nodes[i] = node = _take_node(p_node_pool)->init(this, node_bounds[i]);
					if (node->add_item(p_bounds, p_content, r_item, p_node_pool, p_item_pool)) {
						return true;
					}
				}
			}
			r_item = _take_item(p_item_pool)->init(this, p_bounds, p_content);
			items.push_back(r_item);
			return true;
		}

		void remove_item(Item *p_item, Vector<Item *> &p_item_pool) {
			ERR_FAIL_COND(!p_item || p_item->owner != this);
			int idx = items.find(p_item);
			ERR_FAIL_COND(idx == -1);

			// Unordered remove
			int last = items.size() - 1;
			if (idx != last) {
				items.write[idx] = items[last];
			}
			items.resize(last);

			p_item_pool.push_back(p_item->clear());
		}

		bool has_any_items() const {
			if (items.size() > 0) {
				return true;
			}
			for (int i = 0; i < 4; i++) {
				if (nodes[i] && nodes[i]->has_any_items()) {
					return true;
				}
			}
			return false;
		}

		template <typename BoundsFunc>
		void visit_all_bounds(BoundsFunc p_func) {
			p_func(self_bounds);
			for (int i = 0; i < 4; i++) {
				if (nodes[i]) {
					nodes[i]->visit_all_bounds(p_func);
				}
			}
		}

		template <typename ContentFunc>
		void visit_items_by_bounds(const IsoRect &p_bounds, ContentFunc p_func) {
			if (p_bounds.overlaps(self_bounds)) {
				for (int i = 0; i < items.size(); i++) {
					Item *item = items[i];
					if (p_bounds.overlaps(item->bounds)) {
						p_func(item->content);
					}
				}
				for (int i = 0; i < 4; i++) {
					if (nodes[i]) {
						nodes[i]->visit_items_by_bounds(p_bounds, p_func);
					}
				}
			}
		}

	private:
		void fill_node_bounds() {
			Vector2 size = self_bounds.get_size() * 0.5f;
			Vector2 center = self_bounds.get_center();

			// LT
			node_bounds[0] = IsoRect(center - size, center);
			// RT
			node_bounds[1] = IsoRect(center - size, center);
			node_bounds[1].translate(size.x, 0.0f);
			// LB
			node_bounds[2] = IsoRect(center, center + size);
			node_bounds[2].translate(-size.x, 0.0f);
			// RB
			node_bounds[3] = IsoRect(center, center + size);
		}

		void clear_nodes(Vector<Node *> &p_node_pool, Vector<Item *> &p_item_pool) {
			for (int i = 0; i < 4; i++) {
				if (nodes[i]) {
					p_node_pool.push_back(nodes[i]->clear(p_node_pool, p_item_pool));
					nodes[i] = nullptr;
				}
			}
		}

		void clear_items(Vector<Item *> &p_item_pool) {
			for (int i = 0; i < items.size(); i++) {
				p_item_pool.push_back(items[i]->clear());
			}
			items.clear();
		}

		static Node *_take_node(Vector<Node *> &p_pool) {
			if (p_pool.size() > 0) {
				Node *n = p_pool[p_pool.size() - 1];
				p_pool.resize(p_pool.size() - 1);
				return n;
			}
			return memnew(Node);
		}

		static Item *_take_item(Vector<Item *> &p_pool) {
			if (p_pool.size() > 0) {
				Item *it = p_pool[p_pool.size() - 1];
				p_pool.resize(p_pool.size() - 1);
				return it;
			}
			return memnew(Item);
		}
	};

private:
	int _qt_id;
	static int _gen_qt_id;

	Node *_root_node;
	Vector<Node *> _node_pool;
	Vector<Item *> _item_pool;

	Node *_take_node() {
		if (_node_pool.size() > 0) {
			Node *n = _node_pool[_node_pool.size() - 1];
			_node_pool.resize(_node_pool.size() - 1);
			return n;
		}
		return memnew(Node);
	}

	void _grow_up(bool p_left, bool p_top) {
		IsoRect new_root_bounds = _root_node->self_bounds;
		Vector2 rb_size = new_root_bounds.get_size();
		new_root_bounds.translate(
				p_left ? -rb_size.x : 0.0f,
				p_top ? -rb_size.y : 0.0f);
		new_root_bounds.resize(rb_size * 2.0f);

		Node *new_root = _take_node()->init(nullptr, new_root_bounds);
		if (_root_node->has_any_items()) {
			if (p_left) {
				if (p_top) {
					new_root->nodes[3] = _root_node;
				} else {
					new_root->nodes[1] = _root_node;
				}
			} else {
				if (p_top) {
					new_root->nodes[2] = _root_node;
				} else {
					new_root->nodes[0] = _root_node;
				}
			}
			_root_node->parent = new_root;
		} else {
			_node_pool.push_back(_root_node->clear(_node_pool, _item_pool));
		}
		_root_node = new_root;
	}

	Node *_backward_node_cleanup(Node *p_node) {
		while (p_node && p_node->cleanup_nodes(_node_pool, _item_pool)) {
			p_node = p_node->parent;
		}
		return p_node;
	}

	template <typename ContentFunc>
	void _backward_visit_nodes(Node *p_node, const IsoRect &p_bounds, ContentFunc p_func) {
		while (p_node) {
			for (int i = 0; i < p_node->items.size(); i++) {
				Item *item = p_node->items[i];
				if (p_bounds.overlaps(item->bounds)) {
					p_func(item->content);
				}
			}
			p_node = p_node->parent;
		}
	}

public:
	IsoQuadTree() {
		_qt_id = ++_gen_qt_id;
		_root_node = nullptr;
	}

	~IsoQuadTree() {
		clear();
		// Free pooled nodes and items
		for (int i = 0; i < _node_pool.size(); i++) {
			memdelete(_node_pool[i]);
		}
		for (int i = 0; i < _item_pool.size(); i++) {
			memdelete(_item_pool[i]);
		}
	}

	Item *add_item(const IsoRect &p_bounds, const T &p_content) {
		if (p_bounds.x.get_size() > 0.0f && p_bounds.y.get_size() > 0.0f) {
			if (!_root_node) {
				real_t max_side = IsoUtils::vec2_max_f(p_bounds.get_size());
				Vector2 initial_side = IsoUtils::vec2_from(max_side);
				Vector2 center = p_bounds.get_center();
				IsoRect initial_bounds(center - initial_side * 2.0f, center + initial_side * 2.0f);
				_root_node = _take_node()->init(nullptr, initial_bounds);
			}
			Item *item = nullptr;
			while (!_root_node->add_item(p_bounds, p_content, item, _node_pool, _item_pool)) {
				_grow_up(
						p_bounds.get_center().x < _root_node->self_bounds.get_center().x,
						p_bounds.get_center().y < _root_node->self_bounds.get_center().y);
			}
			return item;
		} else {
			// Zero-size item: not placed in tree
			Item *item;
			if (_item_pool.size() > 0) {
				item = _item_pool[_item_pool.size() - 1];
				_item_pool.resize(_item_pool.size() - 1);
			} else {
				item = memnew(Item);
			}
			item->qt_id = _qt_id;
			return item->init(nullptr, p_bounds, p_content);
		}
	}

	void remove_item(Item *p_item) {
		ERR_FAIL_COND(!p_item);
		Node *item_node = p_item->owner;
		if (item_node) {
			item_node->remove_item(p_item, _item_pool);
			if (item_node->items.size() == 0) {
				_backward_node_cleanup(item_node);
			}
		} else {
			_item_pool.push_back(p_item->clear());
		}
	}

	Item *move_item(const IsoRect &p_bounds, Item *p_item) {
		ERR_FAIL_COND_V(!p_item, nullptr);
		Node *item_node = p_item->owner;
		if (item_node) {
			if (item_node->self_bounds.contains(p_bounds) && item_node->items.size() <= MIN_CHILD_COUNT_PER_NODE) {
				p_item->bounds = p_bounds;
				return p_item;
			} else {
				T content = p_item->content;
				item_node->remove_item(p_item, _item_pool);
				if (item_node->items.size() == 0) {
					Node *cleaned = _backward_node_cleanup(item_node);
					item_node = cleaned ? cleaned : _root_node;
				}
				while (item_node) {
					Item *new_item = nullptr;
					if (item_node->self_bounds.contains(p_bounds)) {
						if (item_node->add_item(p_bounds, content, new_item, _node_pool, _item_pool)) {
							return new_item;
						}
					}
					item_node = item_node->parent;
				}
				return add_item(p_bounds, content);
			}
		} else {
			T content = p_item->content;
			_item_pool.push_back(p_item->clear());
			return add_item(p_bounds, content);
		}
	}

	void clear() {
		if (_root_node) {
			_node_pool.push_back(_root_node->clear(_node_pool, _item_pool));
			_root_node = nullptr;
		}
	}

	template <typename BoundsFunc>
	void visit_all_bounds(BoundsFunc p_func) {
		if (_root_node) {
			_root_node->visit_all_bounds(p_func);
		}
	}

	template <typename ContentFunc>
	void visit_items_by_item(Item *p_item, ContentFunc p_func) {
		ERR_FAIL_COND(!p_item);
		Node *item_node = p_item->owner;
		if (item_node) {
			item_node->visit_items_by_bounds(p_item->bounds, p_func);
			_backward_visit_nodes(item_node->parent, p_item->bounds, p_func);
		}
	}

	template <typename ContentFunc>
	void visit_items_by_bounds(const IsoRect &p_bounds, ContentFunc p_func) {
		if (_root_node) {
			_root_node->visit_items_by_bounds(p_bounds, p_func);
		}
	}
};

template <typename T>
int IsoQuadTree<T>::_gen_qt_id = 0;

#endif // ISO_QUAD_TREE_H
