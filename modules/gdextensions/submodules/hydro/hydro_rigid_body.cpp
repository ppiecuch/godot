/**************************************************************************/
/*  hydro_rigid_body.cpp                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "hydro_rigid_body.h"
#include "clippable_mesh.h"
#include "water_area.h"
#include "watercraft_ballast.h"
#include "watercraft_propulsion.h"
#include "watercraft_rudder.h"

#include "core/class_db.h"
#include "core/math/aabb.h"
#include "core/math/plane.h"
#include "core/os/os.h"
#include "core/pool_vector.h"
#include "core/variant.h"
#include "core/version.h"
#include "scene/3d/immediate_geometry.h"
#include "scene/3d/mesh_instance.h"

// Prior to 3.4.0, Face3::get_area() returned twice the area that it should.
// Issue: https://github.com/godotengine/godot/issues/37048
#if VERSION_HEX < 0x030400
#define FACE3_AREA_FIX(x) ((x)*0.5)
#else
#define FACE3_AREA_FIX(x) (x)
#endif

HydroRigidBody::HydroRigidBody() :
		RigidBody() {
	m_debug_mesh = nullptr;
	m_water_area = nullptr;
	m_volume = 0;
	m_density = 0;
}

void HydroRigidBody::_bind_methods() {
}

void HydroRigidBody::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		for (int i = 0; i < get_child_count(); i++) {
			if (m_hull_mesh.is_empty()) {
				MeshInstance *mesh = Object::cast_to<MeshInstance>(get_child(i));
				if (mesh) {
					m_hull_mesh.load(mesh);
					m_density = get_mass() / m_hull_mesh.get_volume();
				}
			}
			if (!m_debug_mesh)
				m_debug_mesh = Object::cast_to<ImmediateGeometry>(get_child(i));
		}
		if (m_hull_mesh.is_empty()) {
			print_error("HydroRigidBody has no hull mesh!");
		}
	}
}
void HydroRigidBody::_direct_state_changed(Object *p_state) {
	if (m_debug_mesh)
		m_debug_mesh->clear();

	RigidBody::_direct_state_changed(p_state);
	state = Object::cast_to<PhysicsDirectBodyState>(p_state);

	Transform global_transform = get_global_transform();
	Transform local_transform = global_transform.affine_inverse();
	Vector3 origin = global_transform.get_origin();

	//Apply ballast weight
	for (int i = 0; i < m_ballast.size(); i++) {
		WatercraftBallast *ballast = m_ballast[i];
		Vector3 start = global_transform.xform(ballast->m_origin);
		Vector3 weight = state->get_total_gravity() * ballast->m_mass;
		state->add_force(weight, start - origin);
		if (m_debug_mesh)
			draw_debug_vector(weight, start, local_transform);
	}

	//Shortcut out if we aren't in the water
	if (!m_water_area || m_hull_mesh.is_empty())
		return;

	//Add rudders
	PoolVector<Face3> rudder_faces;
	for (int i = 0; i < m_rudders.size(); i++) {
		WatercraftRudder *rudder = m_rudders[i];
		rudder_faces.append_array(rudder->get_faces());
	}
	m_hull_mesh.add_rudder_faces(rudder_faces);

	//Generate water planes
	AABB aabb = global_transform.xform(m_hull_mesh.get_aabb());
	float half_x = aabb.size.x / 2;
	float half_z = aabb.size.z / 2;
	Vector3 wave_center = aabb.get_position() + Vector3(half_x, 0, half_z);

	PoolVector3Array wave_samples;
	wave_samples.resize(5);
	{
		PoolVector3Array::Write wave_samples_writer = wave_samples.write();
		wave_samples_writer[0] = Vector3(wave_center.x, 0, wave_center.z);
		wave_samples_writer[1] = Vector3(wave_center.x + half_x, 0, wave_center.z);
		wave_samples_writer[2] = Vector3(wave_center.x, 0, wave_center.z + half_z);
		wave_samples_writer[3] = Vector3(wave_center.x - half_x, 0, wave_center.z);
		wave_samples_writer[4] = Vector3(wave_center.x, 0, wave_center.z - half_z);
	}
	m_water_area->update_water_heights(wave_samples);

	Plane wave_planes[4];
	wave_planes[0] = Plane(wave_samples[0], wave_samples[1], wave_samples[2]);
	wave_planes[1] = Plane(wave_samples[0], wave_samples[2], wave_samples[3]);
	wave_planes[2] = Plane(wave_samples[0], wave_samples[3], wave_samples[4]);
	wave_planes[3] = Plane(wave_samples[0], wave_samples[4], wave_samples[1]);

	//Clip hull to water planes
	m_hull_mesh.clip_to_plane_quadrant(wave_center, wave_planes, global_transform);

	//are we underwater?
	if (m_hull_mesh.clipped_face_count() == 0)
		return;

	if (m_debug_mesh) {
		draw_debug_mesh(m_hull_mesh, local_transform);
		draw_debug_face(Face3(wave_samples[0], wave_samples[1], wave_samples[2]), local_transform);
		draw_debug_face(Face3(wave_samples[0], wave_samples[2], wave_samples[3]), local_transform);
		draw_debug_face(Face3(wave_samples[0], wave_samples[3], wave_samples[4]), local_transform);
		draw_debug_face(Face3(wave_samples[0], wave_samples[4], wave_samples[1]), local_transform);
	}

	//Apply propulsion
	for (int i = 0; i < m_propulsion.size(); i++) {
		WatercraftPropulsion *prop = m_propulsion[i];
		if (prop->m_direction.length_squared() > 0 && Math::absf(prop->m_value) > CMP_EPSILON) {
			Vector3 start = global_transform.xform(prop->m_origin);
			int prop_quadrant = ClippableMesh::get_quadrant(wave_center, start);
			if (!wave_planes[prop_quadrant].is_point_over(start)) {
				Basis b = global_transform.get_basis();
				Vector3 thrust = b.xform(prop->m_direction * prop->m_value);
				state->add_force(thrust, start - origin);
				if (m_debug_mesh)
					draw_debug_vector(thrust, start, local_transform);
			}
		}
	}

	float fluid_density = m_water_area->get_density();
	float fluid_viscosity = m_water_area->get_viscosity();
	float gravity = -state->get_total_gravity().length();

	//Calculate buoyancy, drag, and lift per-face
	Vector3 base_velocity = get_linear_velocity() - m_water_area->get_flow_direction();
	for (int i = 0; i < m_hull_mesh.clipped_face_count(); i++) {
		const Face3 &f = m_hull_mesh.get_clipped_face(i);
		Vector3 center_tri = f.get_median_point();
		Vector3 normal = f.get_plane().normal;
		float area = FACE3_AREA_FIX(f.get_area());
		int q = m_hull_mesh.get_quadrant(wave_center, center_tri);

		//Buoyant force
		float depth = fabsf(wave_planes[q].distance_to(center_tri));
		Vector3 buoyancy_force = fluid_density * gravity * depth * area * normal;
		buoyancy_force.x = 0;
		buoyancy_force.z = 0;
		state->add_force(buoyancy_force, center_tri - origin);
		if (m_debug_mesh)
			draw_debug_vector(buoyancy_force, center_tri, local_transform);

		//Drag and lift forces
		Vector3 vel = base_velocity + get_angular_velocity().cross(center_tri - origin);
		Vector3 vel_dir = vel.normalized();
		float drag_coef = vel_dir.dot(normal);
		if (drag_coef > 0) {
			float mag = area * fluid_density * fluid_viscosity * vel.length_squared();
			Vector3 drag_lift = -vel_dir * drag_coef * mag;

			float c = vel_dir.cross(normal).length();
			float lift_coef = fmin(c, drag_coef) / fmax(c, drag_coef);
			Vector3 lift_dir = vel_dir.cross(normal).cross(vel_dir).normalized();
			drag_lift += -lift_dir * lift_coef * mag;
			state->add_force(drag_lift, center_tri - origin);
			if (m_debug_mesh)
				draw_debug_vector(drag_lift, center_tri, local_transform);
		}
	}
}

void HydroRigidBody::draw_debug_face(const Face3 &face, const Transform &transform) {
	m_debug_mesh->begin(Mesh::PRIMITIVE_LINE_LOOP);
	m_debug_mesh->set_color(Color(0, 1, 1));
	for (int i = 0; i < 3; i++)
		m_debug_mesh->add_vertex(transform.xform(face.vertex[i]));
	m_debug_mesh->end();
}

void HydroRigidBody::draw_debug_mesh(const ClippableMesh &mesh, const Transform &transform) {
	const float scale = 1.001;
	for (int i = 0; i < mesh.clipped_face_count(); i++) {
		const Face3 &f = mesh.get_clipped_face(i);
		draw_debug_face(f, transform);
		m_debug_mesh->begin(Mesh::PRIMITIVE_LINE_LOOP);
		m_debug_mesh->set_color(Color(1, 1, 0));
		for (int j = 0; j < 3; j++)
			m_debug_mesh->add_vertex(transform.xform(f.vertex[j]) * scale);
		m_debug_mesh->end();
	}
}

void HydroRigidBody::draw_debug_vector(const Vector3 &dir, const Vector3 &origin, const Transform &transform) {
	m_debug_mesh->begin(Mesh::PRIMITIVE_LINES);
	m_debug_mesh->set_color(Color(1, 0, 1));
	m_debug_mesh->add_vertex(transform.xform(origin));
	m_debug_mesh->add_vertex(transform.xform(origin + dir * 50 / get_mass()));
	m_debug_mesh->end();
}
