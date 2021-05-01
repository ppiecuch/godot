/*************************************************************************/
/*  rid.h                                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef RID_H
#define RID_H

#include "core/list.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/os/memory.h"
#include "core/safe_refcount.h"
#include "core/set.h"
#include "core/typedefs.h"
#include "core/vector.h"

class RID_OwnerBase;

class RID_Data {

	friend class RID_OwnerBase;

#ifndef DEBUG_ENABLED
	RID_OwnerBase *_owner;
#endif
	uint32_t _id;

public:
	_FORCE_INLINE_ uint32_t get_id() const { return _id; }

	virtual ~RID_Data();
};

union RID_Props {
	int int_value;
	float float_value;
	Vector2 vec2_value;
	Vector3 vec3_value;

	RID_Props() {}
	RID_Props(int v) { int_value = v; }
	RID_Props(float v) { float_value = v; }
	RID_Props(const Vector2 &v) { vec2_value = v; }
	RID_Props(const Vector3 &v) { vec3_value = v; }
};

class RID {
	friend class RID_OwnerBase;

	mutable RID_Data *_data;
	mutable Vector<RID_Props> _props;

	struct _expand_props {
		template <typename... T>
		_expand_props(T &&...) {}
	};

public:
	_FORCE_INLINE_ RID_Data *get_data() const { return _data; }

	RID_Props get_prop(int p_index) const { return _props[p_index]; }
	bool is_props_valid() const { return _props.size() > 0; }
	size_t get_props_count() const { return _props.size(); }
	template <typename... props_types>
	void set_props(props_types... args) { _expand_props{ 0, (_props.push_back(args), 0)... }; }

	_FORCE_INLINE_ bool operator==(const RID &p_rid) const {

		return _data == p_rid._data;
	}
	_FORCE_INLINE_ bool operator<(const RID &p_rid) const {

		return _data < p_rid._data;
	}
	_FORCE_INLINE_ bool operator<=(const RID &p_rid) const {

		return _data <= p_rid._data;
	}
	_FORCE_INLINE_ bool operator>(const RID &p_rid) const {

		return _data > p_rid._data;
	}
	_FORCE_INLINE_ bool operator!=(const RID &p_rid) const {

		return _data != p_rid._data;
	}
	_FORCE_INLINE_ bool is_valid() const { return _data != NULL; }

	_FORCE_INLINE_ uint32_t get_id() const { return _data ? _data->get_id() : 0; }

	_FORCE_INLINE_ RID() {
		_data = NULL;
	}
};

class RID_OwnerBase {
protected:
	static SafeRefCount refcount;
	_FORCE_INLINE_ void _set_data(RID &p_rid, RID_Data *p_data) {
		p_rid._data = p_data;
		refcount.ref();
		p_data->_id = refcount.get();
#ifndef DEBUG_ENABLED
		p_data->_owner = this;
#endif
	}

#ifndef DEBUG_ENABLED

	_FORCE_INLINE_ bool _is_owner(const RID &p_rid) const {

		return this == p_rid._data->_owner;
	}

	_FORCE_INLINE_ void _remove_owner(RID &p_rid) {

		p_rid._data->_owner = NULL;
	}
#endif

public:
	virtual void get_owned_list(List<RID> *p_owned) = 0;

	static void init_rid();
	virtual ~RID_OwnerBase() {}
};

template <class T>
class RID_Owner : public RID_OwnerBase {
public:
#ifdef DEBUG_ENABLED
	mutable Set<RID_Data *> id_map;
#endif
public:
	_FORCE_INLINE_ RID make_rid(T *p_data) {

		RID rid;
		_set_data(rid, p_data);

#ifdef DEBUG_ENABLED
		id_map.insert(p_data);
#endif

		return rid;
	}

	_FORCE_INLINE_ T *get(const RID &p_rid) {

#ifdef DEBUG_ENABLED

		ERR_FAIL_COND_V(!p_rid.is_valid(), NULL);
		ERR_FAIL_COND_V(!id_map.has(p_rid.get_data()), NULL);
#endif
		return static_cast<T *>(p_rid.get_data());
	}

	_FORCE_INLINE_ T *getornull(const RID &p_rid) {

#ifdef DEBUG_ENABLED

		if (p_rid.get_data()) {
			ERR_FAIL_COND_V(!id_map.has(p_rid.get_data()), NULL);
		}
#endif
		return static_cast<T *>(p_rid.get_data());
	}

	_FORCE_INLINE_ T *getptr(const RID &p_rid) {

		return static_cast<T *>(p_rid.get_data());
	}

	_FORCE_INLINE_ bool owns(const RID &p_rid) const {

		if (p_rid.get_data() == NULL)
			return false;
#ifdef DEBUG_ENABLED
		return id_map.has(p_rid.get_data());
#else
		return _is_owner(p_rid);
#endif
	}

	void free(RID p_rid) {

#ifdef DEBUG_ENABLED
		id_map.erase(p_rid.get_data());
#else
		_remove_owner(p_rid);
#endif
	}

	void get_owned_list(List<RID> *p_owned) {

#ifdef DEBUG_ENABLED

		for (typename Set<RID_Data *>::Element *E = id_map.front(); E; E = E->next()) {
			RID r;
			_set_data(r, static_cast<T *>(E->get()));
			p_owned->push_back(r);
		}
#endif
	}
};

#endif
