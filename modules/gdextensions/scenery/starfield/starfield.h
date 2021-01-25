#ifndef STARFIELD_H
#define STARFIELD_H

#include "core/reference.h"
#include "core/color.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/mesh.h"

#include <vector>

typedef unsigned int layerid_t;

class Starfield : public Reference
{
public:
	Starfield();

	void regenerate();
	void regenerate(layerid_t p_layer);
	void regenerate(layerid_t p_layer, Vector2 p_size);
	void regenerate(layerid_t p_layer, unsigned int p_number_of_stars);
	void regenerate(layerid_t p_layer, Vector2 p_size, unsigned int p_number_of_stars);

	layerid_t add_stars(layerid_t p_number_of_stars = 100, Vector2 p_size = Vector2(), const Color &p_color = Color(160, 160, 160), Ref<Texture> p_tex = Ref<Texture>());

	void move(Vector2 p_movement);

	void set_color(layerid_t p_layer, const Color &p_color);

	void draw(Node2D &canvas);

private:
	Ref<ArrayMesh> _mesh;

	struct StarsLayer {
		PoolPoint2Array positions;
		PoolColorArray colors;
		Vector2 size;
		Color color;
	};

	std::vector<StarsLayer> _layers;

	void _update_mesh();
};

#endif /* STARFIELD_H */
