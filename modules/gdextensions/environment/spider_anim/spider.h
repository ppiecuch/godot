/*************************************************************************/
/*  spider.h                                                             */
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

#ifndef GD_SPIDER_H
#define GD_SPIDER_H

#include "core/math/random_pcg.h"
#include "core/os/os.h"
#include "misc/c++/ptr_vector.h"
#include "scene/2d/physics_body_2d.h"
#include "scene/main/viewport.h"

#include "conf_data.h"
#include "spider_insects.h"
#include "spider_sprite.h"

#include <algorithm>

class Ped;
class Back;
class Body;
class Body2;
class LegAnim;

class CircleShape2D;

typedef stdx::ptr_vector<Insect> InsectVector;
typedef stdx::ptr_vector<Insect>::iterator InsectVectorIter;

typedef std::vector<Ref<LegAnim>> LegAnimVector;

class Spider : public KinematicBody2D {
	GDCLASS(Spider, KinematicBody2D)

private:
	RandomPCG rng;
	unsigned int irand() { return rng.rand(); } // 0 .. MAX
	real_t srand() { return rng.randf() * 2 - 1; } // -1 .. 1
	real_t frand() { return rng.randf(); } // 0 .. 1

	real_t animation_time;
	Rect2 area_size() { return Rect2(Point2(), get_viewport()->get_size()); }

	bool _layout_dirty;
	real_t _speed;
	int _intro;

private:
	int tilt;
	real_t factor_y, factor_z;

	real_t counter, counter_stop;
	real_t bounce_counter;
	real_t jump_counter;
	real_t go_m, go_x, go_y;
	real_t lift_score;
	real_t dir, head_dir;
	real_t pos_x, pos_y, pos_z;

	bool allow_for_dragging;
	bool grep; // dragging body
	int grep_leg; // dragging selected legs
	real_t grep_time;

	int active_leg;

	bool jumping;
	real_t jump_turn;
	int jump_delay;

	real_t colorize;
	int bounce;
	int body_height;
	real_t body_scale;
	real_t shadow_scale;
	real_t shadow_opacity;
	real_t leg_scale;
	real_t leg_dist_scale;
	real_t leg_thickness;
	int leg_raise;
	real_t body_rot, body_dx, body_dy;

	real_t base_speed;

	// manual control
	bool control;
	int key_turn, key_forward;

	int spin_counter, pause_counter;

	bool running;

	unsigned int index;

	void _setup_from_config();
	void _update_spider_scale();
	void _update_tilt();
	void _check_parent_manager();
	void _build_sprites();
	bool _hit_test_point(real_t p_x, real_t p_y) { return false; } // if x,y is within spider

	void _on_tilt_changed();

protected:
	static void _bind_methods();
	void _notification(int p_notification);

	void _get_property_list(List<PropertyInfo> *p_list) const;

public:
	void set_tilt(int p_tilt);
	int get_tilt() const;

	void set_spider_speed(real_t p_speed);
	real_t get_spider_speed() const;

	void set_spider_colorize(real_t p_colorize);
	real_t get_spider_colorize() const;

	void set_spider_bounce(int p_bounce);
	int get_spider_bounce() const;

	void set_spider_scale(real_t p_scale);
	real_t get_spider_scale() const;

	void set_body_scale(real_t p_body_height);
	real_t get_body_scale() const;

	void set_body_height(int p_body_height);
	int get_body_height() const;

	void set_shadow_scale(real_t p_shadow_scale);
	real_t get_shadow_scale() const;

	void set_shadow_opacity(real_t p_opacity);
	real_t get_shadow_opacity() const;

	void set_leg_scale(real_t p_leg_scale);
	real_t get_leg_scale() const;

	void set_leg_dist_scale(real_t p_leg_dist_scale);
	real_t get_leg_dist_scale() const;

	void set_leg_thickness(real_t p_leg_thickness);
	real_t get_leg_thickness() const;

	void set_leg_raise(int p_leg_rise);
	int get_leg_raise() const;

	void set_insects_manager(Ref<InsectsManager> p_manager);
	Ref<InsectsManager> get_insects_manager() const;

	void set_debug_draw(bool p_draw);
	bool get_debug_draw() const;

	void enable_animation(bool p_state);
	bool is_animation_enabled() const;

	void set_location(const Point2 &p_pos, real_t p_rot);
	Point2 get_location() const;

	void update_layout(bool p_leg_mirror);
	void animate(real_t p_delta);
	void move_leg(int p_leg, real_t p_dest_x, real_t p_dest_y, real_t p_grep_x, real_t p_grep_y, real_t p_direction);
	void move_angle(real_t p_direction, real_t p_distance);

	void eat_selected();
	const Insect *get_selected_insect() const { return selected; }
	void set_selected_insect(Insect *p_bug) { selected = p_bug; }
	void update_insects();

	bool jump(int p_counter, real_t p_turn);

	bool hold(bool p_grep);
	inline bool is_hold() const { return (grep || grep_leg > -1); }
	bool select();
	void unselect();
	void drag();
	void shake();

	void find_destination();

	inline void set_cursor_position(const Point2 &p_pos) {
		_cursor.x = p_pos.x;
		_cursor.y = p_pos.y;
	}
	inline Point2 get_current_destination() const { return _dest; }

	Configuration *const cf() { return configuration; }

	Spider();
	~Spider();

private:
	Configuration *configuration;
	SpiderTheme skin; // spider's skin

	uint32_t owner_id;
	Ref<CircleShape2D> collision_circle;

	Ref<InsectsManager> insects_manager;
	Insect *selected; // promote to be eaten
	Insect *close_insect;

	SpiderSprite *parts = 0;
	Ped *pa1 = 0;
	Ped *pb1 = 0;
	Back *back = 0;
	Body *body = 0;
	Body2 *shadow_body = 0;
	SpiderSprite *shadows = 0;

	LegAnimVector legs;

	Point2 _cursor; // use cursor if no target is available
	Point2 _dest; // walk description

	static unsigned int index_counter;
};

#endif // GD_SPIDER_H
