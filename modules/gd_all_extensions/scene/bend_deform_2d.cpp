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

Ref<ElasticSimulation> SimulationController2D::get_simulation() const { return _sim; }

Vector2 SimulationController2D::get_simulation_force_impulse() const { return _simulation_force_impulse; }
real_t SimulationController2D::get_simulation_force_impulse_duration() const { return _simulation_force_impulse_duration; }

void SimulationController2D::_get_property_list(List<PropertyInfo> *p_list) const {

	if (p_list) {
		for (List<PropertyInfo>::Element *E = p_list->front(); E; E = E->next()) {
			PropertyInfo &prop = E->get();
			if (prop.name.to_lower() == "simulation_delta_interval") {
				if (simulation_fixed_delta)
					prop.usage |= PROPERTY_USAGE_EDITOR;
				else
					prop.usage &= ~PROPERTY_USAGE_EDITOR;
			}
		}
	}
}

void SimulationController2D::set_simulation_state(bool p_state) {

	if (simulation_active != p_state) {
		simulation_active = p_state;
		_sim_dirty = p_state;
		emit_signal(simulation_changed);
	}
}

bool SimulationController2D::is_simulation_active() const {

	return simulation_active;
}

void SimulationController2D::set_simulation_fixed_delta(bool p_state) {

	simulation_fixed_delta = p_state;
	emit_signal(simulation_changed);
}

bool SimulationController2D::is_simulation_fixed_delta() const {

	return simulation_fixed_delta;
}

void SimulationController2D::set_simulation_delta(real_t p_delta) {
	ERR_FAIL_COND(p_delta > 1);
	ERR_FAIL_COND(p_delta < 0);

	simulation_delta = p_delta;
}

real_t SimulationController2D::get_simulation_delta() const {

	return simulation_delta;
}

void SimulationController2D::set_simulation_spring_factor(real_t p_factor) {
	ERR_FAIL_COND(p_factor > 1);
	ERR_FAIL_COND(p_factor < 0);

	simulation_spring_factor = p_factor;
	_sim_dirty = true;
}

real_t SimulationController2D::get_simulation_spring_factor() const {

	return simulation_spring_factor;
}

void SimulationController2D::set_simulation_spring_variation(real_t p_factor) {
	ERR_FAIL_COND(p_factor > 1);
	ERR_FAIL_COND(p_factor < 0);

	simulation_spring_variation = p_factor;
	_sim_dirty = true;
}

real_t SimulationController2D::get_simulation_spring_variation() const {

	return simulation_spring_variation;
}

void SimulationController2D::apply_simulation_force_impulse(const Vector2 &p_force, real_t p_duration) {

	_simulation_force_impulse = p_force;
	_simulation_force_impulse_duration = p_duration;
}

void SimulationController2D::simulation_progress(real_t process_delta_time) {
	if (simulation_active) {
		const float dt = (simulation_fixed_delta && simulation_delta > 0) ? simulation_delta : process_delta_time;
		if (_simulation_force_impulse_duration > 0) {
			_sim->simulate( dt, _simulation_force_impulse );
			_simulation_force_impulse_duration -= dt;
		} else {
			_sim->simulate( dt, Vector2(0, 0) );
			emit_signal(simulation_update);
		}
	}
}

void SimulationController2D::reset_simulation() {
	_sim_dirty = true;
}

Node *SimulationController2D::get_owner() const {
	return _sim_owner;
}

void SimulationController2D::set_owner(Node *p_owner) {
	if (_sim_owner && p_owner && _sim_owner != p_owner) {
		ERR_PRINT("This simulation controller is already owned.");
		return;
	}
	if (_sim_owner != p_owner) {
		_sim_owner = p_owner;
	}
}

bool SimulationController2D::has_owner() const {
	return _sim_owner != NULL;
}

