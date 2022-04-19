/*************************************************************************/
/*  flex_particles_system.h                                              */
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

#pragma once

#ifndef FLEX_PARTICLES_SYSTEM_H
#define FLEX_PARTICLES_SYSTEM_H

#include "core/math/rect2.h"
#include "core/math/vector2.h"
#include "core/os/mutex.h"
#include "scene/2d/canvas_item.h"
#include "scene/resources/mesh.h"

#include <functional>
#include <map>
#include <vector>

struct flex_particle_options {
	// @param pos               The starting position of the particle in the world
	// @param velocity          The starting velocity of the particle in the world
	// @param rotation          The starting rotation of the particle in the world
	// @param rotate_velocity   The starting rotation velocity of the particle in the world
	// @param radius            The starting radius
	// @param damping           The damping. Damping causes a particle to lose forces over time
	//                          it can be thought of as 'air friction', etc
	// @param alpha             Transparency of particle
	flex_particle_options(const Vector2 &pos, const Vector2 &velocity, const Vector2 &rotation, const Vector2 &rotate_velocity, real_t radius, real_t damping) :
			pos(pos), velocity(velocity), rotation(rotation), rotate_velocity(rotate_velocity), radius(radius), damping(damping) {}

	// these are all left public since this is simply a helper class
	Vector2 pos;
	Vector2 velocity;
	Vector2 rotation;
	Vector2 rotate_velocity;

	real_t radius;
	real_t damping;
};

class flex_particle {
	int age;
	real_t start_second;
	unsigned long unique_id;

	void set_defaults();

public:
	// left public for easy changing
	Vector2 position;
	Vector2 velocity;
	Vector2 acceleration, acceleration_change;

	Vector2 rotation;
	Vector2 rotate_velocity;

	real_t radius;
	real_t damping;
	real_t mass;
	real_t alpha, alpha_damping;

	Ref<Texture> texture;

	flex_particle &operator=(const flex_particle &p);

	// Causes this particle to repel AWAY from particle b
	// The current algorithm is rather general and will probably want to be
	// customized given your application
	void repel(const flex_particle &b);

	// Update this particle through space. Ie move this particle given it's
	// given acceleration, etc.
	void update(const Vector2 &global_velocity = Vector2(), const Vector2 &global_acceleration = Vector2());

	// Draw this particle and offset its position with `offset`.
	// This will only show a black circle with an inner white
	// circle of size radius.
	void draw(CanvasItem *canvas, real_t progress, const Vector2 &offset);

	void set_unique_id(unsigned long id) { unique_id = id; }
	unsigned long get_unique_id() const { return unique_id; }

	// The particle age can be used to keep track of how long a particle has
	// been around.  The system curently doesn't do anything with particle age
	void set_age(int p_age) { age = p_age; }
	int get_age() const { return age; }

	real_t get_start_seconds() const { return start_second; }

	flex_particle();
	flex_particle(const Vector2 &pos);
	flex_particle(const flex_particle_options &opts);
};

class flex_vector_field {
	// the internal dimensions of the field: (ie, 60x40, etc)
	int _field_width;
	int _field_height;
	int _field_size; // total number of "pixels", ie w * h

	// the external dimensions of the field: (ie, 1024x768)
	// ie what it scales to
	int _external_width;
	int _external_height;

	// offset external calculations by a given amount
	Vector2 _external_offset;

	real_t _scale;
	real_t _hor_shift_pct;

	real_t _sin_x_repeat;
	real_t _sin_y_repeat;
	real_t _sin_x_phase;
	real_t _sin_y_phase;
	real_t _sin_power_value;

	bool _use_sin_map;
	bool _clamp_sin_positive;

	std::vector<Vector2> _field;

	enum ForceType {
		OUT_CIRCLE,
		IN_CIRCLE,
		CLOCK_CIRCLE,
		COUNTER_CLOCK_CIRCLE
	};

	void add_force(real_t x, real_t y, ForceType type, real_t radius, real_t strength);

public:
	// these constants are for display
	static const real_t FORCE_DISPLAY_SCALE;
	static const real_t BASELINE_SCALE;

	// default internal to external scale.  Ie if the external world
	// is 500x500, and our scale is .1, then our vector field has 50x50 points
	static const real_t DEFAULT_SCALE;

