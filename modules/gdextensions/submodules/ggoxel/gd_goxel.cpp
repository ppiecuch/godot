/**************************************************************************/
/*  gd_goxel.cpp                                                          */
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

#include "gd_goxel.h"
#include "core/math/aabb.h"
#include "core/os/file_access.h"

extern "C" {
#include "goxel/goxel.h"
#include "goxel/palette.h"
#include "goxel/render.h"
#include "goxel/shape.h"
#include "goxel/volume.h"
#include "goxel/volume_utils.h"

/* Format functions from goxel_formats.c */
volume_t *goxel_import_vox(const uint8_t *data, int data_size);
int goxel_export_vox(const volume_t *volume, uint8_t **out_data, int *out_size);
volume_t *goxel_import_gox(const uint8_t *data, int data_size);
int goxel_export_gox(const volume_t *volume, uint8_t **out_data, int *out_size);
volume_t *goxel_import_qb(const uint8_t *data, int data_size);
int goxel_export_qb(const volume_t *volume, uint8_t **out_data, int *out_size);
volume_t *goxel_import_txt(const uint8_t *data, int data_size);
int goxel_export_txt(const volume_t *volume, uint8_t **out_data, int *out_size);
volume_t *goxel_import_vxl(const uint8_t *data, int data_size);
volume_t *goxel_import_kvx(const uint8_t *data, int data_size);
int goxel_export_obj(const volume_t *volume, char **out_data, int *out_size);
}

/* ======================================================================= */
/*  GoxelVolume                                                            */
/* ======================================================================= */

GoxelVolume::GoxelVolume() {
	vol = volume_new();
}

GoxelVolume::~GoxelVolume() {
	if (vol) {
		volume_delete(vol);
		vol = nullptr;
	}
}

void GoxelVolume::set_voxel(Vector3 pos, Color color) {
	int ipos[3] = { (int)pos.x, (int)pos.y, (int)pos.z };
	uint8_t v[4] = {
		(uint8_t)CLAMP(color.r * 255.0f, 0, 255),
		(uint8_t)CLAMP(color.g * 255.0f, 0, 255),
		(uint8_t)CLAMP(color.b * 255.0f, 0, 255),
		(uint8_t)CLAMP(color.a * 255.0f, 0, 255),
	};
	volume_set_at(vol, nullptr, ipos, v);
}

Color GoxelVolume::get_voxel(Vector3 pos) const {
	int ipos[3] = { (int)pos.x, (int)pos.y, (int)pos.z };
	uint8_t v[4];
	volume_get_at(vol, nullptr, ipos, v);
	return Color(v[0] / 255.0f, v[1] / 255.0f, v[2] / 255.0f, v[3] / 255.0f);
}

void GoxelVolume::clear() {
	volume_clear(vol);
}

bool GoxelVolume::is_empty() const {
	return volume_is_empty(vol);
}

AABB GoxelVolume::get_bounding_box(bool exact) const {
	int bbox[2][3];
	if (!volume_get_bbox(vol, bbox, exact)) {
		return AABB();
	}
	Vector3 origin(bbox[0][0], bbox[0][1], bbox[0][2]);
	Vector3 size(bbox[1][0] - bbox[0][0],
			bbox[1][1] - bbox[0][1],
			bbox[1][2] - bbox[0][2]);
	return AABB(origin, size);
}

void GoxelVolume::blit(PoolByteArray data, Vector3 pos, Vector3 size) {
	int w = (int)size.x, h = (int)size.y, d = (int)size.z;
	int expected = w * h * d * 4;
	ERR_FAIL_COND(data.size() < expected);
	PoolByteArray::Read r = data.read();
	volume_blit(vol, r.ptr(), (int)pos.x, (int)pos.y, (int)pos.z,
			w, h, d, nullptr);
}

PoolByteArray GoxelVolume::read_region(Vector3 pos, Vector3 size) const {
	int w = (int)size.x, h = (int)size.y, d = (int)size.z;
	int total = w * h * d * 4;
	PoolByteArray result;
	result.resize(total);
	{
		PoolByteArray::Write wr = result.write();
		int ipos[3] = { (int)pos.x, (int)pos.y, (int)pos.z };
		int isize[3] = { w, h, d };
		volume_read(vol, ipos, isize, wr.ptr());
	}
	return result;
}

void GoxelVolume::paint_shape(int shape, Transform box, Color color, int mode) {
	const shape_t *s = nullptr;
	switch (shape) {
		case SHAPE_SPHERE:
			s = &shape_sphere;
			break;
		case SHAPE_CUBE:
			s = &shape_cube;
			break;
		case SHAPE_CYLINDER:
			s = &shape_cylinder;
			break;
		case SHAPE_CONE:
			s = &shape_cone;
			break;
		case SHAPE_TORUS:
			s = &shape_torus;
			break;
		default:
			ERR_FAIL_MSG("Invalid shape index");
	}

	painter_t painter;
	memset(&painter, 0, sizeof(painter));
	painter.mode = mode;
	painter.shape = s;
	painter.color[0] = (uint8_t)CLAMP(color.r * 255.0f, 0, 255);
	painter.color[1] = (uint8_t)CLAMP(color.g * 255.0f, 0, 255);
	painter.color[2] = (uint8_t)CLAMP(color.b * 255.0f, 0, 255);
	painter.color[3] = (uint8_t)CLAMP(color.a * 255.0f, 0, 255);

	/* Convert Transform to float[4][4] column-major. */
	float fbox[4][4];
	for (int col = 0; col < 3; col++) {
		Vector3 c = box.basis.get_axis(col);
		fbox[col][0] = c.x;
		fbox[col][1] = c.y;
		fbox[col][2] = c.z;
		fbox[col][3] = 0;
	}
	fbox[3][0] = box.origin.x;
	fbox[3][1] = box.origin.y;
	fbox[3][2] = box.origin.z;
	fbox[3][3] = 1;

	volume_op(vol, &painter, fbox);
}

void GoxelVolume::merge(Ref<GoxelVolume> other, int mode) {
	ERR_FAIL_COND(other.is_null());
	volume_merge(vol, other->get_volume(), mode, nullptr);
}

int GoxelVolume::get_key() const {
	return (int)volume_get_key(vol);
}

int GoxelVolume::get_tile_count() const {
	return volume_get_tiles_count(vol);
}

/* Phase 2: volume operations. */

void GoxelVolume::move(Transform transform) {
	float mat[4][4];
	for (int col = 0; col < 3; col++) {
		Vector3 c = transform.basis.get_axis(col);
		mat[col][0] = c.x;
		mat[col][1] = c.y;
		mat[col][2] = c.z;
		mat[col][3] = 0;
	}
	mat[3][0] = transform.origin.x;
	mat[3][1] = transform.origin.y;
	mat[3][2] = transform.origin.z;
	mat[3][3] = 1;
	volume_move(vol, mat);
}

void GoxelVolume::crop(AABB box) {
	float fbox[4][4];
	memset(fbox, 0, sizeof(fbox));
	Vector3 center = box.position + box.size * 0.5f;
	Vector3 half = box.size * 0.5f;
	fbox[0][0] = half.x;
	fbox[1][1] = half.y;
	fbox[2][2] = half.z;
	fbox[3][0] = center.x;
	fbox[3][1] = center.y;
	fbox[3][2] = center.z;
	fbox[3][3] = 1;
	volume_crop(vol, fbox);
}

static int _select_color_cond(void *user, const volume_t *volume,
		const int base_pos[3], const int new_pos[3],
		volume_accessor_t *volume_accessor) {
	float threshold = *(float *)user;
	uint8_t base_v[4], new_v[4];
	volume_get_at(volume, volume_accessor, base_pos, base_v);
	volume_get_at(volume, volume_accessor, new_pos, new_v);
	if (new_v[3] == 0)
		return 0;
	/* Color distance in [0..1] range. */
	float dr = (float)(base_v[0] - new_v[0]) / 255.0f;
	float dg = (float)(base_v[1] - new_v[1]) / 255.0f;
	float db = (float)(base_v[2] - new_v[2]) / 255.0f;
	float dist = sqrtf(dr * dr + dg * dg + db * db);
	return dist <= threshold ? 1 : 0;
}

Ref<GoxelVolume> GoxelVolume::select_connected(Vector3 pos, float threshold) {
	Ref<GoxelVolume> result;
	result.instance();
	int ipos[3] = { (int)pos.x, (int)pos.y, (int)pos.z };
	volume_select(vol, ipos, _select_color_cond, &threshold,
			result->get_volume());
	return result;
}

/* Phase 2: quantization. */

PoolColorArray GoxelVolume::generate_palette(int num_colors) {
	PoolColorArray result;
	ERR_FAIL_COND_V(num_colors <= 0 || num_colors > 256, result);
	uint8_t(*pal)[4] = (uint8_t(*)[4])memalloc(num_colors * 4);
	memset(pal, 0, num_colors * 4);
	quantization_gen_palette(vol, num_colors, pal);
	result.resize(num_colors);
	{
		PoolColorArray::Write w = result.write();
		for (int i = 0; i < num_colors; i++) {
			w[i] = Color(pal[i][0] / 255.0f, pal[i][1] / 255.0f,
					pal[i][2] / 255.0f, pal[i][3] / 255.0f);
		}
	}
	memfree(pal);
	return result;
}

void GoxelVolume::quantize(int num_colors) {
	ERR_FAIL_COND(num_colors <= 0 || num_colors > 256);
	uint8_t(*pal)[4] = (uint8_t(*)[4])memalloc(num_colors * 4);
	memset(pal, 0, num_colors * 4);
	quantization_gen_palette(vol, num_colors, pal);

	/* Replace each voxel color with nearest palette color. */
	int bbox[2][3];
	if (!volume_get_bbox(vol, bbox, true)) {
		memfree(pal);
		return;
	}

	for (int z = bbox[0][2]; z < bbox[1][2]; z++)
		for (int y = bbox[0][1]; y < bbox[1][1]; y++)
			for (int x = bbox[0][0]; x < bbox[1][0]; x++) {
				int pos[3] = { x, y, z };
				uint8_t v[4];
				volume_get_at(vol, nullptr, pos, v);
				if (v[3] == 0)
					continue;

				int best = 0;
				int best_dist = INT32_MAX;
				for (int i = 0; i < num_colors; i++) {
					if (pal[i][3] == 0)
						continue;
					int dr = (int)v[0] - pal[i][0];
					int dg = (int)v[1] - pal[i][1];
					int db = (int)v[2] - pal[i][2];
					int d = dr * dr + dg * dg + db * db;
					if (d < best_dist) {
						best_dist = d;
						best = i;
					}
				}
				uint8_t nv[4] = { pal[best][0], pal[best][1],
					pal[best][2], v[3] };
				volume_set_at(vol, nullptr, pos, nv);
			}

	memfree(pal);
}

