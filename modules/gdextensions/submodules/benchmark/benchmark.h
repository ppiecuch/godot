#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "scene/main/node.h"
#include "scene/resources/font.h"

class Benchmark : public Node {
	GDCLASS(Benchmark, Node);

	Ref<BitmapFont> screen_font;
	int num_objects;
	real_t yaw;

protected:
	void _notification(int p_notification);
	static void _bind_methods();

public:
	Benchmark();
	~Benchmark();
};

#endif // BENCHMARK_H
