#include "data/ddls_mesh.h"

#include "core/vector.h"

class DDLS_PathFinder {
	DDLSMesh mesh;
	DDLSAStar astar;
	DDLSFunnel funnel;
	DDLSEntityAI entity;

public:
	DDLSEntityAI get_entity() const;
	void set_entity(DDLSEntityAI p_entity);

	DDLSMesh get_mesh() const;
	void set_mesh(DDLSMesh p_mesh);

	void find_path(const Point2 &p_to, Vector<Point2> &p_result_path);

	DDLS_PathFinder();
};