void GoxelVolume::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_voxel", "pos", "color"), &GoxelVolume::set_voxel);
	ClassDB::bind_method(D_METHOD("get_voxel", "pos"), &GoxelVolume::get_voxel);
	ClassDB::bind_method(D_METHOD("clear"), &GoxelVolume::clear);
	ClassDB::bind_method(D_METHOD("is_empty"), &GoxelVolume::is_empty);
	ClassDB::bind_method(D_METHOD("get_bounding_box", "exact"), &GoxelVolume::get_bounding_box, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("blit", "data", "pos", "size"), &GoxelVolume::blit);
	ClassDB::bind_method(D_METHOD("read_region", "pos", "size"), &GoxelVolume::read_region);
	ClassDB::bind_method(D_METHOD("paint_shape", "shape", "box", "color", "mode"), &GoxelVolume::paint_shape);
	ClassDB::bind_method(D_METHOD("merge", "other", "mode"), &GoxelVolume::merge);
	ClassDB::bind_method(D_METHOD("get_key"), &GoxelVolume::get_key);
	ClassDB::bind_method(D_METHOD("get_tile_count"), &GoxelVolume::get_tile_count);

	ClassDB::bind_method(D_METHOD("move", "transform"), &GoxelVolume::move);
	ClassDB::bind_method(D_METHOD("crop", "box"), &GoxelVolume::crop);
	ClassDB::bind_method(D_METHOD("select_connected", "pos", "threshold"), &GoxelVolume::select_connected);
	ClassDB::bind_method(D_METHOD("generate_palette", "num_colors"), &GoxelVolume::generate_palette);
	ClassDB::bind_method(D_METHOD("quantize", "num_colors"), &GoxelVolume::quantize);

	BIND_ENUM_CONSTANT(PAINT_OVER);
	BIND_ENUM_CONSTANT(PAINT_SUB);
	BIND_ENUM_CONSTANT(PAINT_PAINT);
	BIND_ENUM_CONSTANT(PAINT_MAX);
	BIND_ENUM_CONSTANT(PAINT_INTERSECT);

	BIND_ENUM_CONSTANT(SHAPE_SPHERE);
	BIND_ENUM_CONSTANT(SHAPE_CUBE);
	BIND_ENUM_CONSTANT(SHAPE_CYLINDER);
	BIND_ENUM_CONSTANT(SHAPE_CONE);
	BIND_ENUM_CONSTANT(SHAPE_TORUS);
}

/* ======================================================================= */
/*  GoxelMeshBuilder                                                       */
/* ======================================================================= */

GoxelMeshBuilder::GoxelMeshBuilder() :
		mesh_mode(MESH_MARCHING_CUBES),
		include_ao(false),
		include_tangents(false) {
}

void GoxelMeshBuilder::set_mesh_mode(MeshMode p_mode) {
	mesh_mode = p_mode;
}

GoxelMeshBuilder::MeshMode GoxelMeshBuilder::get_mesh_mode() const {
	return mesh_mode;
}

void GoxelMeshBuilder::set_include_ao(bool p_ao) {
	include_ao = p_ao;
}

bool GoxelMeshBuilder::get_include_ao() const {
	return include_ao;
}

void GoxelMeshBuilder::set_include_tangents(bool p_tangents) {
	include_tangents = p_tangents;
}

bool GoxelMeshBuilder::get_include_tangents() const {
	return include_tangents;
}

/* Build mesh directly from per-tile voxel_vertex_t data, preserving all
 * vertex attributes that volume_generate_mesh() would discard. */
Ref<ArrayMesh> GoxelMeshBuilder::build_mesh(Ref<GoxelVolume> volume) const {
	ERR_FAIL_COND_V(volume.is_null(), Ref<ArrayMesh>());
	ERR_FAIL_COND_V(volume_is_empty(volume->get_volume()), Ref<ArrayMesh>());

	int effects = 0;
	switch (mesh_mode) {
		case MESH_MARCHING_CUBES:
			effects = EFFECT_MARCHING_CUBES;
			break;
		case MESH_MC_SMOOTH:
			effects = EFFECT_MARCHING_CUBES | EFFECT_MC_SMOOTH;
			break;
		case MESH_CUBES:
		default:
			effects = 0;
			break;
	}

	/* Collect raw voxel vertices from all tiles. */
	const int MAX_VERTS_PER_TILE = TILE_SIZE * TILE_SIZE * TILE_SIZE * 6 * 4;
	voxel_vertex_t *tile_verts = (voxel_vertex_t *)memalloc(
			MAX_VERTS_PER_TILE * sizeof(voxel_vertex_t));

	/* Dynamic arrays for accumulated mesh data. */
	Vector<Vector3> all_positions;
	Vector<Vector3> all_normals;
	Vector<Color> all_colors;
	Vector<float> all_tangents; /* packed as x,y,z,w per vertex */
	Vector<Vector2> all_uv2; /* AO + bump packed into UV2 */
	Vector<int> all_indices;

	volume_iterator_t it = volume_get_iterator(volume->get_volume(),
			VOLUME_ITER_TILES | VOLUME_ITER_SKIP_EMPTY);
	int tpos[3];
	while (volume_iter(&it, tpos)) {
		int prim_size = 0, subdivide = 0;
		int nb = volume_generate_vertices(volume->get_volume(), tpos,
				effects, tile_verts, &prim_size, &subdivide);
		if (nb <= 0)
			continue;

		float scale = 1.0f / (float)subdivide;
		int total_verts = nb * prim_size;
		int base = all_positions.size();

		/* Append vertex data. */
		for (int i = 0; i < total_verts; i++) {
			const voxel_vertex_t &vv = tile_verts[i];
			all_positions.push_back(Vector3(
					tpos[0] + vv.pos[0] * scale,
					tpos[1] + vv.pos[1] * scale,
					tpos[2] + vv.pos[2] * scale));
			all_normals.push_back(Vector3(
					vv.normal[0] / 127.0f,
					vv.normal[1] / 127.0f,
					vv.normal[2] / 127.0f));
			all_colors.push_back(Color(
					vv.color[0] / 255.0f,
					vv.color[1] / 255.0f,
					vv.color[2] / 255.0f,
					vv.color[3] / 255.0f));

			if (include_tangents) {
				/* tangent[3] + bitangent sign (w=1.0) */
				all_tangents.push_back(vv.tangent[0] / 127.0f);
				all_tangents.push_back(vv.tangent[1] / 127.0f);
				all_tangents.push_back(vv.tangent[2] / 127.0f);
				all_tangents.push_back(1.0f);
			}

			if (include_ao) {
				/* Pack occlusion UV and bump UV into UV2 channel.
				 * AO is typically looked up from a texture atlas at
				 * occlusion_uv; here we pass normalised coordinates
				 * that a Godot shader can use directly. */
				float ao_u = (vv.occlusion_uv[0] + 0.5f) /
						(16.0f * 8.0f); /* VOXEL_TEXTURE_SIZE=8 */
				float ao_v = (vv.occlusion_uv[1] + 0.5f) /
						(16.0f * 8.0f);
				all_uv2.push_back(Vector2(ao_u, ao_v));
			}
		}

		/* Build indices — triangulate quads if needed. */
		if (prim_size == 3) {
			for (int i = 0; i < total_verts; i++) {
				all_indices.push_back(base + i);
			}
		} else {
			/* Quads → 2 triangles each. */
			for (int q = 0; q < nb; q++) {
				int qbase = base + q * 4;
				all_indices.push_back(qbase + 0);
				all_indices.push_back(qbase + 1);
				all_indices.push_back(qbase + 2);
				all_indices.push_back(qbase + 0);
				all_indices.push_back(qbase + 2);
				all_indices.push_back(qbase + 3);
			}
		}
	}

	memfree(tile_verts);

	if (all_positions.empty()) {
		return Ref<ArrayMesh>();
	}

	/* Convert to pool arrays for ArrayMesh. */
	int vc = all_positions.size();
	int ic = all_indices.size();

	PoolVector3Array vertices;
	PoolVector3Array normals;
	PoolColorArray colors;
	PoolIntArray indices;

	vertices.resize(vc);
	normals.resize(vc);
	colors.resize(vc);
	indices.resize(ic);

	{
		PoolVector3Array::Write vw = vertices.write();
		PoolVector3Array::Write nw = normals.write();
		PoolColorArray::Write cw = colors.write();
		for (int i = 0; i < vc; i++) {
			vw[i] = all_positions[i];
			nw[i] = all_normals[i];
			cw[i] = all_colors[i];
		}
	}
	{
		PoolIntArray::Write iw = indices.write();
		for (int i = 0; i < ic; i++) {
			iw[i] = all_indices[i];
		}
	}

	Array arrays;
	arrays.resize(Mesh::ARRAY_MAX);
	arrays[Mesh::ARRAY_VERTEX] = vertices;
	arrays[Mesh::ARRAY_NORMAL] = normals;
	arrays[Mesh::ARRAY_COLOR] = colors;
	arrays[Mesh::ARRAY_INDEX] = indices;

	if (include_tangents && all_tangents.size() == vc * 4) {
		PoolRealArray tangents;
		tangents.resize(vc * 4);
		{
			PoolRealArray::Write tw = tangents.write();
			for (int i = 0; i < vc * 4; i++) {
				tw[i] = all_tangents[i];
			}
		}
		arrays[Mesh::ARRAY_TANGENT] = tangents;
	}

	if (include_ao && all_uv2.size() == vc) {
		PoolVector2Array uv2;
		uv2.resize(vc);
		{
			PoolVector2Array::Write uw = uv2.write();
			for (int i = 0; i < vc; i++) {
				uw[i] = all_uv2[i];
			}
		}
		arrays[Mesh::ARRAY_TEX_UV2] = uv2;
	}

	Ref<ArrayMesh> mesh;
	mesh.instance();
	mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

	return mesh;
}

void GoxelMeshBuilder::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_mesh_mode", "mode"), &GoxelMeshBuilder::set_mesh_mode);
	ClassDB::bind_method(D_METHOD("get_mesh_mode"), &GoxelMeshBuilder::get_mesh_mode);
	ClassDB::bind_method(D_METHOD("set_include_ao", "ao"), &GoxelMeshBuilder::set_include_ao);
	ClassDB::bind_method(D_METHOD("get_include_ao"), &GoxelMeshBuilder::get_include_ao);
	ClassDB::bind_method(D_METHOD("set_include_tangents", "tangents"), &GoxelMeshBuilder::set_include_tangents);
	ClassDB::bind_method(D_METHOD("get_include_tangents"), &GoxelMeshBuilder::get_include_tangents);
	ClassDB::bind_method(D_METHOD("build_mesh", "volume"), &GoxelMeshBuilder::build_mesh);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "mesh_mode", PROPERTY_HINT_ENUM, "Cubes,MarchingCubes,MCSmooth"), "set_mesh_mode", "get_mesh_mode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "include_ao"), "set_include_ao", "get_include_ao");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "include_tangents"), "set_include_tangents", "get_include_tangents");

	BIND_ENUM_CONSTANT(MESH_CUBES);
	BIND_ENUM_CONSTANT(MESH_MARCHING_CUBES);
	BIND_ENUM_CONSTANT(MESH_MC_SMOOTH);
}

/* ======================================================================= */
/*  VoxelMeshInstance                                                       */
/* ======================================================================= */

VoxelMeshInstance::VoxelMeshInstance() :
		auto_rebuild(true),
		last_key(0) {
}

void VoxelMeshInstance::set_volume(Ref<GoxelVolume> p_volume) {
	volume = p_volume;
	last_key = 0;
}

Ref<GoxelVolume> VoxelMeshInstance::get_volume() const {
	return volume;
}

void VoxelMeshInstance::set_mesh_builder(Ref<GoxelMeshBuilder> p_builder) {
	mesh_builder = p_builder;
}

Ref<GoxelMeshBuilder> VoxelMeshInstance::get_mesh_builder() const {
	return mesh_builder;
}

void VoxelMeshInstance::set_auto_rebuild(bool p_auto) {
	auto_rebuild = p_auto;
	if (auto_rebuild) {
		set_process_internal(true);
	}
}

bool VoxelMeshInstance::get_auto_rebuild() const {
	return auto_rebuild;
}

void VoxelMeshInstance::rebuild_mesh() {
	if (volume.is_null() || mesh_builder.is_null())
		return;
	Ref<ArrayMesh> m = mesh_builder->build_mesh(volume);
	set_mesh(m);
	if (volume.is_valid()) {
		last_key = volume_get_key(volume->get_volume());
	}
}

void VoxelMeshInstance::set_voxel(Vector3 pos, Color color) {
	if (volume.is_null()) {
		volume.instance();
	}
	volume->set_voxel(pos, color);
}

