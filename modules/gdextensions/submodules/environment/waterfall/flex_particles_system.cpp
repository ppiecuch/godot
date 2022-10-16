/*************************************************************************/
/*  flex_particles_system.cpp                                            */
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

#include "flex_particles_system.h"

#include "core/math/math_funcs.h"
#include "core/os/os.h"

#include <algorithm>

flex_particle::flex_particle() {
	set_defaults();
}

flex_particle::flex_particle(const Vector2 &pos) {
	set_defaults();
	position = pos;
}

flex_particle::flex_particle(const flex_particle_options &opts) {
	set_defaults();
	layer = opts.layer;
	position = opts.pos;
	velocity = opts.velocity;
	radius = opts.radius;
	damping = opts.damping;
	alpha = opts.alpha;
	scale = opts.scale;
	texture.set(opts.texture);
}

flex_particle::flex_particle(const flex_particle &p) {
	*this = p;
	texture.owner = this;
}

void flex_particle::set_defaults() {
	layer = 0;
	radius = 2;
	damping = 1;
	mass = 1;
	alpha = 1;
	scale = Vector2(1, 1);
	age = 0;
	start_second = OS::get_singleton()->get_ticks_msec() * 1000;
	unique_id = 0;
	texture.owner = this;
	texture.set(Ref<Texture>());
	_progress = Vector2();
	_progress_alpha = 1;
	_progress_scale = Vector2(1, 1);
}

void flex_particle::update(const Vector2 &global_velocity, const Vector2 &global_acceleration) {
	age++;

	// update position and rotation
	velocity += acceleration + global_acceleration;
	acceleration *= 0;

	if (damping != 1) {
		velocity *= damping;
	}
	position += velocity + global_velocity;
}

void flex_particle::draw(CanvasItem *canvas, const Vector2 &offset, bool debug) {
	const Vector2 _offset = position + offset;
	if (texture.is_valid()) {
		const Rect2 dest_rect{ _offset - texture.dest_size / 2, texture.dest_size };
		canvas->draw_texture_rect(texture.t, dest_rect, false, Color(1, 1, 1, alpha * _progress_alpha));
		if (debug) {
			canvas->draw_rect(dest_rect, Color(0.9, 0.25, 0.25, 0.8), false);
			print_line(vformat("rc: %s", dest_rect));
		}
	} else {
		auto _ellipse = [&](real_t width, real_t height, real_t max_error) {
			ERR_FAIL_COND_V(width < 0, Vector<Point2>());
			ERR_FAIL_COND_V(height < 0, Vector<Point2>());
			ERR_FAIL_COND_V(max_error < 0, Vector<Point2>());

			const real_t f = MAX(width, height);
			int vertex_count = static_cast<int>(Math::ceil(Math_PI / Math::acos(1 - max_error / f)));
			vertex_count = MAX(vertex_count, 2);

			Vector<Point2> vertices;
			for (int i = 0; i < vertex_count; ++i) {
				const real_t t = (Math_TAU / vertex_count) * i;
				const Vector2 v = Vector2(width * Math::cos(t), height * Math::sin(t));
				vertices.push_back(_offset + v);
			}
			vertices.push_back(vertices[0]); // close
			return vertices;
		};

		Size2 rect_size = _progress_scale * scale * Size2(radius, radius);
		Vector<Point2> points = _ellipse(rect_size.width, rect_size.height, 0.25);
		const Color cl = Color(0.9, 0.9, 0.9, alpha * _progress_alpha);
		const real_t r = radius * .4;
		canvas->draw_polyline(points, cl, 1);
		canvas->draw_line(_offset + Point2(r, 0), _offset + Point2(radius, 0), cl, 1);
		canvas->draw_line(_offset - Point2(r, 0), _offset - Point2(radius, 0), cl, 1);
		canvas->draw_circle(_offset, r, Color(0.1 * layer, 0.25, 0.25, alpha * _progress_alpha));
	}
}

