/*************************************************************************/
/*  gd_waterfall.h                                                       */
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

#ifndef GD_WATERFALL_H
#define GD_WATERFALL_H

#include "scene/2d/node_2d.h"
#include "scene/resources/texture.h"

#include "flex_particles_system.h"
#include "particles.h"

enum WaterfallParticlesQuality {
	ParticlesLow,
	ParticlesMedium,
	ParticlesHigh,
	ParticlesQualityCount,
};

class GdWaterfall : public Node2D {
	GDCLASS(GdWaterfall, Node2D);

	bool active;
	real_t speed;
	real_t density;
	WaterfallParticlesQuality textures_quality;
	Rect2 view_rect;

	bool _dirty;
	Ref<Texture> _textures[WATERFALL_PARTICLE_COUNT];
	flex_particle_system _p;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
#ifdef TOOLS_ENABLED
	virtual bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;

	virtual Rect2 _edit_get_rect() const;
	virtual bool _edit_use_rect() const;
#endif
	void set_active(bool p_state);
	bool is_active() const;
	void set_view_rect(const Rect2 &p_rect);
	Rect2 get_view_rect() const;
	void set_density(real_t p_density);
	real_t get_density() const;
	void set_speed(real_t p_speed);
	real_t get_speed() const;
	void set_textures_quality(int p_quality);
	int get_textures_quality() const;

	GdWaterfall();
};

#endif // GD_WATERFALL_H
