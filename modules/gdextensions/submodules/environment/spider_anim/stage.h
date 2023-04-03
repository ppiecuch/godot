/**************************************************************************/
/*  stage.h                                                               */
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

#ifndef GD_SPIDER_STAGE_H
#define GD_SPIDER_STAGE_H

#include "core/math/random_pcg.h"
#include "core/math/vector2.h"
#include "core/os/os.h"
#include "core/reference.h"
#include "scene/2d/node_2d.h"

#include "common/local_space.h"
#include "misc/c++/ptr_vector.h"

#include "options.h"
#include "spider_sprite.h"

#include <algorithm>

class InsectSprite;

struct InsectSkinInfo {
	Array parts_texture;
};

typedef int InsectSkinFace;
typedef stdx::ptr_vector<InsectSprite> InsectVector;

const String stage_tilt_changed_signal = "stage_tilt_changed";
const String stage_manager_changed_signal = "stage_manager_changed";

struct Circle {
	Point2 center;
	real_t radius;
};

class InsectSprite : public SpiderSprite {
	friend class InsectCompByDist;

protected:
	LocalSpace2 lspc;
	Node2D *target;
	Point2 base_position;
	Vector2 velocity;
	real_t base_scale;
	real_t tilt_factor;
	bool pause;

public:
	// --
	bool operator<(const InsectSprite &s1) const { return (get_z_index() < s1.get_z_index()); };
	bool operator==(const InsectSprite &s1) const { return (&s1 == this); }

	void set_target(Node2D *p_target) { target = p_target; }
	Node2D *get_target() const { return target; }

	void set_pause(bool p_state) { pause = p_state; }
	bool get_pause() const { return pause; }

	Vector2 get_velocity() const { return velocity; }

	// --
	virtual void set_base_position(const Point2 &p_pos);
	virtual void set_tilt_factor(real_t p_tilt);

	virtual bool animate(real_t p_delta, const Vector<Circle> &p_obstacles) = 0;
	virtual void hit() = 0;
	virtual void shake() = 0;

	virtual void debug(CanvasItem *p_canvas){};

	InsectSprite(Ref<Texture> p_face, const Rect2 p_bbox);
	virtual ~InsectSprite();
};

class InsectCompByDist {
	Point2 origin;

public:
	inline InsectCompByDist(Vector2 pos) :
			origin(pos) {}
	inline bool operator()(const InsectSprite &s1, const InsectSprite &s2) const {
		const real_t xdiff1 = origin.x - s1.base_position.x;
		const real_t ydiff1 = origin.y - s1.base_position.y;
		const real_t dist1 = xdiff1 * xdiff1 + ydiff1 * ydiff1;
		const real_t xdiff2 = origin.x - s2.base_position.x;
		const real_t ydiff2 = origin.y - s2.base_position.y;
		const real_t dist2 = xdiff2 * xdiff2 + ydiff2 * ydiff2;
		return dist1 < dist2;
	}
};

class InsectBehavior : public Reference {
protected:
	RandomPCG rng;
	real_t srand() { return rng.randf() * 2 - 1; } // -1 .. 1
	real_t frand() { return rng.randf(); } // 0 .. 1

public:
	virtual ~InsectBehavior() {}
	virtual Vector2 get_acceleration_vector_for_agent(const InsectSprite *p_insect) = 0;
	virtual void set_acceleration_vector_for_agent(const InsectSprite *p_insect, Vector2 p_accel) = 0;
};

// -- insects states:
enum InsectState {
	InsectAlive = 0,
	InsectDead = 1,
	InsectNumStates = 2
};

class Insect : public InsectSprite {
	const InsectSkinInfo *theme;

	Ref<InsectBehavior> behavior;
	Configuration *const configuration;
	Configuration *const states;
	Items *const groups;
	InsectState state;

	int pause_duration;

	real_t max_velocity; // movement
	real_t max_acceleration;

	int death_counter; // sprite change
	int travel_counter;

	real_t predict; // obstacles avoidance
	real_t sprite_radius;

	real_t velocity_variance;
	real_t acceleration_variance;

	real_t speeding_up;

	int current_sprite;

	RandomPCG rng;

public:
	unsigned int irand() { return rng.rand(); } // 0 .. MAX
	real_t srand() { return rng.randf() * 2 - 1; } // -1 .. 1
	real_t frand() { return rng.randf(); } // 0 .. 1
	int brand() { return rng.rand() & 1 ? -1 : 1; } // -1 , 1