void flex_particle::repel(const flex_particle &b) {
	// TODO this should be explored, right now it's pretty straight forward

	real_t distance = position.distance_to(b.position);

	if (distance > b.radius + radius) {
		return;
	}

	const Vector2 diff = (position - b.position).normalized();

	// http://www.grantchronicles.com/astro09.htm
	// this is a really lose interpretation.. like not very close
	const real_t force = b.mass * mass / MAX(1, distance);
	const real_t accel = force / mass;

	acceleration += diff * accel;
}

/*flex_particle &flex_particle::operator=(const flex_particle &p) {
	layer = p.layer;
	damping = p.damping;
	radius = p.radius;
	position = p.position;
	velocity = p.velocity;
	alpha = p.alpha;
	scale = p.scale;
	texture.set(p.texture.t);

	return *this;
}*/

// these constants are for display
const real_t flex_vector_field::FORCE_DISPLAY_SCALE = 5.0;
const real_t flex_vector_field::BASELINE_SCALE = 0.2;

// default internal to external scale, pixels to internal mapping
const real_t flex_vector_field::DEFAULT_SCALE = 0.1;

flex_vector_field::flex_vector_field() :
		_field_width(0), _field_height(0), _field_size(0), _external_width(0), _external_height(0), _scale(1), _hor_shift_pct(0), _sin_power_value(1), _use_sin_map(false), _clamp_sin_positive(false), _field() {}

void flex_vector_field::setup_field(int external_width, int external_height, int field_width, int field_height) {
	if (field_width == 0 || field_height == 0) {
		field_width = external_width * DEFAULT_SCALE;
		field_height = external_height * DEFAULT_SCALE;
	}

	_field_width = field_width;
	_field_height = field_height;
	_external_width = external_width;
	_external_height = external_height;

	_field.clear();

	_field_size = _field_width * _field_height;

	_field.resize(_field_size);
	std::fill(_field.begin(), _field.end(), Vector2());

	_hor_shift_pct = 0.0;
	_scale = 1.0;
}

void flex_vector_field::zero_field() {
	std::fill(_field.begin(), _field.end(), Vector2());
}

void flex_vector_field::fade_field(real_t fade_amount) {
	for (auto &p : _field) {
		p.x *= fade_amount;
		p.y *= fade_amount;
	}
}

void flex_vector_field::randomize_field(real_t range) {
	for (auto &p : _field) {
		// random between -1 and 1
		p.x = Math::random(-1.0, 1.0) * range;
		p.y = Math::random(-1.0, 1.0) * range;
	}
}

void flex_vector_field::set_horizontal_shift(real_t shift_pct) {
	if (shift_pct > 1.0) {
		shift_pct -= Math::floor(shift_pct);
	}
	_hor_shift_pct = shift_pct;
}

Vector2 flex_vector_field::get_force_from_pos(real_t x, real_t y) const {
	Vector2 force(0, 0);

	x += _external_offset.x;
	y += _external_offset.y;

	// convert x and y into percentage
	real_t percent_x = x / _external_width;
	real_t percent_y = y / _external_height;

	// range check
	if (percent_x < 0 || percent_x > 1 || percent_y < 0 || percent_y > 1) {
		return force;
	}

	// shift it as needed
	percent_x += _hor_shift_pct;
	percent_x -= int(percent_x);

	int field_pos_x = percent_x * _field_width;
	int field_pos_y = percent_y * _field_height;

	// range check
	field_pos_x = MAX(0, MIN(field_pos_x, _field_width - 1));
	field_pos_y = MAX(0, MIN(field_pos_y, _field_height - 1));

	// pos in vector
	int vec_pos = field_pos_y * _field_width + field_pos_x;

	force = Vector2(_field[vec_pos].x, _field[vec_pos].y); // scale here as values are pretty large.

	if (_use_sin_map) {
		real_t sin_x = Math::sin(field_pos_x / _sin_x_repeat + _sin_x_phase);
		real_t sin_y = Math::sin(field_pos_y / _sin_y_repeat + _sin_y_phase);

		// watch for the like of (-2 ^ .5)
		if (sin_x >= 0 || _sin_power_value >= 1) {
			sin_x = Math::pow(sin_x, _sin_power_value);
		}

		if (sin_y >= 0 || _sin_power_value >= 1) {
			sin_y = Math::pow(sin_y, _sin_power_value);
		}

		if (_clamp_sin_positive) {
			sin_x = MAX(0, sin_x);
			sin_y = MAX(0, sin_y);
		}

		force.x *= sin_x;
		force.y *= sin_y;
	}

	force *= _scale; // scale it as needed

	return force;
}

