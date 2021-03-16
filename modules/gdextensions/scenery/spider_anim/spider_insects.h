#ifndef GD_SPIDER_INSECTS_H
#define GD_SPIDER_INSECTS_H

#include "scene/2d/node_2d.h"

class SpiderInsects : public Node2D {
	GDCLASS(SpiderInsects, Node2D);

	protected:
		static void _bind_methods() {}

		void _notification(int p_what) {}

		SpiderInsects() {}
};

class InsectsManagerInstance : public Node2D {
	GDCLASS(InsectsManagerInstance, Node2D);

	protected:
		static void _bind_methods() {}

		void _notification(int p_what) {}

		InsectsManagerInstance() {}
};

#endif // GD_SPIDER_INSECTS_H
