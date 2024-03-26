/**************************************************************************/
/*  mesh.h                                                                */
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

#ifndef MESH_H
#define MESH_H

#include "core/local_vector.h"
#include "core/math/face3.h"
#include "core/math/triangle_mesh.h"
#include "core/resource.h"
#include "scene/resources/material.h"
#include "scene/resources/shape.h"
#include "servers/visual_server.h"

class Mesh : public Resource {
	GDCLASS(Mesh, Resource);

	mutable Ref<TriangleMesh> triangle_mesh; //cached
	mutable Vector<Vector3> debug_lines;
	Size2 lightmap_size_hint;

protected:
	static void _bind_methods();

public:
	enum {

		NO_INDEX_ARRAY = VisualServer::NO_INDEX_ARRAY,
		ARRAY_WEIGHTS_SIZE = VisualServer::ARRAY_WEIGHTS_SIZE
	};

	enum ArrayType {

		ARRAY_VERTEX = VisualServer::ARRAY_VERTEX,
		ARRAY_NORMAL = VisualServer::ARRAY_NORMAL,
		ARRAY_TANGENT = VisualServer::ARRAY_TANGENT,
		ARRAY_COLOR = VisualServer::ARRAY_COLOR,
		ARRAY_TEX_UV = VisualServer::ARRAY_TEX_UV,
		ARRAY_TEX_UV2 = VisualServer::ARRAY_TEX_UV2,
		ARRAY_BONES = VisualServer::ARRAY_BONES,
		ARRAY_WEIGHTS = VisualServer::ARRAY_WEIGHTS,
		ARRAY_INDEX = VisualServer::ARRAY_INDEX,
		ARRAY_MAX = VisualServer::ARRAY_MAX

	};

	enum ArrayFormat {
		/* ARRAY FORMAT FLAGS */
		ARRAY_FORMAT_VERTEX = 1 << ARRAY_VERTEX, // mandatory
		ARRAY_FORMAT_NORMAL = 1 << ARRAY_NORMAL,
		ARRAY_FORMAT_TANGENT = 1 << ARRAY_TANGENT,
		ARRAY_FORMAT_COLOR = 1 << ARRAY_COLOR,
		ARRAY_FORMAT_TEX_UV = 1 << ARRAY_TEX_UV,
		ARRAY_FORMAT_TEX_UV2 = 1 << ARRAY_TEX_UV2,
		ARRAY_FORMAT_BONES = 1 << ARRAY_BONES,
		ARRAY_FORMAT_WEIGHTS = 1 << ARRAY_WEIGHTS,
		ARRAY_FORMAT_INDEX = 1 << ARRAY_INDEX,

		ARRAY_COMPRESS_BASE = (ARRAY_INDEX + 1),
		ARRAY_COMPRESS_VERTEX = 1 << (ARRAY_VERTEX + ARRAY_COMPRESS_BASE), // mandatory
		ARRAY_COMPRESS_NORMAL = 1 << (ARRAY_NORMAL + ARRAY_COMPRESS_BASE),
		ARRAY_COMPRESS_TANGENT = 1 << (ARRAY_TANGENT + ARRAY_COMPRESS_BASE),
		ARRAY_COMPRESS_COLOR = 1 << (ARRAY_COLOR + ARRAY_COMPRESS_BASE),
		ARRAY_COMPRESS_TEX_UV = 1 << (ARRAY_TEX_UV + ARRAY_COMPRESS_BASE),
		ARRAY_COMPRESS_TEX_UV2 = 1 << (ARRAY_TEX_UV2 + ARRAY_COMPRESS_BASE),
		ARRAY_COMPRESS_BONES = 1 << (ARRAY_BONES + ARRAY_COMPRESS_BASE),
		ARRAY_COMPRESS_WEIGHTS = 1 << (ARRAY_WEIGHTS + ARRAY_COMPRESS_BASE),
		ARRAY_COMPRESS_INDEX = 1 << (ARRAY_INDEX + ARRAY_COMPRESS_BASE),