void flex_vector_field::set_uniform_force(const Rect2 &area, const Vector2 &force) {
	real_t start_x = area.position.x / _external_width * _field_width;
	real_t end_x = (area.position.x + area.size.width) / _external_width * _field_width;

	real_t start_y = area.position.y / _external_height * _field_height;
	real_t end_y = (area.position.y + area.size.height) / _external_height * _field_height;

	// cap the range
	start_x = MAX(0, start_x);
	end_x = MIN(_field_width, end_x);

	start_y = MAX(0, start_y);
	end_y = MIN(_field_height, end_y);

	for (int yy = start_y; yy < end_y; ++yy) {
		for (int xx = start_x; xx < end_x; ++xx) {
			const int index = yy * _field_width + xx;
			_field[index] = force;
		}
	}
}

void flex_vector_field::apply_sin_map(real_t x_repeat, real_t y_repeat, real_t x_phase, real_t y_phase, real_t sin_power_value, bool positive_clamp) {
	_use_sin_map = true;

	constexpr real_t TwoPI = Math_PI * 2;

	_sin_x_repeat = _field_width / TwoPI / x_repeat;
	_sin_y_repeat = _field_height / TwoPI / y_repeat;

	_sin_x_phase = x_phase;
	_sin_y_phase = y_phase;

	_clamp_sin_positive = positive_clamp;
	_sin_power_value = sin_power_value;
}

void flex_vector_field::draw(CanvasItem *canvas, const Vector2 &offset) {
	const real_t scaled_x = _external_width / _field_width;
	const real_t scaled_y = _external_height / _field_height;

	int start_xx, start_yy;
	int end_xx, end_yy;

	start_xx = _external_offset.x / scaled_x;
	start_yy = _external_offset.y / scaled_y;

	end_xx = _field_width / scaled_x;
	end_yy = _field_height / scaled_y;

	start_xx = MAX(0, start_xx);
	start_yy = MAX(0, start_yy);

	end_xx = MIN(_field_width, end_xx);
	end_yy = MIN(_field_height, end_yy);

	const Color draw_color(0, 0, 0);

	// loop y to x to optimize reading of our vector in cache
	for (int yy = start_yy; yy < end_yy; ++yy) {
		for (int xx = start_xx; xx < end_xx; ++xx) {
			// shift index as needed
			const int shift_xx = ((int)(xx + _hor_shift_pct * _field_width)) % _field_width;
			const int vec_pos = yy * (end_xx - start_xx) + shift_xx;

			// get our main force line
			Point2 force_origin{ xx * scaled_x - _external_offset.x + offset.x, yy * scaled_y - _external_offset.y + offset.y };
			Point2 force_end{ force_origin };

			if (_use_sin_map) {
				real_t sin_x = sin(xx / _sin_x_repeat + _sin_x_phase);
				real_t sin_y = sin(yy / _sin_y_repeat + _sin_y_phase);

				// watch for the like of (-2 ^ .5)
				if (sin_x >= 0 || _sin_power_value >= 1) {
					sin_x = Math::pow(sin_x, _sin_power_value);
				}

				if (sin_y >= 0 || _sin_power_value >= 1) {
					sin_y = Math::pow(sin_y, _sin_power_value);
				}

				if (_clamp_sin_positive) {
					sin_x = MAX(0, sin_x);
					sin_y = MAX(0, sin_y);
				}

				force_end.x += _field[vec_pos].x * FORCE_DISPLAY_SCALE * _scale * sin_x;
				force_end.y += _field[vec_pos].y * FORCE_DISPLAY_SCALE * _scale * sin_y;
			} else {
				force_end.x += _field[vec_pos].x * FORCE_DISPLAY_SCALE * _scale;
				force_end.y += _field[vec_pos].y * FORCE_DISPLAY_SCALE * _scale;
			}

			canvas->draw_circle(force_origin, 2, draw_color);
			canvas->draw_line(force_origin, force_end, draw_color);

			// draw peprendicular base line
			Vector2 base_line = force_end - force_origin;

			const real_t force_length = base_line.length();
			base_line.normalize();
			base_line.set_rotation(Math::deg2rad(90.0));

			canvas->draw_line(force_origin - (base_line * force_length * BASELINE_SCALE), force_origin + (base_line * force_length * BASELINE_SCALE), draw_color);
		}
	}
}