void SimulationController2D::_bind_methods() {
	BIND_ENUM_CONSTANT(ElasticSimulation::SIM_ANCHOR_LEFT);
	BIND_ENUM_CONSTANT(ElasticSimulation::SIM_ANCHOR_RIGHT);
	BIND_ENUM_CONSTANT(ElasticSimulation::SIM_ANCHOR_TOP);
	BIND_ENUM_CONSTANT(ElasticSimulation::SIM_ANCHOR_BOTTOM);

	ClassDB::bind_method(D_METHOD("set_simulation_state", "active"), &SimulationController2D::set_simulation_state);
	ClassDB::bind_method(D_METHOD("is_simulation_active"), &SimulationController2D::is_simulation_active);
	ClassDB::bind_method(D_METHOD("set_simulation_fixed_delta", "override"), &SimulationController2D::set_simulation_fixed_delta);
	ClassDB::bind_method(D_METHOD("is_simulation_fixed_delta"), &SimulationController2D::is_simulation_fixed_delta);
	ClassDB::bind_method(D_METHOD("set_simulation_delta", "delta_interval"), &SimulationController2D::set_simulation_delta);
	ClassDB::bind_method(D_METHOD("get_simulation_delta"), &SimulationController2D::get_simulation_delta);
	ClassDB::bind_method(D_METHOD("set_simulation_spring_factor", "spring_factor"), &SimulationController2D::set_simulation_spring_factor);
	ClassDB::bind_method(D_METHOD("get_simulation_spring_factor"), &SimulationController2D::get_simulation_spring_factor);
	ClassDB::bind_method(D_METHOD("set_simulation_spring_variation", "spring_variation"), &SimulationController2D::set_simulation_spring_variation);
	ClassDB::bind_method(D_METHOD("get_simulation_spring_variation"), &SimulationController2D::get_simulation_spring_variation);
	ClassDB::bind_method(D_METHOD("apply_simulation_force_impulse", "force_impulse", "duration"), &SimulationController2D::apply_simulation_force_impulse);
	ClassDB::bind_method(D_METHOD("reset_simulation"), &SimulationController2D::reset_simulation);

	ADD_GROUP("Simulation", "simulation_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "simulation_active"), "set_simulation_state", "is_simulation_active");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "simulation_enable_fixed_delta"), "set_simulation_fixed_delta", "is_simulation_fixed_delta");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "simulation_delta_interval", PROPERTY_HINT_RANGE, "0,1,0.05"), "set_simulation_delta", "get_simulation_delta");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "simulation_spring_factor", PROPERTY_HINT_RANGE, "0,1,0.05"), "set_simulation_spring_factor", "get_simulation_spring_factor");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "simulation_spring_variation", PROPERTY_HINT_RANGE, "0,1,0.05"), "set_simulation_spring_variation", "get_simulation_spring_variation");
	ADD_GROUP("", "");
}

SimulationController2D::SimulationController2D() {
	_sim = Ref<ElasticSimulation>(memnew(ElasticSimulation));
	_sim_dirty = false;
	_sim_owner = NULL;

	simulation_active = false;
	simulation_fixed_delta = false;
	simulation_delta = 0.1;
	simulation_spring_factor = 0.5;
	simulation_spring_variation = 0;

	_simulation_force_impulse = Vector2();
	_simulation_force_impulse_duration = 0;
}

SimulationController2D::~SimulationController2D() {
}


void SimulationControllerInstance2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			set_notify_local_transform(true);
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			if (controller.is_valid()) {
				controller->simulation_progress(get_process_delta_time());
			}
		} break;
		default:
			Node2D::_notification(p_what);
	}
}

Ref<SimulationController2D> SimulationControllerInstance2D::get_controller() const {

	return controller;
}

void SimulationControllerInstance2D::set_controller(const Ref<SimulationController2D> &p_controller) {

	if (controller != p_controller) {
		controller = p_controller;
		emit_signal(controller_changed);
	}
}

void SimulationControllerInstance2D::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_controller"), &SimulationControllerInstance2D::get_controller);
	ClassDB::bind_method(D_METHOD("set_controller", "controller"), &SimulationControllerInstance2D::set_controller);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "controller"), "set_controller", "get_controller");
}

SimulationControllerInstance2D::SimulationControllerInstance2D() {
	controller = Ref<SimulationController2D>(memnew(SimulationController2D));
	controller->set_owner(this);
}

SimulationControllerInstance2D::~SimulationControllerInstance2D() {
	controller->set_owner(NULL);
}

// END


// BEGIN Mesh elastic-deform.