void VoxelMeshInstance::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY:
			if (auto_rebuild) {
				set_process_internal(true);
			}
			break;
		case NOTIFICATION_INTERNAL_PROCESS:
			if (auto_rebuild && volume.is_valid()) {
				uint64_t key = volume_get_key(volume->get_volume());
				if (key != last_key) {
					rebuild_mesh();
				}
			}
			break;
	}
}

void VoxelMeshInstance::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_volume", "volume"), &VoxelMeshInstance::set_volume);
	ClassDB::bind_method(D_METHOD("get_volume"), &VoxelMeshInstance::get_volume);
	ClassDB::bind_method(D_METHOD("set_mesh_builder", "builder"), &VoxelMeshInstance::set_mesh_builder);
	ClassDB::bind_method(D_METHOD("get_mesh_builder"), &VoxelMeshInstance::get_mesh_builder);
	ClassDB::bind_method(D_METHOD("set_auto_rebuild", "auto_rebuild"), &VoxelMeshInstance::set_auto_rebuild);
	ClassDB::bind_method(D_METHOD("get_auto_rebuild"), &VoxelMeshInstance::get_auto_rebuild);
	ClassDB::bind_method(D_METHOD("rebuild_mesh"), &VoxelMeshInstance::rebuild_mesh);
	ClassDB::bind_method(D_METHOD("set_voxel", "pos", "color"), &VoxelMeshInstance::set_voxel);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "volume", PROPERTY_HINT_RESOURCE_TYPE, "GoxelVolume"), "set_volume", "get_volume");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh_builder", PROPERTY_HINT_RESOURCE_TYPE, "GoxelMeshBuilder"), "set_mesh_builder", "get_mesh_builder");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_rebuild"), "set_auto_rebuild", "get_auto_rebuild");
}

/* ======================================================================= */
/*  GoxelPathTracer — PBR Voxel Path Tracer                                */
/* ======================================================================= */

/* Simple xorshift RNG for per-pixel randomness. */
static inline uint32_t pt_rng_next(uint32_t &state) {
	state ^= state << 13;
	state ^= state >> 17;
	state ^= state << 5;
	return state;
}

static inline float pt_rng_float(uint32_t &state) {
	return (pt_rng_next(state) & 0xFFFFFF) / (float)0x1000000;
}

/* sRGB ↔ linear conversion (ported from volume.glsl). */
static inline float srgb_to_linear(float c) {
	return c <= 0.04045f ? c / 12.92f : powf((c + 0.055f) / 1.055f, 2.4f);
}

static inline float linear_to_srgb(float c) {
	return c <= 0.0031308f ? c * 12.92f : 1.055f * powf(c, 1.0f / 2.4f) - 0.055f;
}

static inline Color color_to_linear(const Color &c) {
	return Color(srgb_to_linear(c.r), srgb_to_linear(c.g),
			srgb_to_linear(c.b), c.a);
}

/* Cosine-weighted hemisphere sample. */
static Vector3 pt_cosine_hemisphere(const Vector3 &normal, uint32_t &rng) {
	float u1 = pt_rng_float(rng);
	float u2 = pt_rng_float(rng);
	float r = sqrtf(u1);
	float phi = 2.0f * (float)Math_PI * u2;
	Vector3 d(cosf(phi) * r, sinf(phi) * r, sqrtf(1.0f - u1));

	/* Build tangent frame from normal. */
	Vector3 up = fabsf(normal.y) < 0.999f ? Vector3(0, 1, 0) : Vector3(1, 0, 0);
	Vector3 t = normal.cross(up).normalized();
	Vector3 b = normal.cross(t);
	return (t * d.x + b * d.y + normal * d.z).normalized();
}

/* GGX/Trowbridge-Reitz NDF. */
static inline float D_GGX(float NdotH, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float denom = NdotH * NdotH * (a2 - 1.0f) + 1.0f;
	return a2 / ((float)Math_PI * denom * denom + 1e-7f);
}

/* Schlick fresnel approximation. */
static inline float F_Schlick(float cosTheta, float F0) {
	float t = 1.0f - cosTheta;
	float t2 = t * t;
	return F0 + (1.0f - F0) * t2 * t2 * t;
}

/* Smith G1 term for GGX. */
static inline float G1_Smith(float NdotV, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float denom = NdotV + sqrtf(a2 + (1.0f - a2) * NdotV * NdotV);
	return 2.0f * NdotV / (denom + 1e-7f);
}

/* Smith geometry function: product of G1 for view and light. */
static inline float G_Smith(float NdotV, float NdotL, float roughness) {
	return G1_Smith(NdotV, roughness) * G1_Smith(NdotL, roughness);
}

/* Sample GGX visible normal distribution (importance sampling BRDF). */
static Vector3 pt_sample_ggx(const Vector3 &normal, float roughness,
		uint32_t &rng) {
	float u1 = pt_rng_float(rng);
	float u2 = pt_rng_float(rng);
	float a = roughness * roughness;
	float theta = acosf(sqrtf((1.0f - u1) / (u1 * (a * a - 1.0f) + 1.0f)));
	float phi = 2.0f * (float)Math_PI * u2;

	float st = sinf(theta);
	Vector3 h(cosf(phi) * st, sinf(phi) * st, cosf(theta));

	Vector3 up = fabsf(normal.y) < 0.999f ? Vector3(0, 1, 0) : Vector3(1, 0, 0);
	Vector3 t = normal.cross(up).normalized();
	Vector3 b = normal.cross(t);
	return (t * h.x + b * h.y + normal * h.z).normalized();
}

GoxelPathTracer::GoxelPathTracer() {
	width = 256;
	height = 256;
	num_samples = 16;
	current_sample = 0;
	max_bounces = 2;

	camera_fov = 50.0f;
	camera_ortho = false;

	world_type = WORLD_SKY;
	world_color = Color(0.5f, 0.6f, 0.9f);
	world_energy = 1.0f;

	floor_enabled = false;
	floor_color = Color(0.8f, 0.8f, 0.8f);
	floor_height = 0.0f;

	light_direction = Vector3(0.5f, 1.0f, 0.3f).normalized();
	light_intensity = 1.5f;

	metallic = 0.2f;
	roughness = 0.5f;

	sun_direction = Vector3(0.5f, 0.8f, 0.3f).normalized();
	sun_intensity = 2.0f;

	sky_turbidity = 3.0f;

	accum = nullptr;
	accum_size = 0;
}

GoxelPathTracer::~GoxelPathTracer() {
	if (accum) {
		memfree(accum);
	}
}

void GoxelPathTracer::_ensure_accum() {
	int needed = width * height * 4;
	if (accum_size != needed) {
		if (accum)
			memfree(accum);
		accum = (float *)memalloc(needed * sizeof(float));
		accum_size = needed;
		memset(accum, 0, needed * sizeof(float));
	}
}

/* DDA ray-voxel intersection.
 * Walks through voxel grid, testing each cell for non-zero alpha. */
bool GoxelPathTracer::_intersect_volume(const Vector3 &origin,
		const Vector3 &dir, float max_dist,
		Vector3 &hit_pos, Vector3 &hit_normal, Color &hit_color) const {
	if (volume.is_null() || volume->is_empty())
		return false;

	AABB bbox = volume->get_bounding_box(false);
	if (bbox.size.length_squared() < 0.001f)
		return false;

	/* Expand bbox slightly to avoid edge cases. */
	bbox = bbox.grow(0.01f);

	/* Ray-AABB intersection to find entry point. */
	float tmin = 0, tmax = max_dist;
	for (int i = 0; i < 3; i++) {
		float bmin = bbox.position[i];
		float bmax = bbox.position[i] + bbox.size[i];
		float inv_d = 1.0f / (fabsf(dir[i]) > 1e-8f ? dir[i] : 1e-8f);
		float t1 = (bmin - origin[i]) * inv_d;
		float t2 = (bmax - origin[i]) * inv_d;
		if (t1 > t2) {
			float tmp = t1;
			t1 = t2;
			t2 = tmp;
		}
		tmin = t1 > tmin ? t1 : tmin;
		tmax = t2 < tmax ? t2 : tmax;
	}
	if (tmin > tmax || tmax < 0)
		return false;

	/* Start DDA from entry point. */
	float t = tmin > 0 ? tmin + 0.001f : 0.001f;
	Vector3 p = origin + dir * t;

	/* Step through voxels using DDA. */
	int ix = (int)floorf(p.x);
	int iy = (int)floorf(p.y);
	int iz = (int)floorf(p.z);

	int step_x = dir.x >= 0 ? 1 : -1;
	int step_y = dir.y >= 0 ? 1 : -1;
	int step_z = dir.z >= 0 ? 1 : -1;

	float inv_dx = fabsf(dir.x) > 1e-8f ? 1.0f / fabsf(dir.x) : 1e8f;
	float inv_dy = fabsf(dir.y) > 1e-8f ? 1.0f / fabsf(dir.y) : 1e8f;
	float inv_dz = fabsf(dir.z) > 1e-8f ? 1.0f / fabsf(dir.z) : 1e8f;

	float t_max_x = ((step_x > 0 ? ix + 1.0f : (float)ix) - p.x) /
			(fabsf(dir.x) > 1e-8f ? dir.x : 1e-8f * step_x);
	float t_max_y = ((step_y > 0 ? iy + 1.0f : (float)iy) - p.y) /
			(fabsf(dir.y) > 1e-8f ? dir.y : 1e-8f * step_y);
	float t_max_z = ((step_z > 0 ? iz + 1.0f : (float)iz) - p.z) /
			(fabsf(dir.z) > 1e-8f ? dir.z : 1e-8f * step_z);

	int bmin_x = (int)floorf(bbox.position.x);
	int bmin_y = (int)floorf(bbox.position.y);
	int bmin_z = (int)floorf(bbox.position.z);
	int bmax_x = (int)ceilf(bbox.position.x + bbox.size.x);
	int bmax_y = (int)ceilf(bbox.position.y + bbox.size.y);
	int bmax_z = (int)ceilf(bbox.position.z + bbox.size.z);

	/* Walk the grid. */
	int last_axis = -1;
	for (int steps = 0; steps < 512; steps++) {
		if (ix < bmin_x || ix >= bmax_x ||
				iy < bmin_y || iy >= bmax_y ||
				iz < bmin_z || iz >= bmax_z)
			break;

		/* Check voxel. */
		Color c = volume->get_voxel(Vector3(ix, iy, iz));
		if (c.a > 0.5f) {
			hit_color = c;
			hit_pos = Vector3(ix + 0.5f, iy + 0.5f, iz + 0.5f);
			/* Normal from entry face. */
			hit_normal = Vector3(0, 0, 0);
			if (last_axis == 0)
				hit_normal.x = -step_x;
			else if (last_axis == 1)
				hit_normal.y = -step_y;
			else if (last_axis == 2)
				hit_normal.z = -step_z;
			else
				hit_normal = -dir.normalized(); /* first step */
			return true;
		}

		/* Advance to next voxel. */
		if (t_max_x < t_max_y) {
			if (t_max_x < t_max_z) {
				ix += step_x;
				t_max_x += inv_dx;
				last_axis = 0;
			} else {
				iz += step_z;
				t_max_z += inv_dz;
				last_axis = 2;
			}
		} else {
			if (t_max_y < t_max_z) {
				iy += step_y;
				t_max_y += inv_dy;
				last_axis = 1;
			} else {
				iz += step_z;
				t_max_z += inv_dz;
				last_axis = 2;
			}
		}
	}
	return false;
}