// min mass for the particle, to prevent particles from having a
// 0 mass and messing up calculations
const float flex_particle_system::MIN_PARTICLE_MASS = .1;

// divider for the vec field force.  This allows users to put in vec
// field forces of 3, 5, etc - something human readable, and lower
// the forces within the field to make it managable.
const float flex_particle_system::VEC_FIELD_FORCE_DIVIDER = 100;

flex_particle_system::flex_particle_system(CanvasItem *canvas) {
	_canvas = canvas;
	_options = 0;
	_next_id = 0;

	_world_box = Size2(10, 10);
	_max_particles = 0;

	for (int i = 0; i < SUPPORTED_WALL_CALLBACKS; ++i) {
		_wall_callback_override[i] = false;
		_wall_callbacks[i] = nullptr;
	}

	layer_conf.resize(8);
}

void flex_particle_system::setup_world(const Size2 &world_box) {
	_world_box = world_box;
}

void flex_particle_system::set_wall_callback(std::function<void(flex_particle *)> func, WallCallbackType type, bool override) {
	_wall_callbacks[type] = func;
	_wall_callback_override[type] = override;
}

void flex_particle_system::set_option(Options option, bool enabled, real_t param) {
	if (enabled) {
		_options |= option;
	} else {
		_options &= ~option;
	}
	if (option == VECTOR_FIELD && enabled) {
		// zero param means the vector field will default in size
		_vector_field.setup_field(_world_box.x, _world_box.y, _world_box.x * param, _world_box.y * param);
	}
}

void flex_particle_system::set_layer_option(int layer, Options option, bool enabled) {
	if (layer >= layer_conf.size()) {
		layer_conf.resize(layer);
	}
	if (enabled) {
		layer_conf[layer].options |= option;
	} else {
		layer_conf[layer].options &= ~option;
	}
}

flex_particle *flex_particle_system::get_particle(unsigned long unique_id) {
	MutexLock lock(_update_lock);
	Iterator it = _particles.find(unique_id);
	if (it != _particles.end()) {
		return it->second;
	}
	return nullptr;
}

void flex_particle_system::apply_vector_field(const flex_vector_field &external_vector_field) {
	for (Iterator it = _particles.begin(); it != _particles.end(); ++it) {
		flex_particle *p = it->second;
		const Vector2 vec_field_force = external_vector_field.get_force_from_pos(p->position.x, p->position.y);
		p->acceleration += vec_field_force / MIN(p->mass, MIN_PARTICLE_MASS) / VEC_FIELD_FORCE_DIVIDER;
	}
}

