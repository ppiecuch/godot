/*************************************************************************/
/*  gd_water_2d.h                                                        */
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

#ifndef GD_WATER_2D_H
#define GD_WATER_2D_H

#include "core/int_types.h"
#include "core/math/random_pcg.h"
#include "core/math/rect2.h"
#include "core/variant.h"
#include "core/vector.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/material.h"
#include "scene/resources/texture.h"

/// Water object ...

//     0  .. 2  .. 4
//   0 +--|--+--|--+ ..
//     |    /|    /|
//     -  /  -  /  -
//     |/    |/    |
// n*2 +--|--+--|--+ ..
//     |    /|    /|
//     -  /  -  /  -
//     |/    |/    |
//     +--|--+--|--+ ..

#define NUM_CAUSTICS 32
#define NUM_NORMALS 60

template <int WaterSize>
class WaterRipples : public Reference {
	// grid of N x N fluid cells
	static constexpr int WaterMild = WaterSize / 2;
	static constexpr int GridSize = WaterSize * 2;
	static constexpr int GridLast = GridSize - 1;
	static constexpr int GridArea = GridSize * GridSize;
	static constexpr size_t GridRowSpace = sizeof(Vector3) * GridSize;
	static constexpr int BufferSize = WaterSize + 2;
	static constexpr int BufferSpace = sizeof(int) * BufferSize * BufferSize;
	static constexpr size_t BufferRowSpace = sizeof(int) * BufferSize;
	static constexpr real_t CellSize = 1.0 / GridSize;

	int size_factor; // size of the wave
	int angle; // angle for wave generator

	int water1[BufferSize][BufferSize];
	int water2[BufferSize][BufferSize];

	int smooth[BufferSize][BufferSize]; // buffer used for smoothing operation

	int (*p1)[BufferSize][BufferSize]; // pointer FRONT
	int (*p2)[BufferSize][BufferSize]; // pointer BACK

	// geometric construction (static number of vertices)
	Vector3 (*sommet)[GridSize]; // vertices vector
	Vector3 (*normal)[GridSize]; // quads normals
	Vector3 (*snormal)[GridSize]; // vertices normals (average)
	Vector3 (*snormaln)[GridSize]; // normalized vertices normals
	Vector2 (*uvmap)[GridSize]; // background texture coordinates
	Vector2 (*newuvmap)[GridSize]; // perturbated background coordinates -> refraction
	Vector2 (*envmap)[GridSize]; // envmap coordinates
	int(*sindex); // vertex array index

	struct _debug_grid {
		char array[GridSize][GridSize];

		void set(int x, int y, char c) { array[x][y] = c; }
		void clear() { memset(&array, ' ', sizeof(char) * GridSize * GridSize); }
		void dump(int subsize = GridSize) {
			printf("   ");
			for (int x = 0; x < subsize; x++) {
				printf(" %1d  ", x % 10);
			}
			printf("\n");
			for (int y = 0; y < subsize; y++) {
				printf("%2d ", y);
				for (int x = 0; x < subsize; x++) {
					printf("[%c] ", array[x][y]);
				}
				printf("\n");
			}
		}
	} dbg;

	void new_water(); // fluid calculus
	void smooth_water(); // smooth filter
	void prebuild_water(); // precalculate geometric stuffs
	void build_water(); // build geometry

	void build_strip_index(); // build strip index for vertex arrays
	void build_tri_index(); // build triangles index for vertex arrays

public:
	void init();

	int get_grid_size() const;

	void set_size_factor(int p_factor);
	int get_size_factor() const;

	void set_wave(real_t p_x, real_t p_y, int p_amp); // trace a hole in the fluid cells at normalized [0..1] coords
	void run_wave(real_t p_phase, real_t p_cos, real_t p_sin, int p_amp); // some waves using parametric curves
	void random_wave(); // random hole

	void update(); // next step in fluid model: true if model updated

	Array build_mesh_data(const Rect2 *p_skin_region, const Rect2 *p_envmap_region, bool p_with_wireframe); // build geometric model

	WaterRipples();
	~WaterRipples();
};

class Water2D : public Node2D {
	GDCLASS(Water2D, Node2D);

