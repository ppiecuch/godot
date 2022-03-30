#include "core/reference.h"
#include "scene/2d/node_2d.h"

class GdSparkParticles : public Node2D {
	GDCLASS(GdSpark, Node2D);

	protected:
		static void _bind_methods();

		void _notification(int p_what);

	public:

		Error load_from_file(const String &p_path);

		GdSpark() {}
};
