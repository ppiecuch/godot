#include "bt_composite_node.h"

void BTCompositeNode::add_child_node(BTNode& child, Vector<BehaviorTree::Node*>& node_hierarchy) {
	BTNode* p_parent = get_parent() ? Object::cast_to<BTNode>(get_parent()) : NULL;
	ERR_FAIL_NULL_MSG(p_parent, "Parent node is not a BTNode.");
	if (p_parent) {
		node_hierarchy.push_back(get_behavior_node());
		p_parent->add_child_node(child, node_hierarchy);
	}
}

void BTCompositeNode::remove_child_node(BTNode& child, Vector<BehaviorTree::Node*>& node_hierarchy) {
	BTNode* p_parent = get_parent() ? Object::cast_to<BTNode>(get_parent()) : NULL;
	if (p_parent) {
		node_hierarchy.push_back(get_behavior_node());
		p_parent->remove_child_node(child, node_hierarchy);
	}
}

void BTCompositeNode::move_child_node(BTNode& child, Vector<BehaviorTree::Node*>& node_hierarchy) {
	BTNode* p_parent = get_parent() ? Object::cast_to<BTNode>(get_parent()) : NULL;
	if (p_parent) {
		node_hierarchy.push_back(get_behavior_node());
		p_parent->move_child_node(child, node_hierarchy);
	}
}