Color GoxelPathTracer::_sample_world(const Vector3 &dir) const {
	switch (world_type) {
		case WORLD_UNIFORM:
			return world_color * world_energy;
		case WORLD_SKY: {
			/* Perez/Hosek-inspired sky model with sun disk. */
			float cos_theta = CLAMP(dir.y, 0.0f, 1.0f);
			float theta = acosf(cos_theta);

			/* Turbidity-based sky color gradient. */
			float t_factor = 1.0f / (sky_turbidity * 0.5f + 0.5f);
			Color zenith = world_color * t_factor;
			Color horizon(0.8f, 0.85f, 0.9f);
			float h = powf(1.0f - cos_theta, 3.0f);
			Color sky = zenith.linear_interpolate(horizon, h);

			/* Sun disk. */
			float cos_sun = dir.dot(sun_direction);
			if (cos_sun > 0.9995f) {
				/* Inside sun disk — bright. */
				float sun_pow = powf((cos_sun - 0.9995f) / 0.0005f, 2.0f);
				Color sun_col(1.0f, 0.95f, 0.85f);
				sky = sky + sun_col * sun_pow * sun_intensity;
			} else if (cos_sun > 0.99f) {
				/* Sun corona/glow. */
				float glow = powf((cos_sun - 0.99f) / 0.0095f, 4.0f);
				Color glow_col(1.0f, 0.9f, 0.7f);
				sky = sky + glow_col * glow * sun_intensity * 0.3f;
			}

			/* Atmospheric scattering near horizon. */
			float scatter = powf(1.0f - cos_theta, 5.0f) * 0.3f;
			sky = sky + Color(scatter, scatter * 0.5f, 0.0f);

			(void)theta;
			return sky * world_energy;
		}
		default:
			return Color(0, 0, 0);
	}
}

Color GoxelPathTracer::_trace_ray(const Vector3 &origin, const Vector3 &dir,
		int depth, uint32_t &rng) const {
	Vector3 hit_pos, hit_normal;
	Color hit_color;

	/* Floor intersection. */
	bool hit_floor = false;
	float floor_t = 1e9f;
	Vector3 floor_hit_pos;
	if (floor_enabled && fabsf(dir.y) > 1e-6f) {
		float t = (floor_height - origin.y) / dir.y;
		if (t > 0.001f && t < 1000.0f) {
			floor_t = t;
			floor_hit_pos = origin + dir * t;
			hit_floor = true;
		}
	}

	bool hit_voxel = _intersect_volume(origin, dir, 1000.0f,
			hit_pos, hit_normal, hit_color);

	/* Determine closest hit. */
	float voxel_dist = hit_voxel ? (hit_pos - origin).length() : 1e9f;

	if (!hit_voxel && !hit_floor) {
		return _sample_world(dir);
	}

	Vector3 surface_pos;
	Vector3 surface_normal;
	Color surface_color;
	float mat_metallic = 0.0f;
	float mat_roughness = 0.5f;

	if (hit_floor && floor_t < voxel_dist) {
		surface_pos = floor_hit_pos;
		surface_normal = Vector3(0, 1, 0);
		surface_color = floor_color;
		/* Checkerboard pattern. */
		int cx = (int)floorf(floor_hit_pos.x);
		int cz = (int)floorf(floor_hit_pos.z);
		if ((cx + cz) & 1) {
			surface_color = surface_color * 0.7f;
		}
		mat_metallic = 0.0f;
		mat_roughness = 0.8f;
	} else {
		surface_pos = hit_pos + hit_normal * 0.01f;
		surface_normal = hit_normal;
		/* Convert sRGB voxel color to linear. */
		surface_color = color_to_linear(hit_color);
		mat_metallic = metallic;
		mat_roughness = roughness;
	}

	/* Clamp roughness to avoid singularities. */
	mat_roughness = CLAMP(mat_roughness, 0.04f, 1.0f);

	Vector3 V = -dir;
	float NdotV = MAX(0.001f, surface_normal.dot(V));

	/* F0: dielectric=0.04, metallic=albedo. */
	Color albedo = surface_color;
	float F0_r = 0.04f * (1.0f - mat_metallic) + albedo.r * mat_metallic;
	float F0_g = 0.04f * (1.0f - mat_metallic) + albedo.g * mat_metallic;
	float F0_b = 0.04f * (1.0f - mat_metallic) + albedo.b * mat_metallic;

	/* ---- Direct lighting with Cook-Torrance ---- */
	Color direct(0, 0, 0);

	/* Evaluate two lights: sun and fill light. */
	struct LightSample {
		Vector3 L;
		float intensity;
		float angular_size;
	};
	LightSample lights[2] = {
		{ sun_direction, sun_intensity, 0.01f },
		{ light_direction, light_intensity, 0.02f },
	};

	for (int li = 0; li < 2; li++) {
		Vector3 L = lights[li].L;

		/* Soft shadows: jitter light direction. */
		if (lights[li].angular_size > 0) {
			float jx = (pt_rng_float(rng) - 0.5f) * lights[li].angular_size;
			float jy = (pt_rng_float(rng) - 0.5f) * lights[li].angular_size;
			Vector3 up_l = fabsf(L.y) < 0.999f ? Vector3(0, 1, 0) : Vector3(1, 0, 0);
			Vector3 tl = L.cross(up_l).normalized();
			Vector3 bl = L.cross(tl);
			L = (L + tl * jx + bl * jy).normalized();
		}

		float NdotL = MAX(0.0f, surface_normal.dot(L));
		if (NdotL <= 0.0f)
			continue;

		/* Shadow ray. */
		Vector3 shadow_pos, shadow_normal;
		Color shadow_color;
		bool in_shadow = _intersect_volume(
				surface_pos + surface_normal * 0.02f,
				L, 1000.0f, shadow_pos, shadow_normal, shadow_color);
		if (in_shadow)
			continue;

		/* Cook-Torrance BRDF. */
		Vector3 H = (V + L).normalized();
		float NdotH = MAX(0.0f, surface_normal.dot(H));
		float VdotH = MAX(0.0f, V.dot(H));

		float D = D_GGX(NdotH, mat_roughness);
		float G = G_Smith(NdotV, NdotL, mat_roughness);
		float Fr = F_Schlick(VdotH, F0_r);
		float Fg = F_Schlick(VdotH, F0_g);
		float Fb = F_Schlick(VdotH, F0_b);

		float spec_denom = 4.0f * NdotV * NdotL + 0.001f;
		float spec_common = D * G / spec_denom;

		/* Diffuse: Lambert, energy-conserving. */
		float kD_r = (1.0f - Fr) * (1.0f - mat_metallic);
		float kD_g = (1.0f - Fg) * (1.0f - mat_metallic);
		float kD_b = (1.0f - Fb) * (1.0f - mat_metallic);

		float Li = lights[li].intensity * NdotL;
		direct.r += (kD_r * albedo.r / (float)Math_PI + Fr * spec_common) * Li;
		direct.g += (kD_g * albedo.g / (float)Math_PI + Fg * spec_common) * Li;
		direct.b += (kD_b * albedo.b / (float)Math_PI + Fb * spec_common) * Li;
	}

	/* ---- Ambient occlusion (short-range hemisphere sampling) ---- */
	float ao = 1.0f;
	{
		const int AO_SAMPLES = 4;
		int occluded = 0;
		for (int i = 0; i < AO_SAMPLES; i++) {
			Vector3 ao_dir = pt_cosine_hemisphere(surface_normal, rng);
			Vector3 ao_hit, ao_norm;
			Color ao_col;
			if (_intersect_volume(surface_pos + surface_normal * 0.02f,
						ao_dir, 3.0f, ao_hit, ao_norm, ao_col)) {
				occluded++;
			}
		}
		ao = 1.0f - (float)occluded / AO_SAMPLES * 0.7f;
	}

	/* Ambient from sky. */
	Color ambient = Color(
			albedo.r * world_energy * 0.1f * ao,
			albedo.g * world_energy * 0.1f * ao,
			albedo.b * world_energy * 0.1f * ao);

	Color result = Color(
			direct.r * ao + ambient.r,
			direct.g * ao + ambient.g,
			direct.b * ao + ambient.b);

	/* ---- Indirect bounce (Russian roulette + MIS) ---- */
	if (depth < max_bounces) {
		/* Russian roulette based on albedo luminance. */
		float lum = 0.2126f * albedo.r + 0.7152f * albedo.g + 0.0722f * albedo.b;
		float continue_prob = CLAMP(lum, 0.1f, 0.95f);
		if (pt_rng_float(rng) < continue_prob) {
			float rr_weight = 1.0f / continue_prob;

			/* Multiple importance sampling: choose between diffuse and
			 * specular sampling based on material. */
			Color indirect;
			if (pt_rng_float(rng) > mat_metallic * 0.5f + 0.25f) {
				/* Diffuse sample. */
				Vector3 bounce_dir = pt_cosine_hemisphere(surface_normal, rng);
				indirect = _trace_ray(surface_pos + surface_normal * 0.02f,
						bounce_dir, depth + 1, rng);
				/* Cosine-weighted: contribution = albedo * indirect. */
				result.r += albedo.r * indirect.r * rr_weight *
						(1.0f - mat_metallic);
				result.g += albedo.g * indirect.g * rr_weight *
						(1.0f - mat_metallic);
				result.b += albedo.b * indirect.b * rr_weight *
						(1.0f - mat_metallic);
			} else {
				/* Specular sample (GGX importance sampling). */
				Vector3 H = pt_sample_ggx(surface_normal, mat_roughness, rng);
				Vector3 bounce_dir = (2.0f * V.dot(H) * H - V).normalized();
				if (surface_normal.dot(bounce_dir) > 0) {
					indirect = _trace_ray(
							surface_pos + surface_normal * 0.02f,
							bounce_dir, depth + 1, rng);
					float VdotH = MAX(0.0f, V.dot(H));
					float Fr2 = F_Schlick(VdotH, (F0_r + F0_g + F0_b) / 3.0f);
					result.r += indirect.r * Fr2 * rr_weight;
					result.g += indirect.g * Fr2 * rr_weight;
					result.b += indirect.b * Fr2 * rr_weight;
				}
			}
		}
	}

	return result;
}

void GoxelPathTracer::_render_sample(int sample_index) {
	float inv_w = 1.0f / width;
	float inv_h = 1.0f / height;
	float aspect = (float)width / height;
	float fov_rad = camera_fov * (float)Math_PI / 180.0f;
	float half_h = tanf(fov_rad * 0.5f);

	Vector3 cam_origin = camera_transform.origin;
	Basis cam_basis = camera_transform.basis;

	for (int py = 0; py < height; py++) {
		for (int px = 0; px < width; px++) {
			uint32_t rng = (px * 73856093) ^ (py * 19349663) ^
					(sample_index * 83492791);
			pt_rng_next(rng);

			/* Jittered pixel position. */
			float jx = pt_rng_float(rng);
			float jy = pt_rng_float(rng);
			float sx = (2.0f * (px + jx) * inv_w - 1.0f) * aspect * half_h;
			float sy = (1.0f - 2.0f * (py + jy) * inv_h) * half_h;

			Vector3 ray_dir;
			Vector3 ray_origin;

			if (!camera_ortho) {
				Vector3 local_dir = Vector3(sx, sy, -1.0f).normalized();
				ray_dir = cam_basis.xform(local_dir).normalized();
				ray_origin = cam_origin;
			} else {
				float ortho_size = camera_fov; /* reuse fov as ortho size */
				float osx = (2.0f * (px + jx) * inv_w - 1.0f) *
						ortho_size * 0.5f * aspect;
				float osy = (1.0f - 2.0f * (py + jy) * inv_h) *
						ortho_size * 0.5f;
				ray_origin = cam_origin + cam_basis.xform(Vector3(osx, osy, 0));
				ray_dir = cam_basis.xform(Vector3(0, 0, -1)).normalized();
			}

			Color c = _trace_ray(ray_origin, ray_dir, 0, rng);

			int idx = (py * width + px) * 4;
			accum[idx + 0] += c.r;
			accum[idx + 1] += c.g;
			accum[idx + 2] += c.b;
			accum[idx + 3] += 1.0f;
		}
	}
}

