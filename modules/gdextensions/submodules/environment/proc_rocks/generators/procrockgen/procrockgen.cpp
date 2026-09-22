/**************************************************************************/
/*  procrockgen.cpp                                                       */
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

#include "procrockgen.h"

#include "../rockstudio/rock_studio.h"
#include "../shared/rock_header.h"

#include "core/math/geometry.h"
#include "core/math/plane.h"
#include "modules/opensimplex/open_simplex_noise.h"
#include "scene/resources/gradient.h"
#include "servers/visual_server.h"

// =========================================================================
// Mesh pipeline: icosphere -> noise displacement -> optional cut-plane -> box UV
// =========================================================================

namespace {

Vector<Vector3> compute_smooth_normals(const Vector<Vector3> &p_vertices, const Vector<int> &p_indices) {
	Vector<Vector3> normals;
	normals.resize(p_vertices.size());
	for (int i = 0; i < normals.size(); i++) {
		normals.write[i] = Vector3();
	}
	for (int i = 0; i + 2 < p_indices.size(); i += 3) {
		int i0 = p_indices[i], i1 = p_indices[i + 1], i2 = p_indices[i + 2];
		Vector3 n = ComputeNormal(p_vertices[i0], p_vertices[i1], p_vertices[i2]);
		normals.write[i0] += n;
		normals.write[i1] += n;
		normals.write[i2] += n;
	}
	for (int i = 0; i < normals.size(); i++) {
		Vector3 n = normals[i];
		normals.write[i] = n.length_squared() > CMP_EPSILON ? n.normalized() : Vector3(0, 1, 0);
	}
	return normals;
}

struct ClippedMesh {
	Vector<Vector3> vertices;
	Vector<int> indices;
};

// Clips an indexed triangle mesh against a plane (keeping the side the plane's normal
// points away from) and caps the exposed cross-section with a triangle fan ordered by
// angle around its centroid. This assumes a single, star-shaped cross-section, which
// holds for cutting a lightly-displaced icosphere with one plane.
ClippedMesh clip_and_cap(const Vector<Vector3> &p_vertices, const Vector<int> &p_indices, const Plane &p_plane) {
	ClippedMesh out;
	Vector<Vector3> cap_points;
	const real_t on_plane_epsilon = real_t(CMP_EPSILON) * 100;

	for (int i = 0; i + 2 < p_indices.size(); i += 3) {
		Vector<Vector3> tri;
		tri.push_back(p_vertices[p_indices[i]]);
		tri.push_back(p_vertices[p_indices[i + 1]]);
		tri.push_back(p_vertices[p_indices[i + 2]]);

		Vector<Vector3> clipped = Geometry::clip_polygon(tri, p_plane);
		if (clipped.size() < 3) {
			continue;
		}

		int base = out.vertices.size();
		for (int v = 0; v < clipped.size(); v++) {
			out.vertices.push_back(clipped[v]);
			if (Math::abs(p_plane.distance_to(clipped[v])) <= on_plane_epsilon) {
				cap_points.push_back(clipped[v]);
			}
		}
		for (int v = 1; v + 1 < clipped.size(); v++) {
			out.indices.push_back(base);
			out.indices.push_back(base + v);
			out.indices.push_back(base + v + 1);
		}
	}

	if (cap_points.size() >= 3) {
		Vector3 centroid;
		for (int i = 0; i < cap_points.size(); i++) {
			centroid += cap_points[i];
		}
		centroid /= cap_points.size();

		Vector3 normal = p_plane.normal;
		Vector3 up = Math::abs(normal.dot(Vector3(0, 1, 0))) < 0.99 ? Vector3(0, 1, 0) : Vector3(1, 0, 0);
		Vector3 tangent = up.cross(normal).normalized();
		Vector3 bitangent = normal.cross(tangent);

		Vector<real_t> angles;
		angles.resize(cap_points.size());
		for (int i = 0; i < cap_points.size(); i++) {
			Vector3 d = cap_points[i] - centroid;
			angles.write[i] = Math::atan2(d.dot(bitangent), d.dot(tangent));
		}

		Vector<int> order;
		order.resize(cap_points.size());
		for (int i = 0; i < order.size(); i++) {
			order.write[i] = i;
		}
		for (int i = 1; i < order.size(); i++) {
			int key = order[i];
			real_t key_angle = angles[key];
			int j = i - 1;
			while (j >= 0 && angles[order[j]] > key_angle) {
				order.write[j + 1] = order[j];
				j--;
			}
			order.write[j + 1] = key;
		}

		int base = out.vertices.size();
		for (int i = 0; i < order.size(); i++) {
			out.vertices.push_back(cap_points[order[i]]);
		}
		for (int i = 1; i + 1 < order.size(); i++) {
			int a = base, b = base + i, c = base + i + 1;
			// Orient outward (towards the removed material) regardless of the fan's
			// natural winding, using the same cross(P2-P0,P1-P0) convention as ComputeNormal.
			Vector3 n = ComputeNormal(out.vertices[a], out.vertices[b], out.vertices[c]);
			if (n.dot(p_plane.normal) < 0) {
				out.indices.push_back(a);
				out.indices.push_back(c);
				out.indices.push_back(b);
			} else {
				out.indices.push_back(a);
				out.indices.push_back(b);
				out.indices.push_back(c);
			}
		}
	}

	return out;
}

} // namespace

