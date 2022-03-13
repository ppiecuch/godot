#include "bt_node.h"

void BTNode::move_child_notify(Node *p_child) {
	BTNode* p_btnode = Object::cast_to<BTNode>(p_child);
	ERR_FAIL_NULL_MSG(p_btnode, "Child node is not a BTNode.");

	if (p_btnode) {
		Vector<BehaviorTree::Node*> node_hierarchy;
		move_child_node(*p_btnode, node_hierarchy);
	}
}

void BTNode::add_child_notify(Node *p_child) {
	BTNode* p_btnode = Object::cast_to<BTNode>(p_child);
	ERR_FAIL_NULL_MSG(p_btnode, "Child node is not a BTNode.");
	if (p_btnode) {
		Vector<BehaviorTree::Node*> node_hierarchy;
		add_child_node(*p_btnode, node_hierarchy);
	}
}

void BTNode::remove_child_notify(Node *p_child) {
	BTNode* p_btnode = Object::cast_to<BTNode>(p_child);
	ERR_FAIL_NULL_MSG(p_btnode, "Child node is not a BTNode.");
	if (p_btnode) {
		Vector<BehaviorTree::Node*> node_hierarchy;
		node_hierarchy.push_back(p_btnode->get_behavior_node());
		remove_child_node(*p_btnode, node_hierarchy);
	}
}

void BTNode::_bind_methods() {
	ClassDB::bind_integer_constant( get_class_static(), StringName(), "BH_SUCCESS", BehaviorTree::BH_SUCCESS);
	ClassDB::bind_integer_constant( get_class_static(), StringName(), "BH_FAILURE", BehaviorTree::BH_FAILURE);
	ClassDB::bind_integer_constant( get_class_static(), StringName(), "BH_RUNNING", BehaviorTree::BH_RUNNING);
	ClassDB::bind_integer_constant( get_class_static(), StringName(), "BH_ERROR", BehaviorTree::BH_ERROR);
}