void GoxelPathTracer::start() {
	_ensure_accum();
	memset(accum, 0, accum_size * sizeof(float));
	current_sample = 0;
}

bool GoxelPathTracer::iter() {
	if (current_sample >= num_samples)
		return true;
	_ensure_accum();
	_render_sample(current_sample);
	current_sample++;
	return current_sample >= num_samples;
}

Ref<Image> GoxelPathTracer::get_image() const {
	if (!accum || current_sample == 0)
		return Ref<Image>();

	PoolByteArray data;
	data.resize(width * height * 3);
	{
		PoolByteArray::Write w = data.write();
		float inv_s = 1.0f / current_sample;
		for (int i = 0; i < width * height; i++) {
			float r = accum[i * 4 + 0] * inv_s;
			float g = accum[i * 4 + 1] * inv_s;
			float b = accum[i * 4 + 2] * inv_s;
			/* Tonemap (ACES approximation) + linear→sRGB. */
			r = r * (r * 2.51f + 0.03f) / (r * (r * 2.43f + 0.59f) + 0.14f);
			g = g * (g * 2.51f + 0.03f) / (g * (g * 2.43f + 0.59f) + 0.14f);
			b = b * (b * 2.51f + 0.03f) / (b * (b * 2.43f + 0.59f) + 0.14f);
			r = linear_to_srgb(CLAMP(r, 0.0f, 1.0f));
			g = linear_to_srgb(CLAMP(g, 0.0f, 1.0f));
			b = linear_to_srgb(CLAMP(b, 0.0f, 1.0f));
			w[i * 3 + 0] = (uint8_t)CLAMP(r * 255.0f, 0, 255);
			w[i * 3 + 1] = (uint8_t)CLAMP(g * 255.0f, 0, 255);
			w[i * 3 + 2] = (uint8_t)CLAMP(b * 255.0f, 0, 255);
		}
	}

	Ref<Image> img;
	img.instance();
	img->create(width, height, false, Image::FORMAT_RGB8, data);
	return img;
}

Ref<Image> GoxelPathTracer::render() {
	start();
	while (!iter()) {
	}
	return get_image();
}

int GoxelPathTracer::get_current_sample() const {
	return current_sample;
}

/* Setters/getters. */
void GoxelPathTracer::set_volume(Ref<GoxelVolume> p_volume) { volume = p_volume; }
Ref<GoxelVolume> GoxelPathTracer::get_volume() const { return volume; }
void GoxelPathTracer::set_size(int w, int h) {
	width = MAX(1, w);
	height = MAX(1, h);
}
Vector2 GoxelPathTracer::get_size() const { return Vector2(width, height); }
void GoxelPathTracer::set_num_samples(int p) { num_samples = MAX(1, p); }
int GoxelPathTracer::get_num_samples() const { return num_samples; }
void GoxelPathTracer::set_max_bounces(int p) { max_bounces = CLAMP(p, 0, 8); }
int GoxelPathTracer::get_max_bounces() const { return max_bounces; }
void GoxelPathTracer::set_camera_transform(Transform p) { camera_transform = p; }
Transform GoxelPathTracer::get_camera_transform() const { return camera_transform; }
void GoxelPathTracer::set_camera_fov(float p) { camera_fov = CLAMP(p, 1.0f, 179.0f); }
float GoxelPathTracer::get_camera_fov() const { return camera_fov; }
void GoxelPathTracer::set_camera_ortho(bool p) { camera_ortho = p; }
bool GoxelPathTracer::get_camera_ortho() const { return camera_ortho; }
void GoxelPathTracer::set_world_type(WorldType p) { world_type = p; }
GoxelPathTracer::WorldType GoxelPathTracer::get_world_type() const { return world_type; }
void GoxelPathTracer::set_world_color(Color p) { world_color = p; }
Color GoxelPathTracer::get_world_color() const { return world_color; }
void GoxelPathTracer::set_world_energy(float p) { world_energy = MAX(0, p); }
float GoxelPathTracer::get_world_energy() const { return world_energy; }
void GoxelPathTracer::set_floor_enabled(bool p) { floor_enabled = p; }
bool GoxelPathTracer::get_floor_enabled() const { return floor_enabled; }
void GoxelPathTracer::set_floor_color(Color p) { floor_color = p; }
Color GoxelPathTracer::get_floor_color() const { return floor_color; }
void GoxelPathTracer::set_floor_height(float p) { floor_height = p; }
float GoxelPathTracer::get_floor_height() const { return floor_height; }
void GoxelPathTracer::set_light_direction(Vector3 p) { light_direction = p.normalized(); }
Vector3 GoxelPathTracer::get_light_direction() const { return light_direction; }
void GoxelPathTracer::set_light_intensity(float p) { light_intensity = MAX(0, p); }
float GoxelPathTracer::get_light_intensity() const { return light_intensity; }
void GoxelPathTracer::set_metallic(float p) { metallic = CLAMP(p, 0.0f, 1.0f); }
float GoxelPathTracer::get_metallic() const { return metallic; }
void GoxelPathTracer::set_roughness(float p) { roughness = CLAMP(p, 0.04f, 1.0f); }
float GoxelPathTracer::get_roughness() const { return roughness; }
void GoxelPathTracer::set_sun_direction(Vector3 p) { sun_direction = p.normalized(); }
Vector3 GoxelPathTracer::get_sun_direction() const { return sun_direction; }
void GoxelPathTracer::set_sun_intensity(float p) { sun_intensity = MAX(0, p); }
float GoxelPathTracer::get_sun_intensity() const { return sun_intensity; }
void GoxelPathTracer::set_sky_turbidity(float p) { sky_turbidity = CLAMP(p, 1.0f, 10.0f); }
float GoxelPathTracer::get_sky_turbidity() const { return sky_turbidity; }

void GoxelPathTracer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_volume", "volume"), &GoxelPathTracer::set_volume);
	ClassDB::bind_method(D_METHOD("get_volume"), &GoxelPathTracer::get_volume);
	ClassDB::bind_method(D_METHOD("set_size", "width", "height"), &GoxelPathTracer::set_size);
	ClassDB::bind_method(D_METHOD("get_size"), &GoxelPathTracer::get_size);
	ClassDB::bind_method(D_METHOD("set_num_samples", "samples"), &GoxelPathTracer::set_num_samples);
	ClassDB::bind_method(D_METHOD("get_num_samples"), &GoxelPathTracer::get_num_samples);
	ClassDB::bind_method(D_METHOD("set_max_bounces", "bounces"), &GoxelPathTracer::set_max_bounces);
	ClassDB::bind_method(D_METHOD("get_max_bounces"), &GoxelPathTracer::get_max_bounces);
	ClassDB::bind_method(D_METHOD("set_camera_transform", "transform"), &GoxelPathTracer::set_camera_transform);
	ClassDB::bind_method(D_METHOD("get_camera_transform"), &GoxelPathTracer::get_camera_transform);
	ClassDB::bind_method(D_METHOD("set_camera_fov", "fov"), &GoxelPathTracer::set_camera_fov);
	ClassDB::bind_method(D_METHOD("get_camera_fov"), &GoxelPathTracer::get_camera_fov);
	ClassDB::bind_method(D_METHOD("set_camera_ortho", "ortho"), &GoxelPathTracer::set_camera_ortho);
	ClassDB::bind_method(D_METHOD("get_camera_ortho"), &GoxelPathTracer::get_camera_ortho);
	ClassDB::bind_method(D_METHOD("set_world_type", "type"), &GoxelPathTracer::set_world_type);
	ClassDB::bind_method(D_METHOD("get_world_type"), &GoxelPathTracer::get_world_type);
	ClassDB::bind_method(D_METHOD("set_world_color", "color"), &GoxelPathTracer::set_world_color);
	ClassDB::bind_method(D_METHOD("get_world_color"), &GoxelPathTracer::get_world_color);
	ClassDB::bind_method(D_METHOD("set_world_energy", "energy"), &GoxelPathTracer::set_world_energy);
	ClassDB::bind_method(D_METHOD("get_world_energy"), &GoxelPathTracer::get_world_energy);
	ClassDB::bind_method(D_METHOD("set_floor_enabled", "enabled"), &GoxelPathTracer::set_floor_enabled);
	ClassDB::bind_method(D_METHOD("get_floor_enabled"), &GoxelPathTracer::get_floor_enabled);
	ClassDB::bind_method(D_METHOD("set_floor_color", "color"), &GoxelPathTracer::set_floor_color);
	ClassDB::bind_method(D_METHOD("get_floor_color"), &GoxelPathTracer::get_floor_color);
	ClassDB::bind_method(D_METHOD("set_floor_height", "height"), &GoxelPathTracer::set_floor_height);
	ClassDB::bind_method(D_METHOD("get_floor_height"), &GoxelPathTracer::get_floor_height);
	ClassDB::bind_method(D_METHOD("set_light_direction", "direction"), &GoxelPathTracer::set_light_direction);
	ClassDB::bind_method(D_METHOD("get_light_direction"), &GoxelPathTracer::get_light_direction);
	ClassDB::bind_method(D_METHOD("set_light_intensity", "intensity"), &GoxelPathTracer::set_light_intensity);
	ClassDB::bind_method(D_METHOD("get_light_intensity"), &GoxelPathTracer::get_light_intensity);
	ClassDB::bind_method(D_METHOD("set_metallic", "metallic"), &GoxelPathTracer::set_metallic);
	ClassDB::bind_method(D_METHOD("get_metallic"), &GoxelPathTracer::get_metallic);
	ClassDB::bind_method(D_METHOD("set_roughness", "roughness"), &GoxelPathTracer::set_roughness);
	ClassDB::bind_method(D_METHOD("get_roughness"), &GoxelPathTracer::get_roughness);
	ClassDB::bind_method(D_METHOD("set_sun_direction", "direction"), &GoxelPathTracer::set_sun_direction);
	ClassDB::bind_method(D_METHOD("get_sun_direction"), &GoxelPathTracer::get_sun_direction);
	ClassDB::bind_method(D_METHOD("set_sun_intensity", "intensity"), &GoxelPathTracer::set_sun_intensity);
	ClassDB::bind_method(D_METHOD("get_sun_intensity"), &GoxelPathTracer::get_sun_intensity);
	ClassDB::bind_method(D_METHOD("set_sky_turbidity", "turbidity"), &GoxelPathTracer::set_sky_turbidity);
	ClassDB::bind_method(D_METHOD("get_sky_turbidity"), &GoxelPathTracer::get_sky_turbidity);

	ClassDB::bind_method(D_METHOD("render"), &GoxelPathTracer::render);
	ClassDB::bind_method(D_METHOD("start"), &GoxelPathTracer::start);
	ClassDB::bind_method(D_METHOD("iter"), &GoxelPathTracer::iter);
	ClassDB::bind_method(D_METHOD("get_image"), &GoxelPathTracer::get_image);
	ClassDB::bind_method(D_METHOD("get_current_sample"), &GoxelPathTracer::get_current_sample);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "volume", PROPERTY_HINT_RESOURCE_TYPE, "GoxelVolume"), "set_volume", "get_volume");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "num_samples"), "set_num_samples", "get_num_samples");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_bounces"), "set_max_bounces", "get_max_bounces");
	ADD_PROPERTY(PropertyInfo(Variant::TRANSFORM, "camera_transform"), "set_camera_transform", "get_camera_transform");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "camera_fov"), "set_camera_fov", "get_camera_fov");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "camera_ortho"), "set_camera_ortho", "get_camera_ortho");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "world_type", PROPERTY_HINT_ENUM, "None,Uniform,Sky"), "set_world_type", "get_world_type");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "world_color"), "set_world_color", "get_world_color");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "world_energy"), "set_world_energy", "get_world_energy");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "floor_enabled"), "set_floor_enabled", "get_floor_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "floor_color"), "set_floor_color", "get_floor_color");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "floor_height"), "set_floor_height", "get_floor_height");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "light_direction"), "set_light_direction", "get_light_direction");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "light_intensity"), "set_light_intensity", "get_light_intensity");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "metallic", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_metallic", "get_metallic");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "roughness", PROPERTY_HINT_RANGE, "0.04,1,0.01"), "set_roughness", "get_roughness");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "sun_direction"), "set_sun_direction", "get_sun_direction");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "sun_intensity"), "set_sun_intensity", "get_sun_intensity");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "sky_turbidity", PROPERTY_HINT_RANGE, "1,10,0.1"), "set_sky_turbidity", "get_sky_turbidity");

	BIND_ENUM_CONSTANT(WORLD_NONE);
	BIND_ENUM_CONSTANT(WORLD_UNIFORM);
	BIND_ENUM_CONSTANT(WORLD_SKY);
}