	// Set up the vector field.
	//
	// @param external_width    The width of the world size this vector field will be
	//                          applied to (ie a window size)
	// @param external_height   The height of the world size this vector field will be
	//                          applied to (ie a window size)
	// @param field_width       The width of the internal representation.  Ie how many
	//                          force points internally?  Leave blank to default to a field based
	//                          on the DEFAULT_SCALE
	// @param field_height      The height of the internal representation. Ie how many
	//                          force points internally?  Leave blank to default to a field based
	//                          on the DEFAULT_SCALE
	void setup_field(int external_width, int external_height, int field_width = 0, int field_height = 0);

	// This will set the force for all vector points to 0.
	// This will not change the field size, only set all values to 0 so they have no affect
	void zero_field();

	// This will let you lower the field values over time.  All field forces will
	// be set to oldForce * fadeAmount.  ie pass in .99 or so
	//
	// @param fadeAmount    The percentage of the force to be LEFT.  NewForce = OldForce * fadeAmount
	void fade_field(real_t fade_amount);

	// Randomly sets all field values to a range between -range and range
	void randomize_field(real_t range);

	// Visually displays a section of the field.  This is very valuable for debugging.
	// Only the specified subsection of the field will be shown if a cropSection is passed
	// otherwise the entire field will be drawn.
	//
	// Field is drawn starting at 0,0
	//
	// @param crop_section   If passed only this section of the vector field will be drawn
	// @param offset         Offset each drawing by adding value
	void draw(CanvasItem *canvas, const Rect2 &crop_section = Rect2(), const Vector2 &offset = Vector2());

	// Pulls the value of the closest point in the vector field.
	// This takes into account the fieldOffset, fieldShift, and scale if any of them are set
	//
	// @param x     The x coordinate (external world coordinate)
	// @param y     The y coordinate (external world coordinate)
	//
	// @return      The 2 dimensional force at this location
	Vector2 get_force_from_pos(real_t x, real_t y) const;

	// Adds a force in a circular pattern that directs away from the center.
	//
	// @param x         The x center of the location of force. (external world coordinate).
	// @param y         The y center of the location of force. (external world coordinate).
	// @param radius    The radius that the force is applied to (how big is the circle).
	// @param strength  The strength of the force.
	void add_outward_circle(real_t x, real_t y, real_t radius, real_t strength) { add_force(x, y, OUT_CIRCLE, radius, strength); }

	// Adds a force in a circular pattern that directs towards the center.
	//
	// @param x         The x center of the location of force. (external world coordinate).
	// @param y         The y center of the location of force. (external world coordinate).
	// @param radius    The radius that the force is applied to (how big is the circle).
	// @param strength  The strength of the force.
	void add_inward_circle(real_t x, real_t y, real_t radius, real_t strength) { add_force(x, y, IN_CIRCLE, radius, strength); }

	// Adds a force in a circular pattern that rotates around the center in clockwise fashion
	//
	// @param x         The x center of the location of force. (external world coordinate).
	// @param y         The y center of the location of force. (external world coordinate).
	// @param radius    The radius that the force is applied to (how big is the circle).
	// @param strength  The strength of the force.
	void add_clockwise_circle(real_t x, real_t y, real_t radius, real_t strength) { add_force(x, y, CLOCK_CIRCLE, radius, strength); }

	// Adds a force in a circular pattern that rotates around the center in counter-clockwise fashion
	//
	// @param x         The x center of the location of force. (external world coordinate).
	// @param y         The y center of the location of force. (external world coordinate).
	// @param radius    The radius that the force is applied to (how big is the circle).
	// @param strength  The strength of the force.
	void add_counter_clockwise_circle(real_t x, real_t y, real_t radius, real_t strength) { add_force(x, y, COUNTER_CLOCK_CIRCLE, radius, strength); }

	// Sets a uniform/constant force to a given section of the vector field.  The area
	// is external world coordinates.
	//
	// @param area      Section of the vector field this force is applied to
	// @param force     The value this section of the vector field will be applied to
	void set_uniform_force(const Rect2 &area, const Vector2 &force);

	// Consider you are modeling a whirlpool inside of a larger lake. You want to
	// have this whirlpool move around the lake, so you have two options. You either
	// clear out the vector field and re-generate the whirlpool each time it moves,
	// or you call this and you can shift the entire vector field. Currently this
	// only works on the horizontal (can't do horizontal and vertical shifts) but the
	// vertical could be extended relatively easily.
	//
	// Internall this keeps all values of the vector field the same, it just calculates
	// values given a certain perfentage of shift.
	void set_horizontal_shift(real_t shift_pct);

