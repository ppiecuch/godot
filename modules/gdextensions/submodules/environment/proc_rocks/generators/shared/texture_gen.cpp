/**************************************************************************/
/*  texture_gen.cpp                                                       */
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

#include "texture_gen.h"

#include "core/math/vector3.h"
#include "modules/opensimplex/open_simplex_noise.h"

Ref<Image> make_height_image(int p_size, real_t p_noise_frequency, int p_noise_octaves, real_t p_noise_persistence, int p_randseed) {
	int octaves = CLAMP(p_noise_octaves, 1, 6);
	int seed = p_randseed == 0 ? (int)Math::rand() : p_randseed;

	// Same shape as procrocklib's Perlin node (plain octave sum, fixed lacunarity
	// 2.0) — see generators/procrockgen/procrockgen.cpp's NoiseGraph::make_simple_
	// fractal()/_eval_perlin_fractal(), reimplemented standalone here so this shared
	// module doesn't need procrockgen's private node-graph interpreter for the one
	// noise shape the scalar (non-JSON) texture API ever needs.
	Vector<Ref<OpenSimplexNoise>> octave_noises;
	octave_noises.resize(octaves);
	for (int i = 0; i < octaves; i++) {
		Ref<OpenSimplexNoise> n;
		n.instance();
		n->set_seed(seed + i);
		n->set_octaves(1);
		n->set_period(1.0); // position is pre-scaled by frequency/lacunarity below
		octave_noises.write[i] = n;
	}

	PoolVector<uint8_t> data;
	data.resize(p_size * p_size);
	{
		PoolVector<uint8_t>::Write wd8 = data.write();
		for (int y = 0; y < p_size; y++) {
			for (int x = 0; x < p_size; x++) {
				Vector3 p = Vector3(real_t(x), real_t(y), 0.0) * p_noise_frequency;
				real_t value = 0.0, cur_persistence = 1.0;
				for (int i = 0; i < octave_noises.size(); i++) {
					value += octave_noises[i]->get_noise_3dv(p) * cur_persistence;
					p *= 2.0; // lacunarity
					cur_persistence *= p_noise_persistence;
				}
				real_t v = value * 0.5 + 0.5; // normalize [0..1]
				wd8[y * p_size + x] = uint8_t(CLAMP(v * 255.0, real_t(0.0), real_t(255.0)));
			}
		}
	} // release the write lock before Image's constructor copies `data`
	return Ref<Image>(memnew(Image(p_size, p_size, false, Image::FORMAT_L8, data)));
}

Ref<Image> make_albedo_image_from_gradient(Ref<Image> p_height, Ref<Gradient> p_gradient) {
	int w = p_height->get_width(), h = p_height->get_height();

	Ref<Image> img;
	img.instance();
	img->create(w, h, false, Image::FORMAT_RGB8);

	p_height->lock();
	img->lock();
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			img->set_pixel(x, y, p_gradient->get_color_at_offset(p_height->get_pixel(x, y).r));
		}
	}
	img->unlock();
	p_height->unlock();
	return img;
}

Ref<Image> make_albedo_image(Ref<Image> p_height, const Color &p_low, const Color &p_high) {
	Ref<Gradient> gradient;
	gradient.instance();
	gradient->set_color(0, p_low);
	gradient->set_color(1, p_high);
	return make_albedo_image_from_gradient(p_height, gradient);
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

Ref<Image> make_constant_image(int p_size, const Color &p_color) {
	Ref<Image> img;
	img.instance();
	img->create(p_size, p_size, false, Image::FORMAT_RGB8);
	img->lock();
	for (int y = 0; y < p_size; y++) {
		for (int x = 0; x < p_size; x++) {
			img->set_pixel(x, y, p_color);
		}
	}
	img->unlock();
	return img;
}

Ref<ImageTexture> to_texture(const Ref<Image> &p_image) {
	Ref<ImageTexture> tex;
	tex.instance();
	tex->create_from_image(p_image);
	return tex;
}

ProcRockPipelineTextures rock_pipeline_gen_textures(
		int p_size, real_t p_noise_frequency, int p_noise_octaves, real_t p_noise_persistence, int p_randseed,
		const Color &p_albedo_low, const Color &p_albedo_high, real_t p_normal_strength,
		real_t p_roughness_scale, real_t p_roughness_bias,
		real_t p_metalness_scale, real_t p_metalness_bias,
		real_t p_ao_scale, real_t p_ao_bias) {
	int size = CLAMP(p_size, 8, 4096);
	Ref<Image> height = make_height_image(size, p_noise_frequency, p_noise_octaves, p_noise_persistence, p_randseed);

	ProcRockPipelineTextures textures;
	textures.albedo = to_texture(make_albedo_image(height, p_albedo_low, p_albedo_high));
	textures.normal = to_texture(make_normal_image(height, p_normal_strength));
	textures.roughness = to_texture(make_scaled_grayscale_image(height, p_roughness_scale, p_roughness_bias));
	textures.metalness = to_texture(make_scaled_grayscale_image(height, p_metalness_scale, p_metalness_bias));
	textures.ambient_occlusion = to_texture(make_scaled_grayscale_image(height, p_ao_scale, p_ao_bias));

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