Array rock_pipeline_gen(int p_subdivisions, real_t p_width, real_t p_height, real_t p_depth,
		real_t p_noise_frequency, real_t p_noise_amplitude, int p_noise_octaves, real_t p_noise_persistence,
		int p_randseed, bool p_cutplane_enabled, real_t p_cutplane_offset, bool p_smoothed) {
	if (p_randseed == 0) {
		Math::randomize();
	} else {
		Math::seed((uint64_t)p_randseed);
	}

	IndexedMesh ico = MakeIcosphere(CLAMP(p_subdivisions, 0, 6));

	Ref<OpenSimplexNoise> noise;
	noise.instance();
	noise->set_seed(p_randseed == 0 ? (int)Math::rand() : p_randseed);
	noise->set_octaves(CLAMP(p_noise_octaves, 1, 6));
	noise->set_period(1.0 / MAX(real_t(0.0001), p_noise_frequency));
	noise->set_persistence(p_noise_persistence);

	Vector<Vector3> vertices;
	vertices.resize(ico.first.size());
	for (size_t i = 0; i < ico.first.size(); i++) {
		Vector3 dir = ico.first[i]; // unit-sphere direction
		real_t displacement = noise->get_noise_3dv(dir) * p_noise_amplitude;
		vertices.write[i] = dir * Vector3(p_width, p_height, p_depth) * 0.5 + dir * displacement;
	}

	Vector<int> indices;
	indices.resize(ico.second.size() * 3);
	for (size_t i = 0; i < ico.second.size(); i++) {
		indices.write[i * 3 + 0] = ico.second[i].vertex[0];
		indices.write[i * 3 + 1] = ico.second[i].vertex[1];
		indices.write[i * 3 + 2] = ico.second[i].vertex[2];
	}

	if (p_cutplane_enabled) {
		Vector3 plane_normal = Vector3(Math::randf() * 2 - 1, Math::randf() * 2 - 1, Math::randf() * 2 - 1);
		if (plane_normal.length_squared() < CMP_EPSILON) {
			plane_normal = Vector3(0, 1, 0);
		}
		plane_normal.normalize();

		ClippedMesh clipped = clip_and_cap(vertices, indices, Plane(plane_normal, p_cutplane_offset));
		vertices = clipped.vertices;
		indices = clipped.indices;
	}

	Array mesh_arrays;
	mesh_arrays.resize(VS::ARRAY_MAX);
	if (vertices.size() == 0 || indices.size() == 0) {
		return mesh_arrays;
	}

	if (p_smoothed) {
		Vector<Vector3> normals = compute_smooth_normals(vertices, indices);

		Vector<Vector2> uvs;
		uvs.resize(vertices.size());
		for (int i = 0; i < vertices.size(); i++) {
			int box_dir = rock_studio_get_box_dir(normals[i]);
			uvs.write[i] = rock_studio_get_box_uv(vertices[i], box_dir);
		}

		mesh_arrays[VS::ARRAY_VERTEX] = vertices;
		mesh_arrays[VS::ARRAY_NORMAL] = normals;
		mesh_arrays[VS::ARRAY_TEX_UV] = uvs;
		mesh_arrays[VS::ARRAY_INDEX] = indices;
	} else {
		// rock_studio_make_low_poly() derives its flat normal via cross(v1-v0, v2-v0),
		// the opposite winding convention from ComputeNormal()'s cross(v2-v0, v1-v0) used
		// above — flip winding here so the flat-shaded result faces outward too.
		Vector<int> flipped_indices;
		flipped_indices.resize(indices.size());
		for (int i = 0; i + 2 < indices.size(); i += 3) {
			flipped_indices.write[i] = indices[i];
			flipped_indices.write[i + 1] = indices[i + 2];
			flipped_indices.write[i + 2] = indices[i + 1];
		}

		Array arrays;
		arrays.resize(VS::ARRAY_MAX);
		arrays[VS::ARRAY_VERTEX] = vertices;
		arrays[VS::ARRAY_INDEX] = flipped_indices;

		Ref<ArrayMesh> temp;
		temp.instance();
		temp->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

		Ref<ArrayMesh> low_poly = rock_studio_make_low_poly(temp);
		if (low_poly.is_valid() && low_poly->get_surface_count() > 0) {
			rock_studio_box_uv(low_poly);
			mesh_arrays = low_poly->surface_get_arrays(0);
		}
	}

	return mesh_arrays;
}

