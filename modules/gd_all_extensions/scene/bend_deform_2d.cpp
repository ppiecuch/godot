/*************************************************************************/
/*  bend_deform_2d.cpp                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include <string>
#include <vector>

#include "scene/resources/mesh_data_tool.h"
#include "scene/resources/mesh.h"

#include "bend_deform_2d.h"

// https://www.reddit.com/r/godot/comments/9y74r6/how_to_detect_when_node2d_is_moveddragged_in_the/


// BEGIN Elastic-deform group node.

void ElasticSimulator2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			set_notify_local_transform(true);
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
		} break;
		default:
			Node2D::_notification(p_what);
	}
}

Ref<ElasticSimualtion> ElasticSimulator2D::get_simulator() const { return sim; }

void ElasticSimulator2D::_bind_methods() {
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "force_impulse"), "set_force_impulse", "get_force_impulse");
}

ElasticSimulator2D::ElasticSimulator2D() {
}

ElasticSimulator2D::~ElasticSimulator2D() {
}

// END


// BEGIN Mesh elastic-deform.

void DeformMeshInstance2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			set_notify_local_transform(true);
		} break;
		case NOTIFICATION_DRAW: {
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
		} break;
		case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
			MeshInstance2D::_notification(p_what);
		} break;
		default:
			MeshInstance2D::_notification(p_what);
	}
}

void DeformMeshInstance2D::_bind_methods() {
}

DeformMeshInstance2D::DeformMeshInstance2D() {
}

DeformMeshInstance2D::~DeformMeshInstance2D() {
}

// END


// BEGIN Sprite elastic-deform.

void DeformSprite::_update_simulation() {
	// build new simulation
	if (_sim_id == -1) {
		_sim_id = _sim->make_sim(get_rect(), simulation_geom_segments, simulation_geom_anchor);
	} else {
		_sim->update_sim(_sim_id, get_rect(), simulation_geom_segments, simulation_geom_anchor);
	}
}

void DeformSprite::_update_geom() {
	if (_mesh.is_valid()) {
		if (ArrayMesh *m = Object::cast_to<ArrayMesh>(*_mesh)) {
			MeshDataTool *dataTool = memnew(MeshDataTool);
			dataTool->create_from_surface(_mesh, 0);
			for (int i = 0; i < dataTool->get_vertex_count(); ++i) {
				const Vector3 &vertex = dataTool->get_vertex(i);
				const Vector2 &point = _sim->get_sim_position_at(_sim_id, i);
				dataTool->set_vertex(i, Vector3(point.x, point.y, vertex.z));
			}
			m->surface_remove(0);

			dataTool->commit_to_surface(_mesh);
			memdelete(dataTool);
		}
	}
}

void DeformSprite::_create_geom() {
	const int &segs = simulation_geom_segments;

	const real_t w = get_texture()->get_size().x;
	const real_t h = get_texture()->get_size().y;
	const real_t sh = h / segs;
	const real_t th = 1.0 / segs;

	//             w
	//             |
	//   (0)----(2,5)-------->
	//    |    /   |
	//    |   /    |
	//   (1,3)-----(4)
	//   (6)----(8,11)     -- h-sh*(yy+1)
	//    |    /   |
	//    |   /    |
	// h-(7,9)----(10)     -- h-sh*yy
	//    |
	//   \/
	//
	//  (0)----(2)----(1)---
	//   |    / |      |
	//   |  /   |      |
	//   | /    |      |
	//  (1)----(2)----(1)---
	PoolVector3Array vertices;
    PoolVector2Array textures;
	PoolIntArray indices;
    for (int yy = 0; yy < segs; ++yy) {
        vertices.push_back(Vector3(0, sh*yy, 0));     // 0
        vertices.push_back(Vector3(0, sh*(yy+1), 0)); // 1
        vertices.push_back(Vector3(w, sh*yy, 0));     // 2
        vertices.push_back(Vector3(0, sh*(yy+1), 0)); // 3
        vertices.push_back(Vector3(w, sh*(yy+1), 0)); // 4
        vertices.push_back(Vector3(w, sh*yy, 0));     // 5

        textures.push_back(Vector2(0, th*yy));
        textures.push_back(Vector2(0, th*(yy+1)));
        textures.push_back(Vector2(1, th*yy));
        textures.push_back(Vector2(0, th*(yy+1)));
        textures.push_back(Vector2(1, th*(yy+1)));
        textures.push_back(Vector2(1, th*yy));
	}

	Array array;
	array.resize(VS::ARRAY_MAX);
	array[VS::ARRAY_VERTEX] = vertices;
	array[VS::ARRAY_TEX_UV] = textures;
	array[VS::ARRAY_INDEX] = indices;

	_mesh = Ref<ArrayMesh>(memnew(ArrayMesh));
	_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, array);
}

void DeformSprite::_check_parent_simulator() {
	if (ElasticSimulator2D *sim = Object::cast_to<ElasticSimulator2D>(get_parent())) {
		_sim = sim->get_simulator();
		_sim_shared = true;
	} else if (_sim_shared) {
		_sim = Ref<ElasticSimualtion>(memnew(ElasticSimualtion));
		_sim_shared = false;
	}
	// nothing has changed
}

void DeformSprite::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			set_notify_local_transform(true);
			_check_parent_simulator();
		} break;
		case NOTIFICATION_MOVED_IN_PARENT:
		case NOTIFICATION_ENTER_TREE: {
			_check_parent_simulator();
		} break;
		case NOTIFICATION_DRAW: {
			if (simulation_active) {
				if (_sim_dirty) {
					_update_simulation();
					ERR_FAIL_COND(_sim_id < 0);
					_sim_dirty = false;
				}
				if (simulation_debug) {
					debug_draw();
				}
			} else
				Sprite::_notification(p_what);
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			if (simulation_active && !_sim_shared) {
				const float dt = get_process_delta_time();
				_sim->simulate( (simulation_override_delta && simulation_delta > 0) ? simulation_delta : dt, simulation_force_impulse );
				_geom_dirty = true;
				update();
			}
		} break;
		case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
			Sprite::_notification(p_what);
		} break;
	}
}

void DeformSprite::_get_property_list(List<PropertyInfo> *p_list) const {
	// Hide some properties if we are attached to shared simulator:
	if (p_list) {
		for (List<PropertyInfo>::Element *E = p_list->front(); E; E = E->next()) {
			PropertyInfo &prop = E->get();
			if (prop.name.to_lower().begins_with("simulation_force_impulse")) {
				if (_sim_shared)
					prop.usage &= ~PROPERTY_USAGE_EDITOR;
				else
					prop.usage |= PROPERTY_USAGE_EDITOR;
			}
			if (prop.name.to_lower().begins_with("simulation_delta")) {
				if (simulation_override_delta)
					prop.usage |= PROPERTY_USAGE_EDITOR;
				else
					prop.usage &= ~PROPERTY_USAGE_EDITOR;
			}
		}
	}
}

void DeformSprite::set_simulation_state(bool p_state) {
	if (simulation_active != p_state) {
		simulation_active = p_state;
		set_process_internal(simulation_active);
		_sim_dirty = (_sim_id == -1);
		update();
	}
}

bool DeformSprite::is_simulation_active() const {
	return simulation_active;
}

void DeformSprite::reset_simulation() {
	_sim_dirty = true;
}

void DeformSprite::set_simulation_geom_anchor(ElasticSimualtion::Anchor p_anchor) {

	ERR_FAIL_INDEX((int)p_anchor, 4);
	simulation_geom_anchor = p_anchor;
	_sim_dirty = true;
	update();
}

ElasticSimualtion::Anchor DeformSprite::get_simulation_geom_anchor() const {

	return simulation_geom_anchor;
}

void DeformSprite::set_simulation_geom_segments(int p_segments) {

	if (p_segments > 0) {
		simulation_geom_segments = p_segments;
		_sim_dirty = true;
		update();
	} else
		WARN_PRINT("Invalid segments value.");
}

int DeformSprite::get_simulation_geom_segments() const {

	return simulation_geom_segments;
}

void DeformSprite::set_simulation_force_impulse(const Vector2 &p_force) {

	simulation_force_impulse = p_force;
}

Vector2 DeformSprite::get_simulation_force_impulse() const {

	return simulation_force_impulse;
}

void DeformSprite::set_simulation_override_delta(bool p_state) {
	simulation_override_delta = p_state;
	_change_notify();
}

bool DeformSprite::is_simulation_override_delta() const {
	return simulation_override_delta;
}

void DeformSprite::set_simulation_delta(real_t p_delta) {
	ERR_FAIL_COND(p_delta > 1);
	ERR_FAIL_COND(p_delta < 0);

	simulation_delta = p_delta;
}

real_t DeformSprite::get_simulation_delta() const {
	return simulation_delta;
}

void DeformSprite::set_simulation_pixel_scale(real_t p_scale) {
	ERR_FAIL_COND(p_scale < 1);

	simulation_pixel_scale = p_scale;
	_sim_dirty = true;
	update();
}

real_t DeformSprite::get_simulation_pixel_scale() const {
	return simulation_pixel_scale;
}

void DeformSprite::set_simulation_spring_factor(real_t p_factor) {
	ERR_FAIL_COND(p_factor > 1);
	ERR_FAIL_COND(p_factor < -1);

	simulation_spring_factor = p_factor;
}

real_t DeformSprite::get_simulation_spring_factor() const {
	return simulation_spring_factor;
}

void DeformSprite::set_simulation_debug(bool p_debug) {

	if (simulation_debug != p_debug) {

		simulation_debug = p_debug;
		update();
	}
}

bool DeformSprite::get_simulation_debug() const {

	return simulation_debug;
}

void DeformSprite::debug_draw() {
	const int ccnt = _sim->get_sim_constraint_count(_sim_id);
	for(int i = 0; i < ccnt; i++) {
		const ElasticSimualtion::Constraint &c = _sim->get_sim_constraint_at(_sim_id, i);
		draw_line(c.begin, c.end, Color(1, 1 - Math::abs(c.deviation), 1 - Math::abs(c.deviation), 1));
	}
	const int pcnt = _sim->get_sim_position_count(_sim_id);
	for(int i = 0; i < pcnt; i++) {
		draw_circle(_sim->get_sim_position_at(_sim_id, i), 8, Color::named("yellow"));
	}
	Size2 half = get_rect().size / 2;
	Vector2 force = get_rect().position + half;
	switch(simulation_geom_anchor) {
		case ElasticSimualtion::SIM_ANCHOR_TOP: { force.y += half.y; } break;
		case ElasticSimualtion::SIM_ANCHOR_BOTTOM: { force.y -= half.y; } break;
		case ElasticSimualtion::SIM_ANCHOR_LEFT: { force.x += half.x; } break;
		case ElasticSimualtion::SIM_ANCHOR_RIGHT: { force.x -= half.x; } break;
	}
	draw_line(force, force + simulation_force_impulse, Color::named("green"));
	draw_circle(force + simulation_force_impulse, 8, Color::named("green"));
}

void DeformSprite::_bind_methods() {
	BIND_ENUM_CONSTANT(ElasticSimualtion::SIM_ANCHOR_LEFT);
	BIND_ENUM_CONSTANT(ElasticSimualtion::SIM_ANCHOR_RIGHT);
	BIND_ENUM_CONSTANT(ElasticSimualtion::SIM_ANCHOR_TOP);
	BIND_ENUM_CONSTANT(ElasticSimualtion::SIM_ANCHOR_BOTTOM);

	ClassDB::bind_method(D_METHOD("set_simulation_state", "simulation_active"), &DeformSprite::set_simulation_state);
	ClassDB::bind_method(D_METHOD("is_simulation_active"), &DeformSprite::is_simulation_active);
	ClassDB::bind_method(D_METHOD("set_simulation_geom_segments", "simulation_geom_segments"), &DeformSprite::set_simulation_geom_segments);
	ClassDB::bind_method(D_METHOD("get_simulation_geom_segments"), &DeformSprite::get_simulation_geom_segments);
	ClassDB::bind_method(D_METHOD("set_simulation_geom_anchor", "simulation_grid_anchor"), &DeformSprite::set_simulation_geom_anchor);
	ClassDB::bind_method(D_METHOD("get_simulation_geom_anchor"), &DeformSprite::get_simulation_geom_anchor);
	ClassDB::bind_method(D_METHOD("set_simulation_force_impulse", "simulation_force_impulse"), &DeformSprite::set_simulation_force_impulse);
	ClassDB::bind_method(D_METHOD("get_simulation_force_impulse"), &DeformSprite::get_simulation_force_impulse);
	ClassDB::bind_method(D_METHOD("set_simulation_override_delta", "simulation_override_delta"), &DeformSprite::set_simulation_override_delta);
	ClassDB::bind_method(D_METHOD("is_simulation_override_delta"), &DeformSprite::is_simulation_override_delta);
	ClassDB::bind_method(D_METHOD("set_simulation_delta", "simulation_delta"), &DeformSprite::set_simulation_delta);
	ClassDB::bind_method(D_METHOD("get_simulation_delta"), &DeformSprite::get_simulation_delta);
	ClassDB::bind_method(D_METHOD("set_simulation_spring_factor", "simulation_spring_factor"), &DeformSprite::set_simulation_spring_factor);
	ClassDB::bind_method(D_METHOD("get_simulation_spring_factor"), &DeformSprite::get_simulation_spring_factor);
	ClassDB::bind_method(D_METHOD("set_simulation_pixel_scale", "simulation_pixel_scale"), &DeformSprite::set_simulation_pixel_scale);
	ClassDB::bind_method(D_METHOD("get_simulation_pixel_scale"), &DeformSprite::get_simulation_pixel_scale);
	ClassDB::bind_method(D_METHOD("set_simulation_debug", "simulation_debug"), &DeformSprite::set_simulation_debug);
	ClassDB::bind_method(D_METHOD("get_simulation_debug"), &DeformSprite::get_simulation_debug);
	ClassDB::bind_method(D_METHOD("reset_simulation"), &DeformSprite::reset_simulation);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "simulation_active"), "set_simulation_state", "is_simulation_active");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "simulation_geom_segments", PROPERTY_HINT_RANGE, "1,5,1"), "set_simulation_geom_segments", "get_simulation_geom_segments");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "simulation_grid_anchor", PROPERTY_HINT_ENUM, "Left,Right,Top,Bottom"), "set_simulation_geom_anchor", "get_simulation_geom_anchor");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "simulation_override_delta"), "set_simulation_override_delta", "is_simulation_override_delta");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "simulation_delta", PROPERTY_HINT_RANGE, "0,1,0.05"), "set_simulation_delta", "get_simulation_delta");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "simulation_pixel_scale", PROPERTY_HINT_RANGE, "1,1000,10"), "set_simulation_pixel_scale", "get_simulation_pixel_scale");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "simulation_spring_factor", PROPERTY_HINT_RANGE, "-1,1,0.05"), "set_simulation_spring_factor", "get_simulation_spring_factor");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "simulation_force_impulse"), "set_simulation_force_impulse", "get_simulation_force_impulse");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "simulation_debug"), "set_simulation_debug", "get_simulation_debug");
}

DeformSprite::DeformSprite() {
	_sim = Ref<ElasticSimualtion>(memnew(ElasticSimualtion));
	_sim_shared = false;
	_sim_id = -1;
	_sim_dirty = false;
	_geom_dirty = false;

	simulation_active = false;
	simulation_override_delta = false;
	simulation_delta = 0.2;
	simulation_pixel_scale = 200;
	simulation_spring_factor = 0;
	simulation_geom_segments = 1;
	simulation_geom_anchor = ElasticSimualtion::SIM_ANCHOR_BOTTOM;
	simulation_force_impulse = Vector2();
	simulation_debug = false;

	_disabled_base_notifications.append(NOTIFICATION_DRAW);
}

DeformSprite::~DeformSprite() {
}

// END
