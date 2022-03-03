/*************************************************************************/
/*  stage.cpp                                                            */
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

#include "core/math/math_funcs.h"
#include "scene/main/viewport.h"

#include "common/gd_core.h"
#include "common/gd_math.h"
#include "options.h"
#include "stage.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

// static inline bool _point_in_poly(int nvert, const Vector2 *vert, const Vector2 &test) {
// 	bool c = false;
// 	for (int i = 0, j = nvert - 1; i < nvert; j = i++) {
// 		if ( ((vert[i].y > test.y) != (vert[j].y > test.y)) &&
// 			(test.x < (vert[j].x - vert[i].x) * (test.y - vert[i].y) / (vert[j].y - vert[i].y) + vert[i].x) )
// 			c = !c;
// 	}
// 	return c;
// }

static inline bool _is_empty(Vector2 pt) {
	real_t ab = pt.x - pt.y;
	return ((-(FLT_EPSILON) <= ab) && (ab <= FLT_EPSILON));
}

static inline Vector2 _interp(real_t blend, Vector2 v0, Vector2 v1) {
	return Vector2(v0.x + blend * (v1.x - v0.x), v0.y + blend * (v1.y - v0.y));
}

static inline Vector2 _truncate(Vector2 vec, real_t max) {
	// this is not true truncation, but it is much faster
	if (vec.x > max)
		vec.x = max;
	if (vec.y > max)
		vec.y = max;
	if (vec.y < -max)
		vec.y = -max;
	if (vec.x < -max)
		vec.x = -max;
	return vec;
}

static inline real_t _approximate_length(Vector2 vec) {
	real_t a = vec.x;
	if (a < 0) {
		a = -a;
	}
	real_t b = vec.y;
	if (b < 0) {
		b = -b;
	}
	if (a < b) {
		return b * 0.9375 + a * 0.375;
	}
	return a * 0.9375 + b * 0.375;
}

static inline Vector2 _approximate_normalize(Vector2 vec) {
	const real_t m = _approximate_length(vec);
	if (m != 0) {
		return 1 / m * vec;
	}
	return vec;
}

static inline Vector2 _approximate_truncate(Vector2 vec, real_t threshold) {
	const real_t length = _approximate_length(vec);
	if (length > threshold) {
		return threshold / length * vec;
	}
	return vec;
}

#define _from(C) C, C + sizeof(C) / sizeof(C[0])

namespace ant {
// Ant behaviour
enum InsectsFaces {
	AntFrame_1 = 0,
	AntFrame_2 = 1,
	AntFrame_3 = 2,
	AntFrameDead_4 = 3
};

const ConfItem _conf1[] = {
	ConfItem(Conf_max_acceleration, 10),
	ConfItem(Conf_max_acceleration_variance, 2),
	ConfItem(Conf_max_velocity, 5),
	ConfItem(Conf_max_velocity_variance, 2),
	ConfItem(Conf_sprites_based_on_distance, 1), // change image after some distance
	ConfItem(Conf_speeding_up_after_hit, 1.5),
	ConfItem(Conf_slowing_down_after_hit, 0.06),
	ConfItem(Conf_use_sprite_group, 0) // what set of images
};

const ConfItem _conf2[] = {
	ConfItem(Conf_max_acceleration, 20),
	ConfItem(Conf_max_velocity, 8),
	ConfItem(Conf_use_sprite_group, 0)
};

const ConfItem _conf3[] = {
	ConfItem(Conf_use_sprite_group, 1)
};

Configuration states[] = {
	Configuration(_from(_conf1)),
	Configuration(_from(_conf2)),
	Configuration(_from(_conf3))
};

// texture atlas id/parts:
InsectSkinFace _grp1[] = { AntFrame_1, AntFrame_2, AntFrame_3 };
InsectSkinFace _grp2[] = { AntFrameDead_4 };
Items groups[] = {
	Items(_from(_grp1)),
	Items(_from(_grp2))
};

const ConfItem _conf[] = {
	ConfItem(Conf_distance_between_sprite_changes, 2),
	ConfItem(Conf_dimensions_variance, 20),
	ConfItem(Conf_dimensions_height, 24),
	ConfItem(Conf_dimensions_width, 14),
	ConfItem(Conf_dimensions_anchor_x, -7),
	ConfItem(Conf_dimensions_anchor_y, -12)
};
Configuration configuration(_from(_conf));
} //namespace ant

