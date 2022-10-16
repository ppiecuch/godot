/*************************************************************************/
/*  benchmark.h                                                          */
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

#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "core/map.h"
#include "core/method_bind.h"
#include "main/performance.h"
#include "scene/main/node.h"
#include "scene/resources/font.h"
#include "scene/resources/mesh.h"

class Label;
class GdHistoryPlot;

class Benchmark : public Node {
	GDCLASS(Benchmark, Node);

	Ref<BitmapFont> screen_font;
	Ref<SpatialMaterial> model_mat;
	int num_objects;
	int curr_model;
	real_t glob_yaw;

public:
	typedef enum {
		FIRST_MODEL_TYPE = 0,
		CUBE_MODEL = 0,
		FROG_MODEL,
		KID_MODEL,
		TREX_MODEL,
		ROBOT_MODEL,
		NUM_MODEL_TYPES
	} ModelType;

private:
	// Postprocess effects
	typedef enum {
		FX_GRAYSCALE = 0,
		FX_POSTERIZATION,
		FX_DREAM_VISION,
		FX_PIXELIZER,
		FX_CROSS_HATCHING,
		FX_CROSS_STITCHING,
		FX_PREDATOR_VIEW,
		FX_SCANLINES,
		FX_FISHEYE,
		FX_SOBEL,
		FX_BLOOM,
		FX_BLUR,
		FX_FXAA,
		NUM_FX_TYPES,
		FX_NONE = NUM_FX_TYPES,
	} PostproShader;

	// Shaders info.
	typedef enum {
		FIRST_SHADER_TYPE = 0,
		PHONG = 0,
		GOURAUD,
		UNTEXTURED_PHONG,
		UNTEXTURED_GOURAUD,
		FLAT,
		NUM_SHADER_TYPES,
	} ShaderType;

	// Render and display options
	typedef enum {
		OPTION_UNSHADED,
		OPTION_WIREFRAME,
		OPTION_CULLING,
		OPTION_DEPTHTEST,
		OPTION_MULTISAMPLING,
		OPTION_LINEAR_FILTERING,
		OPTION_MIPMAPING,
		OPTION_CAMERA_LOOKAWAY,
		OPTION_STATS,
		NUM_OPTIONS,
	} RenderOptions;

	bool opts[NUM_OPTIONS] = { false };
	real_t camera_distance, camera_yaw;

	// Models info
	struct ModelInfo {
		String name;
		Ref<Mesh> mesh;
		struct {
			const uint8_t *data;
			size_t size;
		} tex_data; // for texture rebuilding
		Ref<Texture> tex;
		uint32_t tris;
	};

	ModelInfo models[NUM_MODEL_TYPES];

	// instane & model assign
	Vector<std::pair<RID, int>> instances;

	RID _create_instance_from_model(const ModelInfo &info);
	int _get_texture_flags();
	ModelInfo _make_model_from_data(const uint8_t *p_data, const uint8_t *p_tex_img, size_t p_tex_img_size, const String &p_name);
	String _get_stats();
	void _load_resources();
	void _check_instances();
	void _update(float p_delta);
	void _finalize();

	Label *_stats_label;
	Size2 _stats_text_size;

	Map<Performance::Monitor, GdHistoryPlot *> _monitors;
	int plot_buf_index, plot_hist_size;
	bool _dirty;

protected:
	void _notification(int p_notification);
	void _input(const Ref<InputEvent> &p_event);
	static void _bind_methods();

public:
	void set_active(bool p_state);
	bool is_active() const;

	void set_num_objects(int p_num);
	int get_num_objects() const;

	void set_display_model(int p_model);
	int get_display_model() const;

	void set_option(int p_opts, bool p_state);
	bool get_option(int p_opts) const;

	Benchmark();
	~Benchmark();
};

VARIANT_ENUM_CAST(Benchmark::ModelType);

#endif // BENCHMARK_H
