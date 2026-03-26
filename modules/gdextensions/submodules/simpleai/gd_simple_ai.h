/**************************************************************************/
/*  gd_simple_ai.h                                                        */
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

#ifndef GD_SIMPLE_AI_H
#define GD_SIMPLE_AI_H

#include "core/reference.h"
#include "core/variant.h"
#include "scene/main/node.h"

// Forward declarations for SimpleAI types (from simple_ai.h)
namespace ai {
class ICharacter;
class AI;
class TreeNode;
class ICondition;
class AggroMgr;
class Zone;
class GroupMgr;
class IFilter;
namespace movement {
class ISteering;
}
} // namespace ai

class SimpleAIBehaviorTree;
class SimpleAIAggroMgr;
class SimpleAIGroupMgr;

// ---------------------------------------------------------------------------
// SimpleAICharacter — wraps ai::ICharacter
// ---------------------------------------------------------------------------
class SimpleAICharacter : public Reference {
	GDCLASS(SimpleAICharacter, Reference);

	std::shared_ptr<ai::ICharacter> _character;

protected:
	static void _bind_methods();

public:
	void create(int p_id);
	int get_id() const;

	void set_position(const Vector3 &p_pos);
	Vector3 get_position() const;

	void set_orientation(float p_radians);
	float get_orientation() const;

	void set_speed(float p_speed);
	float get_speed() const;

	void set_attribute(const String &p_key, const String &p_value);
	Dictionary get_attributes() const;

	std::shared_ptr<ai::ICharacter> get_internal() const { return _character; }

	SimpleAICharacter();
	~SimpleAICharacter();
};

// ---------------------------------------------------------------------------
// SimpleAICondition — wraps ai::ICondition
// ---------------------------------------------------------------------------
class SimpleAICondition : public Reference {
	GDCLASS(SimpleAICondition, Reference);

	std::shared_ptr<ai::ICondition> _condition;

protected:
	static void _bind_methods();

public:
	// Built-in condition factories (instance methods for ClassDB binding)
	Ref<SimpleAICondition> create_true_cond();
	Ref<SimpleAICondition> create_false_cond();
	Ref<SimpleAICondition> create_and(const Ref<SimpleAICondition> &p_a, const Ref<SimpleAICondition> &p_b);
	Ref<SimpleAICondition> create_or(const Ref<SimpleAICondition> &p_a, const Ref<SimpleAICondition> &p_b);
	Ref<SimpleAICondition> create_not(const Ref<SimpleAICondition> &p_cond);
	Ref<SimpleAICondition> create_has_enemies(int p_min_count = -1);
	Ref<SimpleAICondition> create_is_in_group(int p_group_id = -1);
	Ref<SimpleAICondition> create_is_group_leader(int p_group_id = -1);
	Ref<SimpleAICondition> create_custom(Object *p_object, const StringName &p_method);

	String get_name() const;

	std::shared_ptr<ai::ICondition> get_internal() const { return _condition; }
	void set_internal(const std::shared_ptr<ai::ICondition> &p_cond) { _condition = p_cond; }

	SimpleAICondition();
	~SimpleAICondition();
};

// ---------------------------------------------------------------------------
// SimpleAITreeNode — wraps ai::TreeNode
// ---------------------------------------------------------------------------
class SimpleAITreeNode : public Reference {
	GDCLASS(SimpleAITreeNode, Reference);

	std::shared_ptr<ai::TreeNode> _node;

protected:
	static void _bind_methods();

public:
	// Status enum constants
	enum Status {
		STATUS_UNKNOWN = 0,
		STATUS_CANNOTEXECUTE = 1,
		STATUS_RUNNING = 2,
		STATUS_FINISHED = 3,
		STATUS_FAILED = 4,
		STATUS_EXCEPTION = 5,
	};