class WanderBehavior : public InsectBehavior {
protected:
	Vector2 last_accel;

public:
	WanderBehavior() {}

	Vector2 get_acceleration_vector_for_agent(const InsectSprite *p_insect) {
		if (last_accel.x == 0 && last_accel.y == 0) {
			last_accel.x = srand() * WanderMax;
			last_accel.y = srand() * WanderMax;
		}
		if (Object::cast_to<const Ant>(p_insect)) {
			// now vary the wander accel by a little bit
			last_accel.x += srand() * WanderVariationMax;
			last_accel.y += srand() * WanderVariationMax;

			last_accel = _truncate(last_accel, WanderMax);
		}
		return last_accel;
	}

	void set_acceleration_vector_for_agent(const InsectSprite *p_insect, Vector2 p_accel) {
		last_accel = _truncate(p_accel, WanderMax);
	}
};

class FleeBehavior : public WanderBehavior {
public:
	Vector2 get_acceleration_vector_for_agent(const InsectSprite *p_insect) {
		if (const Ant *ant = Object::cast_to<const Ant>(p_insect)) {
			if (ant->get_target() == 0) {
				return WanderBehavior::get_acceleration_vector_for_agent(ant) * 0.75;
			} else {
				Vector2 wander = WanderBehavior::get_acceleration_vector_for_agent(ant) * 2;
				Vector2 desired = _approximate_normalize(wander + ant->get_position() - ant->get_target()->get_position()) * ant->get_max_velocity();
				return _interp(0.7, desired - ant->get_velocity(), last_accel);
			}
		}

		return last_accel;
	}

	FleeBehavior() {}
};

void InsectSprite::set_base_position(const Point2 &p_pos) {
	base_position = p_pos;
}

void InsectSprite::set_tilt_factor(real_t p_tilt_factor) {
	if (tilt_factor != p_tilt_factor) {
		tilt_factor = p_tilt_factor;
		set_position(base_position.x, base_position.y * p_tilt_factor);
		set_scale_y(base_scale * (p_tilt_factor + (1 - p_tilt_factor) * 0.5));
	}
}

InsectSprite::InsectSprite(Ref<Texture> p_face, const Rect2 p_bbox) :
		SpiderSprite(SpiderSpriteGroupNode) {
	target = nullptr;
	base_position = Point2();
	velocity = Vector2(1, 1);
	base_scale = 1;
	tilt_factor = 1;
	pause = false;

	set_size(p_bbox.size);
	set_offset(p_bbox.position);
	set_texture(p_face);
}

InsectSprite::~InsectSprite() {}

// -- base insect class

Insect::Insect(const InsectSkinInfo *p_theme, Configuration *p_conf, Configuration *p_states_conf, Items *p_groups, const Point2 &p_pos, const Vector2 &p_vel) :
		InsectSprite(p_theme->parts_texture[p_groups[InsectAlive][0]] /* first image by default */
				,
				Rect2(CF(p_conf, Conf_dimensions_anchor_x), CF(p_conf, Conf_dimensions_anchor_y), CF(p_conf, Conf_dimensions_width), CF(p_conf, Conf_dimensions_height))),
		theme(p_theme),
		configuration(p_conf),
		states(p_states_conf),
		groups(p_groups) {
	travel_counter = 0;
	current_sprite = 0;
	pause_duration = 0;

	state = InsectAlive;

	velocity_variance = brand() * (0.25 + 0.75 * frand());
	acceleration_variance = brand() * (0.25 + 0.75 * frand());

	death_counter = 0;
	speeding_up = 1;
	predict = 15; // for obstacles avoidance
	sprite_radius = MAX(CF(p_conf, Conf_dimensions_width), CF(p_conf, Conf_dimensions_height)) / 2;
	velocity = _truncate(velocity, max_velocity);
	base_scale += CF(p_conf, Conf_dimensions_variance) / 100 * frand();
	set_scale(base_scale);

#ifdef DEBUG_ENABLED
	_draw_steer = Vector2();
	_draw_max_forward = 0;
#endif

	update_limits_for_state();
	set_base_position(p_pos);
}