void DeformMeshInstance2D::_configure_controller(SigOperation p_op) {
	ERR_FAIL_COND(controller.is_null());

	// connect signals
	switch (p_op) {
		case SIG_CONNECT: {
			controller->connect(simulation_update, this, "_on_simulation_update");
			controller->connect(simulation_changed, this, "_on_simulation_changed");
		} break;
		case SIG_DISCONNECT: {
			controller->disconnect(simulation_update, this, "_on_simulation_update");
			controller->disconnect(simulation_changed, this, "_on_simulation_changed");
		} break;
	}
}

void DeformMeshInstance2D::_get_property_list(List<PropertyInfo> *p_list) const {

	if (p_list) {
		for (List<PropertyInfo>::Element *E = p_list->front(); E; E = E->next()) {
			PropertyInfo &prop = E->get();
			if (prop.name.to_lower() == "controller") {
				if (SimulationControllerInstance2D *sim = Object::cast_to<SimulationControllerInstance2D>(get_parent()))
					prop.usage &= ~PROPERTY_USAGE_EDITOR;
				else
					prop.usage |= PROPERTY_USAGE_EDITOR;
			}
			if (prop.name.to_lower() == "geometry_deform_force") {
				if (controller.is_valid() && controller->is_simulation_active())
					prop.usage &= ~PROPERTY_USAGE_EDITOR;
				else
					prop.usage |= PROPERTY_USAGE_EDITOR;
			}
		}
	}
}

void DeformMeshInstance2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			set_notify_local_transform(true);
		} break;
		case NOTIFICATION_DRAW: {
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {
			if (controller.is_valid()) {
				controller->simulation_progress(get_process_delta_time());
			}
		} break;
		case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
			MeshInstance2D::_notification(p_what);
		} break;
		default:
			MeshInstance2D::_notification(p_what);
	}
}

Ref<SimulationController2D> DeformMeshInstance2D::get_controller() const {

	return controller;
}

void DeformMeshInstance2D::set_controller(const Ref<SimulationController2D> &p_controller) {

	if(controller != p_controller) {
		_disconnect_controller();
		controller = p_controller;
		if (controller.is_valid()) {
			if (!controller->has_owner())
				controller->set_owner(this);
			_connect_controller();
		}
	}
}

void DeformMeshInstance2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_controller"), &DeformMeshInstance2D::get_controller);
	ClassDB::bind_method(D_METHOD("set_controller", "controller"), &DeformMeshInstance2D::set_controller);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "controller"), "set_controller", "get_controller");
}

DeformMeshInstance2D::DeformMeshInstance2D() {
	controller = Ref<SimulationController2D>(NULL);
}

DeformMeshInstance2D::~DeformMeshInstance2D() {
}

// END


// BEGIN Sprite elastic-deform.

#ifdef TOOLS_ENABLED
bool DeformSprite::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {

	return controller->is_simulation_active() ? _edit_get_rect().has_point(p_point) : Sprite::_edit_is_selected_on_click(p_point, p_tolerance);
}
#endif

void DeformSprite::_update_simulation() {

	ERR_FAIL_COND(controller.is_null());

	Ref<ElasticSimulation> sim = controller->get_simulation();

	ERR_FAIL_COND(sim.is_null());

	const Size2 scaled_rect = get_rect().size * geometry_pixel_unit * get_scale();
	if (_sim_id == -1) {
		_sim_id = sim->make_sim(scaled_rect, geometry_segments, geometry_size_variation, geometry_anchor, controller->get_simulation_spring_factor(), controller->get_simulation_spring_variation());
	} else {
		sim->update_sim(_sim_id, scaled_rect, geometry_segments, geometry_size_variation, geometry_anchor, controller->get_simulation_spring_factor(), controller->get_simulation_spring_variation());
	}
	// activate simulation progress:
	set_process_internal(controller->get_owner() == this && controller->is_simulation_active());
	// request new geometry:
	_mesh = Ref<ArrayMesh>(NULL);
}