	// Scale the vector field, without actually affecting internal values.  You can
	// set this to 0 to temporarily make all forces equal (0,0), then set it to 1
	// later on to get all those forces back to the way they were.
	void set_scale(real_t scale) { _scale = scale; }

	// You have a large screen, and you have 4 different force areas.  You can either
	// create one vector field and set the forces in 4 different areas, or you can have
	// 4 different vector fields and set each one to have an offset so that calculations
	// are based on the new location of the vector field.
	//
	// @param offset    New location for the top left corner of the vector field.  If you set to
	//                  (100, 100) then the top left corner of the vector field will be at point
	//                  (100, 100).
	void set_external_offset(const Vector2 &offset) { _external_offset = offset; }

	// this maps a sin wave onto the grid, causing the values of the field to fluctuate
	// based on the sin map.

	// This applies a sin wave to the values of the field.  This sin wave can be repeated
	// a given number of times over the horizontal and vertical.  This is an easy way to continually
	// and predictably fluctuate the field.  This will be visible when you call draw()
	//
	// @param xRepeat        repeat the sin wave X times on horizontal field
	// @param yRepeat        repeat the sin wave X times on vertical field
	// @param xPhase         offset the x phase
	// @param yPhase         offset the y phase
	// @param sinPowerValue  pow(sin(), sin_power_value) lets you adjust the sin wav
	//                       sin_power_value will be ignored if < 1 and we are working with a negative value
	// @param positiveClamp  lets you force the use of only positive values
	void apply_sin_map(real_t x_repeat, real_t y_repeat, real_t x_phase, real_t y_phase, real_t sin_power_value = 1, bool positive_clamp = false);

	// Clears the sin map so it will no longer be used for calculations
	void clear_sin_map() { _use_sin_map = false; }

	// Returns the internalSize of the field. Remember externalSize * scale = internalSize
	Vector2 get_internal_size() const { return Vector2(_field_width, _field_height); }

	// Returns the externalSize of the field. Remember externalSize * scale = internalSize
	Vector2 get_external_size() const { return Vector2(_external_width, _external_height); }

	// Returns a pointer to the vector that represents the internal field.
	// The size of the vector is internalSize.x * internalSize.y
	// Use this to do advanced adjustments to the field.
	// DO NOT CHANGE THE SIZE OF THE VECTOR
	std::vector<Vector2> *get_field() { return &_field; }

	flex_vector_field();
};

class flex_particle_system {
	// should always be equal to the number of enums above
	static const int SUPPORTED_WALL_CALLBACKS = 4;

	// just makes things a little easier than typing this out every time
	typedef std::map<unsigned long, flex_particle *> Container;
	typedef std::map<unsigned long, flex_particle *>::iterator Iterator;
	typedef std::map<unsigned long, flex_particle *>::reverse_iterator RIterator;
	typedef std::map<unsigned long, flex_particle *>::const_iterator const_Iterator;
	typedef std::map<unsigned long, flex_particle *>::const_reverse_iterator const_RIterator;

	Container _particles; // holds the actual particles
	Size2 _world_box; // if square world, this is the boundaries
	// call back is for special interactions when particles hit wall boundaries
	// the override tells us whether or not we do standard functionality FIRST
	// before the callback or if we let the callback handle everything
	std::function<void(flex_particle *)> _wall_callbacks[SUPPORTED_WALL_CALLBACKS];
	bool _wall_callback_override[SUPPORTED_WALL_CALLBACKS]; // internal array of the different callbacks
	Mutex _update_lock; // update lock, so we can protect memory
	unsigned int _options; // stores options mask
	flex_vector_field _vector_field; // our vector field
	unsigned long _next_id; // for unique_ids
	unsigned int _max_particles; // optional max particles
	CanvasItem *_canvas; // drawing canvas

public:
	// min mass for the particle, to prevent particles from having a
	// 0 mass and messing up calculations
	static const real_t MIN_PARTICLE_MASS;

	// divider for the vec field force.  This allows users to put in vec
	// field forces of 3, 5, etc - something human readable, and lower
	// the forces within the field to make it managable.
	static const real_t VEC_FIELD_FORCE_DIVIDER;

	// callbacks start at top and go in clockwise order for walls
	enum WallCallbackType {
		TOP_WALL,
		RIGHT_WALL,
		BOTTOM_WALL,
		LEFT_WALL
	};