Insect::~Insect() {}

void Insect::hit() {
	static unsigned int create_time = get_timer_ms();

	if (get_timer_ms() - create_time > 2000) { // let him live a little ...
		state = InsectDead;
		update_sprite();
	} else
		speeding_up = states[state][Conf_speeding_up_after_hit];
}

bool Insect::animate(real_t p_delta, const Vector<Circle> &p_obstacles) {
	const Vector2 global_up = Vector2(0, 1);

	if (pause && state != InsectDead) {
		set_rotation(get_rotation() + ((irand() & 7) - 4));
		// force pausing to be no more than 2.5 sec:
		if (pause_duration > PauseDuration) {
			// kill it after:
			state = InsectDead;
			pause_duration = 0;
		}
		++pause_duration;
		return true;
	}

	// if not dead...
	if (state != InsectDead) {
		// calculate new position
		Vector2 accel(1, 1);

		// verify obstacles
		Vector2 steering = steering_for_obstacle_avoidance(p_obstacles);

		if (behavior)
			accel = behavior->get_acceleration_vector_for_agent(this); // TRACE("accel virgin: %f, %f", accel.x, accel.y);
		accel = _approximate_truncate(accel, max_acceleration); // TRACE("accel trunc: %f, %f (%f)", accel.x, accel.y, timeDelta);

		if (!_is_empty(steering)) {
			accel = _interp(AccelDamping, steering, accel);
			if (behavior)
				behavior->set_acceleration_vector_for_agent(this, accel); // store new acceleration
		}
#ifdef DEBUG_ENABLED
		_draw_steer = steering;
#endif

		velocity = _approximate_truncate(accel + velocity, max_velocity);
		const real_t speed = velocity.length();
		if (speed > 0) {
			const Vector2 accel_up = 0.5 * accel;
			const Vector2 accel_fwd = 1 / speed * velocity;
			Vector2 bank_up = (accel_up + lspc.up + global_up).normalized();
			lspc.up = accel_fwd;
			lspc.side = lspc.side.perp(bank_up);
		}

		// if alive, move
		if (state == InsectAlive) {
			Vector2 pos_delta = velocity * speeding_up;
			move_by_xy(pos_delta);

			if (speeding_up > 1)
				speeding_up -= states[state][Conf_slowing_down_after_hit];
			if (speeding_up < 1)
				speeding_up = 1;
		} else if (state == InsectDead) {
			// continue fading ant
			++death_counter;
			if (death_counter > 50) {
				if (death_counter > 150) {
					// remove from world, dealloc
					return false;
				}
				set_alpha(1 - death_counter / 150.);
			}
		}
	}

	return true;
};

void Insect::move_by_xy(const Vector2 &p_move) {
	base_position += p_move;
	travel_counter += p_move.length_squared();
	update_sprite();
};

real_t Insect::calc_rotation() {
	// calc rotation based on velocity
	real_t base_angle = Math::atan(velocity.y / velocity.x);

	while (base_angle < 0)
		base_angle += Math_Two_PI;
	return Radians_To_Degree(base_angle + Math_Half_PI + (velocity.x < 0 ? Math_PI : 0));
}

void Insect::update_limits_for_state() {
	if (state >= InsectNumStates) {
		max_velocity = max_acceleration = 0;
		return;
	}

	const real_t max_velocity_variance = states[state][Conf_max_velocity_variance] * velocity_variance;
	max_velocity = states[state][Conf_max_velocity] + max_velocity_variance;

	const real_t max_acceleration_variance = states[state][Conf_max_acceleration_variance] * acceleration_variance;
	max_acceleration = states[state][Conf_max_acceleration] + max_acceleration_variance;
}

void Insect::update_sprite() {
	if (states[state][Conf_sprites_based_on_distance]) {
		const real_t distance = CF(configuration, Conf_distance_between_sprite_changes);
		if (travel_counter > distance * distance) {
			travel_counter = 0;
			current_sprite++;
		}
	} else
		current_sprite++;

	const int grp = states[state][Conf_use_sprite_group];
	current_sprite = current_sprite % groups[grp].size();
	set_texture(theme->parts_texture[groups[grp][current_sprite]]);
	set_rotation(calc_rotation());
	set_position(base_position.x, base_position.y * tilt_factor);
}