/* ======================================================================= */
/*  GoxelFileIO                                                            */
/* ======================================================================= */

static PoolByteArray _read_file_bytes(const String &path) {
	PoolByteArray result;
	FileAccess *f = FileAccess::open(path, FileAccess::READ);
	if (!f)
		return result;
	int len = f->get_len();
	result.resize(len);
	{
		PoolByteArray::Write w = result.write();
		f->get_buffer(w.ptr(), len);
	}
	f->close();
	memdelete(f);
	return result;
}

static Error _write_file_bytes(const String &path, const uint8_t *data, int size) {
	FileAccess *f = FileAccess::open(path, FileAccess::WRITE);
	if (!f)
		return ERR_FILE_CANT_OPEN;
	f->store_buffer(data, size);
	f->close();
	memdelete(f);
	return OK;
}

Ref<GoxelVolume> GoxelFileIO::import_vox(const String &path) {
	PoolByteArray data = _read_file_bytes(path);
	ERR_FAIL_COND_V(data.size() == 0, Ref<GoxelVolume>());
	PoolByteArray::Read r = data.read();
	volume_t *vol = goxel_import_vox(r.ptr(), data.size());
	ERR_FAIL_COND_V(!vol, Ref<GoxelVolume>());
	Ref<GoxelVolume> gv;
	gv.instance();
	/* Replace the default empty volume with the imported one. */
	volume_set(gv->get_volume(), vol);
	volume_delete(vol);
	return gv;
}

Error GoxelFileIO::export_vox(Ref<GoxelVolume> volume, const String &path) {
	ERR_FAIL_COND_V(volume.is_null(), ERR_INVALID_PARAMETER);
	uint8_t *data = nullptr;
	int size = 0;
	int err = goxel_export_vox(volume->get_volume(), &data, &size);
	ERR_FAIL_COND_V(err != 0 || !data, ERR_CANT_CREATE);
	Error result = _write_file_bytes(path, data, size);
	free(data);
	return result;
}

Ref<GoxelVolume> GoxelFileIO::import_gox(const String &path) {
	PoolByteArray data = _read_file_bytes(path);
	ERR_FAIL_COND_V(data.size() == 0, Ref<GoxelVolume>());
	PoolByteArray::Read r = data.read();
	volume_t *vol = goxel_import_gox(r.ptr(), data.size());
	ERR_FAIL_COND_V(!vol, Ref<GoxelVolume>());
	Ref<GoxelVolume> gv;
	gv.instance();
	volume_set(gv->get_volume(), vol);
	volume_delete(vol);
	return gv;
}

Error GoxelFileIO::export_gox(Ref<GoxelVolume> volume, const String &path) {
	ERR_FAIL_COND_V(volume.is_null(), ERR_INVALID_PARAMETER);
	uint8_t *data = nullptr;
	int size = 0;
	int err = goxel_export_gox(volume->get_volume(), &data, &size);
	ERR_FAIL_COND_V(err != 0 || !data, ERR_CANT_CREATE);
	Error result = _write_file_bytes(path, data, size);
	free(data);
	return result;
}

Ref<GoxelVolume> GoxelFileIO::import_qb(const String &path) {
	PoolByteArray data = _read_file_bytes(path);
	ERR_FAIL_COND_V(data.size() == 0, Ref<GoxelVolume>());
	PoolByteArray::Read r = data.read();
	volume_t *vol = goxel_import_qb(r.ptr(), data.size());
	ERR_FAIL_COND_V(!vol, Ref<GoxelVolume>());
	Ref<GoxelVolume> gv;
	gv.instance();
	volume_set(gv->get_volume(), vol);
	volume_delete(vol);
	return gv;
}

Error GoxelFileIO::export_qb(Ref<GoxelVolume> volume, const String &path) {
	ERR_FAIL_COND_V(volume.is_null(), ERR_INVALID_PARAMETER);
	uint8_t *data = nullptr;
	int size = 0;
	int err = goxel_export_qb(volume->get_volume(), &data, &size);
	ERR_FAIL_COND_V(err != 0 || !data, ERR_CANT_CREATE);
	Error result = _write_file_bytes(path, data, size);
	free(data);
	return result;
}

Ref<GoxelVolume> GoxelFileIO::import_txt(const String &path) {
	PoolByteArray data = _read_file_bytes(path);
	ERR_FAIL_COND_V(data.size() == 0, Ref<GoxelVolume>());
	PoolByteArray::Read r = data.read();
	volume_t *vol = goxel_import_txt(r.ptr(), data.size());
	ERR_FAIL_COND_V(!vol, Ref<GoxelVolume>());
	Ref<GoxelVolume> gv;
	gv.instance();
	volume_set(gv->get_volume(), vol);
	volume_delete(vol);
	return gv;
}

Error GoxelFileIO::export_txt(Ref<GoxelVolume> volume, const String &path) {
	ERR_FAIL_COND_V(volume.is_null(), ERR_INVALID_PARAMETER);
	uint8_t *data = nullptr;
	int size = 0;
	int err = goxel_export_txt(volume->get_volume(), &data, &size);
	ERR_FAIL_COND_V(err != 0 || !data, ERR_CANT_CREATE);
	Error result = _write_file_bytes(path, data, size);
	free(data);
	return result;
}

Ref<GoxelVolume> GoxelFileIO::import_vxl(const String &path) {
	PoolByteArray data = _read_file_bytes(path);
	ERR_FAIL_COND_V(data.size() == 0, Ref<GoxelVolume>());
	PoolByteArray::Read r = data.read();
	volume_t *vol = goxel_import_vxl(r.ptr(), data.size());
	ERR_FAIL_COND_V(!vol, Ref<GoxelVolume>());
	Ref<GoxelVolume> gv;
	gv.instance();
	volume_set(gv->get_volume(), vol);
	volume_delete(vol);
	return gv;
}

Ref<GoxelVolume> GoxelFileIO::import_kvx(const String &path) {
	PoolByteArray data = _read_file_bytes(path);
	ERR_FAIL_COND_V(data.size() == 0, Ref<GoxelVolume>());
	PoolByteArray::Read r = data.read();
	volume_t *vol = goxel_import_kvx(r.ptr(), data.size());
	ERR_FAIL_COND_V(!vol, Ref<GoxelVolume>());
	Ref<GoxelVolume> gv;
	gv.instance();
	volume_set(gv->get_volume(), vol);
	volume_delete(vol);
	return gv;
}

Error GoxelFileIO::export_obj(Ref<GoxelVolume> volume, const String &path) {
	ERR_FAIL_COND_V(volume.is_null(), ERR_INVALID_PARAMETER);
	char *data = nullptr;
	int size = 0;
	int err = goxel_export_obj(volume->get_volume(), &data, &size);
	ERR_FAIL_COND_V(err != 0 || !data, ERR_CANT_CREATE);
	Error result = _write_file_bytes(path, (const uint8_t *)data, size);
	free(data);
	return result;
}

void GoxelFileIO::_bind_methods() {
	ClassDB::bind_method(D_METHOD("import_vox", "path"), &GoxelFileIO::import_vox);
	ClassDB::bind_method(D_METHOD("export_vox", "volume", "path"), &GoxelFileIO::export_vox);
	ClassDB::bind_method(D_METHOD("import_gox", "path"), &GoxelFileIO::import_gox);
	ClassDB::bind_method(D_METHOD("export_gox", "volume", "path"), &GoxelFileIO::export_gox);
	ClassDB::bind_method(D_METHOD("import_qb", "path"), &GoxelFileIO::import_qb);
	ClassDB::bind_method(D_METHOD("export_qb", "volume", "path"), &GoxelFileIO::export_qb);
	ClassDB::bind_method(D_METHOD("import_txt", "path"), &GoxelFileIO::import_txt);
	ClassDB::bind_method(D_METHOD("export_txt", "volume", "path"), &GoxelFileIO::export_txt);
	ClassDB::bind_method(D_METHOD("import_vxl", "path"), &GoxelFileIO::import_vxl);
	ClassDB::bind_method(D_METHOD("import_kvx", "path"), &GoxelFileIO::import_kvx);
	ClassDB::bind_method(D_METHOD("export_obj", "volume", "path"), &GoxelFileIO::export_obj);
}

/* ======================================================================= */
/*  GoxelFilters                                                           */
/* ======================================================================= */

void GoxelFilters::mirror(Ref<GoxelVolume> volume, int axis) {
	ERR_FAIL_COND(volume.is_null());
	ERR_FAIL_COND(axis < 0 || axis > 2);

	volume_t *vol = volume->get_volume();
	int bbox[2][3];
	if (!volume_get_bbox(vol, bbox, true))
		return;

	volume_t *src = volume_dup(vol);
	volume_clear(vol);

	for (int z = bbox[0][2]; z < bbox[1][2]; z++)
		for (int y = bbox[0][1]; y < bbox[1][1]; y++)
			for (int x = bbox[0][0]; x < bbox[1][0]; x++) {
				int pos[3] = { x, y, z };
				uint8_t v[4];
				volume_get_at(src, nullptr, pos, v);
				if (v[3] == 0)
					continue;

				int dst_pos[3] = { x, y, z };
				dst_pos[axis] = bbox[0][axis] + bbox[1][axis] - 1 - pos[axis];
				volume_set_at(vol, nullptr, dst_pos, v);
			}

	volume_delete(src);
}

static void rgb_to_hsl(float r, float g, float b,
		float *h, float *s, float *l) {
	float mx = MAX(r, MAX(g, b));
	float mn = MIN(r, MIN(g, b));
	*l = (mx + mn) * 0.5f;
	if (mx == mn) {
		*h = *s = 0;
		return;
	}
	float d = mx - mn;
	*s = *l > 0.5f ? d / (2.0f - mx - mn) : d / (mx + mn);
	if (mx == r)
		*h = (g - b) / d + (g < b ? 6.0f : 0.0f);
	else if (mx == g)
		*h = (b - r) / d + 2.0f;
	else
		*h = (r - g) / d + 4.0f;
	*h /= 6.0f;
}

static float hue2rgb(float p, float q, float t) {
	if (t < 0)
		t += 1.0f;
	if (t > 1)
		t -= 1.0f;
	if (t < 1.0f / 6.0f)
		return p + (q - p) * 6.0f * t;
	if (t < 0.5f)
		return q;
	if (t < 2.0f / 3.0f)
		return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
	return p;
}