void flex_particle_system::add_particle(flex_particle *p) {
	MutexLock lock(_update_lock);

	p->set_unique_id(_next_id++);

	_particles[p->get_unique_id()] = p;

	while (_max_particles > 0 && _particles.size() > _max_particles) {
		_particles.erase(_particles.begin());
	}
}

bool flex_particle_system::remove_particle(unsigned long unique_id) {
	if (!_update_lock.try_lock()) {
		ERR_PRINT("unable to obtain lock");
		return false;
	}
	Iterator it = _particles.find(unique_id);
	if (it == _particles.end()) {
		return false;
	}
	_particles.erase(it);
	_update_lock.unlock();
	return true;
}

void flex_particle_system::clear() {
	if (_particles.size() == 0) {
		return;
	}
	_update_lock.lock();
	_particles.clear();
	_next_id = 0;
	_update_lock.unlock();
}

void flex_particle_system::mult_force(const Vector2 &force) {
	for (Iterator it = _particles.begin(); it != _particles.end(); ++it) {
		(*it).second->velocity *= force;
	}
}

void flex_particle_system::add_force(const Vector2 &force) {
	for (Iterator it = _particles.begin(); it != _particles.end(); ++it) {
		(*it).second->acceleration += force;
	}
}

void flex_particle_system::set_alpha_change(int layer, const Vector2 &progress_from, real_t alpha_change) {
	ERR_FAIL_COND(progress_from >= 1 || progress_from < 0);
	if (layer >= layer_conf.size()) {
		layer_conf.resize(layer);
	}
	layer_conf[layer].alpha_progress_start = progress_from;
	layer_conf[layer].alpha_change = alpha_change;
}

void flex_particle_system::set_scale_change(int layer, const Vector2 &progress_from, const Vector2 &scale_change) {
	ERR_FAIL_COND(progress_from >= 1 || progress_from < 0);
	if (layer >= layer_conf.size()) {
		layer_conf.resize(layer);
	}
	layer_conf[layer].scale_progress_start = progress_from;
	layer_conf[layer].scale_change = scale_change;
}

