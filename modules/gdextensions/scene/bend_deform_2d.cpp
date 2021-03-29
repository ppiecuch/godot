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
#include <map>

#include "scene/main/viewport.h"
#include "scene/resources/mesh.h"
#include "scene/resources/mesh_data_tool.h"

#include "common/gd_core.h"
#include "common/gd_math.h"

#include "bend_deform_2d.h"

// Reference:
// ----------
// https://www.reddit.com/r/godot/comments/9y74r6/how_to_detect_when_node2d_is_moveddragged_in_the

// BEGIN Elastic-deform group node.

#define _notify_simulation_change() \
	emit_signal("simulation_changed");

static bool _get_node_noise_modulation_value(Object *node, Ref<OpenSimplexNoise> &noise, real_t progress, int resolution, Vector2 &force) {
	if (node->has_meta("__simulation_id")) {
		const int sim_id = node->get_meta("__simulation_id");
		if (sim_id >= 0) {
			if (Node2D *node2d = Object::cast_to<Node2D>(node)) {
				const Vector2 noise_scale = node->get_meta("__noise_modulation_scale");
				const Vector2i pos = node2d->get_global_position();
				force = noise_scale * Vector2::from_rotation(Math::map(noise->get_noise_3d(pos.x / resolution, pos.y / resolution, progress), -1, 1, 0, Math_Two_PI));
				return true;
			}
		}
	}
	return false;
}

static bool _add_node_noise_modulation_value(Object *node, Ref<OpenSimplexNoise> &noise, real_t progress, int resolution, std::map<simid_t, Vector2> &forces) {
	if (node->has_meta("__simulation_id")) {
		const int sim_id = node->get_meta("__simulation_id");
		if (sim_id >= 0) {
			if (Node2D *node2d = Object::cast_to<Node2D>(node)) {
				const Vector2 noise_scale = node2d->get_meta("__noise_modulation_scale");
				const Vector2i pos = node2d->get_global_position();
				const Vector2 force = Vector2::from_rotation(Math::map(noise->get_noise_3d(pos.x / resolution, pos.y / resolution, progress), -1, 1, 0, Math_Two_PI));
				forces[sim_id] = noise_scale * force;
				return true;
			}
		}
	}
	return false;
}

static void _draw_debug_marker(CanvasItem *node, const Point2 &p0, real_t dir, int marker_length, int head_length, int head_width, const Color &marker_color = Color::named("white")) {

	const Vector2 unit = Vector2(Math::cos(dir), Math::sin(dir));
	const Vector2 p1 = p0 + unit * marker_length;

	node->draw_line(p0, p1, marker_color);

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

	const real_t half_width = 0.5 * head_width;

	const Vector2 pt1 = p1 + unit * head_length;
	const Vector2 pt2 = Vector2(pt1.x - head_length * ux + half_width * vx, pt1.y - head_length * uy + half_width * vy);
	const Vector2 pt3 = Vector2(pt1.x - head_length * ux - half_width * vx, pt1.y - head_length * uy - half_width * vy);

	node->draw_line(pt1, pt2, marker_color);
	node->draw_line(pt2, pt3, marker_color);
	node->draw_line(pt3, pt1, marker_color);

	// node->draw_circle(p0, 1, marker_color);
}


Vector<Object*> SimulationController2D::_get_connected_nodes() const {

	// get the list of listeners of "simulation_progress" signal

	List<Connection> connections;
	get_signal_connection_list("simulation_progress", &connections);

	Vector<Object*> targets;
	for (const List<Connection>::Element *E = connections.front(); E; E = E->next()) {
		targets.push_back(E->get().target);
	}
	return targets;
}