static void hsl_to_rgb(float h, float s, float l,
		float *r, float *g, float *b) {
	if (s == 0) {
		*r = *g = *b = l;
		return;
	}
	float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
	float p = 2.0f * l - q;
	*r = hue2rgb(p, q, h + 1.0f / 3.0f);
	*g = hue2rgb(p, q, h);
	*b = hue2rgb(p, q, h - 1.0f / 3.0f);
}

void GoxelFilters::adjust_colors(Ref<GoxelVolume> volume, float hue,
		float saturation, float lightness, float contrast) {
	ERR_FAIL_COND(volume.is_null());

	volume_t *vol = volume->get_volume();
	int bbox[2][3];
	if (!volume_get_bbox(vol, bbox, true))
		return;

	for (int z = bbox[0][2]; z < bbox[1][2]; z++)
		for (int y = bbox[0][1]; y < bbox[1][1]; y++)
			for (int x = bbox[0][0]; x < bbox[1][0]; x++) {
				int pos[3] = { x, y, z };
				uint8_t v[4];
				volume_get_at(vol, nullptr, pos, v);
				if (v[3] == 0)
					continue;

				float r = v[0] / 255.0f;
				float g = v[1] / 255.0f;
				float b = v[2] / 255.0f;

				float h, s, l;
				rgb_to_hsl(r, g, b, &h, &s, &l);

				h = fmodf(h + hue, 1.0f);
				if (h < 0)
					h += 1.0f;
				s = CLAMP(s + saturation, 0.0f, 1.0f);
				l = CLAMP(l + lightness, 0.0f, 1.0f);

				hsl_to_rgb(h, s, l, &r, &g, &b);

				/* Apply contrast. */
				r = CLAMP((r - 0.5f) * (1.0f + contrast) + 0.5f, 0.0f, 1.0f);
				g = CLAMP((g - 0.5f) * (1.0f + contrast) + 0.5f, 0.0f, 1.0f);
				b = CLAMP((b - 0.5f) * (1.0f + contrast) + 0.5f, 0.0f, 1.0f);

				v[0] = (uint8_t)(r * 255.0f);
				v[1] = (uint8_t)(g * 255.0f);
				v[2] = (uint8_t)(b * 255.0f);
				volume_set_at(vol, nullptr, pos, v);
			}
}

void GoxelFilters::wrap(Ref<GoxelVolume> volume, int axis) {
	ERR_FAIL_COND(volume.is_null());
	ERR_FAIL_COND(axis < 0 || axis > 2);

	volume_t *vol = volume->get_volume();
	int bbox[2][3];
	if (!volume_get_bbox(vol, bbox, true))
		return;

	int size = bbox[1][axis] - bbox[0][axis];
	if (size <= 0)
		return;
	int half = size / 2;

	volume_t *src = volume_dup(vol);
	volume_clear(vol);

	for (int z = bbox[0][2]; z < bbox[1][2]; z++)
		for (int y = bbox[0][1]; y < bbox[1][1]; y++)
			for (int x = bbox[0][0]; x < bbox[1][0]; x++) {
				int pos[3] = { x, y, z };
				uint8_t v[4];
				volume_get_at(src, nullptr, pos, v);
				if (v[3] == 0)
					continue;

				int dst_pos[3] = { x, y, z };
				int local = pos[axis] - bbox[0][axis];
				dst_pos[axis] = bbox[0][axis] + ((local + half) % size);
				volume_set_at(vol, nullptr, dst_pos, v);
			}

	volume_delete(src);
}

void GoxelFilters::_bind_methods() {
	ClassDB::bind_method(D_METHOD("mirror", "volume", "axis"), &GoxelFilters::mirror);
	ClassDB::bind_method(D_METHOD("adjust_colors", "volume", "hue", "saturation", "lightness", "contrast"), &GoxelFilters::adjust_colors);
	ClassDB::bind_method(D_METHOD("wrap", "volume", "axis"), &GoxelFilters::wrap);

	BIND_ENUM_CONSTANT(AXIS_X);
	BIND_ENUM_CONSTANT(AXIS_Y);
	BIND_ENUM_CONSTANT(AXIS_Z);
}

/* ======================================================================= */
/*  Doctests                                                               */
/* ======================================================================= */

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_CASE("[Goxel] volume create and destroy") {
	volume_t *v = volume_new();
	CHECK(v != nullptr);
	CHECK(volume_is_empty(v));
	volume_delete(v);
}

TEST_CASE("[Goxel] volume set and get voxel") {
	volume_t *v = volume_new();
	int pos[3] = { 5, 7, 3 };
	uint8_t color[4] = { 200, 100, 50, 255 };
	volume_set_at(v, nullptr, pos, color);

	uint8_t out[4];
	volume_get_at(v, nullptr, pos, out);
	CHECK(out[0] == 200);
	CHECK(out[1] == 100);
	CHECK(out[2] == 50);
	CHECK(out[3] == 255);

	CHECK(!volume_is_empty(v));
	volume_delete(v);
}

TEST_CASE("[Goxel] volume set voxel across tile boundary") {
	volume_t *v = volume_new();
	int pos0[3] = { 0, 0, 0 };
	int pos1[3] = { TILE_SIZE, 0, 0 }; /* next tile */
	uint8_t c0[4] = { 255, 0, 0, 255 };
	uint8_t c1[4] = { 0, 255, 0, 255 };
	volume_set_at(v, nullptr, pos0, c0);
	volume_set_at(v, nullptr, pos1, c1);

	uint8_t out0[4], out1[4];
	volume_get_at(v, nullptr, pos0, out0);
	volume_get_at(v, nullptr, pos1, out1);

	CHECK(out0[0] == 255);
	CHECK(out0[1] == 0);
	CHECK(out1[0] == 0);
	CHECK(out1[1] == 255);
	CHECK(volume_get_tiles_count(v) == 2);
	volume_delete(v);
}

TEST_CASE("[Goxel] volume bounding box") {
	volume_t *v = volume_new();
	int p1[3] = { 2, 3, 4 };
	int p2[3] = { 10, 20, 30 };
	uint8_t c[4] = { 1, 1, 1, 255 };
	volume_set_at(v, nullptr, p1, c);
	volume_set_at(v, nullptr, p2, c);

	int bbox[2][3];
	bool ok = volume_get_bbox(v, bbox, true);
	CHECK(ok);
	CHECK(bbox[0][0] == 2);
	CHECK(bbox[0][1] == 3);
	CHECK(bbox[0][2] == 4);
	CHECK(bbox[1][0] == 11);
	CHECK(bbox[1][1] == 21);
	CHECK(bbox[1][2] == 31);
	volume_delete(v);
}

TEST_CASE("[Goxel] volume clear") {
	volume_t *v = volume_new();
	int pos[3] = { 0, 0, 0 };
	uint8_t c[4] = { 1, 1, 1, 255 };
	volume_set_at(v, nullptr, pos, c);
	CHECK(!volume_is_empty(v));
	volume_clear(v);
	CHECK(volume_is_empty(v));
	volume_delete(v);
}

TEST_CASE("[Goxel] volume key changes on mutation") {
	volume_t *v = volume_new();
	uint64_t k0 = volume_get_key(v);
	int pos[3] = { 0, 0, 0 };
	uint8_t c[4] = { 1, 1, 1, 255 };
	volume_set_at(v, nullptr, pos, c);
	uint64_t k1 = volume_get_key(v);
	CHECK(k1 != k0);
	volume_delete(v);
}

TEST_CASE("[Goxel] volume read region") {
	volume_t *v = volume_new();
	int pos[3] = { 1, 2, 3 };
	uint8_t c[4] = { 42, 84, 126, 200 };
	volume_set_at(v, nullptr, pos, c);

	int rpos[3] = { 0, 0, 0 };
	int rsize[3] = { 4, 4, 4 };
	uint8_t *data = (uint8_t *)calloc(4 * 4 * 4 * 4, 1);
	volume_read(v, rpos, rsize, data);

	int off = ((3 * 4 + 2) * 4 + 1) * 4;
	CHECK(data[off + 0] == 42);
	CHECK(data[off + 1] == 84);
	CHECK(data[off + 2] == 126);
	CHECK(data[off + 3] == 200);
	free(data);
	volume_delete(v);
}

TEST_CASE("[Goxel] volume read region empty") {
	volume_t *v = volume_new();
	int rpos[3] = { 0, 0, 0 };
	int rsize[3] = { 2, 2, 2 };
	uint8_t data[2 * 2 * 2 * 4];
	memset(data, 0xFF, sizeof(data));
	volume_read(v, rpos, rsize, data);

	for (int i = 0; i < (int)sizeof(data); i++) {
		CHECK(data[i] == 0);
	}
	volume_delete(v);
}

TEST_CASE("[Goxel] volume blit") {
	volume_t *v = volume_new();
	uint8_t data[2 * 2 * 1 * 4];
	memset(data, 0, sizeof(data));
	/* Set pixel (0,0,0) = red, (1,1,0) = blue */
	data[0] = 255;
	data[1] = 0;
	data[2] = 0;
	data[3] = 255;
	int idx = (0 * 2 + 1) * 2 + 1; /* z=0, y=1, x=1 */
	data[idx * 4 + 0] = 0;
	data[idx * 4 + 1] = 0;
	data[idx * 4 + 2] = 255;
	data[idx * 4 + 3] = 255;

	volume_blit(v, data, 5, 5, 5, 2, 2, 1, nullptr);

	uint8_t out[4];
	int pos1[3] = { 5, 5, 5 };
	volume_get_at(v, nullptr, pos1, out);
	CHECK(out[0] == 255);
	CHECK(out[3] == 255);

	int pos2[3] = { 6, 6, 5 };
	volume_get_at(v, nullptr, pos2, out);
	CHECK(out[2] == 255);
	CHECK(out[3] == 255);
	volume_delete(v);
}

TEST_CASE("[Goxel] volume tile count") {
	volume_t *v = volume_new();
	CHECK(volume_get_tiles_count(v) == 0);
	int pos0[3] = { 0, 0, 0 };
	uint8_t c[4] = { 1, 1, 1, 255 };
	volume_set_at(v, nullptr, pos0, c);
	CHECK(volume_get_tiles_count(v) == 1);
	int pos1[3] = { TILE_SIZE * 2, 0, 0 };
	volume_set_at(v, nullptr, pos1, c);
	CHECK(volume_get_tiles_count(v) == 2);
	volume_delete(v);
}

TEST_CASE("[Goxel] GoxelVolume Color conversion") {
	Ref<GoxelVolume> gv;
	gv.instance();
	Color in_color(0.5f, 0.25f, 0.75f, 1.0f);
	gv->set_voxel(Vector3(0, 0, 0), in_color);

	Color out = gv->get_voxel(Vector3(0, 0, 0));
	/* Allow ±2/255 tolerance for uint8 quantization. */
	CHECK(Math::abs(out.r - in_color.r) < 0.01f);
	CHECK(Math::abs(out.g - in_color.g) < 0.01f);
	CHECK(Math::abs(out.b - in_color.b) < 0.01f);
	CHECK(Math::abs(out.a - in_color.a) < 0.01f);
}

TEST_CASE("[Goxel] marching cubes generates triangles") {
	volume_t *v = volume_new();
	uint8_t c[4] = { 200, 100, 50, 255 };
	/* Fill a small 3×3×3 solid cube. */
	for (int z = 1; z < 4; z++)
		for (int y = 1; y < 4; y++)
			for (int x = 1; x < 4; x++) {
				int pos[3] = { x, y, z };
				volume_set_at(v, nullptr, pos, c);
			}

	int prim_size = 0, subdivide = 0;
	/* Allocate generous buffer. */
	voxel_vertex_t *out = (voxel_vertex_t *)calloc(
			TILE_SIZE * TILE_SIZE * TILE_SIZE * 15, sizeof(voxel_vertex_t));
	int block_pos[3] = { 0, 0, 0 };
	int nb = volume_generate_vertices(v, block_pos,
			EFFECT_MARCHING_CUBES, out, &prim_size, &subdivide);
	CHECK(nb > 0);
	CHECK(prim_size == 3); /* triangles */
	free(out);
	volume_delete(v);
}