void DeformSprite::_update_geom() {
	ERR_FAIL_COND(_sim_id < 0);

	if (_mesh.is_valid()) {
		ERR_FAIL_COND(controller.is_null());

		Ref<ElasticSimulation> sim = controller->get_simulation();
		ERR_FAIL_COND(sim.is_null());

		PoolVector2Array verts = PoolVector2Array(_mesh_array[VS::ARRAY_VERTEX]);
		ERR_FAIL_COND(verts.size() != sim->get_sim_position_count(_sim_id));

		const Point2 origin = get_rect().position;
		for (int i = 0; i < verts.size(); ++i) {
			verts.set(i, origin + sim->get_sim_position_at(_sim_id, i) / geometry_pixel_unit / get_scale());
		}
		_mesh_array[VS::ARRAY_VERTEX] = verts;

		_mesh->clear_mesh();
		_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, _mesh_array, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);
	} else
		_create_geom();
}

void DeformSprite::_create_geom() {
	const int &segs = geometry_segments;

	const Rect2 rc = get_rect();
	const Rect2 tc = _get_texture_uv_rect();

	const bool vert = geometry_anchor == ElasticSimulation::SIM_ANCHOR_BOTTOM || geometry_anchor == ElasticSimulation::SIM_ANCHOR_TOP;
	const bool horiz = geometry_anchor == ElasticSimulation::SIM_ANCHOR_LEFT || geometry_anchor == ElasticSimulation::SIM_ANCHOR_RIGHT;

	const Vector2
		DXv(rc.size.width/(horiz ? segs : 1), 0),
		DYv(0, rc.size.height/(vert ? segs : 1)),
		HXv(rc.size.width/(horiz ? 2 : 1), 0),
		HYv(0, rc.size.height/(vert ? 2 : 1));
	const Vector2
		DXt(tc.size.width/(horiz ? segs : 1), 0),
		DYt(0, tc.size.height/(vert ? segs : 1)),
		HXt(tc.size.width/(horiz ? 2 : 1), 0),
		HYt(0, tc.size.height/(vert ? 2 : 1));

	typedef std::vector<Vector2> Vec2Vector;

	Vec2Vector steps, tsteps;
	Vector2 starting = rc.position, opposite, step, tstarting = tc.position, topposite, tstep;
	if (geometry_size_variation && segs > 1) {
		switch(geometry_anchor) {
			case ElasticSimulation::SIM_ANCHOR_TOP: { step = HYv; opposite = DXv; tstep = -HYt; topposite = DXt; } break;
			case ElasticSimulation::SIM_ANCHOR_BOTTOM: { starting.y += rc.size.height; step = -HYv; opposite = DXv; tstarting.y += tc.size.height; tstep = -HYt; topposite = DXt; } break;
			case ElasticSimulation::SIM_ANCHOR_LEFT: { step = HXv; opposite = DYv; tstep = HXt; topposite = DYt; } break;
			case ElasticSimulation::SIM_ANCHOR_RIGHT: { starting.x += rc.size.width; step = -HXv; opposite = DYv; tstarting.x += tc.size.width; tstep = -HXt; } break;
		}
		for(int s = 0; s < segs - 1; ++s) {
			if (s == segs - 2) {
				// last two segments
				steps.push_back(1.5 * step);
				steps.push_back(0.5 * step);
				tsteps.push_back(1.5 * tstep);
				tsteps.push_back(0.5 * tstep);
			} else {
				steps.push_back(step);
				tsteps.push_back(tstep);
			}
			step /= 2;
			tstep /= 2;
		}
	} else {
		switch(geometry_anchor) {
			case ElasticSimulation::SIM_ANCHOR_TOP: { step = DYv; opposite = DXv; tstep = -DYt; topposite = DXt; } break;
			case ElasticSimulation::SIM_ANCHOR_BOTTOM: { starting.y += rc.size.y; step = -DYv; opposite = DXv; tstarting.y += tc.size.y; tstep = -DYt; topposite = DXt; } break;
			case ElasticSimulation::SIM_ANCHOR_LEFT: { step = DXv; opposite = DYv; tstep = DXt; topposite = DYt; } break;
			case ElasticSimulation::SIM_ANCHOR_RIGHT: { starting.x += rc.size.x; step = -DXv; opposite = DYv; tstarting.x += tc.size.x; tstep = -DXt; } break;
		}
		steps = Vec2Vector(segs, step);
		tsteps = Vec2Vector(segs, tstep);
	}

	//             w
	//             |
	//   (0)----(2,5)-------->
	//    |    /   |
	//    |   /    |
	//   (1,3)-----(4)
	//   (6)----(8,11)     -- h-sh*(s+1)
	//    |    /   |
	//    |   /    |
	// h-(7,9)----(10)     -- h-sh*s
	//    |
	//   \/
	//
	//  (0)----(2)----(4)---
	//   |    / |    / |
	//   |  /   |  /   |
	//   | /    | /    |
	//  (1)----(3)----(5)---
	PoolVector2Array vertices;
	PoolVector2Array textures;
	PoolIntArray indices;

	for (int s = 0; s < segs + 1; ++s) {
		vertices.push_back(starting);
		vertices.push_back(starting + opposite);

		textures.push_back(tstarting);
		textures.push_back(tstarting + topposite);

		if (s < segs) {
			indices.push_back(s * 2);
			indices.push_back(s * 2 + 1);
			indices.push_back(s * 2 + 2);
			indices.push_back(s * 2 + 1);
			indices.push_back(s * 2 + 3);
			indices.push_back(s * 2 + 2);
		}

		starting += steps[s];
		tstarting += tsteps[s];
	}

	_mesh_array.clear();
	_mesh_array.resize(VS::ARRAY_MAX);
	_mesh_array[VS::ARRAY_VERTEX] = vertices;
	_mesh_array[VS::ARRAY_TEX_UV] = textures;
	_mesh_array[VS::ARRAY_INDEX] = indices;

	_mesh = Ref<ArrayMesh>(memnew(ArrayMesh));
	_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, _mesh_array, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);
}

