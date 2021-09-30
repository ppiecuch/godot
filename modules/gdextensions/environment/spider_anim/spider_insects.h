/*************************************************************************/
/*  spider_insects.h                                                     */
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

#ifndef GD_SPIDER_INSECTS_H
#define GD_SPIDER_INSECTS_H

#include "core/math/random_pcg.h"
#include "core/math/vector2.h"
#include "core/os/os.h"
#include "core/reference.h"
#include "misc/c++/ptr_vector.h"
#include "scene/2d/node_2d.h"

#include "common/local_space.h"

#include "spider_sprite.h"

#include <algorithm>

class Insect;

// -- skins and theme configuration for skins:
enum InsectSpriteParts {
	INSECT_FRAME_1 = 0,
	INSECT_FRAME_2, // 1
	INSECT_FRAME_3, // 2
	INSECT_FRAME_DEAD, // 3
	InsectSpritePartsNum, // 4
};

struct InsectSkinInfo {
	Array parts_texture;
};

typedef int InsectSkinFace;

typedef stdx::ptr_vector<Insect> InsectVector;
typedef stdx::ptr_vector<Insect>::iterator InsectVectorIt;

struct Circle {
	Point2 center;
	real_t radius;
};

class Insect : public SpiderSprite {
	GDCLASS(Insect, SpiderSprite)

public:
	LocalSpace2 lspc;
	Node2D *target;
	Vector2 base_position;
	Vector2 vel;
	real_t base_scale;
	real_t tilt_factor;
	bool pause;

	virtual void set_base_position(Vector2 p_pos);
	virtual void set_tilt_factor(real_t p_tilt);

	// --
	bool operator<(const Insect &s1) const { return (get_z_index() < s1.get_z_index()); };
	bool operator==(const Insect &s1) const { return (&s1 == this); }

	// --
	virtual bool animate(real_t p_delta, const Vector<Circle> &p_obstacles) = 0;
	virtual void hit() = 0;
	virtual void shake() = 0;

	virtual void debug(CanvasItem *p_canvas){};

	Insect(Ref<Texture> p_face, const Rect2 p_bbox);
	virtual ~Insect();
};

class InsectCompByDist {
protected:
	Vector2 origin;

public:
	inline InsectCompByDist(Vector2 pos) :
			origin(pos){};
	inline bool operator()(const Insect &s1, const Insect &s2) const {
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
	virtual Vector2 get_acceleration_vector_for_agent(const Insect *p_insect) = 0;
	virtual void set_acceleration_vector_for_agent(const Insect *p_insect, Vector2 p_accel) = 0;
};

// Insects states:
enum InsectState {

	InsectAlive = 0,
	InsectDead = 1,
	InsectNumStates = 2
};

class ConfigurableInsect : public Insect {
	GDCLASS(ConfigurableInsect, Insect)

protected:
	RandomPCG rng;
	unsigned int irand() { return rng.rand(); } // 0 .. MAX
	real_t srand() { return rng.randf() * 2 - 1; } // -1 .. 1
	real_t frand() { return rng.randf(); } // 0 .. 1
	int brand() { return rng.rand() & 1 ? -1 : 1; } // -1 , 1

	real_t get_timer() const { return OS::get_singleton()->get_system_time_secs(); }
	uint64_t get_timer_ms() const { return OS::get_singleton()->get_ticks_msec(); }

public:
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

public:
	real_t calc_rotation();
	void move_by_xy(real_t p_x, real_t p_y);
	void update_limits_for_state();
	void update_sprite();

	Vector2 steering_for_obstacle_avoidance(const Vector<Circle> &p_obstacles);
	Vector2 steering_to_avoid_one_obstacle(const Vector2 &p_direction, real_t p_max_forward, real_t p_clear_path);

	virtual bool animate(real_t p_delta, const Vector<Circle> &p_obstacles);
	virtual void hit();

	virtual void debug(CanvasItem *p_canvas);

	ConfigurableInsect(const InsectSkinInfo *p_theme, Configuration *p_conf, Configuration *p_states_conf, Items *p_groups, Point2 p_pos, Vector2 p_vel);
	virtual ~ConfigurableInsect();

private:
#ifdef DEBUG_ENABLED
	// debug draw
	Vector2 _draw_steer;
	real_t _draw_max_forward;
#endif
};

class Ant : public ConfigurableInsect {
	GDCLASS(Ant, ConfigurableInsect)

public:
	Ant(const InsectSkinInfo *p_theme, Vector2 p_pos, Vector2 p_vel);
	virtual ~Ant();
};

class InsectsManager : public Reference {
	GDCLASS(InsectsManager, Reference)

	static int insects_eaten;

private:
	SpiderSprite *root; // root where to add bugs
	Vector<Circle> obstacles; // excluding area
	Size2 bounds; // insects limits
	real_t factor_y; // tilting factor
	InsectVector insects; // insects vector (insects are part of the scene)
	int dirty_insects; // if vector has changed
	real_t last_time; // animation step

public:
	void add(Insect *p_insect);
	void eat(const Insect *p_killed);

	void set_tilt_factor(int p_tilt_factor);
	real_t get_tilt_factor() const;

	real_t get_tilt() const;

	Size2 get_bounds() const { return bounds; }
	void set_bounds(const Size2 &p_bounds_rect) { bounds = p_bounds_rect; }

	const Vector<Circle> &get_obstacles() { return obstacles; }
	void set_obstacles(const Vector<Circle> p_circle_obstacles) { obstacles = p_circle_obstacles; }

	int animate(real_t p_delta);

	inline bool empty() const { return insects.empty(); }
	inline bool exists(const Insect *p_insect) const { return std::find(insects.begin(), insects.end(), *p_insect) != insects.end(); }
	inline size_t count() const { return insects.size(); }
	inline int dirty() const { return dirty_insects; }
	const Insect &operator[](int p_index) const { return insects[p_index]; }

	int sort_for_target(Node2D *p_source, Point2 p_location, Insect *&p_target);

	void debug(CanvasItem *p_canvas);

	InsectsManager();
};

class InsectsManagerInstance : public Node2D {
	GDCLASS(InsectsManagerInstance, Node2D)

private:
	Ref<InsectsManager> insects_manager;
	int initial_insects;
	bool active;

protected:
	static void _bind_methods();
	void _notification(int p_notification);

public:
#ifdef TOOLS_ENABLED
	Dictionary _edit_get_state() const;
	void _edit_set_state(const Dictionary &p_state);
	bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
	Rect2 _edit_get_rect() const;
	void _edit_set_rect(const Rect2 &p_rect);
	bool _edit_use_rect() const;
#endif

	Ref<InsectsManager> get_manager() const;

	void spawn_insects(const Point2 &position, int p_num);

	InsectsManagerInstance();
};

#endif // GD_SPIDER_INSECTS_H
