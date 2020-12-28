#ifndef SPHERICAL_WAVES_H
#define SPHERICAL_WAVES_H

#include "core/object.h"
#include "core/reference.h"
#include "core/variant.h"
#include "scene/main/node.h"
#include "scene/3d/spatial.h"
#include "scene/3d/mesh_instance.h"
#include "scene/resources/mesh_data_tool.h"
#include "scene/resources/mesh.h"


class SphericalWaves : public Reference {

	GDCLASS(SphericalWaves, Reference);

	real_t* currentAmplitudes;
	real_t* nextAmplitudes;
	real_t* velocities;

	int x_size, y_size;
	real_t springConstant, friction, twoSquareHalf;

protected:
	static void _bind_methods();

public:
	void init(int x_size, int y_size, real_t springConstant, real_t friction);
	real_t get_amplitude(int x, int y);
	void set_amplitude(int x, int y, real_t value);
	void update(real_t deltaT);
	void set_nodes(Vector<Variant> voxels, int index);
	void set_mesh(const Ref<Mesh> &mesh);

	SphericalWaves();
	~SphericalWaves();
};

#endif // SPHERICAL_WAVES_H