		ARRAY_FLAG_USE_2D_VERTICES = ARRAY_COMPRESS_INDEX << 1,
		ARRAY_FLAG_USE_16_BIT_BONES = ARRAY_COMPRESS_INDEX << 2,
		ARRAY_FLAG_USE_DYNAMIC_UPDATE = ARRAY_COMPRESS_INDEX << 3,
		ARRAY_FLAG_USE_OCTAHEDRAL_COMPRESSION = ARRAY_COMPRESS_INDEX << 4,
		ARRAY_FLAG_USE_VERTEX_CACHE_OPTIMIZATION = ARRAY_COMPRESS_INDEX << 5,

		ARRAY_COMPRESS_DEFAULT = ARRAY_COMPRESS_NORMAL | ARRAY_COMPRESS_TANGENT | ARRAY_COMPRESS_COLOR | ARRAY_COMPRESS_TEX_UV | ARRAY_COMPRESS_TEX_UV2 | ARRAY_COMPRESS_WEIGHTS | ARRAY_FLAG_USE_OCTAHEDRAL_COMPRESSION

	};

	enum PrimitiveType {
		PRIMITIVE_POINTS = VisualServer::PRIMITIVE_POINTS,
		PRIMITIVE_LINES = VisualServer::PRIMITIVE_LINES,
		PRIMITIVE_LINE_STRIP = VisualServer::PRIMITIVE_LINE_STRIP,
		PRIMITIVE_LINE_LOOP = VisualServer::PRIMITIVE_LINE_LOOP,
		PRIMITIVE_TRIANGLES = VisualServer::PRIMITIVE_TRIANGLES,
		PRIMITIVE_TRIANGLE_STRIP = VisualServer::PRIMITIVE_TRIANGLE_STRIP,
		PRIMITIVE_TRIANGLE_FAN = VisualServer::PRIMITIVE_TRIANGLE_FAN,
	};

	enum BlendShapeMode {

		BLEND_SHAPE_MODE_NORMALIZED = VS::BLEND_SHAPE_MODE_NORMALIZED,
		BLEND_SHAPE_MODE_RELATIVE = VS::BLEND_SHAPE_MODE_RELATIVE,
	};

	enum StorageMode {
		STORAGE_MODE_GPU,
		STORAGE_MODE_CPU,
		STORAGE_MODE_CPU_AND_GPU,
	};

	virtual int get_surface_count() const = 0;
	virtual int surface_get_array_len(int p_idx) const = 0;
	virtual int surface_get_array_index_len(int p_idx) const = 0;
	virtual bool surface_is_softbody_friendly(int p_idx) const;
	virtual Array surface_get_arrays(int p_surface) const = 0;
	virtual Array surface_get_blend_shape_arrays(int p_surface) const = 0;
	virtual uint32_t surface_get_format(int p_idx) const = 0;
	virtual PrimitiveType surface_get_primitive_type(int p_idx) const = 0;
	virtual void surface_set_material(int p_idx, const Ref<Material> &p_material) = 0;
	virtual Ref<Material> surface_get_material(int p_idx) const = 0;
	virtual int get_blend_shape_count() const = 0;
	int surface_get_triangle_count(int p_idx) const;
	virtual StringName get_blend_shape_name(int p_index) const = 0;
	virtual void set_blend_shape_name(int p_index, const StringName &p_name) = 0;

	int get_triangle_count() const;
	PoolVector<Face3> get_faces() const;
	Ref<TriangleMesh> generate_triangle_mesh() const;
	Ref<TriangleMesh> generate_triangle_mesh_from_aabb() const;
	void generate_debug_mesh_lines(Vector<Vector3> &r_lines);
	void generate_debug_mesh_indices(Vector<Vector3> &r_points);

	Ref<Shape> create_trimesh_shape() const;
	Ref<Shape> create_convex_shape(bool p_clean = true, bool p_simplify = false) const;

	Ref<Mesh> create_outline(float p_margin) const;