Ref<ElasticSimulation> SimulationController2D::get_simulation() const {

	return _sim;
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

void SimulationController2D::set_simulation_precision(SimulationPrecision p_precision) {
	ERR_FAIL_INDEX(p_precision, SimulationPrecisionCount);

	simulation_precision = p_precision;
}

SimulationController2D::SimulationPrecision SimulationController2D::get_simulation_precision() const {

	return simulation_precision;
}

void SimulationController2D::set_simulation_force(const Vector2 &p_force) {

	simulation_force = p_force;
}

Vector2 SimulationController2D::get_simulation_force() const {

	return simulation_force;
}

#ifdef MODULE_OPENSIMPLEX_ENABLED
void SimulationController2D::set_noise_modulation(bool p_state) {

	noise_modulation = p_state;
}


bool SimulationController2D::is_noise_modulation_active() const {

	return noise_modulation;
}

int SimulationController2D::get_noise_time_scale() const {

	return noise_time_scale;
}

void SimulationController2D::set_noise_time_scale(int p_scale) {

	noise_time_scale = p_scale;
}

void SimulationController2D::set_noise_pixel_resolution(int p_res) {

	noise_pixel_resolution = p_res;
}

int SimulationController2D::get_noise_pixel_resolution() const {

	return noise_pixel_resolution;
}

real_t SimulationController2D::get_current_noise_modulation_dir(const Vector2 &pos) const {

#ifdef MODULE_OPENSIMPLEX_ENABLED
	return Math::map(_noise->get_noise_3d(pos.x / noise_pixel_resolution, pos.y / noise_pixel_resolution, _time_progress * noise_time_scale), -1, 1, 0, Math_Two_PI);
#else
	return 0;
#endif
}
#endif

void SimulationController2D::simulation_progress(real_t p_delta) {

	if (simulation_active) {
		if (noise_modulation) {
			std::map<simid_t, Vector2> forces;
			Vector<Object*> objects = _get_connected_nodes();
			for (int n = 0; n < objects.size(); ++n) {
				_add_node_noise_modulation_value(objects[n], _noise, _time_progress * noise_time_scale, noise_pixel_resolution, forces);
			}
			if (!forces.empty()) {
				for (auto &f : forces)
					f.second *= simulation_force;
				_sim->simulate(_simulation_delta[simulation_precision], forces);
			}
		} else {
			_sim->simulate_all(_simulation_delta[simulation_precision], simulation_force);
		}
		_time_progress += p_delta;
		emit_signal("simulation_progress");
	}
}

void SimulationController2D::reset_simulation() {

	_sim->reset_sim();
	emit_signal("simulation_progress");
}

Vector2 SimulationController2D::get_simulation_force_for_node(Node *p_node) {

#ifdef MODULE_OPENSIMPLEX_ENABLED
	if (noise_modulation) {
		Vector2 node_force;
		if (_get_node_noise_modulation_value(p_node, _noise, _time_progress * noise_time_scale, noise_pixel_resolution, node_force)) {
			return node_force * simulation_force;
		}
	}
#endif
	return simulation_force;
}

void SimulationController2D::_bind_methods() {

	BIND_ENUM_CONSTANT(PRECISION_LOW);
	BIND_ENUM_CONSTANT(PRECISION_MEDIUM);
	BIND_ENUM_CONSTANT(PRECISION_HIGH);

	ClassDB::bind_method(D_METHOD("set_simulation_state", "active"), &SimulationController2D::set_simulation_state);
	ClassDB::bind_method(D_METHOD("is_simulation_active"), &SimulationController2D::is_simulation_active);
	ClassDB::bind_method(D_METHOD("set_simulation_precision", "precision"), &SimulationController2D::set_simulation_precision);
	ClassDB::bind_method(D_METHOD("get_simulation_precision"), &SimulationController2D::get_simulation_precision);
	ClassDB::bind_method(D_METHOD("set_simulation_force", "precision"), &SimulationController2D::set_simulation_force);
	ClassDB::bind_method(D_METHOD("get_simulation_force"), &SimulationController2D::get_simulation_force);
	ClassDB::bind_method(D_METHOD("set_noise_modulation", "state"), &SimulationController2D::set_noise_modulation);
	ClassDB::bind_method(D_METHOD("is_noise_modulation_active"), &SimulationController2D::is_noise_modulation_active);
	ClassDB::bind_method(D_METHOD("set_noise_pixel_resolution", "precision"), &SimulationController2D::set_noise_pixel_resolution);
	ClassDB::bind_method(D_METHOD("get_noise_pixel_resolution"), &SimulationController2D::get_noise_pixel_resolution);
	ClassDB::bind_method(D_METHOD("set_noise_time_scale", "scale"), &SimulationController2D::set_noise_time_scale);
	ClassDB::bind_method(D_METHOD("get_noise_time_scale"), &SimulationController2D::get_noise_time_scale);

	ClassDB::bind_method(D_METHOD("reset_simulation"), &SimulationController2D::reset_simulation);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_simulation_state", "is_simulation_active");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "simulation_precision", PROPERTY_HINT_ENUM, "Low,Medium,High"), "set_simulation_precision", "get_simulation_precision");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "simulation_force"), "set_simulation_force", "get_simulation_force");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "noise_modulation"), "set_noise_modulation", "is_noise_modulation_active");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "noise_time_scale", PROPERTY_HINT_RANGE, "0,100,1,or_greater"), "set_noise_time_scale", "get_noise_time_scale");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "noise_pixel_resolution"), "set_noise_pixel_resolution", "get_noise_pixel_resolution");

	ADD_SIGNAL(MethodInfo("simulation_changed"));
	ADD_SIGNAL(MethodInfo("simulation_progress"));
	ADD_SIGNAL(MethodInfo("simulation_failed"));
}

SimulationController2D::SimulationController2D() {

	_sim = newref(ElasticSimulation);
	_time_progress = 0;

#ifdef MODULE_OPENSIMPLEX_ENABLED
	noise_modulation = false;
	noise_time_scale = 20;
	noise_pixel_resolution = 20;
	_noise = newref(OpenSimplexNoise);
	_noise->set_persistence(0.3); // more smoothness
#endif

	simulation_active = false;
	simulation_precision = PRECISION_MEDIUM;
	simulation_force = Vector2(10,10);
}

SimulationController2D::~SimulationController2D() {
}


#ifdef TOOLS_ENABLED
void SimulationControllerDebugInstance2D::_edit_set_position(const Point2 &p_position) { }
Point2 SimulationControllerDebugInstance2D::_edit_get_position() const { return Point2(); }
void SimulationControllerDebugInstance2D::_edit_set_scale(const Size2 &p_scale) { }
Size2 SimulationControllerDebugInstance2D::_edit_get_scale() const { return Size2(); }
#endif

Transform2D SimulationControllerDebugInstance2D::get_transform() const { return Transform2D(); }

int SimulationControllerDebugInstance2D::get_cell_size() const {

	return cell_size;
}

void SimulationControllerDebugInstance2D::set_cell_size(int p_size) {

	cell_size = p_size;
}

String SimulationControllerDebugInstance2D::get_configuration_warning() const {

	String warning = CanvasItem::get_configuration_warning();
	if (Object::cast_to<SimulationControllerInstance2D>(get_parent()) == nullptr) {
		warning += TTR("A SimulationControllerDebugInstance2D only works with a SimulationControllerInstance2D as parent node.");
	}
	return warning;
}

void SimulationControllerDebugInstance2D::_notification(int p_what) {

	switch (p_what) {
		case NOTIFICATION_PARENTED:
		case NOTIFICATION_ENTER_TREE: {

			update_configuration_warning();
			set_process(Object::cast_to<SimulationControllerInstance2D>(get_parent()) != nullptr);
		} break;
		case NOTIFICATION_PROCESS: {

			update();
		} break;
		case NOTIFICATION_DRAW: {

			if (SimulationControllerInstance2D *instance = Object::cast_to<SimulationControllerInstance2D>(get_parent())) {
					Ref<SimulationController2D> controller = instance->get_controller();
					if (controller.is_valid()) {
						if (Viewport *viewport = get_viewport()) {
							const Size2i size = viewport->get_visible_rect().size;
							for (int r = 0; r < size.y; r += cell_size) {
								for (int c = 0; c < size.x; c += cell_size) {
									const Vector2 pos = Point2(c, r);
									const real_t dir = controller->get_current_noise_modulation_dir(pos);
									_draw_debug_marker(this, pos, dir, cell_size - 2, cell_size / 3, cell_size / 4);
								}
							}
						}
					}
			}
		}
	}
}

void SimulationControllerDebugInstance2D::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_cell_size"), &SimulationControllerDebugInstance2D::get_cell_size);
	ClassDB::bind_method(D_METHOD("set_cell_size"), &SimulationControllerDebugInstance2D::set_cell_size);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "cell_size"), "set_cell_size", "get_cell_size");
}