	// Factory methods (instance methods for ClassDB binding)
	Ref<SimpleAITreeNode> create_selector(const String &p_name, const Ref<SimpleAICondition> &p_cond = Ref<SimpleAICondition>());
	Ref<SimpleAITreeNode> create_sequence(const String &p_name, const Ref<SimpleAICondition> &p_cond = Ref<SimpleAICondition>());
	Ref<SimpleAITreeNode> create_parallel(const String &p_name, const Ref<SimpleAICondition> &p_cond = Ref<SimpleAICondition>());
	Ref<SimpleAITreeNode> create_random_selector(const String &p_name, const Ref<SimpleAICondition> &p_cond = Ref<SimpleAICondition>());
	Ref<SimpleAITreeNode> create_probability_selector(const String &p_name, const Ref<SimpleAICondition> &p_cond = Ref<SimpleAICondition>());
	Ref<SimpleAITreeNode> create_idle(const String &p_name, int p_millis);
	Ref<SimpleAITreeNode> create_fail(const String &p_name);
	Ref<SimpleAITreeNode> create_succeed(const String &p_name);
	Ref<SimpleAITreeNode> create_invert(const String &p_name, const Ref<SimpleAICondition> &p_cond = Ref<SimpleAICondition>());
	Ref<SimpleAITreeNode> create_limit(const String &p_name, int p_max);

	int get_id() const;
	String get_name() const;
	void set_name(const String &p_name);
	String get_type() const;

	bool add_child(const Ref<SimpleAITreeNode> &p_child);
	int get_child_count() const;

	std::shared_ptr<ai::TreeNode> get_internal() const { return _node; }
	void set_internal(const std::shared_ptr<ai::TreeNode> &p_node) { _node = p_node; }

	SimpleAITreeNode();
	~SimpleAITreeNode();
};

VARIANT_ENUM_CAST(SimpleAITreeNode::Status);

// ---------------------------------------------------------------------------
// SimpleAITask — GDScript-callable task leaf node
// ---------------------------------------------------------------------------
class SimpleAITask : public Reference {
	GDCLASS(SimpleAITask, Reference);

	std::shared_ptr<ai::TreeNode> _node;
	Object *_callback_object;
	StringName _callback_method;
	String _task_name;

protected:
	static void _bind_methods();

public:
	void create(const String &p_name);
	void set_callback(Object *p_object, const StringName &p_method);

	Ref<SimpleAITreeNode> as_tree_node() const;

	std::shared_ptr<ai::TreeNode> get_internal() const { return _node; }

	SimpleAITask();
	~SimpleAITask();
};

// ---------------------------------------------------------------------------
// SimpleAIAggroMgr — wraps ai::AggroMgr
// ---------------------------------------------------------------------------
class SimpleAIAggroMgr : public Reference {
	GDCLASS(SimpleAIAggroMgr, Reference);

	ai::AggroMgr *_mgr; // non-owning pointer into AI instance
	bool _owned;

protected:
	static void _bind_methods();

public:
	void add_aggro(int p_character_id, float p_amount);
	Dictionary get_highest_entry() const;
	Array get_entries() const;
	void clear();
	void update(int64_t p_delta_ms);

	void set_reduce_by_ratio(float p_ratio_per_second, float p_min_aggro);
	void set_reduce_by_value(float p_value_per_second);

	void set_internal(ai::AggroMgr *p_mgr) {
		_mgr = p_mgr;
		_owned = false;
	}

	SimpleAIAggroMgr();
	~SimpleAIAggroMgr();
};

// ---------------------------------------------------------------------------
// SimpleAIBehaviorTree — wraps ai::AI (the brain)
// ---------------------------------------------------------------------------
class SimpleAIBehaviorTree : public Reference {
	GDCLASS(SimpleAIBehaviorTree, Reference);

	std::shared_ptr<ai::AI> _ai;

protected:
	static void _bind_methods();

public:
	void create(const Ref<SimpleAITreeNode> &p_root);
	void update(int64_t p_delta_ms);

	void set_character(const Ref<SimpleAICharacter> &p_character);
	Ref<SimpleAICharacter> get_character() const;

	void set_behaviour(const Ref<SimpleAITreeNode> &p_root);

	void set_pause(bool p_pause);
	bool is_paused() const;

	Ref<SimpleAIAggroMgr> get_aggro_manager();

	std::shared_ptr<ai::AI> get_internal() const { return _ai; }

	SimpleAIBehaviorTree();
	~SimpleAIBehaviorTree();
};

// ---------------------------------------------------------------------------
// SimpleAIGroupMgr — wraps ai::GroupMgr
// ---------------------------------------------------------------------------
class SimpleAIGroupMgr : public Reference {
	GDCLASS(SimpleAIGroupMgr, Reference);