	virtual AABB get_aabb() const = 0;
	virtual void set_storage_mode(StorageMode p_storage_mode);

	void set_lightmap_size_hint(const Vector2 &p_size);
	Size2 get_lightmap_size_hint() const;
	void clear_cache() const;

	typedef Vector<PoolVector<Vector3>> (*ConvexDecompositionFunc)(const real_t *p_vertices, int p_vertex_count, const uint32_t *p_triangles, int p_triangle_count, int p_max_convex_hulls, Vector<PoolVector<uint32_t>> *r_convex_indices);

	static ConvexDecompositionFunc convex_decomposition_function;

	Vector<Ref<Shape>> convex_decompose(int p_max_convex_hulls = -1) const;

	Mesh();
};

class ArrayMesh : public Mesh {
	GDCLASS(ArrayMesh, Mesh);
	RES_BASE_EXTENSION("mesh");

private:
	// Storing the mesh data on CPU
	struct CPUSurface {
		Array arrays;
		Array blend_shapes;
		PrimitiveType primitive_type;
		int num_verts = 0;
		int num_inds = 0;
	};

	struct Surface {
		String name;
		AABB aabb;
		Ref<Material> material;
		bool is_active;
		bool is_2d;
		// Watch for bugs here.
		// When calling add_surface() rather than add_surface_from_arrays(),
		// the creation flags will be unset, and left at default.
		// Conversion from CPU to GPU memory assumes that creation_flags are
		// correct, so is only TRULY safe when used with add_surface_from_arrays().
		uint32_t creation_flags = ARRAY_COMPRESS_DEFAULT;
		uint32_t creation_format = 0;
	};
	Vector<Surface> surfaces;
	LocalVector<CPUSurface *> _cpu_surfaces;
	RID mesh;
	AABB aabb;
	BlendShapeMode blend_shape_mode;
	Vector<StringName> blend_shapes;
	AABB custom_aabb;
	String comment;

	Ref<Mesh> _copy_surfaces(Ref<ArrayMesh> p_dest, int p_from, int p_num = 1);
	Ref<Mesh> _copy_surfaces(Ref<ArrayMesh> p_dest, LocalVector<int> p_surfs);
	void _recompute_aabb();
	void _update_submesh_info();

	bool submeshes_active, _submeshes_dirty;
	struct {
		LocalVector<LocalVector<int>> surfs;
		Map<String, int> names_map;
		void clear() { surfs.clear(), names_map.clear(); }
		void update_submesh(const String &p_name, int p_surf) {
			if (names_map.has(p_name)) {
				surfs[names_map[p_name]].push_back(p_surf);
			} else {
				names_map[p_name] = surfs.size();
				surfs.push_back(LocalVector<int>(1, &p_surf));
			}
		}
		_FORCE_INLINE_ int size() const { return surfs.size(); }
		_FORCE_INLINE_ bool has(const String &p_name) const { return names_map.has(p_name); }
		_FORCE_INLINE_ int get_submesh_index(const String &p_name) const { return names_map[p_name]; }
		_FORCE_INLINE_ const LocalVector<int> &operator[](int p_index) { return surfs[p_index]; }
		_FORCE_INLINE_ const LocalVector<int> &operator[](const String &p_name) { return surfs[names_map[p_name]]; }
	} submeshes;

	// Data can be held on GPU, CPU or both.
	// CPU is quicker for modifications, but can't be
	// used for rendering.
	bool _on_cpu = false;
	bool _on_gpu = true;
	StorageMode _storage_mode = STORAGE_MODE_GPU;

	void add_surface_from_arrays_cpu(PrimitiveType p_primitive, const Array &p_arrays, const Array &p_blend_shapes);
	void add_surface_from_arrays_cpu_with_probe(PrimitiveType p_primitive, const Array &p_arrays, const Array &p_blend_shapes, uint32_t p_flags, int p_surface_id);
	void clear_cpu_surfaces();
	bool on_cpu() const { return _on_cpu && ((int)_cpu_surfaces.size() == surfaces.size()); }

protected:
	virtual bool _is_generated() const { return false; }

	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	static void _bind_methods();

public:
	int get_submesh_count();
	Ref<Mesh> get_submesh_by_index(int p_idx);
	Ref<Mesh> get_submesh_with_name(const String &p_name);