SimulationControllerDebugInstance2D::SimulationControllerDebugInstance2D() {

	cell_size = 20;
}


void SimulationControllerInstance2D::_on_controller_changed() {

	set_process_internal(controller->is_simulation_active());
}

void SimulationControllerInstance2D::_notification(int p_what) {

	switch (p_what) {
		case NOTIFICATION_READY: {

			if (controller.is_null()) {
				// create default controller
				set_controller(Ref<SimulationController2D>(memnew(SimulationController2D)));
			}
			add_child(memnew(SimulationControllerDebugInstance2D));
		} break;
		case NOTIFICATION_INTERNAL_PROCESS: {

			const real_t dt = get_process_delta_time();
			controller->simulation_progress(dt);
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

void SimulationControllerInstance2D::_bind_methods() {

	ClassDB::bind_method(D_METHOD("get_controller"), &SimulationControllerInstance2D::get_controller);
	ClassDB::bind_method(D_METHOD("set_controller"), &SimulationControllerInstance2D::set_controller);

	ClassDB::bind_method(D_METHOD("_on_controller_changed"), &SimulationControllerInstance2D::_on_controller_changed);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "controller"), "set_controller", "get_controller");
}

SimulationControllerInstance2D::SimulationControllerInstance2D() {

	motion_debug = false;
}

SimulationControllerInstance2D::~SimulationControllerInstance2D() {
}

// END

// BEGIN Mesh elastic-deform.

void DeformMeshInstance2D::_update_meta() {

	set_meta("__noise_modulation_scale", noise_scale);
	set_meta("__simulation_id", _sim_id);
}

void DeformMeshInstance2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {

			set_notify_local_transform(true);
		} break;
		case NOTIFICATION_DRAW: {

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

Vector2 DeformMeshInstance2D::get_noise_scale() const {

	return noise_scale;
}

void DeformMeshInstance2D::set_noise_scale(Vector2 p_scale) {

	noise_scale = p_scale;
	_update_meta();
}

void DeformMeshInstance2D::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_controller", "controller"), &DeformMeshInstance2D::set_controller);
	ClassDB::bind_method(D_METHOD("get_controller"), &DeformMeshInstance2D::get_controller);
	ClassDB::bind_method(D_METHOD("set_noise_scale", "scale"), &DeformMeshInstance2D::set_noise_scale);
	ClassDB::bind_method(D_METHOD("get_noise_scale"), &DeformMeshInstance2D::get_noise_scale);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "controller"), "set_controller", "get_controller");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "noise_scale"), "set_noise_scale", "get_noise_scale");
}

