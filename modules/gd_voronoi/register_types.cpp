#include "register_types.h"
#include "voronoi.h"

void register_gd_voronoi_types() {

	ClassDB::register_class<Voronoi>();
	ClassDB::register_class<VoronoiDiagram>();
	ClassDB::register_class<VoronoiSite>();
	ClassDB::register_class<VoronoiEdge>();

}

void unregister_gd_voronoi_types() {

}

