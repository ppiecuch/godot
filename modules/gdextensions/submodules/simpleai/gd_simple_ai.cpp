/**************************************************************************/
/*  gd_simple_ai.cpp                                                      */
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

#include "gd_simple_ai.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif
#include "doctest/doctest_godot.h"

#include "core/os/os.h"
#include "simple_ai/simple_ai.h"

// ---------------------------------------------------------------------------
// glm <-> Godot conversion helpers
// ---------------------------------------------------------------------------
static inline glm::vec3 to_glm(const Vector3 &v) { return glm::vec3(v.x, v.y, v.z); }
static inline Vector3 to_godot(const glm::vec3 &v) { return Vector3(v.x, v.y, v.z); }

// ---------------------------------------------------------------------------
// GdCharacter — concrete ICharacter subclass used internally
// ---------------------------------------------------------------------------
class GdCharacter : public ai::ICharacter {
public:
	explicit GdCharacter(ai::CharacterId id) :
			ai::ICharacter(id) {}
};

// ===========================================================================
// GdCallbackTask — ITask subclass that delegates to GDScript via Object::call
// ===========================================================================
class GdCallbackTask : public ai::ITask {
	Object *_obj;
	StringName _method;

public:
	GdCallbackTask(const std::string &name, Object *obj, const StringName &method) :
			ai::ITask(name, "", ai::True::get()), _obj(obj), _method(method) {
		_type = "GdCallbackTask";
	}

	ai::TreeNodeStatus doAction(const ai::AIPtr &entity, int64_t deltaMillis) override {
		if (!_obj || !ObjectDB::instance_validate(_obj)) {
			return ai::FAILED;
		}
		Variant result = _obj->call(_method, (int)entity->getCharacter()->getId(), (int)deltaMillis);
		int status = result;
		if (status < 0 || status > 5) {
			return ai::FAILED;
		}
		return static_cast<ai::TreeNodeStatus>(status);
	}
};

// ===========================================================================
// GdCallbackCondition — ICondition that delegates to GDScript
// ===========================================================================
class GdCallbackCondition : public ai::ICondition {
	Object *_obj;
	StringName _method;

public:
	GdCallbackCondition(Object *obj, const StringName &method) :
			ai::ICondition("GdCallback", ""), _obj(obj), _method(method) {}

	bool evaluate(const ai::AIPtr &entity) override {
		if (!_obj || !ObjectDB::instance_validate(_obj)) {
			return false;
		}
		Variant result = _obj->call(_method, (int)entity->getCharacter()->getId());
		return (bool)result;
	}
};

// ===========================================================================
// SimpleAICharacter
// ===========================================================================

SimpleAICharacter::SimpleAICharacter() {}
SimpleAICharacter::~SimpleAICharacter() {}

void SimpleAICharacter::create(int p_id) {
	_character = std::make_shared<GdCharacter>((ai::CharacterId)p_id);
}

int SimpleAICharacter::get_id() const {
	ERR_FAIL_COND_V(!_character, -1);
	return (int)_character->getId();
}

void SimpleAICharacter::set_position(const Vector3 &p_pos) {
	ERR_FAIL_COND(!_character);
	_character->setPosition(to_glm(p_pos));
}

Vector3 SimpleAICharacter::get_position() const {
	if (!_character) {
		return Vector3();
	}
	return to_godot(_character->getPosition());
}

void SimpleAICharacter::set_orientation(float p_radians) {
	if (!_character) {
		return;
	}
	_character->setOrientation(p_radians);
}

float SimpleAICharacter::get_orientation() const {
	if (!_character) {
		return 0.0f;
	}
	return _character->getOrientation();
}

void SimpleAICharacter::set_speed(float p_speed) {
	if (!_character) {
		return;
	}
	_character->setSpeed(p_speed);
}

float SimpleAICharacter::get_speed() const {
	if (!_character) {
		return 0.0f;
	}
	return _character->getSpeed();
}

void SimpleAICharacter::set_attribute(const String &p_key, const String &p_value) {
	ERR_FAIL_COND(!_character);
	_character->setAttribute(p_key.utf8().get_data(), p_value.utf8().get_data());
}

Dictionary SimpleAICharacter::get_attributes() const {
	Dictionary d;
	ERR_FAIL_COND_V(!_character, d);
	for (const auto &kv : _character->getAttributes()) {
		d[String(kv.first.c_str())] = String(kv.second.c_str());
	}
	return d;
}

void SimpleAICharacter::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create", "id"), &SimpleAICharacter::create);
	ClassDB::bind_method(D_METHOD("get_id"), &SimpleAICharacter::get_id);
	ClassDB::bind_method(D_METHOD("set_position", "position"), &SimpleAICharacter::set_position);
	ClassDB::bind_method(D_METHOD("get_position"), &SimpleAICharacter::get_position);
	ClassDB::bind_method(D_METHOD("set_orientation", "radians"), &SimpleAICharacter::set_orientation);
	ClassDB::bind_method(D_METHOD("get_orientation"), &SimpleAICharacter::get_orientation);
	ClassDB::bind_method(D_METHOD("set_speed", "speed"), &SimpleAICharacter::set_speed);
	ClassDB::bind_method(D_METHOD("get_speed"), &SimpleAICharacter::get_speed);
	ClassDB::bind_method(D_METHOD("set_attribute", "key", "value"), &SimpleAICharacter::set_attribute);
	ClassDB::bind_method(D_METHOD("get_attributes"), &SimpleAICharacter::get_attributes);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "position"), "set_position", "get_position");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "orientation"), "set_orientation", "get_orientation");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "speed"), "set_speed", "get_speed");
}

// ===========================================================================
// SimpleAICondition
// ===========================================================================

SimpleAICondition::SimpleAICondition() {}
SimpleAICondition::~SimpleAICondition() {}

Ref<SimpleAICondition> SimpleAICondition::create_true_cond() {
	Ref<SimpleAICondition> ref;
	ref.instance();
	ref->_condition = ai::True::get();
	return ref;
}

Ref<SimpleAICondition> SimpleAICondition::create_false_cond() {
	Ref<SimpleAICondition> ref;
	ref.instance();
	ref->_condition = ai::False::get();
	return ref;
}

Ref<SimpleAICondition> SimpleAICondition::create_and(const Ref<SimpleAICondition> &p_a, const Ref<SimpleAICondition> &p_b) {
	ERR_FAIL_COND_V(p_a.is_null() || p_b.is_null(), Ref<SimpleAICondition>());
	Ref<SimpleAICondition> ref;
	ref.instance();
	ai::Conditions conds;
	conds.push_back(p_a->_condition);
	conds.push_back(p_b->_condition);
	ref->_condition = std::make_shared<ai::And>(conds);
	return ref;
}

Ref<SimpleAICondition> SimpleAICondition::create_or(const Ref<SimpleAICondition> &p_a, const Ref<SimpleAICondition> &p_b) {
	ERR_FAIL_COND_V(p_a.is_null() || p_b.is_null(), Ref<SimpleAICondition>());
	Ref<SimpleAICondition> ref;
	ref.instance();
	ai::Conditions conds;
	conds.push_back(p_a->_condition);
	conds.push_back(p_b->_condition);
	ref->_condition = std::make_shared<ai::Or>(conds);
	return ref;
}

Ref<SimpleAICondition> SimpleAICondition::create_not(const Ref<SimpleAICondition> &p_cond) {
	ERR_FAIL_COND_V(p_cond.is_null(), Ref<SimpleAICondition>());
	Ref<SimpleAICondition> ref;
	ref.instance();
	ref->_condition = std::make_shared<ai::Not>(p_cond->_condition);
	return ref;
}

Ref<SimpleAICondition> SimpleAICondition::create_has_enemies(int p_min_count) {
	Ref<SimpleAICondition> ref;
	ref.instance();
	std::string params = p_min_count >= 0 ? std::to_string(p_min_count) : "";
	ref->_condition = std::make_shared<ai::HasEnemies>(params);
	return ref;
}

Ref<SimpleAICondition> SimpleAICondition::create_is_in_group(int p_group_id) {
	Ref<SimpleAICondition> ref;
	ref.instance();
	std::string params = p_group_id >= 0 ? std::to_string(p_group_id) : "";
	ref->_condition = std::make_shared<ai::IsInGroup>(params);
	return ref;
}

Ref<SimpleAICondition> SimpleAICondition::create_is_group_leader(int p_group_id) {
	Ref<SimpleAICondition> ref;
	ref.instance();
	std::string params = p_group_id >= 0 ? std::to_string(p_group_id) : "";
	ref->_condition = std::make_shared<ai::IsGroupLeader>(params);
	return ref;
}