TEST_CASE("[Goxel] pathtracer renders non-black image") {
	/* Build a small volume with a bright voxel. */
	Ref<GoxelVolume> gv;
	gv.instance();
	for (int z = 0; z < 3; z++)
		for (int y = 0; y < 3; y++)
			for (int x = 0; x < 3; x++)
				gv->set_voxel(Vector3(x, y, z), Color(1, 0, 0, 1));

	Ref<GoxelPathTracer> pt;
	pt.instance();
	pt->set_volume(gv);
	pt->set_size(16, 16);
	pt->set_num_samples(1);
	pt->set_max_bounces(0);
	pt->set_world_type(GoxelPathTracer::WORLD_UNIFORM);
	pt->set_world_color(Color(0.5f, 0.6f, 0.8f));
	pt->set_world_energy(1.0f);
	pt->set_light_direction(Vector3(0.5f, 1.0f, 0.5f).normalized());
	pt->set_light_intensity(1.0f);
	/* Camera looking at the cube from front. */
	Transform cam;
	cam.origin = Vector3(1.5f, 1.5f, 10.0f);
	cam.basis = Basis(); /* identity — looking along -Z */
	pt->set_camera_transform(cam);
	pt->set_camera_fov(60.0f);

	Ref<Image> img = pt->render();
	CHECK(img.is_valid());
	CHECK(img->get_width() == 16);
	CHECK(img->get_height() == 16);

	/* At least some non-black pixels should exist. */
	img->lock();
	bool found_nonblack = false;
	for (int y = 0; y < 16 && !found_nonblack; y++)
		for (int x = 0; x < 16 && !found_nonblack; x++) {
			Color c = img->get_pixel(x, y);
			if (c.r > 0.01f || c.g > 0.01f || c.b > 0.01f)
				found_nonblack = true;
		}
	img->unlock();
	CHECK(found_nonblack);
}

TEST_CASE("[Goxel] pathtracer PBR renders specular highlight") {
	Ref<GoxelVolume> gv;
	gv.instance();
	for (int z = 0; z < 5; z++)
		for (int y = 0; y < 5; y++)
			for (int x = 0; x < 5; x++)
				gv->set_voxel(Vector3(x, y, z), Color(0.8f, 0.8f, 0.8f, 1));

	Ref<GoxelPathTracer> pt;
	pt.instance();
	pt->set_volume(gv);
	pt->set_size(32, 32);
	pt->set_num_samples(4);
	pt->set_max_bounces(1);
	pt->set_metallic(0.9f);
	pt->set_roughness(0.1f);
	pt->set_world_type(GoxelPathTracer::WORLD_SKY);
	pt->set_sun_direction(Vector3(0.3f, 0.8f, 0.3f).normalized());
	pt->set_sun_intensity(3.0f);
	pt->set_light_direction(Vector3(0.5f, 1.0f, 0.5f).normalized());
	pt->set_light_intensity(2.0f);
	Transform cam;
	cam.origin = Vector3(2.5f, 2.5f, 15.0f);
	cam.basis = Basis();
	pt->set_camera_transform(cam);
	pt->set_camera_fov(50.0f);

	Ref<Image> img = pt->render();
	CHECK(img.is_valid());

	/* Find max brightness — specular highlight should produce bright pixels. */
	img->lock();
	float max_lum = 0;
	for (int y = 0; y < 32; y++)
		for (int x = 0; x < 32; x++) {
			Color c = img->get_pixel(x, y);
			float lum = c.r * 0.299f + c.g * 0.587f + c.b * 0.114f;
			if (lum > max_lum)
				max_lum = lum;
		}
	img->unlock();
	CHECK(max_lum > 0.3f);
}

TEST_CASE("[Goxel] import VOX format") {
	/* Build a minimal valid VOX file in memory. */
	uint8_t vox_data[8 + 12 + 24 + 12 + 4 + 4 + 12 + 1024];
	memset(vox_data, 0, sizeof(vox_data));
	int p = 0;
	memcpy(vox_data + p, "VOX ", 4);
	p += 4;
	vox_data[p] = 150;
	p += 4;
	/* MAIN chunk */
	memcpy(vox_data + p, "MAIN", 4);
	p += 4;
	p += 8; /* content=0, children=rest */
	/* SIZE */
	memcpy(vox_data + p, "SIZE", 4);
	p += 4;
	vox_data[p] = 12;
	p += 4;
	p += 4;
	vox_data[p] = 2;
	p += 4; /* sx */
	vox_data[p] = 2;
	p += 4; /* sy */
	vox_data[p] = 2;
	p += 4; /* sz */
	/* XYZI */
	memcpy(vox_data + p, "XYZI", 4);
	p += 4;
	vox_data[p] = 8;
	p += 4;
	p += 4;
	vox_data[p] = 1;
	p += 4; /* 1 voxel */
	vox_data[p++] = 0; /* x */
	vox_data[p++] = 0; /* y */
	vox_data[p++] = 0; /* z */
	vox_data[p++] = 1; /* color index */

	volume_t *vol = goxel_import_vox(vox_data, p);
	CHECK(vol != nullptr);
	CHECK(!volume_is_empty(vol));
	volume_delete(vol);
}

TEST_CASE("[Goxel] export VOX round-trip") {
	volume_t *v = volume_new();
	int pos[3] = { 0, 0, 0 };
	uint8_t c[4] = { 255, 128, 64, 255 };
	volume_set_at(v, nullptr, pos, c);
	int pos2[3] = { 1, 0, 0 };
	uint8_t c2[4] = { 64, 128, 255, 255 };
	volume_set_at(v, nullptr, pos2, c2);

	uint8_t *data = nullptr;
	int size = 0;
	int err = goxel_export_vox(v, &data, &size);
	CHECK(err == 0);
	CHECK(data != nullptr);
	CHECK(size > 0);

	/* Re-import. */
	volume_t *v2 = goxel_import_vox(data, size);
	CHECK(v2 != nullptr);
	CHECK(!volume_is_empty(v2));

	/* Check first voxel color is preserved. */
	uint8_t out[4];
	int rpos[3] = { 0, 0, 0 };
	volume_get_at(v2, nullptr, rpos, out);
	CHECK(out[3] == 255);
	CHECK(out[0] == 255);
	CHECK(out[1] == 128);
	CHECK(out[2] == 64);

	free(data);
	volume_delete(v);
	volume_delete(v2);
}

TEST_CASE("[Goxel] import GOX format") {
	/* Minimal GOX: header + BL16 + LAYR */
	int bl16_size = 16 * 16 * 16 * 4;
	int total = 8 + (8 + bl16_size + 4) + (8 + 20 + 4);
	uint8_t *gox = (uint8_t *)calloc(1, total);
	int p = 0;
	memcpy(gox + p, "GOX ", 4);
	p += 4;
	gox[p] = 2;
	p += 4;

	/* BL16 chunk. */
	memcpy(gox + p, "BL16", 4);
	p += 4;
	gox[p] = (uint8_t)(bl16_size & 0xFF);
	gox[p + 1] = (uint8_t)((bl16_size >> 8) & 0xFF);
	gox[p + 2] = (uint8_t)((bl16_size >> 16) & 0xFF);
	gox[p + 3] = (uint8_t)((bl16_size >> 24) & 0xFF);
	p += 4;
	/* Put one colored voxel at local (0,0,0). */
	gox[p + 0] = 200;
	gox[p + 1] = 100;
	gox[p + 2] = 50;
	gox[p + 3] = 255;
	p += bl16_size;
	p += 4; /* CRC */

	/* LAYR chunk: 1 entry placing block at (0,0,0). */
	memcpy(gox + p, "LAYR", 4);
	p += 4;
	gox[p] = 20;
	p += 4; /* length = 20 bytes */
	/* block_idx=0, px=0, py=0, pz=0, flags=0 */
	p += 20;
	p += 4; /* CRC */

	volume_t *vol = goxel_import_gox(gox, p);
	CHECK(vol != nullptr);
	CHECK(!volume_is_empty(vol));

	uint8_t out[4];
	int pos[3] = { 0, 0, 0 };
	volume_get_at(vol, nullptr, pos, out);
	CHECK(out[0] == 200);
	CHECK(out[1] == 100);

	free(gox);
	volume_delete(vol);
}

TEST_CASE("[Goxel] mirror filter") {
	Ref<GoxelVolume> gv;
	gv.instance();
	gv->set_voxel(Vector3(0, 0, 0), Color(1, 0, 0, 1));
	gv->set_voxel(Vector3(3, 0, 0), Color(0, 1, 0, 1));

	Ref<GoxelFilters> filters;
	filters.instance();
	filters->mirror(gv, GoxelFilters::AXIS_X);

	Color c0 = gv->get_voxel(Vector3(0, 0, 0));
	Color c3 = gv->get_voxel(Vector3(3, 0, 0));
	CHECK(c0.g > 0.9f); /* green moved to x=0 */
	CHECK(c3.r > 0.9f); /* red moved to x=3 */
}

TEST_CASE("[Goxel] color adjust filter") {
	Ref<GoxelVolume> gv;
	gv.instance();
	gv->set_voxel(Vector3(0, 0, 0), Color(0.5f, 0.5f, 0.5f, 1));

	Ref<GoxelFilters> filters;
	filters.instance();
	filters->adjust_colors(gv, 0, 0, 0.2f, 0);

	Color c = gv->get_voxel(Vector3(0, 0, 0));
	/* Lightness increased — color should be brighter. */
	CHECK(c.r > 0.6f);
}

TEST_CASE("[Goxel] generate palette") {
	Ref<GoxelVolume> gv;
	gv.instance();
	gv->set_voxel(Vector3(0, 0, 0), Color(1, 0, 0, 1));
	gv->set_voxel(Vector3(1, 0, 0), Color(0, 1, 0, 1));
	gv->set_voxel(Vector3(2, 0, 0), Color(0, 0, 1, 1));

	PoolColorArray pal = gv->generate_palette(4);
	CHECK(pal.size() == 4);
	/* At least some colors should be non-zero. */
	bool has_color = false;
	for (int i = 0; i < pal.size(); i++) {
		Color c = pal[i];
		if (c.r > 0.1f || c.g > 0.1f || c.b > 0.1f)
			has_color = true;
	}
	CHECK(has_color);
}

TEST_CASE("[Goxel] volume move transform") {
	Ref<GoxelVolume> gv;
	gv.instance();
	gv->set_voxel(Vector3(0, 0, 0), Color(1, 0, 0, 1));

	Transform t;
	t.origin = Vector3(5, 0, 0);
	gv->move(t);

	Color c_old = gv->get_voxel(Vector3(0, 0, 0));
	Color c_new = gv->get_voxel(Vector3(5, 0, 0));
	CHECK(c_old.a < 0.1f);
	CHECK(c_new.r > 0.9f);
}

TEST_CASE("[Goxel] volume crop") {
	Ref<GoxelVolume> gv;
	gv.instance();
	for (int x = 0; x < 10; x++)
		gv->set_voxel(Vector3(x, 0, 0), Color(1, 0, 0, 1));

	/* Crop to first 5 voxels. */
	gv->crop(AABB(Vector3(0, 0, 0), Vector3(5, 1, 1)));

	Color c4 = gv->get_voxel(Vector3(4, 0, 0));
	Color c5 = gv->get_voxel(Vector3(5, 0, 0));
	CHECK(c4.a > 0.9f);
	CHECK(c5.a < 0.1f);
}

#endif // DOCTEST