Vector2 Insect::steering_for_obstacle_avoidance(const Vector<Circle> &p_obstacles) {
	constexpr real_t safety_margin = 0.5;
	const real_t max_forward = predict * _approximate_length(velocity);

	Vector2 v;
	real_t min_dist = Math_NAN;
#ifdef DEBUG
	_draw_max_forward = max_forward;
#endif
	for (int ob = 0; ob < p_obstacles.size(); ob++) {
		const Circle &obstacle = p_obstacles[ob];

		const real_t obs_rad = obstacle.radius;
		const real_t total_rad = sprite_radius + obs_rad;
		if (total_rad > get_position().distance_to(obstacle.center)) {
			TRACE("%08x: touch with %f x %f (%f > %f)\n",
					this,
					obstacle.center.x, obstacle.center.y, total_rad,
					get_position().distance_to(obstacle.center));
		}

		Vector2 local_obstacle = lspc.localize_position(get_position(), obstacle.center);

		if (local_obstacle.y > 0.0f && local_obstacle.y - obs_rad < max_forward) {
			Vector2 local_center = { local_obstacle.x, local_obstacle.y };
			const real_t off_path_dist = _approximate_length(local_center);
			const real_t safety_radius = sprite_radius * safety_margin;
			if (off_path_dist < total_rad + safety_radius) {
				const real_t clear_path = local_obstacle.y - obs_rad;
				if (min_dist > clear_path) {
					min_dist = clear_path;
					v = steering_to_avoid_one_obstacle(lspc.globalize_direction(local_center), max_forward, clear_path);
				}
			}
		}
	}

	return v;
}

Vector2 Insect::steering_to_avoid_one_obstacle(const Vector2 &p_direction, real_t p_max_forward, real_t p_clear_path) {
	const real_t factor = CLAMP((p_max_forward - p_clear_path) / p_max_forward, 0, 1);

	const Vector2 breaking = _approximate_truncate(-0.2 * factor * factor * max_acceleration * lspc.up, 0.5 * _approximate_length(velocity));
	return -factor * max_acceleration * _approximate_normalize(p_direction) + breaking;
}

void Insect::debug(CanvasItem *p_canvas) {
#ifdef DEBUG_ENABLED
	const Color blue = Color::named("blue");
	const Color green = Color::named("green");
	const Color red = Color::named("red");
	// steering force
	p_canvas->draw_line(get_position(), get_position() + _draw_steer, green);
	// velocity force
	p_canvas->draw_line(get_position(), get_position() + velocity * 5, red);
	// predict view
	Vector2 p[4] = {
		/* 0 */ sprite_radius * lspc.up,
		/* 1 */ _draw_max_forward * lspc.up + sprite_radius * lspc.up,
		/* 2 */ _draw_max_forward * lspc.up + -sprite_radius * lspc.up,
		/* 3 */ -sprite_radius * lspc.up
	};
	for (int i = 0; i < 4; i++) {
		p[i] = get_position() + p[i];
	}
	for (int i = 0; i < 3; i++) {
		p_canvas->draw_line(p[i], p[i + 1], blue);
	}
#endif
}

// -- Ant

Ant::Ant(const InsectSkinInfo *theme, Vector2 pos, Vector2 vel) :
		Insect(theme, &ant::configuration, &ant::states[0], &ant::groups[0], pos, vel) {
	set_behavior(Ref<InsectBehavior>(frand() < 0.75 ? memnew(WanderBehavior()) : memnew(FleeBehavior())));
}

Ant::~Ant() {}

// sort with copy:
template <class Iter, class Comp>
inline stdx::ptr_vector<typename std::iterator_traits<Iter>::value_type> sort_copy(Iter begin, Iter end, Comp cp) {
	stdx::ptr_vector<typename std::iterator_traits<Iter>::value_type> v(begin, end); // local copy
	stdx::sort(v.begin(), v.end(), cp);
	return v;
}

template <class T>
inline void copy_ptr_vector(T &dest, T &src) {
	dest.fill_null(src.size());
	memcpy(dest.addr(0), src.addr(0), sizeof(void *) * src.size());
}

// Insects/stage manager

int SpiderStage::insects_eaten;