Ref<SimpleAICondition> SimpleAICondition::create_custom(Object *p_object, const StringName &p_method) {
	ERR_FAIL_NULL_V(p_object, Ref<SimpleAICondition>());
	Ref<SimpleAICondition> ref;
	ref.instance();
	ref->_condition = std::make_shared<GdCallbackCondition>(p_object, p_method);
	return ref;
}

String SimpleAICondition::get_name() const {
	ERR_FAIL_COND_V(!_condition, "");
	return String(_condition->getName().c_str());
}

void SimpleAICondition::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_true_cond"), &SimpleAICondition::create_true_cond);
	ClassDB::bind_method(D_METHOD("create_false_cond"), &SimpleAICondition::create_false_cond);
	ClassDB::bind_method(D_METHOD("create_and", "a", "b"), &SimpleAICondition::create_and);
	ClassDB::bind_method(D_METHOD("create_or", "a", "b"), &SimpleAICondition::create_or);
	ClassDB::bind_method(D_METHOD("create_not", "condition"), &SimpleAICondition::create_not);
	ClassDB::bind_method(D_METHOD("create_has_enemies", "min_count"), &SimpleAICondition::create_has_enemies, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("create_is_in_group", "group_id"), &SimpleAICondition::create_is_in_group, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("create_is_group_leader", "group_id"), &SimpleAICondition::create_is_group_leader, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("create_custom", "object", "method"), &SimpleAICondition::create_custom);
	ClassDB::bind_method(D_METHOD("get_name"), &SimpleAICondition::get_name);
}

// ===========================================================================
// SimpleAITreeNode
// ===========================================================================

SimpleAITreeNode::SimpleAITreeNode() {}
SimpleAITreeNode::~SimpleAITreeNode() {}

// Helper: create node via type string using the internal constructors
static std::shared_ptr<ai::TreeNode> _make_node(const String &p_type, const String &p_name, const String &p_params, const Ref<SimpleAICondition> &p_cond) {
	std::string name = p_name.utf8().get_data();
	std::string params = p_params.utf8().get_data();
	ai::ConditionPtr cond = (p_cond.is_valid() && p_cond->get_internal()) ? p_cond->get_internal() : ai::True::get();

	if (p_type == "PrioritySelector") {
		return std::make_shared<ai::PrioritySelector>(name, params, cond);
	} else if (p_type == "Sequence") {
		return std::make_shared<ai::Sequence>(name, params, cond);
	} else if (p_type == "Parallel") {
		return std::make_shared<ai::Parallel>(name, params, cond);
	} else if (p_type == "RandomSelector") {
		return std::make_shared<ai::RandomSelector>(name, params, cond);
	} else if (p_type == "ProbabilitySelector") {
		return std::make_shared<ai::ProbabilitySelector>(name, params, cond);
	} else if (p_type == "Fail") {
		return std::make_shared<ai::Fail>(name, params, cond);
	} else if (p_type == "Succeed") {
		return std::make_shared<ai::Succeed>(name, params, cond);
	} else if (p_type == "Invert") {
		return std::make_shared<ai::Invert>(name, params, cond);
	} else if (p_type == "Limit") {
		return std::make_shared<ai::Limit>(name, params, cond);
	} else if (p_type == "Idle") {
		return std::make_shared<ai::Idle>(name, params, cond);
	}
	return std::shared_ptr<ai::TreeNode>();
}

static Ref<SimpleAITreeNode> _wrap_node(const std::shared_ptr<ai::TreeNode> &p_node) {
	if (!p_node) {
		return Ref<SimpleAITreeNode>();
	}
	Ref<SimpleAITreeNode> ref;
	ref.instance();
	ref->set_internal(p_node);
	return ref;
}

Ref<SimpleAITreeNode> SimpleAITreeNode::create_selector(const String &p_name, const Ref<SimpleAICondition> &p_cond) {
	return _wrap_node(_make_node("PrioritySelector", p_name, "", p_cond));
}

Ref<SimpleAITreeNode> SimpleAITreeNode::create_sequence(const String &p_name, const Ref<SimpleAICondition> &p_cond) {
	return _wrap_node(_make_node("Sequence", p_name, "", p_cond));
}

Ref<SimpleAITreeNode> SimpleAITreeNode::create_parallel(const String &p_name, const Ref<SimpleAICondition> &p_cond) {
	return _wrap_node(_make_node("Parallel", p_name, "", p_cond));
}

Ref<SimpleAITreeNode> SimpleAITreeNode::create_random_selector(const String &p_name, const Ref<SimpleAICondition> &p_cond) {
	return _wrap_node(_make_node("RandomSelector", p_name, "", p_cond));
}

Ref<SimpleAITreeNode> SimpleAITreeNode::create_probability_selector(const String &p_name, const Ref<SimpleAICondition> &p_cond) {
	return _wrap_node(_make_node("ProbabilitySelector", p_name, "", p_cond));
}

Ref<SimpleAITreeNode> SimpleAITreeNode::create_idle(const String &p_name, int p_millis) {
	return _wrap_node(_make_node("Idle", p_name, String::num_int64(p_millis), Ref<SimpleAICondition>()));
}

Ref<SimpleAITreeNode> SimpleAITreeNode::create_fail(const String &p_name) {
	return _wrap_node(_make_node("Fail", p_name, "", Ref<SimpleAICondition>()));
}

Ref<SimpleAITreeNode> SimpleAITreeNode::create_succeed(const String &p_name) {
	return _wrap_node(_make_node("Succeed", p_name, "", Ref<SimpleAICondition>()));
}

Ref<SimpleAITreeNode> SimpleAITreeNode::create_invert(const String &p_name, const Ref<SimpleAICondition> &p_cond) {
	return _wrap_node(_make_node("Invert", p_name, "", p_cond));
}

Ref<SimpleAITreeNode> SimpleAITreeNode::create_limit(const String &p_name, int p_max) {
	return _wrap_node(_make_node("Limit", p_name, String::num_int64(p_max), Ref<SimpleAICondition>()));
}

int SimpleAITreeNode::get_id() const {
	ERR_FAIL_COND_V(!_node, -1);
	return _node->getId();
}

String SimpleAITreeNode::get_name() const {
	ERR_FAIL_COND_V(!_node, "");
	return String(_node->getName().c_str());
}

void SimpleAITreeNode::set_name(const String &p_name) {
	ERR_FAIL_COND(!_node);
	_node->setName(p_name.utf8().get_data());
}

String SimpleAITreeNode::get_type() const {
	ERR_FAIL_COND_V(!_node, "");
	return String(_node->getType().c_str());
}

bool SimpleAITreeNode::add_child(const Ref<SimpleAITreeNode> &p_child) {
	ERR_FAIL_COND_V(!_node, false);
	ERR_FAIL_COND_V(p_child.is_null() || !p_child->_node, false);
	return _node->addChild(p_child->_node);
}

int SimpleAITreeNode::get_child_count() const {
	ERR_FAIL_COND_V(!_node, 0);
	return (int)_node->getChildren().size();
}

void SimpleAITreeNode::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_selector", "name", "condition"), &SimpleAITreeNode::create_selector, DEFVAL(Ref<SimpleAICondition>()));
	ClassDB::bind_method(D_METHOD("create_sequence", "name", "condition"), &SimpleAITreeNode::create_sequence, DEFVAL(Ref<SimpleAICondition>()));
	ClassDB::bind_method(D_METHOD("create_parallel", "name", "condition"), &SimpleAITreeNode::create_parallel, DEFVAL(Ref<SimpleAICondition>()));
	ClassDB::bind_method(D_METHOD("create_random_selector", "name", "condition"), &SimpleAITreeNode::create_random_selector, DEFVAL(Ref<SimpleAICondition>()));
	ClassDB::bind_method(D_METHOD("create_probability_selector", "name", "condition"), &SimpleAITreeNode::create_probability_selector, DEFVAL(Ref<SimpleAICondition>()));
	ClassDB::bind_method(D_METHOD("create_idle", "name", "millis"), &SimpleAITreeNode::create_idle);
	ClassDB::bind_method(D_METHOD("create_fail", "name"), &SimpleAITreeNode::create_fail);
	ClassDB::bind_method(D_METHOD("create_succeed", "name"), &SimpleAITreeNode::create_succeed);
	ClassDB::bind_method(D_METHOD("create_invert", "name", "condition"), &SimpleAITreeNode::create_invert, DEFVAL(Ref<SimpleAICondition>()));
	ClassDB::bind_method(D_METHOD("create_limit", "name", "max_count"), &SimpleAITreeNode::create_limit);

	ClassDB::bind_method(D_METHOD("get_id"), &SimpleAITreeNode::get_id);
	ClassDB::bind_method(D_METHOD("get_name"), &SimpleAITreeNode::get_name);
	ClassDB::bind_method(D_METHOD("set_name", "name"), &SimpleAITreeNode::set_name);
	ClassDB::bind_method(D_METHOD("get_type"), &SimpleAITreeNode::get_type);
	ClassDB::bind_method(D_METHOD("add_child", "child"), &SimpleAITreeNode::add_child);
	ClassDB::bind_method(D_METHOD("get_child_count"), &SimpleAITreeNode::get_child_count);

	BIND_ENUM_CONSTANT(STATUS_UNKNOWN);
	BIND_ENUM_CONSTANT(STATUS_CANNOTEXECUTE);
	BIND_ENUM_CONSTANT(STATUS_RUNNING);
	BIND_ENUM_CONSTANT(STATUS_FINISHED);
	BIND_ENUM_CONSTANT(STATUS_FAILED);
	BIND_ENUM_CONSTANT(STATUS_EXCEPTION);
}