	Ref<WaterRipples<25>> water0;
	Ref<WaterRipples<30>> water1;
	Ref<WaterRipples<50>> water2;

	Ref<ShaderMaterial> material;
	RID mesh_item, caustics_item;
	Ref<ArrayMesh> mesh, mesh_caustics, mesh_wireframe;
	Ref<CanvasItemMaterial> material_caustics;
	bool active;
	bool wireframe;
	bool details_map;
	bool caustics;
	real_t caustics_speed_rate;
	real_t caustics_alpha;
	real_t wave_speed_rate;
	real_t details_speed_rate;
	int level_quality;

	Ref<Texture> texture_skin, texture_mask, texture_envmap; // main skin/bg, mask and environment map texture

	struct SequenceInfo {
		Vector<Ref<Texture>> frames;
		int current_frame = 0;
		real_t speed_alteration = 0;
		real_t progress = 0;
		bool next(real_t delta, real_t speed_rate) {
			progress += delta;
			if (progress > speed_rate) {
				current_frame = (++current_frame) % frames.size();
				progress = 0;
				return true;
			}
			return false;
		}
		_FORCE_INLINE_ Ref<Texture> texture() const { return frames[current_frame]; }
		_FORCE_INLINE_ bool is_valid() const { return frames.size() > 0 && texture().is_valid(); }
	} animation_details, animation_caustics; // additional animation layers

	enum {
		UPDATE_WATER = 1,
		UPDATE_DETAILS = 2,
		UPDATE_CAUSTICS = 4,
		UPDATE_ALL = UPDATE_WATER | UPDATE_DETAILS | UPDATE_CAUSTICS,
	};
	int _update_material;

protected:
	static void _bind_methods();
	void _notification(int p_notification);
	void _get_property_list(List<PropertyInfo> *p_list) const {
		if (p_list) {
			for (List<PropertyInfo>::Element *E = p_list->front(); E; E = E->next()) {
				PropertyInfo &prop = E->get();
				if (prop.name.to_lower() == "material" || prop.name.to_lower() == "use_parent_material") {
					prop.usage &= ~PROPERTY_USAGE_EDITOR;
					prop.usage |= PROPERTY_USAGE_NOEDITOR;
				}
			}
		}
	}
	void _changed_callback(Object *p_changed, const char *p_prop);

public:
#ifdef TOOLS_ENABLED
	Dictionary _edit_get_state() const;
	void _edit_set_state(const Dictionary &p_state);
	bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;
	Rect2 _edit_get_rect() const;
	void _edit_set_rect(const Rect2 &p_rect);
	bool _edit_use_rect() const;

	Size2 get_view_size() const {
		return get_scale() * 5;
	}
	void set_view_size(const Size2 &p_size) {
		set_scale(p_size / 5);
	}
#endif

	void set_active(bool p_state);
	bool is_active() const;

	void set_skin_texture(const Ref<Texture> &p_texture);
	Ref<Texture> get_skin_texture() const;
	void set_mask_texture(const Ref<Texture> &p_texture);
	Ref<Texture> get_mask_texture() const;

	void set_wireframe(bool p_state);
	bool is_wireframe() const;
	void set_details_map(bool p_state);
	bool is_details_map() const;
	void set_caustics(bool p_state);
	bool is_caustics() const;

	void set_wave_speed_rate(real_t p_rate);
	real_t get_wave_speed_rate() const;
	void set_wave_size_factor(int p_factor);
	int get_wave_size_factor() const;

	void set_caustics_grid_size(int p_num);
	int get_caustics_grid_size() const;
	void set_caustics_active_rate(real_t p_rate);
	real_t get_caustics_active_rate() const;
	void set_caustics_speed_rate(real_t p_rate);
	real_t get_caustics_speed_rate() const;
	void set_caustics_alpha(real_t p_alpha);
	real_t get_caustics_alpha() const;

	void set_wave(real_t p_x, real_t p_y, int p_amp);
	void run_wave(real_t p_phase, real_t p_cos, real_t p_sin, int p_amp);
	void random_wave();

	Water2D();
};

#endif /* GD_WATER_2D_H */
