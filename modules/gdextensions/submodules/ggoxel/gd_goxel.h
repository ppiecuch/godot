/**************************************************************************/
/*  gd_goxel.h                                                            */
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

#ifndef GD_GOXEL_H
#define GD_GOXEL_H

#include "core/image.h"
#include "core/pool_vector.h"
#include "core/reference.h"
#include "scene/3d/mesh_instance.h"
#include "scene/resources/mesh.h"

/* Forward-declare goxel C types — full headers included only in .cpp */
extern "C" {
typedef struct volume volume_t;
}

/* ----------------------------------------------------------------------- */
/*  GoxelVolume                                                            */
/* ----------------------------------------------------------------------- */

class GoxelVolume : public Reference {
	GDCLASS(GoxelVolume, Reference);

public:
	enum PaintMode {
		PAINT_OVER = 1,
		PAINT_SUB = 2,
		PAINT_PAINT = 4,
		PAINT_MAX = 5,
		PAINT_INTERSECT = 6,
	};

	enum Shape {
		SHAPE_SPHERE = 0,
		SHAPE_CUBE = 1,
		SHAPE_CYLINDER = 2,
		SHAPE_CONE = 3,
		SHAPE_TORUS = 4,
	};

private:
	volume_t *vol;

protected:
	static void _bind_methods();

public:
	GoxelVolume();
	~GoxelVolume();

	void set_voxel(Vector3 pos, Color color);
	Color get_voxel(Vector3 pos) const;

	void clear();
	bool is_empty() const;
	AABB get_bounding_box(bool exact = true) const;

	void blit(PoolByteArray data, Vector3 pos, Vector3 size);
	PoolByteArray read_region(Vector3 pos, Vector3 size) const;

	void paint_shape(int shape, Transform box, Color color, int mode);
	void merge(Ref<GoxelVolume> other, int mode);

	int get_key() const;
	int get_tile_count() const;

	/* Phase 2: volume operations. */
	void move(Transform transform);
	void crop(AABB box);
	Ref<GoxelVolume> select_connected(Vector3 pos, float threshold);

	/* Phase 2: quantization. */
	PoolColorArray generate_palette(int num_colors);
	void quantize(int num_colors);

	volume_t *get_volume() const { return vol; }
};

VARIANT_ENUM_CAST(GoxelVolume::PaintMode);
VARIANT_ENUM_CAST(GoxelVolume::Shape);

/* ----------------------------------------------------------------------- */
/*  GoxelMeshBuilder                                                       */
/* ----------------------------------------------------------------------- */

class GoxelMeshBuilder : public Reference {
	GDCLASS(GoxelMeshBuilder, Reference);

public:
	enum MeshMode {
		MESH_CUBES = 0,
		MESH_MARCHING_CUBES = 1,
		MESH_MC_SMOOTH = 2,
	};

private:
	MeshMode mesh_mode;
	bool include_ao;
	bool include_tangents;

protected:
	static void _bind_methods();

public:
	GoxelMeshBuilder();

	void set_mesh_mode(MeshMode p_mode);
	MeshMode get_mesh_mode() const;

	void set_include_ao(bool p_ao);
	bool get_include_ao() const;

	void set_include_tangents(bool p_tangents);
	bool get_include_tangents() const;

	Ref<ArrayMesh> build_mesh(Ref<GoxelVolume> volume) const;
};

VARIANT_ENUM_CAST(GoxelMeshBuilder::MeshMode);

/* ----------------------------------------------------------------------- */
/*  VoxelMeshInstance                                                       */
/* ----------------------------------------------------------------------- */

class VoxelMeshInstance : public MeshInstance {
	GDCLASS(VoxelMeshInstance, MeshInstance);

private:
	Ref<GoxelVolume> volume;
	Ref<GoxelMeshBuilder> mesh_builder;
	bool auto_rebuild;
	uint64_t last_key;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	VoxelMeshInstance();

	void set_volume(Ref<GoxelVolume> p_volume);
	Ref<GoxelVolume> get_volume() const;

	void set_mesh_builder(Ref<GoxelMeshBuilder> p_builder);
	Ref<GoxelMeshBuilder> get_mesh_builder() const;

	void set_auto_rebuild(bool p_auto);
	bool get_auto_rebuild() const;

	void rebuild_mesh();
	void set_voxel(Vector3 pos, Color color);
};

/* ----------------------------------------------------------------------- */
/*  GoxelPathTracer                                                        */
/* ----------------------------------------------------------------------- */

class GoxelPathTracer : public Reference {
	GDCLASS(GoxelPathTracer, Reference);

public:
	enum WorldType {
		WORLD_NONE = 0,
		WORLD_UNIFORM = 1,
		WORLD_SKY = 2,
	};

private:
	Ref<GoxelVolume> volume;
	int width;
	int height;
	int num_samples;
	int current_sample;
	int max_bounces;

	/* Camera. */
	Transform camera_transform;
	float camera_fov;
	bool camera_ortho;

	/* World / environment. */
	WorldType world_type;
	Color world_color;
	float world_energy;