// ===========================================================================
// SimpleAITask
// ===========================================================================

SimpleAITask::SimpleAITask() :
		_callback_object(nullptr) {}
SimpleAITask::~SimpleAITask() {}

void SimpleAITask::create(const String &p_name) {
	_task_name = p_name;
	// Node is created lazily when callback is set
}

void SimpleAITask::set_callback(Object *p_object, const StringName &p_method) {
	ERR_FAIL_NULL(p_object);
	_callback_object = p_object;
	_callback_method = p_method;
	_node = std::make_shared<GdCallbackTask>(
			_task_name.utf8().get_data(), _callback_object, _callback_method);
}

Ref<SimpleAITreeNode> SimpleAITask::as_tree_node() const {
	ERR_FAIL_COND_V(!_node, Ref<SimpleAITreeNode>());
	return _wrap_node(_node);
}

void SimpleAITask::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create", "name"), &SimpleAITask::create);
	ClassDB::bind_method(D_METHOD("set_callback", "object", "method"), &SimpleAITask::set_callback);
	ClassDB::bind_method(D_METHOD("as_tree_node"), &SimpleAITask::as_tree_node);
}

// ===========================================================================
// SimpleAIAggroMgr
// ===========================================================================

SimpleAIAggroMgr::SimpleAIAggroMgr() :
		_mgr(nullptr), _owned(false) {}

SimpleAIAggroMgr::~SimpleAIAggroMgr() {
	if (_owned && _mgr) {
		delete _mgr;
	}
}

void SimpleAIAggroMgr::add_aggro(int p_character_id, float p_amount) {
	ERR_FAIL_COND(!_mgr);
	_mgr->addAggro((ai::CharacterId)p_character_id, p_amount);
}

Dictionary SimpleAIAggroMgr::get_highest_entry() const {
	Dictionary d;
	ERR_FAIL_COND_V(!_mgr, d);
	ai::EntryPtr entry = _mgr->getHighestEntry();
	if (entry) {
		d["id"] = (int)entry->getCharacterId();
		d["aggro"] = entry->getAggro();
	}
	return d;
}

Array SimpleAIAggroMgr::get_entries() const {
	Array arr;
	ERR_FAIL_COND_V(!_mgr, arr);
	for (const auto &e : _mgr->getEntries()) {
		Dictionary d;
		d["id"] = (int)e.getCharacterId();
		d["aggro"] = e.getAggro();
		arr.push_back(d);
	}
	return arr;
}

void SimpleAIAggroMgr::clear() {
	// No direct clear — reset aggro entries is not exposed in SimpleAI.
	// Workaround: not implemented, user should manage AI lifecycle.
	ERR_FAIL_COND(!_mgr);
}

void SimpleAIAggroMgr::update(int64_t p_delta_ms) {
	ERR_FAIL_COND(!_mgr);
	_mgr->update(p_delta_ms);
}

void SimpleAIAggroMgr::set_reduce_by_ratio(float p_ratio_per_second, float p_min_aggro) {
	ERR_FAIL_COND(!_mgr);
	_mgr->setReduceByRatio(p_ratio_per_second, p_min_aggro);
}

void SimpleAIAggroMgr::set_reduce_by_value(float p_value_per_second) {
	ERR_FAIL_COND(!_mgr);
	_mgr->setReduceByValue(p_value_per_second);
}

void SimpleAIAggroMgr::_bind_methods() {
	ClassDB::bind_method(D_METHOD("add_aggro", "character_id", "amount"), &SimpleAIAggroMgr::add_aggro);
	ClassDB::bind_method(D_METHOD("get_highest_entry"), &SimpleAIAggroMgr::get_highest_entry);
	ClassDB::bind_method(D_METHOD("get_entries"), &SimpleAIAggroMgr::get_entries);
	ClassDB::bind_method(D_METHOD("clear"), &SimpleAIAggroMgr::clear);
	ClassDB::bind_method(D_METHOD("update", "delta_ms"), &SimpleAIAggroMgr::update);
	ClassDB::bind_method(D_METHOD("set_reduce_by_ratio", "ratio_per_second", "min_aggro"), &SimpleAIAggroMgr::set_reduce_by_ratio);
	ClassDB::bind_method(D_METHOD("set_reduce_by_value", "value_per_second"), &SimpleAIAggroMgr::set_reduce_by_value);
}

// ===========================================================================
// SimpleAIBehaviorTree
// ===========================================================================

SimpleAIBehaviorTree::SimpleAIBehaviorTree() {}
SimpleAIBehaviorTree::~SimpleAIBehaviorTree() {}

void SimpleAIBehaviorTree::create(const Ref<SimpleAITreeNode> &p_root) {
	ERR_FAIL_COND(p_root.is_null() || !p_root->get_internal());
	_ai = std::make_shared<ai::AI>(p_root->get_internal());
}

void SimpleAIBehaviorTree::update(int64_t p_delta_ms) {
	ERR_FAIL_COND(!_ai);
	_ai->update(p_delta_ms, false);
	// Execute the behaviour tree via shared_ptr
	ai::TreeNodePtr behaviour = _ai->getBehaviour();
	if (behaviour) {
		auto self = _ai; // ensure shared_ptr stays alive
		behaviour->execute(self, p_delta_ms);
	}
}

void SimpleAIBehaviorTree::set_character(const Ref<SimpleAICharacter> &p_character) {
	ERR_FAIL_COND(!_ai);
	ERR_FAIL_COND(p_character.is_null() || !p_character->get_internal());
	_ai->setCharacter(p_character->get_internal());
}

Ref<SimpleAICharacter> SimpleAIBehaviorTree::get_character() const {
	ERR_FAIL_COND_V(!_ai, Ref<SimpleAICharacter>());
	ai::ICharacterPtr chr = _ai->getCharacter();
	if (!chr) {
		return Ref<SimpleAICharacter>();
	}
	Ref<SimpleAICharacter> ref;
	ref.instance();
	// Note: creates a new wrapper around the same shared_ptr
	ref->create(chr->getId());
	return ref;
}

void SimpleAIBehaviorTree::set_behaviour(const Ref<SimpleAITreeNode> &p_root) {
	ERR_FAIL_COND(!_ai);
	ERR_FAIL_COND(p_root.is_null() || !p_root->get_internal());
	_ai->setBehaviour(p_root->get_internal());
}

void SimpleAIBehaviorTree::set_pause(bool p_pause) {
	ERR_FAIL_COND(!_ai);
	_ai->setPause(p_pause);
}

bool SimpleAIBehaviorTree::is_paused() const {
	ERR_FAIL_COND_V(!_ai, false);
	return _ai->isPause();
}

Ref<SimpleAIAggroMgr> SimpleAIBehaviorTree::get_aggro_manager() {
	ERR_FAIL_COND_V(!_ai, Ref<SimpleAIAggroMgr>());
	Ref<SimpleAIAggroMgr> ref;
	ref.instance();
	ref->set_internal(&_ai->getAggroMgr());
	return ref;
}

void SimpleAIBehaviorTree::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create", "root_node"), &SimpleAIBehaviorTree::create);
	ClassDB::bind_method(D_METHOD("update", "delta_ms"), &SimpleAIBehaviorTree::update);
	ClassDB::bind_method(D_METHOD("set_character", "character"), &SimpleAIBehaviorTree::set_character);
	ClassDB::bind_method(D_METHOD("get_character"), &SimpleAIBehaviorTree::get_character);
	ClassDB::bind_method(D_METHOD("set_behaviour", "root_node"), &SimpleAIBehaviorTree::set_behaviour);
	ClassDB::bind_method(D_METHOD("set_pause", "pause"), &SimpleAIBehaviorTree::set_pause);
	ClassDB::bind_method(D_METHOD("is_paused"), &SimpleAIBehaviorTree::is_paused);
	ClassDB::bind_method(D_METHOD("get_aggro_manager"), &SimpleAIBehaviorTree::get_aggro_manager);
}