Rect2 DeformSprite::_get_texture_uv_rect() const {

	Rect2 rc(0, 0, 1, 1);
	if (AtlasTexture *atlas = Object::cast_to<AtlasTexture>(*get_texture())) {
		if (atlas->get_atlas().is_valid()) {
			const float tw = atlas->get_atlas()->get_width();
			const float th = atlas->get_atlas()->get_height();
			const Rect2 region = atlas->get_region();
			rc = Rect2(region.position.x / tw, region.position.y / th, region.size.x / tw, region.size.y / th);
		}
	}
	if (is_region()) {
		const float tw = get_texture()->get_width();
		const float th = get_texture()->get_height();
		const Rect2 region = get_region_rect();
		rc = Rect2(region.position.x / tw, region.position.y / th, region.size.x / tw, region.size.y / th);
	}
	return rc;
}

void DeformSprite::_check_parent_simulator() {

	if (SimulationControllerInstance2D *instance = Object::cast_to<SimulationControllerInstance2D>(get_parent())) {
		if (controller != instance->get_controller()) {
			if (controller.is_valid()) {
				_disconnect_controller();
			}
			controller = get_parent();
			if (controller.is_valid()) {
				_connect_controller();
			}
		}
	} else {
		if (controller.is_valid() && !controller->has_owner()) {
			// controller without owner is not usefull - what node is running it ?
			controller = Ref<SimulationController2D>(NULL);
		}
	}
}

void DeformSprite::_configure_controller(SigOperation p_op) {
	ERR_FAIL_COND(controller.is_null());

	// connect signals
	switch (p_op) {
		case SIG_CONNECT: {
			controller->connect(simulation_update, this, "_on_simulation_update");
			controller->connect(simulation_changed, this, "_on_simulation_changed");
		} break;
		case SIG_DISCONNECT: {
			controller->disconnect(simulation_update, this, "_on_simulation_update");
			controller->disconnect(simulation_changed, this, "_on_simulation_changed");
		} break;
	}
}

void DeformSprite::_on_simulation_update() {

	_geom_dirty = true;
	update();
}

void DeformSprite::_on_simulation_changed() {

	_sim_dirty = true;
	update();
}

void DeformSprite::_notification(int p_what) {

	switch (p_what) {
		case NOTIFICATION_READY: {
			set_notify_local_transform(true);
			_check_parent_simulator();
		} break;
		case NOTIFICATION_PARENTED:
		case NOTIFICATION_ENTER_TREE: {
			_check_parent_simulator();
		} break;
		case NOTIFICATION_UNPARENTED: {
		} break;
		case NOTIFICATION_DRAW: {
			if (controller.is_valid()) {
				if (_sim_dirty) {
					_update_simulation();
					_sim_dirty = false;
					_update_geom();
					_geom_dirty = false;
				}
				if (_mesh.is_valid() && get_texture().is_valid()) {
					if (_geom_dirty) {
						_update_geom();
						_geom_dirty = false;
					}
					draw_mesh(_mesh, get_texture(), get_normal_map());
				}
				if (geometry_debug) {
					debug_draw();
				}
			} else
				Sprite::_notification(p_what);
		} break;
		case NOTIFICATION_LOCAL_TRANSFORM_CHANGED: {
			Sprite::_notification(p_what);
		} break;
	}
}