DeformMeshInstance2D::DeformMeshInstance2D() {

	controller = Ref<SimulationController2D>(NULL);

	_sim_id = -1;
	noise_scale = Vector2(1, 0);

	_update_meta();
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

void DeformSprite::_edit_set_rect(const Rect2 &p_rect) {

	Sprite::_edit_set_rect(p_rect);
	_sim_dirty = true;
}
#endif

void DeformSprite::_update_meta() {

	set_meta("__noise_modulation_scale", noise_scale);
	set_meta("__simulation_id", _sim_id);
}

void DeformSprite::_update_simulation() {

	ERR_FAIL_COND(controller.is_null());

	Ref<ElasticSimulation> sim = controller->get_simulation();

	ERR_FAIL_COND(sim.is_null());

	const Size2 scaled_rect = get_rect().size * geometry_pixel_unit * get_scale();
	if (_sim_id == -1) {
		_sim_id = sim->make_sim(scaled_rect, geometry_segments, geometry_size_variation, geometry_anchor, geometry_stiffness, physics_variation);
		_update_meta();
	} else {
		sim->update_sim(_sim_id, scaled_rect, geometry_segments, geometry_size_variation, geometry_anchor, geometry_stiffness, physics_variation);
	}
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
		ERR_FAIL_COND(verts.size() != sim->get_sim_particles_count(_sim_id));

		const Point2 &origin = get_rect().position;
		const Size2 &sc = ONE / geometry_pixel_unit / get_scale();
		for (int i = 0; i < verts.size(); ++i) {
			verts.set(i, origin + sim->get_sim_particle_pos(_sim_id, i) * sc);
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
				steps.push_back(step);
				steps.push_back(step);
				tsteps.push_back(tstep);
				tsteps.push_back(tstep);
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

void DeformSprite::_check_parent_controller() {
	SimulationControllerInstance2D *new_controller = nullptr;

	if (SimulationControllerInstance2D *instance = Object::cast_to<SimulationControllerInstance2D>(get_parent())) {
		// always use parent controller
		new_controller = instance;
	} else if (controller.is_null()) {
		// look depeer for new controller, if no parent controller and
		// controller is not valid
		Node *next = get_parent(); while(next) {
			if (SimulationControllerInstance2D *instance = Object::cast_to<SimulationControllerInstance2D>(next)) {
				new_controller = instance;
				break;
			}
			next = next->get_parent();
		}
	}
	if (new_controller) {
		set_controller(new_controller->get_controller());
		DEBUG_PRINT("Parent/shared controller connected to the node");
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

void DeformSprite::_on_texture_changed() {

	// Rebuild simulation
	if (get_texture().is_valid()) {
		if (Ref<ElasticSimulation> sim = controller->get_simulation())
			if (_sim_id >= 0)
				sim->remove_sim(_sim_id);
		_sim_id = -1;
		_update_meta();
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

void DeformSprite::set_geometry_stiffness(real_t p_stiffness) {
	ERR_FAIL_COND(p_stiffness > 1);
	ERR_FAIL_COND(p_stiffness < 0);

	if (geometry_stiffness != p_stiffness) {
		geometry_stiffness = p_stiffness;
		_sim_dirty = true;
		update();
	}
}

real_t DeformSprite::get_geometry_stiffness() const {

	return geometry_stiffness;
}

void DeformSprite::set_physics_variation(bool p_variation) {
	if (physics_variation != p_variation) {
		physics_variation = p_variation;
		_sim_dirty = true;
		update();
	}
}

bool DeformSprite::is_physics_variation() const {

	return physics_variation;
}

Vector2 DeformSprite::get_noise_scale() const {

	return noise_scale;
}

void DeformSprite::set_noise_scale(Vector2 p_scale) {

	noise_scale = p_scale;
	_update_meta();
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

inline Point2 middle_point(const Point2 &a, const Point2 &b) { return (a + b) / 2; }

void DeformSprite::debug_draw_geometry() {
	ERR_FAIL_COND(controller.is_null());

	Ref<ElasticSimulation> sim = controller->get_simulation();
	ERR_FAIL_COND(sim.is_null());

	const Point2 &origin = get_rect().position;
	const Size2 &sc = ONE / geometry_pixel_unit / get_scale();
	const int ccnt = sim->get_sim_constraint_count(_sim_id);
	for (int i = 0; i < ccnt; i++) {
		const ElasticSimulation::Constraint &c = sim->get_sim_constraint_at(_sim_id, i);
		const float color_diff = c.deviation * 0.5;
		draw_line(origin + c.begin * sc, origin + c.end * sc, Color(0.5 + color_diff, 0.5 - color_diff, 0, 1));
	}
	const int pcnt = sim->get_sim_particles_count(_sim_id);
	ERR_FAIL_COND(pcnt<4);
	const Color yellow = Color::named("yellow");
	for (int i = 0; i < pcnt; i++) {
		draw_rect(Rect2(origin + sim->get_sim_particle_pos(_sim_id, i) * sc, sim->get_sim_particle_mass(_sim_id, i) * get_scale().inv()), yellow);
	}
	for(int p = 0; p < pcnt - 2; p += 2) {
		const Point2 &p1 = middle_point(sim->get_sim_particle_pos(_sim_id, p), sim->get_sim_particle_pos(_sim_id, p+1));
		const Point2 &p2 = middle_point(sim->get_sim_particle_pos(_sim_id, p+2), sim->get_sim_particle_pos(_sim_id, p+3));
		draw_line(origin + p1 * sc, origin + p2 * sc, yellow);
	}

	const real_t force_scale = 1;
	const Color force_color = Color(0.5, 1, 0, 0.8);
	const Vector2 force = origin + middle_point(sim->get_sim_particle_pos(_sim_id, pcnt - 1), sim->get_sim_particle_pos(_sim_id, pcnt - 2)) * sc;
	const Vector2 force_end = force + force_scale * controller->get_simulation_force_for_node(this) / get_scale();
	draw_line(force, force_end, force_color);
	draw_rect(Rect2(force_end, get_scale().inv()), force_color, true);
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
	ClassDB::bind_method(D_METHOD("set_geometry_stiffness", "factor"), &DeformSprite::set_geometry_stiffness);
	ClassDB::bind_method(D_METHOD("get_geometry_stiffness"), &DeformSprite::get_geometry_stiffness);
	ClassDB::bind_method(D_METHOD("set_physics_variation", "variation"), &DeformSprite::set_physics_variation);
	ClassDB::bind_method(D_METHOD("is_physics_variation"), &DeformSprite::is_physics_variation);
	ClassDB::bind_method(D_METHOD("set_geometry_debug", "debug"), &DeformSprite::set_geometry_debug);
	ClassDB::bind_method(D_METHOD("get_geometry_debug"), &DeformSprite::get_geometry_debug);

	ClassDB::bind_method(D_METHOD("get_simulation_id"), &DeformSprite::get_simulation_id);

	ClassDB::bind_method(D_METHOD("set_noise_scale"), &DeformSprite::set_noise_scale);
	ClassDB::bind_method(D_METHOD("get_noise_scale"), &DeformSprite::get_noise_scale);

	ClassDB::bind_method(D_METHOD("_on_simulation_update"), &DeformSprite::_on_simulation_update);
	ClassDB::bind_method(D_METHOD("_on_simulation_changed"), &DeformSprite::_on_simulation_changed);
	ClassDB::bind_method(D_METHOD("_on_texture_changed"), &DeformSprite::_on_texture_changed);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "controller"), "set_controller", "get_controller");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "noise_scale"), "set_noise_scale", "get_noise_scale");

	ADD_GROUP("Geometry", "geometry_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "geometry_segments", PROPERTY_HINT_RANGE, "1,5,1"), "set_geometry_segments", "get_geometry_segments");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "geometry_size_variation"), "set_geometry_size_variation", "is_geometry_size_variation");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "geometry_anchor", PROPERTY_HINT_ENUM, "Left,Right,Top,Bottom"), "set_geometry_anchor", "get_geometry_anchor");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "geometry_pixel_unit", PROPERTY_HINT_RANGE, "0,1,0.01,or_greater"), "set_geometry_pixel_unit", "get_geometry_pixel_unit");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "geometry_stiffness", PROPERTY_HINT_RANGE, "0,1,0.05"), "set_geometry_stiffness", "get_geometry_stiffness");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "geometry_physics_variation"), "set_physics_variation", "is_physics_variation");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "geometry_debug"), "set_geometry_debug", "get_geometry_debug");
}

DeformSprite::DeformSprite() {
	controller = Ref<SimulationController2D>(NULL);

	geometry_segments = 1;
	geometry_size_variation = false;
	geometry_anchor = ElasticSimulation::SIM_ANCHOR_BOTTOM;
	geometry_pixel_unit = 1;
	geometry_stiffness = 0.5;
	physics_variation = false;
	noise_scale = Vector2(1, 0);
	geometry_debug = false;

	_sim_id = -1;
	_sim_dirty = false;
	_geom_dirty = false;

	_disabled_base_notifications.append(NOTIFICATION_DRAW); // we want only our drawing code
	_update_meta();
}

DeformSprite::~DeformSprite() {

	// TODO: Dereference __state_motion_iterator (?)
}

// END