// ===========================================================================
// SimpleAIGroupMgr
// ===========================================================================

SimpleAIGroupMgr::SimpleAIGroupMgr() :
		_mgr(nullptr) {}
SimpleAIGroupMgr::~SimpleAIGroupMgr() {}

bool SimpleAIGroupMgr::add(int p_group_id, const Ref<SimpleAIBehaviorTree> &p_ai) {
	ERR_FAIL_COND_V(!_mgr, false);
	ERR_FAIL_COND_V(p_ai.is_null() || !p_ai->get_internal(), false);
	return _mgr->add((ai::GroupId)p_group_id, p_ai->get_internal());
}

bool SimpleAIGroupMgr::remove(int p_group_id, const Ref<SimpleAIBehaviorTree> &p_ai) {
	ERR_FAIL_COND_V(!_mgr, false);
	ERR_FAIL_COND_V(p_ai.is_null() || !p_ai->get_internal(), false);
	return _mgr->remove((ai::GroupId)p_group_id, p_ai->get_internal());
}

int SimpleAIGroupMgr::get_group_size(int p_group_id) const {
	ERR_FAIL_COND_V(!_mgr, 0);
	return _mgr->getGroupSize((ai::GroupId)p_group_id);
}

Vector3 SimpleAIGroupMgr::get_position(int p_group_id) const {
	ERR_FAIL_COND_V(!_mgr, Vector3());
	return to_godot(_mgr->getPosition((ai::GroupId)p_group_id));
}

bool SimpleAIGroupMgr::is_in_group(int p_group_id, const Ref<SimpleAIBehaviorTree> &p_ai) const {
	ERR_FAIL_COND_V(!_mgr, false);
	ERR_FAIL_COND_V(p_ai.is_null() || !p_ai->get_internal(), false);
	return _mgr->isInGroup((ai::GroupId)p_group_id, p_ai->get_internal());
}

void SimpleAIGroupMgr::_bind_methods() {
	ClassDB::bind_method(D_METHOD("add", "group_id", "ai"), &SimpleAIGroupMgr::add);
	ClassDB::bind_method(D_METHOD("remove", "group_id", "ai"), &SimpleAIGroupMgr::remove);
	ClassDB::bind_method(D_METHOD("get_group_size", "group_id"), &SimpleAIGroupMgr::get_group_size);
	ClassDB::bind_method(D_METHOD("get_position", "group_id"), &SimpleAIGroupMgr::get_position);
	ClassDB::bind_method(D_METHOD("is_in_group", "group_id", "ai"), &SimpleAIGroupMgr::is_in_group);
}

// ===========================================================================
// SimpleAIFilter
// ===========================================================================

SimpleAIFilter::SimpleAIFilter() {}
SimpleAIFilter::~SimpleAIFilter() {}

Ref<SimpleAIFilter> SimpleAIFilter::select_zone() {
	Ref<SimpleAIFilter> ref;
	ref.instance();
	ref->_filter = std::make_shared<ai::SelectZone>("");
	return ref;
}

Ref<SimpleAIFilter> SimpleAIFilter::select_highest_aggro() {
	Ref<SimpleAIFilter> ref;
	ref.instance();
	ref->_filter = ai::SelectHighestAggro::get();
	return ref;
}

Ref<SimpleAIFilter> SimpleAIFilter::select_group_leader(int p_group_id) {
	Ref<SimpleAIFilter> ref;
	ref.instance();
	ref->_filter = std::make_shared<ai::SelectGroupLeader>(std::to_string(p_group_id));
	return ref;
}

Ref<SimpleAIFilter> SimpleAIFilter::select_group_members(int p_group_id) {
	Ref<SimpleAIFilter> ref;
	ref.instance();
	ref->_filter = std::make_shared<ai::SelectGroupMembers>(std::to_string(p_group_id));
	return ref;
}

Ref<SimpleAIFilter> SimpleAIFilter::select_empty() {
	Ref<SimpleAIFilter> ref;
	ref.instance();
	ref->_filter = ai::SelectEmpty::get();
	return ref;
}

Ref<SimpleAIFilter> SimpleAIFilter::select_all() {
	Ref<SimpleAIFilter> ref;
	ref.instance();
	ref->_filter = ai::SelectAll::get();
	return ref;
}

Ref<SimpleAIFilter> SimpleAIFilter::create_union(const Ref<SimpleAIFilter> &p_a, const Ref<SimpleAIFilter> &p_b) {
	ERR_FAIL_COND_V(p_a.is_null() || p_b.is_null(), Ref<SimpleAIFilter>());
	Ref<SimpleAIFilter> ref;
	ref.instance();
	ai::Filters filters;
	filters.push_back(p_a->_filter);
	filters.push_back(p_b->_filter);
	ref->_filter = std::make_shared<ai::Union>("", filters);
	return ref;
}

Ref<SimpleAIFilter> SimpleAIFilter::create_intersection(const Ref<SimpleAIFilter> &p_a, const Ref<SimpleAIFilter> &p_b) {
	ERR_FAIL_COND_V(p_a.is_null() || p_b.is_null(), Ref<SimpleAIFilter>());
	Ref<SimpleAIFilter> ref;
	ref.instance();
	ai::Filters filters;
	filters.push_back(p_a->_filter);
	filters.push_back(p_b->_filter);
	ref->_filter = std::make_shared<ai::Intersection>("", filters);
	return ref;
}

Ref<SimpleAIFilter> SimpleAIFilter::create_first(const Ref<SimpleAIFilter> &p_sub) {
	ERR_FAIL_COND_V(p_sub.is_null(), Ref<SimpleAIFilter>());
	Ref<SimpleAIFilter> ref;
	ref.instance();
	ai::Filters filters;
	filters.push_back(p_sub->_filter);
	ref->_filter = std::make_shared<ai::First>("", filters);
	return ref;
}

Ref<SimpleAIFilter> SimpleAIFilter::create_last(const Ref<SimpleAIFilter> &p_sub) {
	ERR_FAIL_COND_V(p_sub.is_null(), Ref<SimpleAIFilter>());
	Ref<SimpleAIFilter> ref;
	ref.instance();
	ai::Filters filters;
	filters.push_back(p_sub->_filter);
	ref->_filter = std::make_shared<ai::Last>("", filters);
	return ref;
}

Ref<SimpleAIFilter> SimpleAIFilter::create_random(const Ref<SimpleAIFilter> &p_sub, int p_count) {
	ERR_FAIL_COND_V(p_sub.is_null(), Ref<SimpleAIFilter>());
	Ref<SimpleAIFilter> ref;
	ref.instance();
	ai::Filters filters;
	filters.push_back(p_sub->_filter);
	ref->_filter = std::make_shared<ai::Random>(std::to_string(p_count), filters);
	return ref;
}

String SimpleAIFilter::get_name() const {
	ERR_FAIL_COND_V(!_filter, "");
	return String(_filter->getName().c_str());
}

void SimpleAIFilter::_bind_methods() {
	ClassDB::bind_method(D_METHOD("select_zone"), &SimpleAIFilter::select_zone);
	ClassDB::bind_method(D_METHOD("select_highest_aggro"), &SimpleAIFilter::select_highest_aggro);
	ClassDB::bind_method(D_METHOD("select_group_leader", "group_id"), &SimpleAIFilter::select_group_leader);
	ClassDB::bind_method(D_METHOD("select_group_members", "group_id"), &SimpleAIFilter::select_group_members);
	ClassDB::bind_method(D_METHOD("select_empty"), &SimpleAIFilter::select_empty);
	ClassDB::bind_method(D_METHOD("select_all"), &SimpleAIFilter::select_all);
	ClassDB::bind_method(D_METHOD("create_union", "a", "b"), &SimpleAIFilter::create_union);
	ClassDB::bind_method(D_METHOD("create_intersection", "a", "b"), &SimpleAIFilter::create_intersection);
	ClassDB::bind_method(D_METHOD("create_first", "sub_filter"), &SimpleAIFilter::create_first);
	ClassDB::bind_method(D_METHOD("create_last", "sub_filter"), &SimpleAIFilter::create_last);
	ClassDB::bind_method(D_METHOD("create_random", "sub_filter", "count"), &SimpleAIFilter::create_random, DEFVAL(1));
	ClassDB::bind_method(D_METHOD("get_name"), &SimpleAIFilter::get_name);
}

// ===========================================================================
// SimpleAISteering
// ===========================================================================

SimpleAISteering::SimpleAISteering() {}
SimpleAISteering::~SimpleAISteering() {}