	/* Floor. */
	bool floor_enabled;
	Color floor_color;
	float floor_height;

	/* Light (fill light). */
	Vector3 light_direction;
	float light_intensity;

	/* PBR material properties. */
	float metallic;
	float roughness;

	/* Sun light (separate from fill light). */
	Vector3 sun_direction;
	float sun_intensity;

	/* Sky model. */
	float sky_turbidity;

	/* Accumulation buffer (float RGBA per pixel). */
	float *accum;
	int accum_size;

	void _ensure_accum();
	void _render_sample(int sample_index);
	Color _trace_ray(const Vector3 &origin, const Vector3 &dir,
			int depth, uint32_t &rng) const;
	bool _intersect_volume(const Vector3 &origin, const Vector3 &dir,
			float max_dist, Vector3 &hit_pos, Vector3 &hit_normal,
			Color &hit_color) const;
	Color _sample_world(const Vector3 &dir) const;

protected:
	static void _bind_methods();

public:
	GoxelPathTracer();
	~GoxelPathTracer();

	void set_volume(Ref<GoxelVolume> p_volume);
	Ref<GoxelVolume> get_volume() const;

	void set_size(int w, int h);
	Vector2 get_size() const;

	void set_num_samples(int p_samples);
	int get_num_samples() const;

	void set_max_bounces(int p_bounces);
	int get_max_bounces() const;

	void set_camera_transform(Transform p_xform);
	Transform get_camera_transform() const;

	void set_camera_fov(float p_fov);
	float get_camera_fov() const;

	void set_camera_ortho(bool p_ortho);
	bool get_camera_ortho() const;

	void set_world_type(WorldType p_type);
	WorldType get_world_type() const;

	void set_world_color(Color p_color);
	Color get_world_color() const;

	void set_world_energy(float p_energy);
	float get_world_energy() const;

	void set_floor_enabled(bool p_enabled);
	bool get_floor_enabled() const;

	void set_floor_color(Color p_color);
	Color get_floor_color() const;

	void set_floor_height(float p_height);
	float get_floor_height() const;

	void set_light_direction(Vector3 p_dir);
	Vector3 get_light_direction() const;

	void set_light_intensity(float p_intensity);
	float get_light_intensity() const;

	/* PBR material. */
	void set_metallic(float p_metallic);
	float get_metallic() const;

	void set_roughness(float p_roughness);
	float get_roughness() const;

	/* Sun. */
	void set_sun_direction(Vector3 p_dir);
	Vector3 get_sun_direction() const;

	void set_sun_intensity(float p_intensity);
	float get_sun_intensity() const;

	/* Sky. */
	void set_sky_turbidity(float p_turbidity);
	float get_sky_turbidity() const;

	/* Rendering. */
	Ref<Image> render(); /* Blocking full render. */
	void start(); /* Reset accumulation. */
	bool iter(); /* Render one sample; returns true when done. */
	Ref<Image> get_image() const; /* Get current accumulated result. */
	int get_current_sample() const;
};

VARIANT_ENUM_CAST(GoxelPathTracer::WorldType);

/* ----------------------------------------------------------------------- */
/*  GoxelFileIO                                                            */
/* ----------------------------------------------------------------------- */

class GoxelFileIO : public Reference {
	GDCLASS(GoxelFileIO, Reference);

protected:
	static void _bind_methods();

public:
	/* VOX (MagicaVoxel) */
	Ref<GoxelVolume> import_vox(const String &path);
	Error export_vox(Ref<GoxelVolume> volume, const String &path);

	/* GOX (native goxel) */
	Ref<GoxelVolume> import_gox(const String &path);
	Error export_gox(Ref<GoxelVolume> volume, const String &path);

	/* Qubicle QB */
	Ref<GoxelVolume> import_qb(const String &path);
	Error export_qb(Ref<GoxelVolume> volume, const String &path);

	/* Text XYZ+RGB */
	Ref<GoxelVolume> import_txt(const String &path);
	Error export_txt(Ref<GoxelVolume> volume, const String &path);

	/* VXL (Ace of Spades) — import only */
	Ref<GoxelVolume> import_vxl(const String &path);

	/* KVX (Voxlap) — import only */
	Ref<GoxelVolume> import_kvx(const String &path);

	/* Wavefront OBJ — export only */
	Error export_obj(Ref<GoxelVolume> volume, const String &path);
};

/* ----------------------------------------------------------------------- */
/*  GoxelFilters                                                           */
/* ----------------------------------------------------------------------- */

class GoxelFilters : public Reference {
	GDCLASS(GoxelFilters, Reference);

protected:
	static void _bind_methods();

public:
	enum MirrorAxis {
		AXIS_X = 0,
		AXIS_Y = 1,
		AXIS_Z = 2,
	};

	void mirror(Ref<GoxelVolume> volume, int axis);
	void adjust_colors(Ref<GoxelVolume> volume, float hue,
			float saturation, float lightness, float contrast);
	void wrap(Ref<GoxelVolume> volume, int axis);
};

VARIANT_ENUM_CAST(GoxelFilters::MirrorAxis);

#endif // GD_GOXEL_H
