#ifndef _SHAKERNODE_H_
#define _SHAKERNODE_H_

#include "core/node_path.h"
#include "scene/2d/node_2d.h"

class ShakerNode : public Node2D {
	GDCLASS(ShakerNode, Node2D);

	bool affect_position;
	bool affect_rotation;
	real_t max_rotation_shake;
	real_t max_position_shake;
	NodePath target_node_path;
	real_t trauma_decay;
	bool use_time_scale;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_target_node_path(const NodePath &p_node);
	void set_target_group_name(const String &p_name);

	ShakerNode();
};

#endif /* _SHAKERNODE_H_ */