Ref<SimpleAISteering> SimpleAISteering::create_wander(float p_rotation_deg) {
	Ref<SimpleAISteering> ref;
	ref.instance();
	ref->_steering = std::make_shared<ai::movement::Wander>(std::to_string(ai::toRadians(p_rotation_deg)));
	return ref;
}

Ref<SimpleAISteering> SimpleAISteering::create_group_seek(int p_group_id) {
	Ref<SimpleAISteering> ref;
	ref.instance();
	ref->_steering = std::make_shared<ai::movement::GroupSeek>(std::to_string(p_group_id));
	return ref;
}

Ref<SimpleAISteering> SimpleAISteering::create_group_flee(int p_group_id) {
	Ref<SimpleAISteering> ref;
	ref.instance();
	ref->_steering = std::make_shared<ai::movement::GroupFlee>(std::to_string(p_group_id));
	return ref;
}

Ref<SimpleAISteering> SimpleAISteering::create_target_seek(const Vector3 &p_target) {
	Ref<SimpleAISteering> ref;
	ref.instance();
	glm::vec3 t = to_glm(p_target);
	std::string params = std::to_string(t.x) + ":" + std::to_string(t.y) + ":" + std::to_string(t.z);
	ref->_steering = std::make_shared<ai::movement::TargetSeek>(params);
	return ref;
}

Ref<SimpleAISteering> SimpleAISteering::create_target_flee(const Vector3 &p_target) {
	Ref<SimpleAISteering> ref;
	ref.instance();
	glm::vec3 t = to_glm(p_target);
	std::string params = std::to_string(t.x) + ":" + std::to_string(t.y) + ":" + std::to_string(t.z);
	ref->_steering = std::make_shared<ai::movement::TargetFlee>(params);
	return ref;
}

Ref<SimpleAISteering> SimpleAISteering::create_selection_seek() {
	Ref<SimpleAISteering> ref;
	ref.instance();
	ref->_steering = std::make_shared<ai::movement::SelectionSeek>("");
	return ref;
}

Ref<SimpleAISteering> SimpleAISteering::create_selection_flee() {
	Ref<SimpleAISteering> ref;
	ref.instance();
	ref->_steering = std::make_shared<ai::movement::SelectionFlee>("");
	return ref;
}

String SimpleAISteering::get_name() const {
	ERR_FAIL_COND_V(!_steering, "");
	return "Steering";
}

void SimpleAISteering::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_wander", "rotation_deg"), &SimpleAISteering::create_wander, DEFVAL(10.0f));
	ClassDB::bind_method(D_METHOD("create_group_seek", "group_id"), &SimpleAISteering::create_group_seek);
	ClassDB::bind_method(D_METHOD("create_group_flee", "group_id"), &SimpleAISteering::create_group_flee);
	ClassDB::bind_method(D_METHOD("create_target_seek", "target"), &SimpleAISteering::create_target_seek);
	ClassDB::bind_method(D_METHOD("create_target_flee", "target"), &SimpleAISteering::create_target_flee);
	ClassDB::bind_method(D_METHOD("create_selection_seek"), &SimpleAISteering::create_selection_seek);
	ClassDB::bind_method(D_METHOD("create_selection_flee"), &SimpleAISteering::create_selection_flee);
	ClassDB::bind_method(D_METHOD("get_name"), &SimpleAISteering::get_name);
}

// ===========================================================================
// SimpleAIZone
// ===========================================================================

SimpleAIZone::SimpleAIZone() :
		_zone(nullptr), _zone_name("default") {
}

SimpleAIZone::~SimpleAIZone() {
	if (_zone) {
		delete _zone;
		_zone = nullptr;
	}
}

void SimpleAIZone::set_zone_name(const String &p_name) {
	_zone_name = p_name;
	if (_zone) {
		delete _zone;
	}
#ifdef NO_THREADS
	_zone = new ai::Zone(p_name.utf8().get_data(), 1);
#else
	int threads = MAX(1, OS::get_singleton()->get_processor_count() - 1);
	_zone = new ai::Zone(p_name.utf8().get_data(), threads);
#endif
}

String SimpleAIZone::get_zone_name() const {
	return _zone_name;
}

void SimpleAIZone::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			if (!_zone) {
#ifdef NO_THREADS
				_zone = new ai::Zone(_zone_name.utf8().get_data(), 1);
#else
				int threads = MAX(1, OS::get_singleton()->get_processor_count() - 1);
				_zone = new ai::Zone(_zone_name.utf8().get_data(), threads);
#endif
			}
			set_process(true);
		} break;
		case NOTIFICATION_PROCESS: {
			if (_zone) {
				int64_t dt = (int64_t)(get_process_delta_time() * 1000.0);
				_zone->update(dt);
			}
		} break;
	}
}

bool SimpleAIZone::add_ai(const Ref<SimpleAIBehaviorTree> &p_ai) {
	ERR_FAIL_COND_V(!_zone, false);
	ERR_FAIL_COND_V(p_ai.is_null() || !p_ai->get_internal(), false);
	return _zone->addAI(p_ai->get_internal());
}

bool SimpleAIZone::remove_ai(const Ref<SimpleAIBehaviorTree> &p_ai) {
	ERR_FAIL_COND_V(!_zone, false);
	ERR_FAIL_COND_V(p_ai.is_null() || !p_ai->get_internal(), false);
	return _zone->removeAI(p_ai->get_internal());
}

bool SimpleAIZone::destroy_ai(int p_character_id) {
	ERR_FAIL_COND_V(!_zone, false);
	return _zone->destroyAI((ai::CharacterId)p_character_id);
}

int SimpleAIZone::get_ai_count() const {
	ERR_FAIL_COND_V(!_zone, 0);
	return (int)_zone->size();
}

Ref<SimpleAIGroupMgr> SimpleAIZone::get_group_manager() {
	ERR_FAIL_COND_V(!_zone, Ref<SimpleAIGroupMgr>());
	Ref<SimpleAIGroupMgr> ref;
	ref.instance();
	ref->set_internal(&_zone->getGroupMgr());
	return ref;
}

void SimpleAIZone::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_zone_name", "name"), &SimpleAIZone::set_zone_name);
	ClassDB::bind_method(D_METHOD("get_zone_name"), &SimpleAIZone::get_zone_name);
	ClassDB::bind_method(D_METHOD("add_ai", "behavior_tree"), &SimpleAIZone::add_ai);
	ClassDB::bind_method(D_METHOD("remove_ai", "behavior_tree"), &SimpleAIZone::remove_ai);
	ClassDB::bind_method(D_METHOD("destroy_ai", "character_id"), &SimpleAIZone::destroy_ai);
	ClassDB::bind_method(D_METHOD("get_ai_count"), &SimpleAIZone::get_ai_count);
	ClassDB::bind_method(D_METHOD("get_group_manager"), &SimpleAIZone::get_group_manager);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "zone_name"), "set_zone_name", "get_zone_name");
}

// ===========================================================================
// DOCTESTS
// ===========================================================================

#ifdef DOCTEST

// Test helper: factory instances for calling instance-method factories
static Ref<SimpleAITreeNode> _node_factory() {
	Ref<SimpleAITreeNode> r;
	r.instance();
	return r;
}
static Ref<SimpleAICondition> _cond_f() {
	Ref<SimpleAICondition> r;
	r.instance();
	return r;
}
static Ref<SimpleAIFilter> _filter_f() {
	Ref<SimpleAIFilter> r;
	r.instance();
	return r;
}
static Ref<SimpleAISteering> _steer_f() {
	Ref<SimpleAISteering> r;
	r.instance();
	return r;
}