	void enable_multimesh(bool p_state);
	bool is_multimesh() const;

	void set_comment(const String &p_comment);
	String get_comment() const;

	void add_surface_from_arrays(PrimitiveType p_primitive, const Array &p_arrays, const Array &p_blend_shapes = Array(), uint32_t p_flags = ARRAY_COMPRESS_DEFAULT);
	void add_surface(uint32_t p_format, PrimitiveType p_primitive, const PoolVector<uint8_t> &p_array, int p_vertex_count, const PoolVector<uint8_t> &p_index_array, int p_index_count, const AABB &p_aabb, const Vector<PoolVector<uint8_t>> &p_blend_shapes = Vector<PoolVector<uint8_t>>(), const Vector<AABB> &p_bone_aabbs = Vector<AABB>());

	Array surface_get_arrays(int p_surface) const G_OVERRIDE;
	Array surface_get_blend_shape_arrays(int p_surface) const G_OVERRIDE;

	void add_blend_shape(const StringName &p_name);
	int get_blend_shape_count() const G_OVERRIDE;
	StringName get_blend_shape_name(int p_index) const G_OVERRIDE;
	void set_blend_shape_name(int p_index, const StringName &p_name) G_OVERRIDE;
	void clear_blend_shapes();

	void set_blend_shape_mode(BlendShapeMode p_mode);
	BlendShapeMode get_blend_shape_mode() const;

	void surface_update_region(int p_surface, int p_offset, const PoolVector<uint8_t> &p_data);

	int get_surface_count() const G_OVERRIDE;
	void surface_remove(int p_idx);
	void clear_surfaces();

	void surface_set_active(int p_idx, bool p_active);
	bool surface_is_active(int p_idx) const;
	void surface_set_custom_aabb(int p_idx, const AABB &p_aabb); //only recognized by driver

	int surface_get_array_len(int p_idx) const G_OVERRIDE;
	int surface_get_array_index_len(int p_idx) const G_OVERRIDE;
	uint32_t surface_get_format(int p_idx) const G_OVERRIDE;
	PrimitiveType surface_get_primitive_type(int p_idx) const G_OVERRIDE;

	virtual void surface_set_material(int p_idx, const Ref<Material> &p_material) G_OVERRIDE;
	virtual Ref<Material> surface_get_material(int p_idx) const G_OVERRIDE;

	int surface_find_by_name(const String &p_name) const;
	void surface_set_name(int p_idx, const String &p_name);
	String surface_get_name(int p_idx) const;

	void add_surface_from_mesh_data(const Geometry::MeshData &p_mesh_data);

	void set_custom_aabb(const AABB &p_custom);
	AABB get_custom_aabb() const;

	AABB get_aabb() const G_OVERRIDE;
	virtual RID get_rid() const G_OVERRIDE;

	void regen_normalmaps();

	Error lightmap_unwrap(const Transform &p_base_transform = Transform(), float p_texel_size = 0.05);
	Error lightmap_unwrap_cached(int *&r_cache_data, unsigned int &r_cache_size, bool &r_used_cache, const Transform &p_base_transform = Transform(), float p_texel_size = 0.05);

	virtual void reload_from_file() G_OVERRIDE;
	void clear_mesh();

	virtual void set_storage_mode(StorageMode p_storage_mode);

	ArrayMesh();

	~ArrayMesh();
};

VARIANT_ENUM_CAST(Mesh::ArrayType);
VARIANT_ENUM_CAST(Mesh::ArrayFormat);
VARIANT_ENUM_CAST(Mesh::PrimitiveType);
VARIANT_ENUM_CAST(Mesh::BlendShapeMode);

#endif // MESH_H