void DeformSprite::_get_property_list(List<PropertyInfo> *p_list) const {

	if (p_list) {
		for (List<PropertyInfo>::Element *E = p_list->front(); E; E = E->next()) {
			PropertyInfo &prop = E->get();
			if (prop.name.to_lower() == "controller") {
				if (SimulationControllerInstance2D *sim = Object::cast_to<SimulationControllerInstance2D>(get_parent()))
					prop.usage &= ~PROPERTY_USAGE_EDITOR;
				else
					prop.usage |= PROPERTY_USAGE_EDITOR;
			}
			if (prop.name.to_lower() == "geometry_deform_force") {
				if (controller.is_valid() && controller->is_simulation_active())
					prop.usage &= ~PROPERTY_USAGE_EDITOR;
				else
					prop.usage |= PROPERTY_USAGE_EDITOR;
			}
		}
	}
}

void DeformSprite::set_geometry_anchor(ElasticSimulation::Anchor p_anchor) {
	ERR_FAIL_INDEX((int)p_anchor, 4);

	geometry_anchor = p_anchor;
	_sim_dirty = true;
	update();
}

ElasticSimulation::Anchor DeformSprite::get_geometry_anchor() const {

	return geometry_anchor;
}

void DeformSprite::set_geometry_segments(int p_segments) {
	ERR_FAIL_COND(p_segments<=0);

	geometry_segments = p_segments;
	_sim_dirty = true;
	update();
}

int DeformSprite::get_geometry_segments() const {

	return geometry_segments;
}

void DeformSprite::set_geometry_pixel_unit(real_t p_unit) {
	ERR_FAIL_COND(p_unit < 0.001);
	ERR_FAIL_COND(p_unit > 1);

	geometry_pixel_unit = p_unit;
	_sim_dirty = true;
	update();
}

real_t DeformSprite::get_geometry_pixel_unit() const {

	return geometry_pixel_unit;
}

void DeformSprite::set_geometry_size_variation(bool p_state) {

	geometry_size_variation = p_state;
	_sim_dirty = true;
	update();
}

bool DeformSprite::is_geometry_size_variation() const {

	return geometry_size_variation;
}

void DeformSprite::set_geometry_debug(bool p_debug) {

	if (geometry_debug != p_debug) {

		geometry_debug = p_debug;
		update();
	}
}

bool DeformSprite::get_geometry_debug() const {

	return geometry_debug;
}

Ref<SimulationController2D> DeformSprite::get_controller() const {

	return controller;
}

void DeformSprite::set_controller(const Ref<SimulationController2D> &p_controller) {

	if(controller != p_controller) {
		_disconnect_controller();
		controller = p_controller;
		if (controller.is_valid()) {
			if (!controller->has_owner())
				controller->set_owner(this);
			_connect_controller();
		}
	}
}

