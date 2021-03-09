#ifndef GD_SPIDER_H
#define GD_SPIDER_H

#include "scene/2d/node_2d.h"

class Spider : public Node2D {
	GDCLASS(Spider, Node2D);

	protected:
		static void _bind_methods() {}

		void _notification(int p_what) {}

		Spider() {}
};

#endif // GD_SPIDER_H
