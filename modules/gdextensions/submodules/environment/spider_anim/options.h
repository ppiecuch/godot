/**************************************************************************/
/*  options.h                                                             */
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

#ifndef GD_SPIDER_OPTIONS_H
#define GD_SPIDER_OPTIONS_H

#include "common/gd_math.h"
#include "core/math/math_defs.h"

#include <map>
#include <vector>

// Available configurations options
enum ConfigOptions {
	// -- insects configuration:
	Conf_max_acceleration = 1,
	Conf_max_acceleration_variance,
	Conf_max_velocity,
	Conf_max_velocity_variance,
	Conf_sprites_based_on_distance,
	Conf_speeding_up_after_hit,
	Conf_slowing_down_after_hit,
	Conf_use_sprite_group,
	Conf_distance_between_sprite_changes,
	Conf_dimensions_variance,
	Conf_dimensions_width,
	Conf_dimensions_height,
	Conf_dimensions_anchor_x,
	Conf_dimensions_anchor_y,
	// -- spider configuration:
	Conf_spider_size,
	Conf_spider_body_scale,
	Conf_spider_leg_scale,
	Conf_spider_leg_thickness,
	Conf_spider_leg_dist_scale,
	Conf_spider_leg_raise,
	Conf_spider_speed,
	Conf_spider_bounce,
	Conf_spider_body_height
};

typedef real_t confvalue_t;

#define CF(_cf, _attr) (*_cf)[_attr]

typedef std::pair<ConfigOptions, confvalue_t> ConfItem;
typedef std::map<ConfigOptions, confvalue_t> Configuration;
typedef std::vector<int> Items;

// -- Configuration
constexpr real_t DefaultTilt = 45 * Math_PI_by_180;

constexpr int SpidersMaxSpiders = 50;
constexpr int SpiderPedFactorX = 15;
constexpr int SpiderPedFactorY = 15;

// -- Durations
constexpr real_t IntroTime = 1.5;
constexpr real_t EatPauseTime = 1;
constexpr real_t JumpTime = 2.5;
constexpr real_t JumpDelay = 1.5;

// -- Ant behavior
constexpr real_t WanderMax = 2.0;
constexpr real_t WanderVariationMax = 0.5;
constexpr real_t AccelDamping = 0.7;
constexpr int PauseDuration = 60;

#endif // GD_SPIDER_OPTIONS_H