	// sets internal options:
	//   VERTICAL_WRAP     = particles infinitely wrap around top and bottom of screen
	//                       (ie particle that goes off top instantly shows on bottom)
	//   HORIZONTAL_WRAP   = particles infiinitely wrap around sides of screen
	//   VECTOR_FIELD      = use the ofxLabFlexVectorField for calculations
	//   VECTOR_FIELD_DRAW = draw the ofxLabFlexVectorField forces (visual reference tool)
	enum Options {
		VERTICAL_WRAP = (1u << 0),
		HORIZONTAL_WRAP = (1u << 1),
		VECTOR_FIELD = (1u << 2),
		VECTOR_FIELD_DRAW = (1u << 3),
		DETECT_COLLISIONS = (1u << 4)
	};

	// Configure the particle system with given world box.
	void setup_world(const Size2 &world_box);

	// Updates all particles in the system (using `global_velocity` for all particles
	// velocity alteration), applies vector fields if option is enabled calls callbacks
	// that are eneabled, etc.
	void update(const Vector2 &global_velocity = Vector2(), const Vector2 &global_acceleration = Vector2());

	// Draws the particles inside the given window_stencil that
	// might be rotated of `rotate` degree. Offset each particle by
	// adding `offset`.
	void draw(const Rect2 &window_stencil, const Vector2 &offset = Vector2(), real_t rotate = 0.0);

	// Inserts an flex_particle into the system.
	// NOTE: no memory management is done by this system
	void add_particle(flex_particle *particle);

	// Remove a given particle from the system.  Note that you CANNOT
	// remove a particle while the system is updating. The system locks
	// to prevent this. Return true if the particle was removed, false otherwise
	// NOTE: no memory management is done by this system
	bool remove_particle(unsigned long unique_id);

	// Clears all particles and resets unique_id counter
	// NOTE: no memory management is done by this system
	void clear();

	// Multiply the velocity of all particled by this force
	void mult_force(const Vector2 &force);

	// Add to the acceleration of all particled by this force
	void add_force(const Vector2 &force);

	// Return the number of particles currently in the system
	int get_num_particles() const { return _particles.size(); }

	// Set one of the given wall callbacks.  This callback will be called
	// while the particle system is in update() and a specific event occurs.
	// An example is a wall bounce.  If the particle hits a wall we will call
	// the appropriate wall callback.  If the override is true, then the particle
	// system will not bounce the ball, and the callback must do this
	// if the override is false, the ball will bunce and then the callback will be called
	//
	// @param func      function pointer that will act as the callback
	// @param type      type of callback (see WallCallbackType)
	// @param override  if true the particle system will not use internal logic
	//                  and call the callback instead.  If false the system will
	//                  do internal logic and then call the callback
	void set_wall_callback(std::function<void(flex_particle *)> func, WallCallbackType type, bool override);

	// Enable or diable a given option. See Options enum
	// (optional `param` parameter is depending on option)
	void set_option(Options option, bool enabled, real_t param = 0.0);

	// Get the internal container that holds our particles.  Be careful
	// as there is no lock associated with this container so you could easily
	// break things.
	Container const *get_particles() { return &_particles; }

	// Get an individual particle out of the system based on the uniqueID.
	// Be careful not to touch this particle while the system is updating.
	flex_particle *get_particle(unsigned long unique_id);

	// Return a pointer to the internal vector field that the particle
	// system is using. This allows the user to configure the vector field
	// as they need. This is good for any force type you want applied to all particles
	flex_vector_field *get_vector_field() { return &_vector_field; }

	// Apply a given flex_vector_field to the particles.  This is useful for advanced
	// functionality where multiple vector fields are needed.  Note that the particle
	// x,y will be plugged in directly to the vectorField.getForce(x,y), no translation
	// occurs.
	void apply_vector_field(const flex_vector_field &vector_field);

	// Sets the maxinum number of particles that the system will hold.  Once we reach the limit
	// the addition of a new particle will result in the deletion of oldest particle (lowest unique_id)
	void set_max_particles(unsigned int max_particles) { _max_particles = max_particles; }

	// Checks to see if the particle falls within our cropping range
	//
	// @param particle      particle to test
	// @param ws            the cropping rectangle
	// @param rotation      rotation of the cropping rectangle
	// @return              true if the particle should be drawn, false otherwise
	bool should_draw(const flex_particle *particle, const Rect2 &ws, real_t rotation);

	flex_particle_system(CanvasItem *canvas);
};

#endif // FLEX_PARTICLES_SYSTEM_H