	real_t get_timer() const { return OS::get_singleton()->get_system_time_secs(); }
	uint64_t get_timer_ms() const { return OS::get_singleton()->get_ticks_msec(); }

	void set_behavior(const Ref<InsectBehavior> &p_behavior) { behavior = p_behavior; }
	Ref<InsectBehavior> get_behavior() { return behavior; }

	real_t get_max_velocity() const { return max_velocity; }
	real_t get_max_acceleration() const { return max_acceleration; }

	real_t calc_rotation();
	void move_by_xy(const Point2 &p_dist);
	void update_limits_for_state();
	void update_sprite();

	Vector2 steering_for_obstacle_avoidance(const Vector<Circle> &p_obstacles);
	Vector2 steering_to_avoid_one_obstacle(const Vector2 &p_direction, real_t p_max_forward, real_t p_clear_path);

	virtual bool animate(real_t p_delta, const Vector<Circle> &p_obstacles);
	virtual void hit();

	virtual void debug(CanvasItem *p_canvas);

	Insect(const InsectSkinInfo *p_theme, Configuration *p_conf, Configuration *p_states_conf, Items *p_groups, const Point2 &p_pos, const Vector2 &p_vel);
	virtual ~Insect();

private:
#ifdef DEBUG_ENABLED
	// debug draw
	Vector2 _draw_steer;
	real_t _draw_max_forward;
#endif
};

class Ant : public Insect {
public:
	Ant(const InsectSkinInfo *p_theme, Vector2 p_pos, Vector2 p_vel);
	virtual ~Ant();
};

class SpiderStage : public Resource {
	GDCLASS(SpiderStage, Resource);

	static int insects_eaten;

private:
	SpiderSprite *root; // root where to add insects
	Vector<Circle> obstacles; // excluding area
	Size2 bounds; // insects limits
	real_t stage_tilt; // tilting
	InsectVector insects; // insects vector (insects are part of the scene)
	int dirty_insects; // if vector has changed
	real_t last_time; // animation step

	real_t _factor_y; // tilting factor

protected:
	static void _bind_methods();

public:
	void add(InsectSprite *p_insect);
	void eat(const InsectSprite *p_killed);

	void set_stage_tilt(real_t p_tilt);
	real_t get_stage_tilt() const;

	void set_stage_tilt_degree(int p_tilt);
	int get_stage_tilt_degree() const;

	Size2 get_bounds() const { return bounds; }
	void set_bounds(const Size2 &p_bounds_rect) { bounds = p_bounds_rect; }

	const Vector<Circle> &get_obstacles() { return obstacles; }
	void set_obstacles(const Vector<Circle> p_circle_obstacles) { obstacles = p_circle_obstacles; }

	Node2D *get_root_node();

	int animate(real_t p_delta);

	inline bool empty() const { return insects.empty(); }
	inline bool exists(const InsectSprite *p_insect) const { return std::find(insects.begin(), insects.end(), *p_insect) != insects.end(); }
	inline size_t count() const { return insects.size(); }
	inline int dirty() const { return dirty_insects; }
	const InsectSprite &operator[](int p_index) const { return insects[p_index]; }

	int sort_for_target(Node2D *p_source, Point2 p_location, InsectSprite *&p_target);

	void debug(CanvasItem *p_canvas);

	SpiderStage();
};

class SpiderStageInstance : public Node2D {
	GDCLASS(SpiderStageInstance, Node2D);

private:
	Ref<SpiderStage> stage_manager;
	int initial_insects;
	bool active;

protected:
	static void _bind_methods();
	void _notification(int p_notification);

public:
#ifdef TOOLS_ENABLED
	Dictionary _edit_get_state() const;
	void _edit_set_state(const Dictionary &p_stage);
	bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
	Rect2 _edit_get_rect() const;
	void _edit_set_rect(const Rect2 &p_rect);
	bool _edit_use_rect() const;
#endif

	void set_stage_manager(const Ref<SpiderStage> &p_state);
	Ref<SpiderStage> get_stage_manager() const;

	int get_spiders_connected() const;
	int get_insects_count() const;
	Size2 get_stage_bounds() const;

	void spawn_insects(const Point2 &p_position, int p_num);

	SpiderStageInstance();
};

#endif // GD_SPIDER_STAGE_H