// =========================================================================
// Texture pipeline: noise height field -> albedo / normal / roughness / metalness / AO
// =========================================================================

namespace {

Ref<Image> make_height_image(int p_size, real_t p_noise_frequency, int p_noise_octaves, real_t p_noise_persistence, int p_randseed) {
	Ref<OpenSimplexNoise> noise;
	noise.instance();
	noise->set_seed(p_randseed == 0 ? (int)Math::rand() : p_randseed);
	noise->set_octaves(CLAMP(p_noise_octaves, 1, 6));
	noise->set_period(1.0 / MAX(real_t(0.0001), p_noise_frequency));
	noise->set_persistence(p_noise_persistence);
	return noise->get_image(p_size, p_size);
}

Ref<Image> make_albedo_image(Ref<Image> p_height, const Color &p_low, const Color &p_high) {
	int w = p_height->get_width(), h = p_height->get_height();
	Ref<Gradient> gradient;
	gradient.instance();
	gradient->set_color(0, p_low);
	gradient->set_color(1, p_high);

	Ref<Image> img;
	img.instance();
	img->create(w, h, false, Image::FORMAT_RGB8);

	p_height->lock();
	img->lock();
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			img->set_pixel(x, y, gradient->get_color_at_offset(p_height->get_pixel(x, y).r));
		}
	}
	img->unlock();
	p_height->unlock();
	return img;
}

Ref<Image> make_normal_image(Ref<Image> p_height, real_t p_strength) {
	int w = p_height->get_width(), h = p_height->get_height();
	Ref<Image> img;
	img.instance();
	img->create(w, h, false, Image::FORMAT_RGB8);

	p_height->lock();
	img->lock();
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			real_t hl = p_height->get_pixel(CLAMP(x - 1, 0, w - 1), y).r;
			real_t hr = p_height->get_pixel(CLAMP(x + 1, 0, w - 1), y).r;
			real_t hd = p_height->get_pixel(x, CLAMP(y - 1, 0, h - 1)).r;
			real_t hu = p_height->get_pixel(x, CLAMP(y + 1, 0, h - 1)).r;
			Vector3 n = Vector3(-(hr - hl) * p_strength, -(hu - hd) * p_strength, 1.0).normalized();
			img->set_pixel(x, y, Color(n.x * 0.5 + 0.5, n.y * 0.5 + 0.5, n.z * 0.5 + 0.5));
		}
	}
	img->unlock();
	p_height->unlock();
	return img;
}