int SpiderStage::animate(real_t p_delta) {
	ERR_FAIL_NULL_V(root, insects.size());

	dirty_insects = 0; // reset flag
	// insects:
	if (!insects.empty()) {
		for (int cn = insects.size() - 1; cn >= 0; --cn) {
			InsectSprite &s = insects[cn];
			if (!s.animate(p_delta, obstacles)) {
				insects.detach(insects.begin() + cn);
				root->remove_child(&s);
				dirty_insects++; // insects changed
			} else {
				const Point2 &xy = s.get_position();
				if (xy.x > bounds.width || xy.y > bounds.height) { // out of bounds?
					insects.detach(insects.begin() + cn); // kill the ant!
					root->remove_child(&s);
					dirty_insects++; // insects changed
				}
			}
		}
	}
	last_time += p_delta;
	return insects.size();
}

// return insects sorted according to
// the position of given node/spider
int SpiderStage::sort_for_target(Node2D *p_source, Point2 p_location, InsectSprite *&p_target) {
	p_target = 0;
	if (!insects.empty()) {
		InsectCompByDist cp(p_location);
		stdx::sort(insects.begin(), insects.end(), cp);

		bool clean_rest = false;
		int i = 0;

		for (InsectSprite &insect : insects) {
			if (clean_rest) {
				if (insect.get_target() == p_source)
					insect.set_target(nullptr);
			} else if (insect.get_target() == p_source && i < SpidersMaxSpiders) {
				p_target = &insect; // selected insect
				clean_rest = true; // clean rest of the insects
			} else if (insects[i].get_target() == nullptr && !p_target) {
				p_target = &insect; // first free insect
			}
			i++;
		}
	}

	return insects.size();
}

// add insect to the board:
//
//  - insx: x position of new insect
//  - insy: y position of new insect
//
// in general  - adding an insect is not a problem: it's
// not invalidate any reference to the insects stored inside
// spiders
void SpiderStage::add(InsectSprite *p_insect) {
	ERR_FAIL_NULL(root);

	p_insect->set_tilt_factor(_factor_y);
	root->add_child(p_insect);
	insects.push_back(p_insect);
}

// all reference should be invalidate after removing any insect

void SpiderStage::eat(const InsectSprite *p_killed) {
	ERR_FAIL_NULL(root);
	ERR_FAIL_NULL(p_killed);

	auto f = std::find(insects.begin(), insects.end(), *p_killed);
	if (f != insects.end()) {
		insects.detach(f); // remove killed bug from insects list
		root->remove_child(const_cast<InsectSprite *>(p_killed)); // and free
		dirty_insects++;
		insects_eaten++;
	}
}

void SpiderStage::set_stage_tilt(real_t p_tilt) {
	if (stage_tilt != p_tilt) {
		stage_tilt = p_tilt;
		_factor_y = Math::cos(stage_tilt);
		for (InsectSprite &insect : insects) {
			insect.set_tilt_factor(_factor_y);
		}
		emit_signal(stage_tilt_changed_signal, stage_tilt);
	}
}

real_t SpiderStage::get_stage_tilt() const {
	return stage_tilt;
}

void SpiderStage::set_stage_tilt_degree(int p_tilt) {
	set_stage_tilt(p_tilt * Math_PI_by_180);
}
int SpiderStage::get_stage_tilt_degree() const {
	return Math::round(stage_tilt * Math_180_by_PI);
}

Node2D *SpiderStage::get_root_node() {
	if (root) {
		return root;
	} else {
		return root = memnew(SpiderSprite(SpiderSpriteGroupNode));
	}
}

void SpiderStage::debug(CanvasItem *p_canvas) {
	ERR_FAIL_NULL(p_canvas);

	for (InsectSprite &insect : insects) {
		insect.debug(p_canvas);
	}
}

void SpiderStage::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_stage_tilt", "stage"), &SpiderStage::set_stage_tilt);
	ClassDB::bind_method(D_METHOD("get_stage_tilt"), &SpiderStage::get_stage_tilt);
	ClassDB::bind_method(D_METHOD("set_stage_tilt_degree", "stage"), &SpiderStage::set_stage_tilt_degree);
	ClassDB::bind_method(D_METHOD("get_stage_tilt_degree"), &SpiderStage::get_stage_tilt_degree);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "stage_tilt_degree", PROPERTY_HINT_RANGE, "15,45,1"), "set_stage_tilt_degree", "get_stage_tilt_degree");

	ADD_SIGNAL(MethodInfo(stage_tilt_changed_signal));
}

