/**************************************************************************/
/*  gd_waterfall.h                                                        */
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

#ifndef GD_WATERFALL_H
#define GD_WATERFALL_H

#include "scene/2d/node_2d.h"
#include "scene/resources/texture.h"

#include "flex_particles_system.h"

#include <vector>

enum WaterfallTexturesQuality {
	NO_TEXTURES,
	TEXTURES_LOW,
	TEXTURES_MEDIUM,
	TEXTURES_HIGH,
	TEXTURES_QUALITY_NUM,
};

//
//              +----------+
//              |          |
//              |  - - -   |
//              |    -     | ---- stretch_from
//              |   -  -   |  |   . -+
//       ---    |          |  |      |
// splash /\    |  *   *   | \/   * -+ stretch_amount
// height |     |    *     |
//        | \___+___ __ ___+___/
//
//              <-- splash spread -->

class GdWaterfall : public Node2D {
	GDCLASS(GdWaterfall, Node2D);

	bool active;
	int particle_radius;
	int particle_accel;
	int particle_stretch_from, particle_stretch_amount;
	int particle_fade_from, particle_fade_amount;
	int waterfall_splash_spread, waterfall_splash_height;
	int waterfall_density;
	real_t waterfall_speed;
	WaterfallTexturesQuality textures_quality;
	Rect2 view_rect;
	bool clip_view;
#ifdef TOOLS_ENABLED
	int layers_preview;
	bool textures_debug;
#endif
	bool _update, _reload, _rebuild;
	flex_particle_system _p;
	std::vector<flex_particle> _particles;

	Vector<Ref<Texture>> _cache[TEXTURES_QUALITY_NUM];

	void _cache_textures(const unsigned char *particles_images[], WaterfallTexturesQuality quality);
	void _build_particles();
	void _update_particles();
	void _check_textures_cache();
	void _reload_textures();

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
	void set_canvas_clipping(bool p_state);
	bool is_canvas_clipping() const;
	void set_waterfall_density(int p_density);
	int get_waterfall_density() const;
	void set_waterfall_speed(real_t p_speed);
	real_t get_waterfall_speed() const;
	void set_waterfall_splash_spread(int p_spalsh_spread);
	int get_waterfall_splash_spread() const;
	void set_waterfall_splash_height(int p_splash_height);
	int get_waterfall_splash_height() const;
	void set_particle_radius(int p_radius);
	int get_particle_radius() const;
	void set_particle_acceleration(int p_accel);
	int get_particle_acceleration() const;
	void set_particle_stretch_from(int p_stretch_from);
	int get_particle_stretch_from() const;
	void set_particle_stretch_amount(int p_stretch_amount);
	int get_particle_stretch_amount() const;
	void set_particle_fade_from(int p_fade_from);
	int get_particle_fade_from() const;
	void set_particle_fade_amount(int p_fade_amount);
	int get_particle_fade_amount() const;
	void set_textures_quality(int p_quality);
	int get_textures_quality() const;
#ifdef TOOLS_ENABLED
	void set_layers_preview(int p_layers);
	int get_layers_preview() const;
	void set_particle_texture_debug(bool p_debug);
	bool get_particle_texture_debug() const;
#endif

	GdWaterfall();
};

VARIANT_ENUM_CAST(WaterfallTexturesQuality);

#endif // GD_WATERFALL_H