Ref<Image> make_scaled_grayscale_image(Ref<Image> p_height, real_t p_scale, real_t p_bias) {
	int w = p_height->get_width(), h = p_height->get_height();
	Ref<Image> img;
	img.instance();
	img->create(w, h, false, Image::FORMAT_L8);

	p_height->lock();
	img->lock();
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			real_t v = CLAMP(p_height->get_pixel(x, y).r * p_scale + p_bias, real_t(0.0), real_t(1.0));
			img->set_pixel(x, y, Color(v, v, v));
		}
	}
	img->unlock();
	p_height->unlock();
	return img;
}

} // namespace

ProcRockPipelineTextures rock_pipeline_gen_textures(
		int p_size, real_t p_noise_frequency, int p_noise_octaves, real_t p_noise_persistence, int p_randseed,
		const Color &p_albedo_low, const Color &p_albedo_high, real_t p_normal_strength,
		real_t p_roughness_scale, real_t p_roughness_bias,
		real_t p_metalness_scale, real_t p_metalness_bias,
		real_t p_ao_scale, real_t p_ao_bias) {
	int size = CLAMP(p_size, 8, 4096);
	Ref<Image> height = make_height_image(size, p_noise_frequency, p_noise_octaves, p_noise_persistence, p_randseed);

	ProcRockPipelineTextures textures;
	textures.albedo.instance();
	textures.albedo->create_from_image(make_albedo_image(height, p_albedo_low, p_albedo_high));
	textures.normal.instance();
	textures.normal->create_from_image(make_normal_image(height, p_normal_strength));
	textures.roughness.instance();
	textures.roughness->create_from_image(make_scaled_grayscale_image(height, p_roughness_scale, p_roughness_bias));
	textures.metalness.instance();
	textures.metalness->create_from_image(make_scaled_grayscale_image(height, p_metalness_scale, p_metalness_bias));
	textures.ambient_occlusion.instance();
	textures.ambient_occlusion->create_from_image(make_scaled_grayscale_image(height, p_ao_scale, p_ao_bias));

	return textures;
}

Ref<SpatialMaterial> rock_pipeline_make_material(const ProcRockPipelineTextures &p_textures) {
	Ref<SpatialMaterial> material;
	material.instance();

	material->set_texture(SpatialMaterial::TEXTURE_ALBEDO, p_textures.albedo);
	material->set_texture(SpatialMaterial::TEXTURE_NORMAL, p_textures.normal);
	material->set_feature(SpatialMaterial::FEATURE_NORMAL_MAPPING, true);
	material->set_texture(SpatialMaterial::TEXTURE_ROUGHNESS, p_textures.roughness);
	material->set_roughness_texture_channel(SpatialMaterial::TEXTURE_CHANNEL_GRAYSCALE);
	material->set_texture(SpatialMaterial::TEXTURE_METALLIC, p_textures.metalness);
	material->set_metallic_texture_channel(SpatialMaterial::TEXTURE_CHANNEL_GRAYSCALE);
	material->set_texture(SpatialMaterial::TEXTURE_AMBIENT_OCCLUSION, p_textures.ambient_occlusion);
	material->set_ao_texture_channel(SpatialMaterial::TEXTURE_CHANNEL_GRAYSCALE);
	material->set_feature(SpatialMaterial::FEATURE_AMBIENT_OCCLUSION, true);

	return material;
}

// Editor-only baked texture packs (ProcRock dock "Demo Texture" picker) live in
// modules/gdextensions/editor/proc_rocks_editor_plugin.cpp — that's the only caller,
// and it's TOOLS_ENABLED-only already, so the loader belongs there, not in the
// generator API.