SpiderStage::SpiderStage() {
	root = nullptr;

	dirty_insects = 0;
	last_time = 0;

	set_stage_tilt(DefaultTilt);
}

// Instance of Insects Manager

#ifdef TOOLS_ENABLED
Dictionary SpiderStageInstance::_edit_get_state() const {
	Dictionary state = Node2D::_edit_get_state();
	if (stage_manager) {
		state["bounds"] = stage_manager->get_bounds();
	}
	return state;
}

void SpiderStageInstance::_edit_set_state(const Dictionary &p_state) {
	Node2D::_edit_set_state(p_state);
	if (stage_manager) {
		stage_manager->set_bounds(p_state["bounds"]);
	}
}

bool SpiderStageInstance::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	return _edit_get_rect().has_point(p_point);
};

Rect2 SpiderStageInstance::_edit_get_rect() const {
	return Rect2(Point2(), stage_manager->get_bounds());
}

void SpiderStageInstance::_edit_set_rect(const Rect2 &p_rect) {
	stage_manager->set_bounds(p_rect.size);
	_change_notify();
}

bool SpiderStageInstance::_edit_use_rect() const {
	return true;
}
#endif

void SpiderStageInstance::set_stage_manager(const Ref<SpiderStage> &p_stage) {
	stage_manager = p_stage;
	emit_signal(stage_manager_changed_signal);
}

Ref<SpiderStage> SpiderStageInstance::get_stage_manager() const {
	return stage_manager;
}

int SpiderStageInstance::get_spiders_connected() const {
	ERR_FAIL_NULL_V(stage_manager, 0);

	List<Connection> connections;
	stage_manager->get_signal_connection_list(stage_tilt_changed_signal, &connections);
	return connections.size();
}

int SpiderStageInstance::get_insects_count() const {
	ERR_FAIL_NULL_V(stage_manager, 0);

	return stage_manager->count();
}

void SpiderStageInstance::spawn_insects(const Point2 &position, int p_num) {
	ERR_FAIL_NULL(stage_manager);
	ERR_FAIL_COND(p_num <= 0);

	for (int n = 0; n < p_num; n++) {
		// stage_manager->add(memnew(Ant));
	}
}

Size2 SpiderStageInstance::get_stage_bounds() const {
	ERR_FAIL_NULL_V(stage_manager, Size2());

	return stage_manager->get_bounds();
}

void SpiderStageInstance::_notification(int p_notification) {
	switch (p_notification) {
		case NOTIFICATION_READY: {
			if (!stage_manager) {
				stage_manager = newref(SpiderStage);
				emit_signal(stage_manager_changed_signal);
			}
		} break;

		case NOTIFICATION_ENTER_TREE: {
			if (stage_manager) {
				if (Viewport *viewport = get_viewport()) {
					if (stage_manager->get_bounds() == Vector2()) {
						stage_manager->set_bounds(viewport->get_size());
					}
				}
			}
		} break;
	}
}

void SpiderStageInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_stage_manager", "stage"), &SpiderStageInstance::set_stage_manager);
	ClassDB::bind_method(D_METHOD("get_stage_manager"), &SpiderStageInstance::get_stage_manager);

	ClassDB::bind_method(D_METHOD("get_spiders_connected"), &SpiderStageInstance::get_spiders_connected);
	ClassDB::bind_method(D_METHOD("get_insects_count"), &SpiderStageInstance::get_insects_count);
	ClassDB::bind_method(D_METHOD("get_stage_bounds"), &SpiderStageInstance::get_stage_bounds);
	ClassDB::bind_method(D_METHOD("spawn_insects", "position", "number"), &SpiderStageInstance::spawn_insects);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "stage_manager", PROPERTY_HINT_RESOURCE_TYPE, "SpiderStage"), "set_stage_manager", "get_stage_manager");
}

SpiderStageInstance::SpiderStageInstance() {
	stage_manager = newref(SpiderStage);
}
