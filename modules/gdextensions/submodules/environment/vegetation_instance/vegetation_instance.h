/*************************************************************************/
/*  vegetation_instance.h                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

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
	void populate(const NodePath &surface_source);

	void set_instance_count(const uint16_t &p_instance_count);
	uint16_t get_instance_count() const;

	void set_populate_axis(const uint8_t &p_populate_axis);
	uint8_t get_populate_axis() const;

	void set_tilt_random(const float &p_tilt_random);
	float get_tilt_random() const;

	void set_rotate_random(const float &p_rotate_random);
	float get_rotate_random() const;

	void set_scale_random(const float &p_scale_random);
	float get_scale_random() const;

	void set_scale_amount(const float &p_scale_amount);
	float get_scale_amount() const;

	void set_custom_normal(const Vector3 &p_custom_normal);
	Vector3 get_custom_normal() const;

	void set_custom_offset(const Vector3 &p_custom_offset);
	Vector3 get_custom_offset() const;

	VegetationInstance();
	~VegetationInstance();
};

#endif // VEGETATIONINSTANCE_H
