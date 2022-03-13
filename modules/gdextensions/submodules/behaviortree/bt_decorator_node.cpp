#include "bt_decorator_node.h"
#include "bt_string_names.h"

BTDecoratorNode::BTDecoratorNode()
	: delegate(*this)
{
}

void BTDecoratorNode::_bind_methods() {
	BIND_VMETHOD( MethodInfo(BTStringNames::get_singleton()->_continue, PropertyInfo(Variant::INT,"index"), PropertyInfo(Variant::NIL,"context")) );
	BIND_VMETHOD( MethodInfo(BTStringNames::get_singleton()->_prepare, PropertyInfo(Variant::INT,"index"), PropertyInfo(Variant::NIL,"context")) );
	BIND_VMETHOD( MethodInfo(BTStringNames::get_singleton()->_pre_update, PropertyInfo(Variant::INT,"index"), PropertyInfo(Variant::NIL,"context")) );
	BIND_VMETHOD( MethodInfo(BTStringNames::get_singleton()->_post_update, PropertyInfo(Variant::INT,"index"), PropertyInfo(Variant::NIL,"context"), PropertyInfo(Variant::INT,"child_state")) );
	BIND_VMETHOD( MethodInfo(BTStringNames::get_singleton()->_abort, PropertyInfo(Variant::INT,"index"), PropertyInfo(Variant::NIL,"context")) );
}

void BTDecoratorNode::add_child_node(BTNode& child, Vector<BehaviorTree::Node*>& node_hierarchy) {
	BTNode* p_parent = get_parent() ? Object::cast_to<BTNode>(get_parent()) : NULL;
	ERR_FAIL_NULL_MSG(p_parent, "Parent node is not a BTNode.");
	ERR_FAIL_COND_MSG(get_child_count() > 1, "Decorator node only allowed one child.");
	if (p_parent && get_child_count() <= 1) {
		node_hierarchy.push_back(get_behavior_node());
		p_parent->add_child_node(child, node_hierarchy);
	}
}

void BTDecoratorNode::remove_child_node(BTNode& child, Vector<BehaviorTree::Node*>& node_hierarchy) {
	BTNode* p_parent = get_parent() ? Object::cast_to<BTNode>(get_parent()) : NULL;
	if (p_parent) {
		node_hierarchy.push_back(get_behavior_node());
		p_parent->remove_child_node(child, node_hierarchy);
	}
}

void BTDecoratorNode::move_child_node(BTNode& child, Vector<BehaviorTree::Node*>& node_hierarchy) {
	BTNode* p_parent = get_parent() ? Object::cast_to<BTNode>(get_parent()) : NULL;
	ERR_FAIL_NULL_MSG(p_parent, "Parent node is not a BTNode.");
	ERR_FAIL_COND_MSG(get_child_count() > 1, "Decorator node only allowed one child.");
	if (p_parent && get_child_count() <= 1) {
		node_hierarchy.push_back(get_behavior_node());
		p_parent->move_child_node(child, node_hierarchy);
	}
}

void BTDecoratorNode::Delegate::restore_running(
		BehaviorTree::VirtualMachine& vm,
		BehaviorTree::IndexType index,
		void* context,
	BehaviorTree::VMRunningData& running_data) {
	super::restore_running(vm, index, context, running_data);
	script_call(BTStringNames::get_singleton()->_continue, index, context);
}

void BTDecoratorNode::Delegate::prepare(
		BehaviorTree::VirtualMachine& vm,
		BehaviorTree::IndexType index,
		void* context,
	BehaviorTree::VMRunningData& running_data) {
	super::prepare(vm, index, context, running_data);
	script_call(BTStringNames::get_singleton()->_prepare, index, context);
}

BehaviorTree::E_State BTDecoratorNode::Delegate::pre_update(BehaviorTree::IndexType index, void* context, BehaviorTree::VMRunningData&) {
	Variant result_state = script_call(BTStringNames::get_singleton()->_pre_update, index, context);
	ERR_FAIL_COND_V_MSG( result_state.get_type() != Variant::INT, BehaviorTree::BH_ERROR, "Variant type is not int." );
	return static_cast<BehaviorTree::E_State>(static_cast<int>(result_state));
}

BehaviorTree::E_State BTDecoratorNode::Delegate::post_update(
		BehaviorTree::IndexType index,
		void* context,
		BehaviorTree::E_State child_state,
		BehaviorTree::VMRunningData&) {
	Variant result_state = script_call(BTStringNames::get_singleton()->_post_update, index, context, child_state);
	ERR_FAIL_COND_V_MSG( result_state.get_type() != Variant::INT, BehaviorTree::BH_ERROR, "Variant type is not int." );
	return static_cast<BehaviorTree::E_State>(static_cast<int>(result_state));
}

void BTDecoratorNode::Delegate::abort(BehaviorTree::VirtualMachine& vm, BehaviorTree::IndexType index, void* context, BehaviorTree::VMRunningData& running_data) {
	super::abort(vm, index, context, running_data);
	script_call(BTStringNames::get_singleton()->_abort, index, context);
}