	ai::GroupMgr *_mgr; // non-owning pointer into Zone

protected:
	static void _bind_methods();

public:
	bool add(int p_group_id, const Ref<SimpleAIBehaviorTree> &p_ai);
	bool remove(int p_group_id, const Ref<SimpleAIBehaviorTree> &p_ai);
	int get_group_size(int p_group_id) const;
	Vector3 get_position(int p_group_id) const;
	bool is_in_group(int p_group_id, const Ref<SimpleAIBehaviorTree> &p_ai) const;

	void set_internal(ai::GroupMgr *p_mgr) { _mgr = p_mgr; }

	SimpleAIGroupMgr();
	~SimpleAIGroupMgr();
};

// ---------------------------------------------------------------------------
// SimpleAIFilter — wraps ai::IFilter
// ---------------------------------------------------------------------------
class SimpleAIFilter : public Reference {
	GDCLASS(SimpleAIFilter, Reference);

	std::shared_ptr<ai::IFilter> _filter;

protected:
	static void _bind_methods();

public:
	// Leaf filters (instance methods for ClassDB binding)
	Ref<SimpleAIFilter> select_zone();
	Ref<SimpleAIFilter> select_highest_aggro();
	Ref<SimpleAIFilter> select_group_leader(int p_group_id);
	Ref<SimpleAIFilter> select_group_members(int p_group_id);
	Ref<SimpleAIFilter> select_empty();
	Ref<SimpleAIFilter> select_all();
	// Composite filters
	Ref<SimpleAIFilter> create_union(const Ref<SimpleAIFilter> &p_a, const Ref<SimpleAIFilter> &p_b);
	Ref<SimpleAIFilter> create_intersection(const Ref<SimpleAIFilter> &p_a, const Ref<SimpleAIFilter> &p_b);
	Ref<SimpleAIFilter> create_first(const Ref<SimpleAIFilter> &p_sub);
	Ref<SimpleAIFilter> create_last(const Ref<SimpleAIFilter> &p_sub);
	Ref<SimpleAIFilter> create_random(const Ref<SimpleAIFilter> &p_sub, int p_count = 1);

	String get_name() const;

	std::shared_ptr<ai::IFilter> get_internal() const { return _filter; }
	void set_internal(const std::shared_ptr<ai::IFilter> &p_f) { _filter = p_f; }

	SimpleAIFilter();
	~SimpleAIFilter();
};

// ---------------------------------------------------------------------------
// SimpleAISteering — wraps ai::movement::ISteering
// ---------------------------------------------------------------------------
class SimpleAISteering : public Reference {
	GDCLASS(SimpleAISteering, Reference);

	std::shared_ptr<ai::movement::ISteering> _steering;

protected:
	static void _bind_methods();

public:
	Ref<SimpleAISteering> create_wander(float p_rotation_deg = 10.0f);
	Ref<SimpleAISteering> create_group_seek(int p_group_id);
	Ref<SimpleAISteering> create_group_flee(int p_group_id);
	Ref<SimpleAISteering> create_target_seek(const Vector3 &p_target);
	Ref<SimpleAISteering> create_target_flee(const Vector3 &p_target);
	Ref<SimpleAISteering> create_selection_seek();
	Ref<SimpleAISteering> create_selection_flee();

	String get_name() const;

	std::shared_ptr<ai::movement::ISteering> get_internal() const { return _steering; }
	void set_internal(const std::shared_ptr<ai::movement::ISteering> &p_s) { _steering = p_s; }

	SimpleAISteering();
	~SimpleAISteering();
};

// ---------------------------------------------------------------------------
// SimpleAIZone — wraps ai::Zone as a Node (ticks in _process)
// ---------------------------------------------------------------------------
class SimpleAIZone : public Node {
	GDCLASS(SimpleAIZone, Node);

	ai::Zone *_zone;
	String _zone_name;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void set_zone_name(const String &p_name);
	String get_zone_name() const;

	bool add_ai(const Ref<SimpleAIBehaviorTree> &p_ai);
	bool remove_ai(const Ref<SimpleAIBehaviorTree> &p_ai);
	bool destroy_ai(int p_character_id);
	int get_ai_count() const;

	Ref<SimpleAIGroupMgr> get_group_manager();

	ai::Zone *get_internal() const { return _zone; }

	SimpleAIZone();
	~SimpleAIZone();
};

#endif // GD_SIMPLE_AI_H
