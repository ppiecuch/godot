/**************************************************************************/
/*  spider.h                                                              */
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

#ifndef GD_SPIDER_H
#define GD_SPIDER_H

#include "core/math/random_pcg.h"
#include "core/os/os.h"
#include "misc/c++/ptr_vector.h"
#include "scene/main/viewport.h"

#include "options.h"
#include "spider_sprite.h"
#include "stage.h"

#include <algorithm>

class Ped;
class Back;
class Body;
class Body2;
class LegAnim;

class CircleShape2D;

const String spider_tilt_changed_signal = "tilt_changed";

class Spider : public Node2D {
	GDCLASS(Spider, Node2D)

	typedef std::vector<Ref<LegAnim>> LegAnimVector;

	struct SteeringInfo {
		Vector2 direction;
		real_t interest;
		real_t danger;
		Variant collision;
		real_t weight;
	};

private:
	real_t tilt, factor_y, factor_z;
	Vector2 go;
	real_t go_m;

	real_t counter, counter_stop;
	real_t bounce_counter;
	real_t lift_score;
	real_t dir, head_dir;
	real_t pos_x, pos_y, pos_z;

	bool allow_for_dragging;
	bool grep; // dragging body
	int grep_leg; // dragging selected legs
	real_t grep_time;

	int active_leg;

	bool jumping;
	real_t jump_turn, jump_timer, jump_delay;

	real_t colorize;
	int bounce;
	int body_height;
	real_t body_scale;
	real_t shadow_scale, shadow_opacity;
	real_t leg_scale, leg_dist_scale, leg_thickness;
	int leg_raise;
	real_t body_rot, body_dx, body_dy;

	int spin_counter;
	real_t pause_timer;
	real_t spider_base_speed;
	bool running;

	bool intro;
	real_t intro_duration;
	real_t intro_timer; // seconds of intro

	// destinations
	bool walk_animation;
	Point2 walk_destination; // idle walk destination
	InsectSprite *close_insect, *selected; // promote to be eaten

	// manual control
	bool manual_control;
	int manual_key_turn, manual_key_forward;
	real_t manual_user_speed;

	void _update_layout(bool p_leg_mirror);
	void _setup_from_config();
	void _update_spider_scale();
	void _update_tilt();
	void _check_parent_manager();
	void _build_sprites();
	String _parts_group_name() const;

	void _eat_selected();
	void _update_insects();

	// steering
	bool steering;
	Vector<SteeringInfo> steering_context;
	int steering_fov;
	int steering_ahead;
	real_t steering_direction;
	void _init_steering(int num_rays = 0);
	void _update_steering(real_t p_delta);

	// parts definitions:
	SpiderSprite *parts = 0;
	Ped *pa1 = 0;
	Ped *pb1 = 0;
	Back *back = 0;
	Body *body = 0;
	Body2 *shadow_body = 0;
	SpiderSprite *shadows = 0;

	LegAnimVector legs;

	SpiderTheme skin; // spider's skin
	Ref<SpiderStage> stage_manager; // stage controller

	bool _layout_dirty;
	real_t _speed; // current speed
	Vector2 _dest; // current walk destination

#ifdef TOOLS_ENABLED
	Ref<Font> _debug_font;
#endif
	bool _debug_draw;

protected:
	static void _bind_methods();
	void _notification(int p_notification);

	void _get_property_list(List<PropertyInfo> *p_list) const;

	RandomPCG rng;
	unsigned int irand() { return rng.rand(); } // 0 .. MAX
	real_t srand() { return rng.randf() * 2 - 1; } // -1 .. 1
	real_t frand() { return rng.randf(); } // 0 .. 1

	real_t animation_time;
	Rect2 stage() { return Rect2(Point2(), get_viewport()->get_size()); }

public:
	void set_tilt(real_t p_tilt);
	real_t get_tilt() const;
	void set_tilt_degree(int p_tilt);
	int get_tilt_degree() const;

	void set_spider_theme(int p_theme);
	int get_spider_theme() const;

	void set_spider_base_speed(real_t p_speed);
	real_t get_spider_base_speed() const;

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

	void set_stage_manager(Ref<SpiderStage> p_manager);
	Ref<SpiderStage> get_stage_manager() const;

	void enable_manual_control(bool p_state);
	bool is_manual_control() const;

	void set_manual_key_turn(int p_turn);
	int get_manual_key_turn() const;

	void set_manual_key_forward(int p_dir);
	int get_manual_key_forward() const;

	void set_manual_user_speed(real_t p_speed);
	real_t get_manual_user_speed() const;

	void enable_animation(bool p_state);
	bool is_animation_enabled() const;

	void enable_steering(bool p_state);
	bool is_steering_enabled() const;

	void set_steering_num_rays(int p_num_rays);
	int get_steering_num_rays() const;

	void set_steering_fov(int p_fov);
	int get_steering_fov() const;

	void set_steering_ahead(int p_ahead);
	int get_steering_ahead() const;

	void set_debug_draw(bool p_draw);
	bool get_debug_draw() const;

	void set_location(const Point2 &p_pos, real_t p_rot);
	Point2 get_location() const;

	void animate(real_t p_delta);
	void move_leg(int p_leg, real_t p_dest_x, real_t p_dest_y, real_t p_grep_x, real_t p_grep_y, real_t p_direction);
	void move_angle(real_t p_delta, real_t p_direction, real_t p_distance);

	const InsectSprite *get_selected_insect() const { return selected; }

	bool jump(real_t p_counter, real_t p_turn);

	bool hold(bool p_grep, const Vector2 &p_cursor);
	inline bool is_hold() const { return (grep || grep_leg > -1); }
	bool select(const Vector2 &p_cursor);
	void unselect(const Vector2 &p_cursor);
	void drag();
	void shake();

	void set_walk_destination(const Vector2 &p_dest) { walk_destination = p_dest; }
	Vector2 get_walk_destination() const { return walk_destination; }
	void find_destination();

	void enable_intro(bool p_intro) { intro = p_intro; }
	bool is_intro_enabled() const { return intro; }
	void set_intro_duration(real_t p_intro) { intro_duration = p_intro; }
	real_t get_intro_duration() const { return intro_duration; }

	Dictionary get_status() const;
	String get_info() const;

	static unsigned index_counter;

	Spider();
	~Spider();
};

#endif // GD_SPIDER_H