void flex_particle_system::update(const Vector2 &global_velocity, const Vector2 &global_acceleration) {
	MutexLock lock(_update_lock);

	for (Iterator it = _particles.begin(); it != _particles.end(); ++it) {
		flex_particle *p = it->second;
		const LayerConf &cfg = layer_conf[p->layer];
		const Vector2 progress{
			CLAMP(p->position.x / _world_box.width, 0, 1),
			CLAMP(p->position.y / _world_box.height, 0, 1)
		};
		if (p->_progress != progress) {
			// update progress changes
			p->_progress = progress;
			if (cfg.options & PROGRESS_ALPHA) {
				p->_progress_alpha =
						(progress.x >= cfg.alpha_progress_start.x ? Math::map1(progress.x, cfg.alpha_progress_start.x, 1, 1, 1 - cfg.alpha_change) : 1) *
						(progress.y >= cfg.alpha_progress_start.y ? Math::map1(progress.y, cfg.alpha_progress_start.y, 1, 1, 1 - cfg.alpha_change) : 1);
			}
			if (cfg.options & PROGRESS_SCALE) {
				p->_progress_scale = Vector2(
						progress.x >= cfg.scale_progress_start.x ? Math::map1(progress.x, cfg.alpha_progress_start.x, 1, 1, 1 + cfg.scale_change.x) : 1,
						progress.y >= cfg.scale_progress_start.y ? Math::map1(progress.y, cfg.alpha_progress_start.y, 1, 1, 1 + cfg.scale_change.y) : 1);
				p->texture.update_size(); // update texture size
			}
		}
		p->update(global_velocity, global_acceleration);
		if (_options & DETECT_COLLISIONS) {
			// check collision
			Iterator inner_it = it;
			for (++inner_it; inner_it != _particles.end(); ++inner_it) {
				flex_particle *inner_p = inner_it->second;
				p->repel(*inner_p);
				inner_p->repel(*p);
			}
		}
		if (_options & VECTOR_FIELD) {
			const Vector2 vec_field_force = _vector_field.get_force_from_pos(p->position.x, p->position.y);
			p->acceleration += vec_field_force / MIN(p->mass, MIN_PARTICLE_MASS) / VEC_FIELD_FORCE_DIVIDER;
		}

		// NOTE
		// ----
		//  These wall routines are repeatative, but since we want to hit each wall one by one
		//  we have all 4.
		//
		// LOGIC
		//  The logic for each is as follows
		//
		//  - check to see if we hit given wall
		//    * if we have a callback and override
		//      - call callback
		//    * else
		//      - if we have a wrap set
		//        * do the wrap
		//      - else
		//        * bounce the particle
		//      - if we have a callback
		//        * call the callback

		const Size2 hbox = p->get_bounding_box() / 2; // particle half size

		// top wall
		if (p->position.y <= -hbox.height) {
			// if callback and override, only call callback
			if (_wall_callbacks[TOP_WALL] && _wall_callback_override[TOP_WALL]) {
				_wall_callbacks[TOP_WALL](it->second);
			} else {
				// otherwise do our internal logic, and hit the callback if one exists
				if (_options & VERTICAL_WRAP) {
					p->position.y = _world_box.height + hbox.height;
				} else {
					// put it in the right direction
					p->velocity.y = (p->velocity.y < 0 ? p->velocity.y * -1 : p->velocity.y);
				}
				if (_wall_callbacks[TOP_WALL]) {
					_wall_callbacks[TOP_WALL](it->second);
				}
			}
		}
		// right wall
		if (p->position.x - p->radius >= _world_box.width) {
			if (_wall_callbacks[RIGHT_WALL] && _wall_callback_override[RIGHT_WALL]) {
				_wall_callbacks[RIGHT_WALL](it->second);
			} else {
				if (_options & HORIZONTAL_WRAP) {
					p->position.x = 0;
				} else {
					p->velocity.x = (p->velocity.x > 0 ? p->velocity.x * -1 : p->velocity.x);
				}
				if (_wall_callbacks[RIGHT_WALL]) {
					_wall_callbacks[RIGHT_WALL](it->second);
				}
			}
		}
		// bottom wall
		if (p->position.y >= _world_box.height + hbox.height) {
			if (_wall_callbacks[BOTTOM_WALL] && _wall_callback_override[BOTTOM_WALL]) {
				_wall_callbacks[BOTTOM_WALL](it->second);
			} else {
				if (_options & VERTICAL_WRAP) {
					p->position.y = -hbox.height;
				} else {
					p->velocity.y = (p->velocity.y > 0 ? p->velocity.y * -1 : p->velocity.y);
				}
				if (_wall_callbacks[BOTTOM_WALL]) {
					_wall_callbacks[BOTTOM_WALL](it->second);
				}
			}
		}
		// left wall
		if (p->position.x + p->radius <= 0) {
			if (_wall_callbacks[LEFT_WALL] && _wall_callback_override[LEFT_WALL]) {
				_wall_callbacks[LEFT_WALL](it->second);
			} else {
				if (_options & HORIZONTAL_WRAP) {
					p->position.x = _world_box.width;
				} else {
					p->velocity.x = (p->velocity.x < 0 ? p->velocity.x * -1 : p->velocity.x);
				}
				if (_wall_callbacks[LEFT_WALL]) {
					_wall_callbacks[LEFT_WALL](it->second);
				}
			}
		}
	}
}

void flex_particle_system::draw(const Vector2 &offset, bool debug) {
	for (Iterator it = _particles.begin(); it != _particles.end(); ++it) {
		flex_particle *p = it->second;
		if (layer_conf[p->layer].visible) {
			p->draw(_canvas, offset, debug);
		}
	}
	if ((_options & VECTOR_FIELD) && (_options & VECTOR_FIELD_DRAW)) {
		_vector_field.draw(_canvas, offset);
	}
}
