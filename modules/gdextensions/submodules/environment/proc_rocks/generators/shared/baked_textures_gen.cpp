/**************************************************************************/
/*  baked_textures_gen.cpp                                                */
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

#include "baked_textures_gen.h"

#include <cstring>

#include "editor/proc_rocks_demo/baked_textures.h"

namespace {

Ref<Image> load_embedded_jpg(const uint8_t *p_data, unsigned int p_size) {
	PoolByteArray buf;
	buf.resize(p_size);
	{
		PoolByteArray::Write w = buf.write();
		memcpy(w.ptr(), p_data, p_size);
	}
	Ref<Image> img;
	img.instance();
	img->load_jpg_from_buffer(buf);
	return img;
}

} // namespace

ProcRockPipelineTextures load_baked_textures(ProcRockBakedTexturePack p_pack) {
	ProcRockPipelineTextures textures;
	// Baked sets have no metalness map (rock isn't metallic) — a constant black texture
	// is equivalent to metallic=0 without needing a dedicated scalar path.
	Ref<ImageTexture> non_metal = to_texture(make_constant_image(8, Color(0, 0, 0)));

	switch (p_pack) {
		case PROCROCK_BAKED_GRAVEL:
			textures.albedo = to_texture(load_embedded_jpg(gravel_albedo_jpg_data, gravel_albedo_jpg_size));
			textures.normal = to_texture(load_embedded_jpg(gravel_normals_jpg_data, gravel_normals_jpg_size));
			textures.roughness = to_texture(load_embedded_jpg(gravel_roughness_jpg_data, gravel_roughness_jpg_size));
			textures.ambient_occlusion = to_texture(load_embedded_jpg(gravel_ambientOcc_jpg_data, gravel_ambientOcc_jpg_size));
			textures.metalness = non_metal;
			break;
		case PROCROCK_BAKED_MOSSY:
			textures.albedo = to_texture(load_embedded_jpg(moss_albedo_jpg_data, moss_albedo_jpg_size));
			textures.normal = to_texture(load_embedded_jpg(moss_normals_jpg_data, moss_normals_jpg_size));
			textures.roughness = to_texture(load_embedded_jpg(moss_roughness_jpg_data, moss_roughness_jpg_size));
			textures.ambient_occlusion = to_texture(load_embedded_jpg(moss_ambientOcc_jpg_data, moss_ambientOcc_jpg_size));
			textures.metalness = non_metal;
			break;
		case PROCROCK_BAKED_ROCK:
		default:
			// Only a single combined albedo texture ships for this pack.
			textures.albedo = to_texture(load_embedded_jpg(rock_jpg_data, rock_jpg_size));
			textures.normal = to_texture(make_constant_image(8, Color(0.5, 0.5, 1.0)));
			textures.roughness = to_texture(make_constant_image(8, Color(0.6, 0.6, 0.6)));
			textures.ambient_occlusion = to_texture(make_constant_image(8, Color(1, 1, 1)));
			textures.metalness = non_metal;
			break;
	}

	return textures;
}
