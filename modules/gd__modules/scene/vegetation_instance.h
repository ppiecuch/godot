/* vegetation_instance.h */

#ifndef VEGETATIONINSTANCE_H
#define VEGETATIONINSTANCE_H

#include "scene/3d/multimesh_instance.h"

class VegetationInstance : public MultiMeshInstance {
    GDCLASS(VegetationInstance, MultiMeshInstance);

	uint16_t instance_count;
	float tilt_random;
	float rotate_random;
	float scale_random;
	float scale_amount;
	uint8_t populate_axis;
	Vector3 custom_normal;
	Vector3 custom_offset;

protected:

	static void _bind_methods();


public:

	void populate(const NodePath & surface_source);

	void set_instance_count(const uint16_t& p_instance_count);
	uint16_t get_instance_count() const;

	void set_populate_axis(const uint8_t& p_populate_axis);
	uint8_t get_populate_axis() const;

	void set_tilt_random(const float& p_tilt_random);
	float get_tilt_random() const;

	void set_rotate_random(const float& p_rotate_random);
	float get_rotate_random() const;

	void set_scale_random(const float& p_scale_random);
	float get_scale_random() const;

	void set_scale_amount(const float& p_scale_amount);
	float get_scale_amount() const;

	void set_custom_normal(const Vector3 & p_custom_normal);
	Vector3 get_custom_normal() const;

	void set_custom_offset(const Vector3 & p_custom_offset);
	Vector3 get_custom_offset() const;

	VegetationInstance();
	~VegetationInstance();
};

#endif // VEGETATIONINSTANCE_H
