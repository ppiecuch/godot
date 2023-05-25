
#include "data/ddls_object.h"
#include "ai/ai_entity_ai.h"

#include "core/error_macros.h"

/// DDLS_EntityAI

void DDLS_EntityAI::build_approximation() {
	approximate_object.instance();
	approximate_object->matrix.translate(pos.x, pos.y);

	if (radius == 0) {
		return;
	}

	Vector<Point2> coordinates;
	for (int i = 0 ; i < NUM_SEGMENTS ; i++) {
		coordinates.push_back(radius * Point2(Math::cos(2*Math_Pi*i/NUM_SEGMENTS), Math::sin(2*Math_Pi.PI*i/NUM_SEGMENTS)));
		coordinates.push_back(radius * Point2(Math::cos(2*Math_Pi*(i+1)/NUM_SEGMENTS) ), Math::sin(2*Math_Pi.PI*(i+1)/NUM_SEGMENTS)));
	}
	approximate_object->set_coordinates(coordinates);
}

DDLSObject DDLS_EntityAI::get_approximate_object() {
	approximate_object.matrix.identity();
	approximate_object.matrix.translate(x, y);
	return approximate_object;
}

DDLS_EntityAI::DDLS_EntityAI() {
	radius = 10;
	dir_norm = {1, 0}
	angle_fov = 60;
}