TEST_SUITE("[[simpleai]] SimpleAI") {
	// -----------------------------------------------------------------------
	// Character tests
	// -----------------------------------------------------------------------

	TEST_CASE("[simpleai] character creation and properties") {
		Ref<SimpleAICharacter> chr;
		chr.instance();
		chr->create(42);

		CHECK(chr->get_id() == 42);
		CHECK(chr->get_position() == Vector3());
		CHECK(chr->get_orientation() == doctest::Approx(0.0f));
		CHECK(chr->get_speed() == doctest::Approx(0.0f));

		SUBCASE("set/get position") {
			chr->set_position(Vector3(1.0f, 2.0f, 3.0f));
			Vector3 pos = chr->get_position();
			CHECK(pos.x == doctest::Approx(1.0f));
			CHECK(pos.y == doctest::Approx(2.0f));
			CHECK(pos.z == doctest::Approx(3.0f));
		}

		SUBCASE("set/get orientation") {
			chr->set_orientation(1.57f);
			CHECK(chr->get_orientation() == doctest::Approx(1.57f));
		}

		SUBCASE("set/get speed") {
			chr->set_speed(5.5f);
			CHECK(chr->get_speed() == doctest::Approx(5.5f));
		}

		SUBCASE("attributes") {
			chr->set_attribute("Name", "TestEntity");
			chr->set_attribute("Team", "Blue");
			Dictionary attrs = chr->get_attributes();
			CHECK(attrs.size() == 2);
			CHECK(String(attrs["Name"]) == "TestEntity");
			CHECK(String(attrs["Team"]) == "Blue");
		}
	}

	// -----------------------------------------------------------------------
	// TreeNode factory tests
	// -----------------------------------------------------------------------

	TEST_CASE("[simpleai] tree node factories") {
		SUBCASE("selector") {
			Ref<SimpleAITreeNode> node = _node_factory()->create_selector("root");
			REQUIRE(node.is_valid());
			CHECK(node->get_name() == "root");
			CHECK(node->get_type() == "PrioritySelector");
			CHECK(node->get_child_count() == 0);
		}

		SUBCASE("sequence") {
			Ref<SimpleAITreeNode> node = _node_factory()->create_sequence("seq1");
			REQUIRE(node.is_valid());
			CHECK(node->get_type() == "Sequence");
		}

		SUBCASE("parallel") {
			Ref<SimpleAITreeNode> node = _node_factory()->create_parallel("par1");
			REQUIRE(node.is_valid());
			CHECK(node->get_type() == "Parallel");
		}

		SUBCASE("random selector") {
			Ref<SimpleAITreeNode> node = _node_factory()->create_random_selector("rnd1");
			REQUIRE(node.is_valid());
			CHECK(node->get_type() == "RandomSelector");
		}

		SUBCASE("probability selector") {
			Ref<SimpleAITreeNode> node = _node_factory()->create_probability_selector("prob1");
			REQUIRE(node.is_valid());
			CHECK(node->get_name() == "prob1");
		}

		SUBCASE("idle") {
			Ref<SimpleAITreeNode> node = _node_factory()->create_idle("wait", 500);
			REQUIRE(node.is_valid());
			CHECK(node->get_type() == "Idle");
		}

		SUBCASE("fail") {
			Ref<SimpleAITreeNode> node = _node_factory()->create_fail("fail1");
			REQUIRE(node.is_valid());
			CHECK(node->get_type() == "Fail");
		}

		SUBCASE("succeed") {
			Ref<SimpleAITreeNode> node = _node_factory()->create_succeed("ok1");
			REQUIRE(node.is_valid());
			CHECK(node->get_type() == "Succeed");
		}

		SUBCASE("invert") {
			Ref<SimpleAITreeNode> node = _node_factory()->create_invert("inv1");
			REQUIRE(node.is_valid());
			CHECK(node->get_type() == "Invert");
		}

		SUBCASE("limit") {
			Ref<SimpleAITreeNode> node = _node_factory()->create_limit("lim1", 3);
			REQUIRE(node.is_valid());
			CHECK(node->get_type() == "Limit");
		}
	}

	TEST_CASE("[simpleai] tree node add_child") {
		Ref<SimpleAITreeNode> root = _node_factory()->create_selector("root");
		Ref<SimpleAITreeNode> child1 = _node_factory()->create_sequence("seq1");
		Ref<SimpleAITreeNode> child2 = _node_factory()->create_sequence("seq2");

		CHECK(root->add_child(child1));
		CHECK(root->add_child(child2));
		CHECK(root->get_child_count() == 2);
	}

	TEST_CASE("[simpleai] tree node set_name") {
		Ref<SimpleAITreeNode> node = _node_factory()->create_selector("original");
		CHECK(node->get_name() == "original");
		node->set_name("renamed");
		CHECK(node->get_name() == "renamed");
	}

	// -----------------------------------------------------------------------
	// Condition tests
	// -----------------------------------------------------------------------

	TEST_CASE("[simpleai] conditions true/false") {
		// Can't fully evaluate without an AI, but we can create and verify names
		Ref<SimpleAICondition> t = _cond_f()->create_true_cond();
		Ref<SimpleAICondition> f = _cond_f()->create_false_cond();
		REQUIRE(t.is_valid());
		REQUIRE(f.is_valid());
		CHECK(t->get_name() == "True");
		CHECK(f->get_name() == "False");
	}

	TEST_CASE("[simpleai] conditions composition") {
		Ref<SimpleAICondition> t = _cond_f()->create_true_cond();
		Ref<SimpleAICondition> f = _cond_f()->create_false_cond();

		SUBCASE("and") {
			Ref<SimpleAICondition> a = _cond_f()->create_and(t, f);
			REQUIRE(a.is_valid());
			CHECK(a->get_name() == "And");
		}

		SUBCASE("or") {
			Ref<SimpleAICondition> o = _cond_f()->create_or(t, f);
			REQUIRE(o.is_valid());
			CHECK(o->get_name() == "Or");
		}

		SUBCASE("not") {
			Ref<SimpleAICondition> n = _cond_f()->create_not(t);
			REQUIRE(n.is_valid());
			CHECK(n->get_name() == "Not");
		}
	}

	TEST_CASE("[simpleai] has_enemies condition") {
		Ref<SimpleAICondition> cond = _cond_f()->create_has_enemies(1);
		REQUIRE(cond.is_valid());
		CHECK(cond->get_name() == "HasEnemies");
	}

	TEST_CASE("[simpleai] is_in_group condition") {
		Ref<SimpleAICondition> cond = _cond_f()->create_is_in_group(5);
		REQUIRE(cond.is_valid());
		CHECK(cond->get_name() == "IsInGroup");
	}

	TEST_CASE("[simpleai] is_group_leader condition") {
		Ref<SimpleAICondition> cond = _cond_f()->create_is_group_leader(3);
		REQUIRE(cond.is_valid());
		CHECK(cond->get_name() == "IsGroupLeader");
	}

	// -----------------------------------------------------------------------
	// AggroMgr tests
	// -----------------------------------------------------------------------

	TEST_CASE("[simpleai] aggro manager") {
		// Create a behavior tree to get access to its aggro manager
		Ref<SimpleAITreeNode> root = _node_factory()->create_selector("root");
		Ref<SimpleAITreeNode> idle = _node_factory()->create_idle("idle", 100);
		root->add_child(idle);

		Ref<SimpleAIBehaviorTree> bt;
		bt.instance();
		bt->create(root);

		Ref<SimpleAICharacter> chr;
		chr.instance();
		chr->create(1);
		bt->set_character(chr);

		Ref<SimpleAIAggroMgr> aggro = bt->get_aggro_manager();
		REQUIRE(aggro.is_valid());

		SUBCASE("add and get highest") {
			aggro->add_aggro(10, 50.0f);
			aggro->add_aggro(20, 100.0f);
			aggro->add_aggro(30, 25.0f);

			Dictionary highest = aggro->get_highest_entry();
			CHECK(int(highest["id"]) == 20);
			CHECK(float(highest["aggro"]) == doctest::Approx(100.0f));
		}

		SUBCASE("get all entries") {
			aggro->add_aggro(10, 50.0f);
			aggro->add_aggro(20, 100.0f);

			Array entries = aggro->get_entries();
			CHECK(entries.size() == 2);
		}

		SUBCASE("additive aggro") {
			aggro->add_aggro(10, 50.0f);
			aggro->add_aggro(10, 30.0f);

			Dictionary highest = aggro->get_highest_entry();
			CHECK(int(highest["id"]) == 10);
			CHECK(float(highest["aggro"]) == doctest::Approx(80.0f));
		}

		SUBCASE("reduce by value") {
			aggro->set_reduce_by_value(10.0f);
			aggro->add_aggro(10, 50.0f);
			aggro->update(1000); // 1 second -> reduces by 10

			Array entries = aggro->get_entries();
			REQUIRE(entries.size() == 1);
			Dictionary e = entries[0];
			CHECK(float(e["aggro"]) == doctest::Approx(40.0f));
		}
	}

	// -----------------------------------------------------------------------
	// BehaviorTree execution tests
	// -----------------------------------------------------------------------

	TEST_CASE("[simpleai] behavior tree basic execution") {
		// Build: Selector -> Idle(100ms)
		Ref<SimpleAITreeNode> root = _node_factory()->create_selector("root");
		Ref<SimpleAITreeNode> idle = _node_factory()->create_idle("idle", 100);
		root->add_child(idle);

		Ref<SimpleAIBehaviorTree> bt;
		bt.instance();
		bt->create(root);

		Ref<SimpleAICharacter> chr;
		chr.instance();
		chr->create(1);
		chr->set_position(Vector3(5, 0, 5));
		chr->set_speed(2.0f);
		bt->set_character(chr);

		// First update — idle should be RUNNING (100ms timer not done)
		bt->update(50);
		// Character should still exist
		CHECK(chr->get_position().x == doctest::Approx(5.0f));
	}

	TEST_CASE("[simpleai] behavior tree pause") {
		Ref<SimpleAITreeNode> root = _node_factory()->create_selector("root");
		Ref<SimpleAITreeNode> idle = _node_factory()->create_idle("idle", 100);
		root->add_child(idle);

		Ref<SimpleAIBehaviorTree> bt;
		bt.instance();
		bt->create(root);

		Ref<SimpleAICharacter> chr;
		chr.instance();
		chr->create(2);
		bt->set_character(chr);

		CHECK_FALSE(bt->is_paused());
		bt->set_pause(true);
		CHECK(bt->is_paused());
		bt->set_pause(false);
		CHECK_FALSE(bt->is_paused());
	}

	TEST_CASE("[simpleai] behavior tree fail decorator") {
		Ref<SimpleAITreeNode> root = _node_factory()->create_selector("root");
		// Fail decorator wraps an idle (always returns FAILED regardless of child)
		Ref<SimpleAITreeNode> fail = _node_factory()->create_fail("always_fail");
		Ref<SimpleAITreeNode> fail_child = _node_factory()->create_idle("wrapped", 50);
		fail->add_child(fail_child);
		// Fallback idle runs when fail returns FAILED
		Ref<SimpleAITreeNode> idle = _node_factory()->create_idle("fallback", 50);
		root->add_child(fail);
		root->add_child(idle);

		Ref<SimpleAIBehaviorTree> bt;
		bt.instance();
		bt->create(root);

		Ref<SimpleAICharacter> chr;
		chr.instance();
		chr->create(3);
		bt->set_character(chr);

		// Fail wraps child and returns FAILED, selector falls through to idle
		bt->update(10);
		CHECK(root->get_child_count() == 2);
	}

	TEST_CASE("[simpleai] behavior tree sequence node") {
		// Sequence: idle(10) -> idle(10)
		Ref<SimpleAITreeNode> seq = _node_factory()->create_sequence("seq");
		Ref<SimpleAITreeNode> idle1 = _node_factory()->create_idle("idle1", 10);
		Ref<SimpleAITreeNode> idle2 = _node_factory()->create_idle("idle2", 10);
		seq->add_child(idle1);
		seq->add_child(idle2);

		Ref<SimpleAIBehaviorTree> bt;
		bt.instance();
		bt->create(seq);

		Ref<SimpleAICharacter> chr;
		chr.instance();
		chr->create(4);
		bt->set_character(chr);

		// First tick — idle1 starts running
		bt->update(5);
		CHECK(seq->get_child_count() == 2);

		// Tick past idle1 completion
		bt->update(10);
		// idle1 should be done, idle2 running
		CHECK(seq->get_child_count() == 2);
	}

	// -----------------------------------------------------------------------
	// Task callback test
	// -----------------------------------------------------------------------

	TEST_CASE("[simpleai] task creation") {
		Ref<SimpleAITask> task;
		task.instance();
		task->create("patrol");

		// Without callback, as_tree_node should fail (expected error)
		Ref<SimpleAITreeNode> node;
		EXPECT_ERROR(node = task->as_tree_node());
		CHECK(node.is_null());
	}

	// -----------------------------------------------------------------------
	// Filter tests
	// -----------------------------------------------------------------------

	TEST_CASE("[simpleai] filter factories") {
		SUBCASE("select_zone") {
			Ref<SimpleAIFilter> f = _filter_f()->select_zone();
			REQUIRE(f.is_valid());
			CHECK(f->get_name() == "SelectZone");
		}

		SUBCASE("select_highest_aggro") {
			Ref<SimpleAIFilter> f = _filter_f()->select_highest_aggro();
			REQUIRE(f.is_valid());
			CHECK(f->get_name() == "SelectHighestAggro");
		}

		SUBCASE("select_group_leader") {
			Ref<SimpleAIFilter> f = _filter_f()->select_group_leader(1);
			REQUIRE(f.is_valid());
			CHECK(f->get_name() == "SelectGroupLeader");
		}

		SUBCASE("select_group_members") {
			Ref<SimpleAIFilter> f = _filter_f()->select_group_members(1);
			REQUIRE(f.is_valid());
			CHECK(f->get_name() == "SelectGroupMembers");
		}

		SUBCASE("select_empty") {
			Ref<SimpleAIFilter> f = _filter_f()->select_empty();
			REQUIRE(f.is_valid());
			CHECK(f->get_name() == "SelectEmpty");
		}

		SUBCASE("select_all") {
			Ref<SimpleAIFilter> f = _filter_f()->select_all();
			REQUIRE(f.is_valid());
			CHECK(f->get_name() == "SelectAll");
		}
	}

	TEST_CASE("[simpleai] composite filters") {
		Ref<SimpleAIFilter> zone = _filter_f()->select_zone();
		Ref<SimpleAIFilter> aggro = _filter_f()->select_highest_aggro();

		SUBCASE("union") {
			Ref<SimpleAIFilter> u = _filter_f()->create_union(zone, aggro);
			REQUIRE(u.is_valid());
			CHECK(u->get_name() == "Union");
		}

		SUBCASE("intersection") {
			Ref<SimpleAIFilter> i = _filter_f()->create_intersection(zone, aggro);
			REQUIRE(i.is_valid());
			CHECK(i->get_name() == "Intersection");
		}

		SUBCASE("first") {
			Ref<SimpleAIFilter> f = _filter_f()->create_first(zone);
			REQUIRE(f.is_valid());
			CHECK(f->get_name() == "First");
		}

		SUBCASE("last") {
			Ref<SimpleAIFilter> l = _filter_f()->create_last(zone);
			REQUIRE(l.is_valid());
			CHECK(l->get_name() == "Last");
		}

		SUBCASE("random") {
			Ref<SimpleAIFilter> r = _filter_f()->create_random(zone, 2);
			REQUIRE(r.is_valid());
			CHECK(r->get_name() == "Random");
		}
	}

	// -----------------------------------------------------------------------
	// Steering tests
	// -----------------------------------------------------------------------

	TEST_CASE("[simpleai] steering factories") {
		SUBCASE("wander") {
			Ref<SimpleAISteering> s = _steer_f()->create_wander(15.0f);
			REQUIRE(s.is_valid());
		}

		SUBCASE("group_seek") {
			Ref<SimpleAISteering> s = _steer_f()->create_group_seek(1);
			REQUIRE(s.is_valid());
		}

		SUBCASE("group_flee") {
			Ref<SimpleAISteering> s = _steer_f()->create_group_flee(1);
			REQUIRE(s.is_valid());
		}

		SUBCASE("target_seek") {
			Ref<SimpleAISteering> s = _steer_f()->create_target_seek(Vector3(10, 0, 10));
			REQUIRE(s.is_valid());
		}

		SUBCASE("target_flee") {
			Ref<SimpleAISteering> s = _steer_f()->create_target_flee(Vector3(10, 0, 10));
			REQUIRE(s.is_valid());
		}

		SUBCASE("selection_seek") {
			Ref<SimpleAISteering> s = _steer_f()->create_selection_seek();
			REQUIRE(s.is_valid());
		}

		SUBCASE("selection_flee") {
			Ref<SimpleAISteering> s = _steer_f()->create_selection_flee();
			REQUIRE(s.is_valid());
		}
	}

	// -----------------------------------------------------------------------
	// Zone tests
	// -----------------------------------------------------------------------

	TEST_CASE("[simpleai] zone direct API") {
		// Test Zone without scene tree (direct instantiation)
		ai::Zone zone("test_zone", 1); // single-threaded for testing

		// Create AI entities
		auto chr1 = std::make_shared<GdCharacter>(1);
		chr1->setPosition(glm::vec3(0, 0, 0));
		chr1->setSpeed(1.0f);

		auto chr2 = std::make_shared<GdCharacter>(2);
		chr2->setPosition(glm::vec3(10, 0, 10));
		chr2->setSpeed(1.0f);

		auto idle_node = std::make_shared<ai::Idle>("idle", "100", ai::True::get());
		auto root1 = std::make_shared<ai::PrioritySelector>("root1", "", ai::True::get());
		root1->addChild(idle_node);

		auto idle_node2 = std::make_shared<ai::Idle>("idle2", "100", ai::True::get());
		auto root2 = std::make_shared<ai::PrioritySelector>("root2", "", ai::True::get());
		root2->addChild(idle_node2);

		auto ai1 = std::make_shared<ai::AI>(root1);
		ai1->setCharacter(chr1);

		auto ai2 = std::make_shared<ai::AI>(root2);
		ai2->setCharacter(chr2);

		SUBCASE("add and count") {
			zone.addAI(ai1);
			zone.update(0); // flush scheduled adds
			CHECK(zone.size() == 1);

			zone.addAI(ai2);
			zone.update(0);
			CHECK(zone.size() == 2);
		}

		SUBCASE("update ticks AI") {
			zone.addAI(ai1);
			zone.update(0);
			zone.update(50); // tick AI behavior trees
			CHECK(zone.size() == 1);
		}

		SUBCASE("remove") {
			zone.addAI(ai1);
			zone.addAI(ai2);
			zone.update(0);
			CHECK(zone.size() == 2);

			zone.removeAI(ai1);
			zone.update(0);
			CHECK(zone.size() == 1);
		}

		SUBCASE("destroy by id") {
			zone.addAI(ai1);
			zone.update(0);
			CHECK(zone.size() == 1);

			zone.destroyAI(1);
			zone.update(0);
			CHECK(zone.size() == 0);
		}
	}

	// -----------------------------------------------------------------------
	// GroupMgr tests
	// -----------------------------------------------------------------------

	TEST_CASE("[simpleai] group manager") {
		ai::Zone zone("group_test", 1);

		auto chr1 = std::make_shared<GdCharacter>(1);
		chr1->setPosition(glm::vec3(0, 0, 0));
		auto chr2 = std::make_shared<GdCharacter>(2);
		chr2->setPosition(glm::vec3(10, 0, 0));
		auto chr3 = std::make_shared<GdCharacter>(3);
		chr3->setPosition(glm::vec3(20, 0, 0));

		auto idle = std::make_shared<ai::Idle>("idle", "100", ai::True::get());
		auto r1 = std::make_shared<ai::PrioritySelector>("r1", "", ai::True::get());
		r1->addChild(idle);
		auto r2 = std::make_shared<ai::PrioritySelector>("r2", "", ai::True::get());
		r2->addChild(std::make_shared<ai::Idle>("idle2", "100", ai::True::get()));
		auto r3 = std::make_shared<ai::PrioritySelector>("r3", "", ai::True::get());
		r3->addChild(std::make_shared<ai::Idle>("idle3", "100", ai::True::get()));

		auto ai1 = std::make_shared<ai::AI>(r1);
		ai1->setCharacter(chr1);
		auto ai2 = std::make_shared<ai::AI>(r2);
		ai2->setCharacter(chr2);
		auto ai3 = std::make_shared<ai::AI>(r3);
		ai3->setCharacter(chr3);

		zone.addAI(ai1);
		zone.addAI(ai2);
		zone.addAI(ai3);
		zone.update(0);

		ai::GroupMgr &grp = zone.getGroupMgr();

		SUBCASE("add to group") {
			CHECK(grp.add(1, ai1));
			CHECK(grp.add(1, ai2));
			CHECK(grp.getGroupSize(1) == 2);
		}

		SUBCASE("leader is first added") {
			grp.add(1, ai1);
			grp.add(1, ai2);
			ai::AIPtr leader = grp.getLeader(1);
			REQUIRE(leader);
			CHECK(leader->getId() == 1);
		}

		SUBCASE("is_in_group") {
			grp.add(1, ai1);
			grp.add(1, ai2);
			CHECK(grp.isInGroup(1, ai1));
			CHECK(grp.isInGroup(1, ai2));
			CHECK_FALSE(grp.isInGroup(1, ai3));
		}

		SUBCASE("group position average") {
			grp.add(1, ai1); // pos (0,0,0)
			grp.add(1, ai2); // pos (10,0,0)
			grp.update(0); // recalculate positions
			glm::vec3 pos = grp.getPosition(1);
			CHECK(pos.x == doctest::Approx(5.0f));
			CHECK(pos.y == doctest::Approx(0.0f));
			CHECK(pos.z == doctest::Approx(0.0f));
		}

		SUBCASE("remove from group") {
			grp.add(1, ai1);
			grp.add(1, ai2);
			grp.add(1, ai3);
			CHECK(grp.getGroupSize(1) == 3);

			grp.remove(1, ai2);
			CHECK(grp.getGroupSize(1) == 2);
			CHECK_FALSE(grp.isInGroup(1, ai2));
		}

		SUBCASE("nonexistent group") {
			CHECK(grp.getGroupSize(999) == 0);
			CHECK_FALSE(grp.isInGroup(999, ai1));
		}
	}

	// -----------------------------------------------------------------------
	// Integration test: full pipeline
	// -----------------------------------------------------------------------

	TEST_CASE("[simpleai] integration: character + tree + zone update cycle") {
		// Create zone (single-threaded for testing)
		ai::Zone zone("integration", 1);

		// Create character
		auto chr = std::make_shared<GdCharacter>(1);
		chr->setPosition(glm::vec3(5, 0, 5));
		chr->setSpeed(2.0f);

		// Build tree: Selector -> Sequence -> Idle(50ms)
		auto idle = std::make_shared<ai::Idle>("idle", "50", ai::True::get());
		auto seq = std::make_shared<ai::Sequence>("seq", "", ai::True::get());
		seq->addChild(idle);
		auto root = std::make_shared<ai::PrioritySelector>("root", "", ai::True::get());
		root->addChild(seq);

		auto brain = std::make_shared<ai::AI>(root);
		brain->setCharacter(chr);

		// Add aggro
		brain->getAggroMgr().addAggro(99, 100.0f);
		ai::EntryPtr highest = brain->getAggroMgr().getHighestEntry();
		REQUIRE(highest);
		CHECK(highest->getCharacterId() == 99);

		// Add to zone and tick
		zone.addAI(brain);
		zone.update(0); // flush add
		CHECK(zone.size() == 1);

		// Tick several times
		for (int i = 0; i < 5; i++) {
			zone.update(20);
		}

		// Character position should be unchanged (idle doesn't move)
		CHECK(chr->getPosition().x == doctest::Approx(5.0f));
		CHECK(chr->getPosition().z == doctest::Approx(5.0f));
	}

	TEST_CASE("[simpleai] integration: godot wrappers end-to-end") {
		// Build tree using Godot wrappers
		Ref<SimpleAITreeNode> root = _node_factory()->create_selector("root");
		Ref<SimpleAITreeNode> idle = _node_factory()->create_idle("idle", 200);
		root->add_child(idle);

		// Create behavior tree
		Ref<SimpleAIBehaviorTree> bt;
		bt.instance();
		bt->create(root);

		// Create and attach character
		Ref<SimpleAICharacter> chr;
		chr.instance();
		chr->create(10);
		chr->set_position(Vector3(1, 2, 3));
		chr->set_speed(5.0f);
		bt->set_character(chr);

		// Tick
		bt->update(50);

		// Verify character is intact
		CHECK(chr->get_position().x == doctest::Approx(1.0f));
		CHECK(chr->get_speed() == doctest::Approx(5.0f));

		// Verify aggro access
		Ref<SimpleAIAggroMgr> aggro = bt->get_aggro_manager();
		REQUIRE(aggro.is_valid());
		aggro->add_aggro(42, 75.0f);
		Dictionary h = aggro->get_highest_entry();
		CHECK(int(h["id"]) == 42);
		CHECK(float(h["aggro"]) == doctest::Approx(75.0f));

		// Verify pause
		bt->set_pause(true);
		CHECK(bt->is_paused());
		bt->set_pause(false);

		// Multiple updates
		for (int i = 0; i < 10; i++) {
			bt->update(30);
		}
	}

	TEST_CASE("[simpleai] integration: behavior tree with conditional nodes") {
		// Build: Selector -> [Sequence(has_enemies) -> fail, Idle]
		Ref<SimpleAICondition> has_enemies = _cond_f()->create_has_enemies();
		Ref<SimpleAITreeNode> chase_seq = _node_factory()->create_sequence("chase", has_enemies);
		Ref<SimpleAITreeNode> fail = _node_factory()->create_fail("chase_fail");
		chase_seq->add_child(fail);

		Ref<SimpleAITreeNode> idle = _node_factory()->create_idle("patrol", 100);
		Ref<SimpleAITreeNode> root = _node_factory()->create_selector("root");
		root->add_child(chase_seq);
		root->add_child(idle);

		Ref<SimpleAIBehaviorTree> bt;
		bt.instance();
		bt->create(root);

		Ref<SimpleAICharacter> chr;
		chr.instance();
		chr->create(1);
		bt->set_character(chr);

		// No enemies — chase condition should fail, fall through to idle
		bt->update(10);
		CHECK(root->get_child_count() == 2);

		// Add aggro enemy
		Ref<SimpleAIAggroMgr> aggro = bt->get_aggro_manager();
		aggro->add_aggro(99, 100.0f);

		// Now has_enemies is true, chase_seq runs, hits fail node
		bt->update(10);
		CHECK(root->get_child_count() == 2);
	}
}

#endif // DOCTEST
