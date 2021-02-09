/*************************************************************************/
/*  bend_deform_2d.cpp                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "scene/resources/mesh.h"
#include "scene/resources/mesh_data_tool.h"

#include "bend_deform_2d.h"

// https://www.reddit.com/r/godot/comments/9y74r6/how_to_detect_when_node2d_is_moveddragged_in_the/

#ifdef DEBUG_ENABLED
#define DEBUG_PRINT(m_text) print_line(m_text);
#else
#define DEBUG_PRINT(m_text)
#endif

static const Vector2 ONE = Vector2(1, 1);

struct MotionTextureIterator : public Reference {
	Point2 position, next_position;
	real_t pk;
	int interpolation;
	MotionTextureIterator(const Point2 &pt, real_t dir) :
			interpolation(0) {
		const real_t dx = Math::cos(dir);
		const real_t dy = Math::sin(dir);
		const real_t slope = Math::tan(dir);
		pk = slope < 1 ? 2 * dy - dx : 2 * dx - dy;
		position = next_position = Point2(Math::round(pt.x), Math::round(pt.y));
	}
	MotionTextureIterator(const Variant &var) {}
	void set_position(const Point2 &pt) { position = pt; }
	Point2 &get_position() { return position; }
	Point2 get_position() const { return position; }
	void set_next_position(const Point2 &pt) { next_position = pt; }
	Point2 &get_next_position() { return next_position; }
	Point2 get_next_position() const { return next_position; }
	real_t &get_pk() { return pk; }
	real_t get_pk() const { return pk; }
	int &get_interpolation() { return interpolation; }
	int get_interpolation() const { return interpolation; }
	MotionTextureIterator *next_step(real_t dir, const Size2 &limit) {
		position = next_position;
		const real_t dx = Math::cos(dir);
		const real_t dy = Math::sin(dir);
		const real_t slope = Math::tan(dir);
		if (slope < 1) {
			++next_position.x;
			if (pk < 0) {
				pk += 2 * dy;
			} else {
				pk += 2 * (dy - dx);
				++next_position.y;
			}
		} else {
			++next_position.y;
			if (pk < 0) {
				pk += 2 * dx;
			} else {
				pk += 2 * (dx - dy);
				++next_position.x;
			}
		}
		next_position = next_position.posmodv(limit);
		return this;
	}
};

// BEGIN Elastic-deform group node.

#define _notify_simulation_change() \
	emit_signal("simulation_changed");

Vector2 SimulationController2D::_get_motion_normalized_displacement(Point2 p_coord) {

	motion_image->lock();
	Color c = motion_image->get_pixel(p_coord.x, p_coord.y);
	motion_image->unlock();

	switch (motion_packing) {
		case PACKING_16BIT: {
			const uint32_t _c = c.to_argb32();
			return Vector2((_c & 0xFFFF0000) >> 16, _c & 0xFFFF) / 32767.5 - ONE;
		}
		case PACKING_8BIT: {
			return Vector2(c.r, c.g) * 2.0 - ONE;
		}
		default:
			WARN_PRINT("Unknown packing format!")
	}

	return Vector2();
}

Ref<Image> SimulationController2D::get_motion_image() const {
	return motion_image;
}

void SimulationController2D::set_motion_image(const Ref<Image> &p_image) {

	if (motion_image != p_image) {
		motion_image = p_image;
		_change_notify();
		emit_signal("motion_image_changed");
	}
}

void SimulationController2D::set_motion_packing(MotionPacking p_packing) {
	ERR_FAIL_COND(p_packing > 1);

	motion_packing = p_packing;
}

SimulationController2D::MotionPacking SimulationController2D::get_motion_packing() const {

	return motion_packing;
}

Vector2 SimulationController2D::get_motion_value(Node *p_node, real_t p_amp, int p_interpolations) {
	ERR_FAIL_COND_V(!p_node->has_meta("__state_motion_iterator"), Vector2());

	const real_t interpolation_period = p_interpolations ? 1.0 / (p_interpolations + 1) : 0.0;
	Ref<MotionTextureIterator> info = p_node->get_meta("__state_motion_iterator");
	Vector2 displace;
	const Vector2 curr = displace = _get_motion_normalized_displacement(info->get_position());
	const real_t interpolation = info->get_interpolation() * interpolation_period;
	if (interpolation > 0) {
		const Vector2 next = _get_motion_normalized_displacement(info->get_next_position());
		ERR_FAIL_COND_V(interpolation > 1, next * p_amp);
		if (curr != next) {
			displace = curr.linear_interpolate(next, interpolation);
		}
	}
	return displace * p_amp;
}

Ref<ElasticSimulation> SimulationController2D::get_simulation() const {
	return _sim;
}

Vector2 SimulationController2D::get_simulation_force_impulse() const {
	return _simulation_force_impulse;
}
real_t SimulationController2D::get_simulation_force_impulse_duration() const {
	return _simulation_force_impulse_duration;
}

void SimulationController2D::set_simulation_state(bool p_state) {

	if (simulation_active != p_state) {
		simulation_active = p_state;
		_notify_simulation_change();
	}
}

bool SimulationController2D::is_simulation_active() const {

	return simulation_active;
}

void SimulationController2D::set_simulation_delta(real_t p_delta) {
	ERR_FAIL_COND(p_delta > 1);
	ERR_FAIL_COND(p_delta < 0);

	if (simulation_delta != p_delta) {
		simulation_delta = p_delta;
		_notify_simulation_change();
	}
}

real_t SimulationController2D::get_simulation_delta() const {

	return simulation_delta;
}

void SimulationController2D::apply_simulation_force_impulse(const Vector2 &p_force, real_t p_duration) {

	_simulation_force_impulse = p_force;
	_simulation_force_impulse_duration = p_duration;
}

void SimulationController2D::apply_deform_force(int sim_id, const Vector2 &p_force) {
	ERR_FAIL_COND(_sim.is_null());

	_sim->deform(sim_id, simulation_delta, p_force);
	emit_signal("simulation_progress");
}

void SimulationController2D::simulation_progress() {

	if (simulation_active) {
		if (_simulation_force_impulse_duration > 0) {
			_sim->simulate(simulation_delta, _simulation_force_impulse);
			_simulation_force_impulse_duration -= simulation_delta;
		} else {
			_sim->simulate(simulation_delta, Vector2(0, 0));
		}
		emit_signal("simulation_progress");
	}
}

void SimulationController2D::reset_simulation() {

	_sim->reset_sim();
	emit_signal("simulation_progress");
}

void SimulationController2D::_bind_methods() {

	BIND_ENUM_CONSTANT(ElasticSimulation::SIM_ANCHOR_LEFT);
	BIND_ENUM_CONSTANT(ElasticSimulation::SIM_ANCHOR_RIGHT);
	BIND_ENUM_CONSTANT(ElasticSimulation::SIM_ANCHOR_TOP);
	BIND_ENUM_CONSTANT(ElasticSimulation::SIM_ANCHOR_BOTTOM);

	BIND_ENUM_CONSTANT(MotionPacking::PACKING_8BIT);
	BIND_ENUM_CONSTANT(MotionPacking::PACKING_16BIT);

	ClassDB::bind_method(D_METHOD("set_motion_image", "image"), &SimulationController2D::set_motion_image);
	ClassDB::bind_method(D_METHOD("get_motion_image"), &SimulationController2D::get_motion_image);
	ClassDB::bind_method(D_METHOD("set_motion_packing", "packing"), &SimulationController2D::set_motion_packing);
	ClassDB::bind_method(D_METHOD("get_motion_packing"), &SimulationController2D::get_motion_packing);
	ClassDB::bind_method(D_METHOD("set_simulation_state", "active"), &SimulationController2D::set_simulation_state);
	ClassDB::bind_method(D_METHOD("is_simulation_active"), &SimulationController2D::is_simulation_active);
	ClassDB::bind_method(D_METHOD("set_simulation_delta", "delta_interval"), &SimulationController2D::set_simulation_delta);
	ClassDB::bind_method(D_METHOD("get_simulation_delta"), &SimulationController2D::get_simulation_delta);
	ClassDB::bind_method(D_METHOD("apply_simulation_force_impulse", "force_impulse", "duration"), &SimulationController2D::apply_simulation_force_impulse);
	ClassDB::bind_method(D_METHOD("reset_simulation"), &SimulationController2D::reset_simulation);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_simulation_state", "is_simulation_active");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "delta_interval", PROPERTY_HINT_RANGE, "0,1,0.05,or_greater"), "set_simulation_delta", "get_simulation_delta");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "motion_image", PROPERTY_HINT_RESOURCE_TYPE, "Image"), "set_motion_image", "get_motion_image");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "packing", PROPERTY_HINT_ENUM, "2 channels,4 channels"), "set_motion_packing", "get_motion_packing");

	ADD_SIGNAL(MethodInfo("motion_image_changed"));
	ADD_SIGNAL(MethodInfo("simulation_changed"));
	ADD_SIGNAL(MethodInfo("simulation_progress"));
}

SimulationController2D::SimulationController2D() {

	_sim = Ref<ElasticSimulation>(memnew(ElasticSimulation));

	motion_packing = PACKING_16BIT;

	simulation_active = false;
	simulation_delta = 0.1;

	_simulation_force_impulse = Vector2();
	_simulation_force_impulse_duration = 0;
}

SimulationController2D::~SimulationController2D() {
}

void SimulationControllerInstance2D::_draw_debug_marker(const Point2 &p0, real_t dir, int marker_length, int head_length, int head_width, const Color &marker_color1, const Color &marker_color2) {

	const Vector2 unit = Vector2(Math::cos(dir), Math::sin(dir));
	const Vector2 p1 = p0 + unit * marker_length;

	draw_line(p0, p1, marker_color2);

	const real_t dx = p1.x - p0.x;
	const real_t dy = p1.y - p0.y;
	const real_t length = Math::sqrt(dx * dx + dy * dy);

	if (head_length < 1 || length < head_length) return;

	// ux,uy is a unit vector parallel to the line.
	const real_t ux = dx / length;
	const real_t uy = dy / length;
	// vx,vy is a unit vector perpendicular to ux,uy
	const real_t vx = -uy;
	const real_t vy = ux;

	const real_t half_width = 0.5f * head_width;

	const Vector2 pt1 = p1 + unit * head_length;
	const Vector2 pt2 = Vector2(pt1.x - head_length * ux + half_width * vx, pt1.y - head_length * uy + half_width * vy);
	const Vector2 pt3 = Vector2(pt1.x - head_length * ux - half_width * vx, pt1.y - head_length * uy - half_width * vy);

	draw_line(pt1, pt2, marker_color2);
	draw_line(pt2, pt3, marker_color2);
	draw_line(pt3, pt1, marker_color2);

	draw_circle(p0, 2, marker_color2);
	draw_circle(p0, 1, marker_color1);
}

void SimulationControllerInstance2D::_on_controller_changed() {
	set_process_internal(controller->is_simulation_active());
	update();
}

void SimulationControllerInstance2D::_notification(int p_what) {

	switch (p_what) {
		case NOTIFICATION_READY: {
			if (controller.is_null()) {
				// create default controller
				set_controller(Ref<SimulationController2D>(memnew(SimulationController2D)));
			}
		} break;
		case NOTIFICATION_DRAW: {
			if (motion_debug) {
				if (controller.is_valid() && controller->get_motion_image().is_valid()) {
					const int cnt = get_child_count();
					if (cnt > 0) {
						if (_motion_texture.is_null()) {
							_motion_texture.instance();
							_motion_texture->create_from_image(controller->get_motion_image());
						}
						draw_texture(_motion_texture, Point2());
						for (int n = 0; n < cnt; ++n) {
							int sim_id = -1;
							real_t trajectory_dir;
							Node *node = get_child(n);
							if (node->has_meta("__state_motion_iterator")) {
								if (DeformSprite *ds = Object::cast_to<DeformSprite>(node)) {
									sim_id = ds->get_simulation_id();
									trajectory_dir = ds->get_motion_direction();
								} else if (DeformMeshInstance2D *dm = Object::cast_to<DeformMeshInstance2D>(node)) {
									sim_id = dm->get_simulation_id();
									trajectory_dir = dm->get_motion_direction();
								}
								if (sim_id >= 0) {
									const MotionTextureIterator *info = (MotionTextureIterator *)(Object *)node->get_meta("__state_motion_iterator");
									_draw_debug_marker(info->get_position(), trajectory_dir, 10, 3, 5);
								}
							}
						}
					}
				}
			}
		}
		case NOTIFICATION_INTERNAL_PROCESS: {
			if (controller.is_valid()) {
				if (controller->get_motion_image().is_valid()) {
					for (int n = 0; n < get_child_count(); ++n) {
						int sim_id = -1;
						Vector2 trajectory_origin;
						real_t trajectory_dir;
						real_t amplify;
						int interpolations;
						Node *node = get_child(n);
						if (DeformSprite *ds = Object::cast_to<DeformSprite>(node)) {
							sim_id = ds->get_simulation_id();
							trajectory_dir = ds->get_motion_direction();
							trajectory_origin = ds->get_motion_trajectory_origin();
							amplify = ds->get_motion_amplify();
							interpolations = ds->get_motion_interpolations();
						} else if (DeformMeshInstance2D *dm = Object::cast_to<DeformMeshInstance2D>(node)) {
							sim_id = dm->get_simulation_id();
							trajectory_dir = dm->get_motion_direction();
							trajectory_origin = dm->get_motion_trajectory_origin();
							amplify = dm->get_motion_amplify();
							interpolations = dm->get_motion_interpolations();
						}
						if (sim_id >= 0) {
							const Size2 image_size = controller->get_motion_image()->get_size();

							Ref<MotionTextureIterator> it = node->has_meta("__state_motion_iterator") ? (Ref<MotionTextureIterator>)node->get_meta("__state_motion_iterator") : Ref<MotionTextureIterator>(memnew(MotionTextureIterator(trajectory_origin * image_size, trajectory_dir))->next_step(trajectory_dir, image_size));

							ERR_FAIL_COND(it.is_null());

							// save new iterator
							node->set_meta("__state_motion_iterator", it);

							// directly modify iterator
							int &interpolation_step = it->get_interpolation();

							controller->apply_deform_force(sim_id, controller->get_motion_value(node, amplify, interpolations));

							if (interpolation_step == interpolations) {
								interpolation_step = 0;
								it->next_step(trajectory_dir, image_size);
							} else
								++interpolation_step;
						}
					}
					if (motion_debug)
						update();
				} else {
					controller->simulation_progress();
				}
			}
		} break;
	}
}

Ref<SimulationController2D> SimulationControllerInstance2D::get_controller() const {

	return controller;
}

void SimulationControllerInstance2D::set_controller(const Ref<SimulationController2D> &p_controller) {

	if (controller != p_controller) {
		if (controller.is_valid())
			controller->disconnect("simulation_changed", this, "_on_controller_changed");
		controller = p_controller;
		if (controller.is_valid())
			controller->connect("simulation_changed", this, "_on_controller_changed");
		set_process_internal(controller.is_valid() && controller->is_simulation_active());
	}
}

bool SimulationControllerInstance2D::get_motion_debug() const {

	return motion_debug;
}

void SimulationControllerInstance2D::set_motion_debug(const bool &p_debug) {

	if (motion_debug != p_debug) {
		motion_debug = p_debug;
		update();
	}
}

void SimulationControllerInstance2D::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_controller"), &SimulationControllerInstance2D::get_controller);
	ClassDB::bind_method(D_METHOD("set_controller"), &SimulationControllerInstance2D::set_controller);
	ClassDB::bind_method(D_METHOD("get_motion_debug"), &SimulationControllerInstance2D::get_motion_debug);
	ClassDB::bind_method(D_METHOD("set_motion_debug"), &SimulationControllerInstance2D::set_motion_debug);

	ClassDB::bind_method(D_METHOD("_on_controller_changed"), &SimulationControllerInstance2D::_on_controller_changed);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "controller"), "set_controller", "get_controller");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "motion_debug"), "set_motion_debug", "get_motion_debug");
}

SimulationControllerInstance2D::SimulationControllerInstance2D() {

	motion_debug = false;
}

SimulationControllerInstance2D::~SimulationControllerInstance2D() {
}

// END

// BEGIN Mesh elastic-deform.

void DeformMeshInstance2D::_get_property_list(List<PropertyInfo> *p_list) const {

	if (p_list) {
		for (List<PropertyInfo>::Element *E = p_list->front(); E; E = E->next()) {
			PropertyInfo &prop = E->get();
			if (prop.name.to_lower() == "controller") {
				if (Object::cast_to<SimulationControllerInstance2D>(get_parent()))
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
				controller->simulation_progress();
			}
		} break;
	}
}

int DeformMeshInstance2D::get_simulation_id() const {
	return _sim_id;
}

Ref<SimulationController2D> DeformMeshInstance2D::get_controller() const {

	return controller;
}

void DeformMeshInstance2D::set_controller(const Ref<SimulationController2D> &p_controller) {

	if (controller != p_controller) {
		if (controller.is_valid()) {
			controller->disconnect("simulation_progress", this, "_on_simulation_update");
			controller->disconnect("simulation_changed", this, "_on_simulation_changed");
		}
		controller = p_controller;
		if (controller.is_valid()) {
			controller->connect("simulation_progress", this, "_on_simulation_update");
			controller->connect("simulation_changed", this, "_on_simulation_changed");
		}
	}
}

int DeformMeshInstance2D::get_motion_interpolations() const {

	return motion_interpolations;
}

void DeformMeshInstance2D::set_motion_interpolations(int p_steps) {

	if (motion_interpolations != p_steps) {
		motion_interpolations = p_steps;
	}
}

real_t DeformMeshInstance2D::get_motion_direction() const {

	return motion_trajectory_direction;
}

void DeformMeshInstance2D::set_motion_direction(real_t p_angle) {

	if (motion_trajectory_direction != p_angle) {
		motion_trajectory_direction = p_angle;
	}
}

real_t DeformMeshInstance2D::get_motion_direction_degree() const {

	return Math::rad2deg(get_motion_direction());
}

void DeformMeshInstance2D::set_motion_direction_degree(real_t p_degree) {

	set_motion_direction(Math::deg2rad(p_degree));
}

Vector2 DeformMeshInstance2D::get_motion_trajectory_origin() const {

	return motion_trajectory_origin;
}

void DeformMeshInstance2D::set_motion_trajectory_origin(Vector2 p_origin) {

	if (motion_trajectory_origin != p_origin) {
		motion_trajectory_origin = p_origin;
	}
}

real_t DeformMeshInstance2D::get_motion_amplify() const {

	return motion_amplify;
}

void DeformMeshInstance2D::set_motion_amplify(real_t p_amp) {

	if (motion_amplify != p_amp) {
		motion_amplify = p_amp;
	}
}

void DeformMeshInstance2D::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_controller"), &DeformMeshInstance2D::get_controller);
	ClassDB::bind_method(D_METHOD("set_controller", "controller"), &DeformMeshInstance2D::set_controller);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "controller"), "set_controller", "get_controller");
}

DeformMeshInstance2D::DeformMeshInstance2D() {

	controller = Ref<SimulationController2D>(NULL);

	motion_trajectory_direction = 0;
	motion_trajectory_origin = Vector2();
	motion_amplify = 2.0;
	motion_interpolations = 20;
}

DeformMeshInstance2D::~DeformMeshInstance2D() {

	// TODO: Dereference __state_motion_iterator (?)
}

// END

// BEGIN Sprite elastic-deform.

#ifdef TOOLS_ENABLED
bool DeformSprite::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {

	return controller.is_valid() ? _edit_get_rect().has_point(p_point) : Sprite::_edit_is_selected_on_click(p_point, p_tolerance);
}
#endif

void DeformSprite::_update_simulation() {

	ERR_FAIL_COND(controller.is_null());

	Ref<ElasticSimulation> sim = controller->get_simulation();

	ERR_FAIL_COND(sim.is_null());

	const Size2 scaled_rect = get_rect().size * geometry_pixel_unit * get_scale();
	if (_sim_id == -1) {
		_sim_id = sim->make_sim(scaled_rect, geometry_segments, geometry_size_variation, geometry_anchor, geometry_spring_factor, geometry_spring_variation);
	} else {
		sim->update_sim(_sim_id, scaled_rect, geometry_segments, geometry_size_variation, geometry_anchor, geometry_spring_factor, geometry_spring_variation);
	}
	// activate simulation progress:
	set_process_internal(!_is_parent_controller() && controller->is_simulation_active());
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
			DXv(rc.size.width / (horiz ? segs : 1), 0),
			DYv(0, rc.size.height / (vert ? segs : 1)),
			HXv(rc.size.width / (horiz ? 2 : 1), 0),
			HYv(0, rc.size.height / (vert ? 2 : 1));
	const Vector2
			DXt(tc.size.width / (horiz ? segs : 1), 0),
			DYt(0, tc.size.height / (vert ? segs : 1)),
			HXt(tc.size.width / (horiz ? 2 : 1), 0),
			HYt(0, tc.size.height / (vert ? 2 : 1));

	typedef std::vector<Vector2> Vec2Vector;

	Vec2Vector steps, tsteps;
	Vector2 starting = rc.position, opposite, step, tstarting = tc.position, topposite, tstep;
	if (geometry_size_variation && segs > 1) {
		switch (geometry_anchor) {
			case ElasticSimulation::SIM_ANCHOR_TOP: {
				step = HYv;
				opposite = DXv;
				tstep = -HYt;
				topposite = DXt;
			} break;
			case ElasticSimulation::SIM_ANCHOR_BOTTOM: {
				starting.y += rc.size.height;
				step = -HYv;
				opposite = DXv;
				tstarting.y += tc.size.height;
				tstep = -HYt;
				topposite = DXt;
			} break;
			case ElasticSimulation::SIM_ANCHOR_LEFT: {
				step = HXv;
				opposite = DYv;
				tstep = HXt;
				topposite = DYt;
			} break;
			case ElasticSimulation::SIM_ANCHOR_RIGHT: {
				starting.x += rc.size.width;
				step = -HXv;
				opposite = DYv;
				tstarting.x += tc.size.width;
				tstep = -HXt;
			} break;
			default: {
				ERR_FAIL_MSG("Invalid anchor value.");
			}
		}
		for (int s = 0; s < segs - 1; ++s) {
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
		switch (geometry_anchor) {
			case ElasticSimulation::SIM_ANCHOR_TOP: {
				step = DYv;
				opposite = DXv;
				tstep = -DYt;
				topposite = DXt;
			} break;
			case ElasticSimulation::SIM_ANCHOR_BOTTOM: {
				starting.y += rc.size.y;
				step = -DYv;
				opposite = DXv;
				tstarting.y += tc.size.y;
				tstep = -DYt;
				topposite = DXt;
			} break;
			case ElasticSimulation::SIM_ANCHOR_LEFT: {
				step = DXv;
				opposite = DYv;
				tstep = DXt;
				topposite = DYt;
			} break;
			case ElasticSimulation::SIM_ANCHOR_RIGHT: {
				starting.x += rc.size.x;
				step = -DXv;
				opposite = DYv;
				tstarting.x += tc.size.x;
				tstep = -DXt;
			} break;
			default: {
				ERR_FAIL_MSG("Invalid anchor value.");
			}
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

bool DeformSprite::_is_parent_controller() const {

	if (SimulationControllerInstance2D *instance = Object::cast_to<SimulationControllerInstance2D>(get_parent())) {
		return controller != instance->get_controller();
	}
	return false;
}

void DeformSprite::_check_parent_controller() {

	if (SimulationControllerInstance2D *instance = Object::cast_to<SimulationControllerInstance2D>(get_parent())) {
		set_controller(instance->get_controller());
		DEBUG_PRINT("Parent controller connected to the node");
	} else {
		controller = Ref<SimulationController2D>(NULL);
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
			_check_parent_controller();
			connect("texture_changed", this, "_on_texture_changed");
		} break;
		case NOTIFICATION_PARENTED:
		case NOTIFICATION_ENTER_TREE: {
			_check_parent_controller();
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
					debug_draw_geometry();
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
				if (Object::cast_to<SimulationControllerInstance2D>(get_parent()))
					prop.usage &= ~PROPERTY_USAGE_EDITOR;
				else
					prop.usage |= PROPERTY_USAGE_EDITOR;
			}
		}
	}
}

void DeformSprite::_on_texture_changed() {

	// Rebuild simulation
	if (get_texture().is_valid()) {
		if (Ref<ElasticSimulation> sim = controller->get_simulation())
			if (_sim_id >= 0)
				sim->remove_sim(_sim_id);
		_sim_id = -1;
		update();
	}
}

int DeformSprite::get_simulation_id() const {
	return _sim_id;
}

void DeformSprite::set_geometry_anchor(ElasticSimulation::Anchor p_anchor) {
	ERR_FAIL_INDEX(p_anchor, ElasticSimulation::SIM_ANCHOR_COUNT);

	geometry_anchor = p_anchor;
	_sim_dirty = true;
	update();
}

ElasticSimulation::Anchor DeformSprite::get_geometry_anchor() const {

	return geometry_anchor;
}

void DeformSprite::set_geometry_segments(int p_segments) {
	ERR_FAIL_COND(p_segments <= 0);

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

void DeformSprite::set_geometry_spring_factor(real_t p_factor) {
	ERR_FAIL_COND(p_factor > 1);
	ERR_FAIL_COND(p_factor < 0);

	if (geometry_spring_factor != p_factor) {
		geometry_spring_factor = p_factor;
		_sim_dirty = true;
		update();
	}
}

real_t DeformSprite::get_geometry_spring_factor() const {

	return geometry_spring_factor;
}

void DeformSprite::set_geometry_spring_variation(real_t p_factor) {
	ERR_FAIL_COND(p_factor > 1);
	ERR_FAIL_COND(p_factor < 0);

	if (geometry_spring_variation != p_factor) {
		geometry_spring_variation = p_factor;
		_sim_dirty = true;
		update();
	}
}

real_t DeformSprite::get_geometry_spring_variation() const {

	return geometry_spring_variation;
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

int DeformSprite::get_motion_interpolations() const {

	return motion_interpolations;
}

void DeformSprite::set_motion_interpolations(int p_steps) {

	if (motion_interpolations != p_steps) {
		motion_interpolations = p_steps;
	}
}

real_t DeformSprite::get_motion_direction() const {

	return motion_trajectory_direction;
}

void DeformSprite::set_motion_direction(real_t p_angle) {

	const real_t angle360 = 2 * Math_PI;
	const real_t angle90 = Math_PI;

	// only supporting <0, 90> range
	p_angle = Math::fmod(Math::fmod(p_angle, angle360) + angle360, angle90);

	if (motion_trajectory_direction != p_angle) {
		motion_trajectory_direction = p_angle;
	}
}

real_t DeformSprite::get_motion_direction_degree() const {

	return Math::rad2deg(get_motion_direction());
}

void DeformSprite::set_motion_direction_degree(real_t p_degree) {

	set_motion_direction(Math::deg2rad(p_degree));
}

Vector2 DeformSprite::get_motion_trajectory_origin() const {

	return motion_trajectory_origin;
}

void DeformSprite::set_motion_trajectory_origin(Vector2 p_origin) {
	ERR_FAIL_COND(p_origin > ONE);

	if (motion_trajectory_origin != p_origin) {
		motion_trajectory_origin = p_origin;
	}
}

real_t DeformSprite::get_motion_amplify() const {

	return motion_amplify;
}

void DeformSprite::set_motion_amplify(real_t p_amp) {
	ERR_FAIL_COND(p_amp <= 0);
	ERR_FAIL_COND(p_amp > 10);

	if (motion_amplify != p_amp) {
		motion_amplify = p_amp;
	}
}

Ref<SimulationController2D> DeformSprite::get_controller() const {

	return controller;
}

void DeformSprite::set_controller(const Ref<SimulationController2D> &p_controller) {

	if (controller != p_controller) {
		if (controller.is_valid()) {
			controller->disconnect("simulation_progress", this, "_on_simulation_update");
			controller->disconnect("simulation_changed", this, "_on_simulation_changed");
			if (Ref<ElasticSimulation> sim = controller->get_simulation()) {
				sim->remove_sim(_sim_id);
			}
		}
		controller = p_controller;
		if (controller.is_valid()) {
			controller->connect("simulation_progress", this, "_on_simulation_update");
			controller->connect("simulation_changed", this, "_on_simulation_changed");
		}
		_sim_dirty = true;
		update();
	}
}

void DeformSprite::deform_geometry(const Vector2 &force) {
	ERR_FAIL_COND(controller.is_null());
	ERR_FAIL_COND(_sim_id == -1);

	controller->apply_deform_force(_sim_id, force);

	_deform_force = force;
	update();
}

void DeformSprite::debug_draw_geometry() {
	ERR_FAIL_COND(controller.is_null());

	Ref<ElasticSimulation> sim = controller->get_simulation();
	ERR_FAIL_COND(sim.is_null());

	const Point2 origin = get_rect().position;
	const int ccnt = sim->get_sim_constraint_count(_sim_id);
	for (int i = 0; i < ccnt; i++) {
		const ElasticSimulation::Constraint &c = sim->get_sim_constraint_at(_sim_id, i);
		draw_line(origin + c.begin / geometry_pixel_unit / get_scale(), origin + c.end / geometry_pixel_unit / get_scale(), Color(1, 1 - Math::abs(c.deviation), 1 - Math::abs(c.deviation), 1));
	}
	const int pcnt = sim->get_sim_position_count(_sim_id);
	for (int i = 0; i < pcnt; i++) {
		draw_rect(Rect2(origin + sim->get_sim_position_at(_sim_id, i) / geometry_pixel_unit / get_scale(), get_scale().inv()), Color::named("yellow"));
	}
	const Size2 half = get_rect().size / 2;
	Vector2 force = get_rect().position + half;
	switch (geometry_anchor) {
		case ElasticSimulation::SIM_ANCHOR_TOP: {
			force.y += half.y;
		} break;
		case ElasticSimulation::SIM_ANCHOR_BOTTOM: {
			force.y -= half.y;
		} break;
		case ElasticSimulation::SIM_ANCHOR_LEFT: {
			force.x += half.x;
		} break;
		case ElasticSimulation::SIM_ANCHOR_RIGHT: {
			force.x -= half.x;
		} break;
		default: {
			ERR_FAIL_MSG("Invalid anchor value.");
		}
	}
	const real_t force_scale = 2;
	if (controller->is_simulation_active() && controller->get_motion_image().is_null()) {
		const Color force_color = Color(0.5, 1, 0, CLAMP(controller->get_simulation_force_impulse_duration(), 0, 1));
		const Vector2 force_end = force + force_scale * controller->get_simulation_force_impulse() / get_scale();
		draw_line(force, force_end, force_color);
		draw_rect(Rect2(force_end, 2.0 * get_scale().inv()), force_color, true);
	} else {
		const Color force_color = Color(0.5, 1, 0, 0.8);
		const Vector2 force_end = force + force_scale * (controller->get_motion_image().is_valid() ? controller->get_motion_value(this, motion_amplify, motion_interpolations) : _deform_force) / get_scale();
		draw_line(force, force_end, force_color);
		draw_rect(Rect2(force_end, 2.0 * get_scale().inv()), force_color, true);
	}
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
	ClassDB::bind_method(D_METHOD("set_geometry_spring_factor", "spring_factor"), &DeformSprite::set_geometry_spring_factor);
	ClassDB::bind_method(D_METHOD("get_geometry_spring_factor"), &DeformSprite::get_geometry_spring_factor);
	ClassDB::bind_method(D_METHOD("set_geometry_spring_variation", "spring_variation"), &DeformSprite::set_geometry_spring_variation);
	ClassDB::bind_method(D_METHOD("get_geometry_spring_variation"), &DeformSprite::get_geometry_spring_variation);
	ClassDB::bind_method(D_METHOD("set_geometry_debug", "debug"), &DeformSprite::set_geometry_debug);
	ClassDB::bind_method(D_METHOD("get_geometry_debug"), &DeformSprite::get_geometry_debug);

	ClassDB::bind_method(D_METHOD("set_motion_amplify"), &DeformSprite::set_motion_amplify);
	ClassDB::bind_method(D_METHOD("get_motion_amplify"), &DeformSprite::get_motion_amplify);
	ClassDB::bind_method(D_METHOD("set_motion_direction"), &DeformSprite::set_motion_direction);
	ClassDB::bind_method(D_METHOD("get_motion_direction"), &DeformSprite::get_motion_direction);
	ClassDB::bind_method(D_METHOD("set_motion_direction_degree"), &DeformSprite::set_motion_direction_degree);
	ClassDB::bind_method(D_METHOD("get_motion_direction_degree"), &DeformSprite::get_motion_direction_degree);
	ClassDB::bind_method(D_METHOD("set_motion_trajectory_origin"), &DeformSprite::set_motion_trajectory_origin);
	ClassDB::bind_method(D_METHOD("get_motion_trajectory_origin"), &DeformSprite::get_motion_trajectory_origin);
	ClassDB::bind_method(D_METHOD("set_motion_interpolations", "steps"), &DeformSprite::set_motion_interpolations);
	ClassDB::bind_method(D_METHOD("get_motion_interpolations"), &DeformSprite::get_motion_interpolations);

	ClassDB::bind_method(D_METHOD("_on_simulation_update"), &DeformSprite::_on_simulation_update);
	ClassDB::bind_method(D_METHOD("_on_simulation_changed"), &DeformSprite::_on_simulation_changed);

	ClassDB::bind_method(D_METHOD("deform_geometry", "force"), &DeformSprite::deform_geometry);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "controller"), "set_controller", "get_controller");

	ADD_GROUP("Geometry", "geometry_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "geometry_segments", PROPERTY_HINT_RANGE, "1,5,1"), "set_geometry_segments", "get_geometry_segments");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "geometry_enable_size_variation"), "set_geometry_size_variation", "is_geometry_size_variation");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "geometry_anchor", PROPERTY_HINT_ENUM, "Left,Right,Top,Bottom"), "set_geometry_anchor", "get_geometry_anchor");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "geometry_pixel_unit", PROPERTY_HINT_RANGE, "0,1,0.001,or_greater"), "set_geometry_pixel_unit", "get_geometry_pixel_unit");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "geometry_spring_factor", PROPERTY_HINT_RANGE, "0,1,0.05"), "set_geometry_spring_factor", "get_geometry_spring_factor");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "geometry_spring_variation", PROPERTY_HINT_RANGE, "0,1,0.05"), "set_geometry_spring_variation", "get_geometry_spring_variation");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "geometry_debug"), "set_geometry_debug", "get_geometry_debug");

	ADD_GROUP("Motion Image", "motion_");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "motion_amplify", PROPERTY_HINT_RANGE, "0,10,0.5,or_greater"), "set_motion_amplify", "get_motion_amplify");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "motion_direction", PROPERTY_HINT_RANGE, "-360,360,0.1,or_lesser,or_greater"), "set_motion_direction_degree", "get_motion_direction_degree");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "motion_origin"), "set_motion_trajectory_origin", "get_motion_trajectory_origin");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "motion_interpolations"), "set_motion_interpolations", "get_motion_interpolations");
}

DeformSprite::DeformSprite() {
	controller = Ref<SimulationController2D>(NULL);

	motion_trajectory_direction = Math::deg2rad(30.0);
	motion_trajectory_origin = Vector2();
	motion_amplify = 2.0;
	motion_interpolations = 20;

	geometry_segments = 1;
	geometry_size_variation = false;
	geometry_anchor = ElasticSimulation::SIM_ANCHOR_BOTTOM;
	geometry_pixel_unit = 0.1;
	geometry_spring_factor = 0.5;
	geometry_spring_variation = 0;
	geometry_debug = false;

	_sim_id = -1;
	_sim_dirty = false;
	_geom_dirty = false;

	_deform_force = Vector2();

	_disabled_base_notifications.append(NOTIFICATION_DRAW);
}

DeformSprite::~DeformSprite() {

	// TODO: Dereference __state_motion_iterator (?)
}

// END