void DeformSprite::debug_draw() {
	ERR_FAIL_COND(controller.is_null());
	Ref<ElasticSimulation> sim = controller->get_simulation();
	ERR_FAIL_COND(sim.is_null());
	const Point2 origin = get_rect().position;
	const int ccnt = sim->get_sim_constraint_count(_sim_id);
	for(int i = 0; i < ccnt; i++) {
		const ElasticSimulation::Constraint &c = sim->get_sim_constraint_at(_sim_id, i);
		draw_line(origin + c.begin / geometry_pixel_unit / get_scale(), origin + c.end / geometry_pixel_unit / get_scale(), Color(1, 1 - Math::abs(c.deviation), 1 - Math::abs(c.deviation), 1));
	}
	const int pcnt = sim->get_sim_position_count(_sim_id);
	for(int i = 0; i < pcnt; i++) {
		draw_rect(Rect2(origin + sim->get_sim_position_at(_sim_id, i) / geometry_pixel_unit / get_scale(), get_scale().inv()), Color::named("yellow"));
	}
	const Size2 half = get_rect().size / 2;
	Vector2 force = get_rect().position + half;
	switch(geometry_anchor) {
		case ElasticSimulation::SIM_ANCHOR_TOP: { force.y += half.y; } break;
		case ElasticSimulation::SIM_ANCHOR_BOTTOM: { force.y -= half.y; } break;
		case ElasticSimulation::SIM_ANCHOR_LEFT: { force.x += half.x; } break;
		case ElasticSimulation::SIM_ANCHOR_RIGHT: { force.x -= half.x; } break;
	}
	const Color force_color = Color(0.5, 1, 0, MIN(1, MAX(0, controller->get_simulation_force_impulse_duration())));
	const real_t force_scale = 2;
	const Vector2 force_end = force + force_scale * controller->get_simulation_force_impulse() / get_scale();
	draw_line(force, force_end, force_color);
	draw_rect(Rect2(force_end, 2.0 * get_scale().inv()), force_color, true);
}

void DeformSprite::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_controller"), &DeformSprite::get_controller);
	ClassDB::bind_method(D_METHOD("set_controller", "controller"), &DeformSprite::set_controller);

	ClassDB::bind_method(D_METHOD("set_geometry_segments", "segments"), &DeformSprite::set_geometry_segments);
	ClassDB::bind_method(D_METHOD("get_geometry_segments"), &DeformSprite::get_geometry_segments);
	ClassDB::bind_method(D_METHOD("set_geometry_size_variation", "size_variation"), &DeformSprite::set_geometry_size_variation);
	ClassDB::bind_method(D_METHOD("is_geometry_size_variation"), &DeformSprite::is_geometry_size_variation);
	ClassDB::bind_method(D_METHOD("set_geometry_anchor", "anchor"), &DeformSprite::set_geometry_anchor);
	ClassDB::bind_method(D_METHOD("get_geometry_anchor"), &DeformSprite::get_geometry_anchor);
	ClassDB::bind_method(D_METHOD("set_geometry_pixel_unit", "pixel_unit"), &DeformSprite::set_geometry_pixel_unit);
	ClassDB::bind_method(D_METHOD("get_geometry_pixel_unit"), &DeformSprite::get_geometry_pixel_unit);
	ClassDB::bind_method(D_METHOD("set_geometry_debug", "debug"), &DeformSprite::set_geometry_debug);
	ClassDB::bind_method(D_METHOD("get_geometry_debug"), &DeformSprite::get_geometry_debug);

	ClassDB::bind_method(D_METHOD("_on_simulation_update"), &DeformSprite::_on_simulation_update);
	ClassDB::bind_method(D_METHOD("_on_simulation_changed"), &DeformSprite::_on_simulation_changed);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "controller"), "set_controller", "get_controller");

	ADD_GROUP("Geometry", "geometry_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "geometry_segments", PROPERTY_HINT_RANGE, "1,5,1"), "set_geometry_segments", "get_geometry_segments");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "geometry_enable_size_variation"), "set_geometry_size_variation", "is_geometry_size_variation");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "geometry_anchor", PROPERTY_HINT_ENUM, "Left,Right,Top,Bottom"), "set_geometry_anchor", "get_geometry_anchor");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "geometry_pixel_unit", PROPERTY_HINT_RANGE, "0,1,0.001,or_greater"), "set_geometry_pixel_unit", "get_geometry_pixel_unit");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "geometry_debug"), "set_geometry_debug", "get_geometry_debug");
	ADD_GROUP("", "");
}

DeformSprite::DeformSprite() {
	controller = Ref<SimulationController2D>(NULL);

	geometry_segments = 1;
	geometry_size_variation = false;
	geometry_anchor = ElasticSimulation::SIM_ANCHOR_BOTTOM;
	geometry_pixel_unit = 0.1;
	geometry_deform_force = Vector2();
	geometry_debug = false;

	_sim_id = -1;
	_sim_dirty = false;
	_geom_dirty = false;

	_disabled_base_notifications.append(NOTIFICATION_DRAW);
}

DeformSprite::~DeformSprite() {
}

// END
