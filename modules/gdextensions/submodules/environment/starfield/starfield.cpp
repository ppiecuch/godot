/*************************************************************************/
/*  starfield.cpp                                                        */
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

#include "starfield.h"
#include "starfield_res.h"

#include <functional>
#include <random>
#include <vector>

#include "common/gd_core.h"
#include "common/gd_pack.h"

// Reference:
// ----------
// https://github.com/marian42/starfield
// https://github.com/PixelProphecy/gml_starfield_generator
// https://github.com/mshang/starfield
// https://github.com/transitive-bullshit/react-starfield-animation
// https://github.com/rocketwagon/jquery-starfield
// https://github.com/jparise/pygame-starfield
// https://github.com/NQX/StarfieldJS
// https://github.com/noelleleigh/starfield-maker
// https://github.com/zatakeshi/Pygame-Parallax-Scrolling-Starfield
// https://github.com/johnprattchristian/starfielder

namespace {

std::mt19937 RandomGenerator;
std::uniform_real_distribution<float> RandomDistributionAlpha(0.1, 1.0);
std::function<float()> RandomAlpha;

inline void random_seed() {
	std::random_device rd;
	RandomGenerator.seed(rd());
	RandomAlpha = std::bind(RandomDistributionAlpha, RandomGenerator);
}

inline real_t random_value(const int low, const int high) {
	return std::uniform_int_distribution<int>{ low, high }(RandomGenerator);
}

static std::vector<float> _alpha_lookup{
	0.6462, 0.6870, 0.6670, 0.7671, 0.3030, 0.4929, 0.2815, 0.6442,
	0.3401, 0.9845, 0.8320, 0.3762, 0.7365, 0.6013, 0.6089, 0.1346,
	0.4758, 0.2487, 0.8488, 0.1849, 0.4089, 0.2104, 0.6042, 0.8024,
	0.4067, 0.9428, 0.1295, 0.2970, 0.6488, 0.7909, 0.2311, 0.1137,
	0.7310, 0.7010, 0.2790, 0.2187, 0.6707, 0.7851, 0.1197, 0.9316,
	0.6610, 0.1679, 0.6417, 0.3727, 0.7128, 0.4739, 0.3426, 0.4673,
	0.9990, 0.8499, 0.3561, 0.7083, 0.6838, 0.8036, 0.9736, 0.6326,
	0.8653, 0.3328, 0.7176, 0.3611, 0.3104, 0.4559, 0.3494, 0.6628,
	0.6496, 0.2488, 0.5653, 0.5508, 0.5700, 0.7294, 0.6159, 0.7945,
	0.1402, 0.8441, 0.8384, 0.8550, 0.2441, 0.8147, 0.2361, 0.7882,
	0.8873, 0.8873, 0.8094, 0.8949, 0.3640, 0.4038, 0.2805, 0.8390,
	0.8565, 0.7006, 0.9633, 0.8428, 0.2800, 0.3657, 0.4986, 0.2607,
	0.9052, 0.8094, 0.4922, 0.6909, 0.6575, 0.3623, 0.3835, 0.4072,
	0.4112, 0.9143, 0.2019, 0.3582, 0.1722, 0.4323, 0.6113, 0.4459,
	0.9118, 0.4709, 0.2843, 0.9970, 0.4669, 0.9020, 0.8138, 0.5592,
	0.1876, 0.2444, 0.1682, 0.4917, 0.5552, 0.3405, 0.1563, 0.2642,
	0.2936, 0.7190, 0.5185, 0.1350, 0.2426, 0.9194, 0.8382, 0.1145,
	0.3434, 0.5320, 0.6270, 0.1091, 0.8007, 0.3067, 0.2632, 0.4090,
	0.5879, 0.5129, 0.3172, 0.6842, 0.8515, 0.2461, 0.9068, 0.4170,
	0.6433, 0.8788, 0.3140, 0.4898, 0.9538, 0.1634, 0.8505, 0.1426,
	0.1021, 0.6239, 0.8938, 0.2153, 0.5939, 0.9687, 0.6825, 0.4009,
	0.3884, 0.7777, 0.3931, 0.7675, 0.2161, 0.7178, 0.6807, 0.8459,
	0.6105, 0.7685, 0.8024, 0.1058, 0.9258, 0.6114, 0.4634, 0.9548,
	0.3765, 0.9430, 0.7885, 0.3102, 0.9802, 0.5791, 0.9699, 0.7221,
	0.7053, 0.7691, 0.1030, 0.8765, 0.1978, 0.1516, 0.9522, 0.7499,
	0.4933, 0.3619, 0.3403, 0.4490, 0.1958, 0.8930, 0.4965, 0.2145,
	0.5879, 0.3393, 0.3697, 0.4153, 0.9890, 0.8510, 0.5336, 0.6036,
	0.4606, 0.7112, 0.9495, 0.9479, 0.5071, 0.4910, 0.6440, 0.6229,
	0.7191, 0.7307, 0.2083, 0.5413, 0.2917, 0.8857, 0.5282, 0.1083,
	0.6476, 0.3468, 0.2657, 0.9747, 0.3981, 0.7326, 0.2327, 0.2484,
	0.8534, 0.9252, 0.7614, 0.6388, 0.1079, 0.2621, 0.6749, 0.8501,
	0.8894, 0.8110, 0.4473, 0.7483, 0.2035, 0.8135, 0.9283, 0.1020
};

static Ref<Starfield::CacheInfo> _texture_cache;

} // namespace

void Starfield::_regenerate() {
	for (unsigned int l = 0; l < _layers.size(); ++l) {
		regenerate(l);
	}
}

void Starfield::_update_mesh() {
	if (_mesh_solid.is_valid()) {
		_mesh_solid->clear_mesh();
	}
	if (_mesh_textured.is_valid()) {
		_mesh_textured->clear_mesh();
	}
	Array mesh_array;
	for (auto &layer : _layers) {
		mesh_array.clear();
		mesh_array.resize(VS::ARRAY_MAX);
		mesh_array[VS::ARRAY_VERTEX] = layer.vertexes;
		mesh_array[VS::ARRAY_COLOR] = layer.colors;

		if (layer.uv.size() > 0) {
			mesh_array[VS::ARRAY_TEX_UV] = layer.uv;

			if (_mesh_textured.is_null()) {
				_mesh_textured = newref(ArrayMesh);
			}
			_mesh_textured->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);
		} else {
			if (_mesh_solid.is_null()) {
				_mesh_solid = newref(ArrayMesh);
			}
			if (layer.star_size == 0 && layer.texture_id == STAR_POINT) {
				_mesh_solid->add_surface_from_arrays(Mesh::PRIMITIVE_POINTS, mesh_array, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);
			} else {
				_mesh_solid->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);
			}
		}
	}
	_needs_refresh = false;
}

layerid_t Starfield::add_stars(int p_number_of_stars, Size2 p_layer_size, real_t p_star_size, StarTexture p_texture_id, const Color &p_color) {
	if (p_texture_id > STAR_TEXTURE_VALID) {
		WARN_PRINT("Invalid texture id.");
		p_texture_id = STAR1_TEXTURE;
	}

	layerid_t lid = _layers.size();
	_layers.push_back({
			p_number_of_stars,
			PoolVector2Array(),
			PoolVector2Array(),
			PoolColorArray(),
			p_layer_size,
			Vector2(1, 1),
			true,
			p_texture_id,
			p_star_size,
			0,
			p_color,
			true,
	});
	_needs_refresh = true;
	return lid;
}

void Starfield::move(real_t p_delta, Vector2 p_movement) {
	static real_t _timer = 0;
	static int _counter = 0;

	for (auto &layer : _layers) {
		auto w = layer.vertexes.write();
		auto c = layer.colors.write();
		auto uv = layer.uv.write();

		const bool valid_colors_array = layer.vertexes.size() == layer.colors.size();

		const Vector2 sidex(layer.star_size, 0);
		const Vector2 sidey(0, layer.star_size);

		for (int p = 0; p < layer.vertexes.size(); ++p) {
			Vector2 &position = w[p];
			real_t alpha = 1;
			// light
			if (valid_colors_array) {
				alpha = c[p].a;
				if (layer.star_pulsation) {
					alpha = _alpha_lookup[p & 0xff];
					const real_t alpha_modulation = Math::sin(_timer * layer.star_pulsation);
					const real_t new_alpha = c[p].a = alpha * alpha_modulation * alpha_modulation;
					if (layer.star_size > 0) {
						// only fade opposite vertexes:
						// (*)--+
						//  | / |
						//  +--(*)
						c[p + 4].a = new_alpha;
					}
				}
			}
			const Vector2 delta = p_movement * (layer.movement_scale_with_alpha ? alpha : 1) * layer.movement_scale;
			position += delta; // move
			bool wrapx = false, wrapy = false; // wrap
			if ((wrapx = position.x < -layer.star_size)) {
				position = { layer.layer_size.x + layer.star_size, random_value(0, layer.layer_size.y) };
			} else if ((wrapx = position.x > layer.layer_size.x + layer.star_size)) {
				position = { 0, random_value(-layer.star_size, layer.layer_size.y) };
			}
			if ((wrapy = position.y < 0)) {
				position = { random_value(0, layer.layer_size.x), layer.layer_size.y };
			} else if ((wrapy = position.y > layer.layer_size.y)) {
				position = { random_value(0, layer.layer_size.x), 0 };
			}
			// regenerate rest of the quad if necessery
			if (layer.star_size > 0 || layer.texture_id != STAR_POINT) {
				int quad_origin = p;
				if (wrapx || wrapy) {
					// w[p] = position;        // +--+
					w[++p] = position + sidex; // | /
					w[++p] = position + sidey; // +

					w[++p] = position + sidex; //           +
					w[++p] = position + sidex + sidey; // / |
					w[++p] = position + sidey; //        +--+
				} else {
					w[++p] += delta;
					w[++p] += delta;
					w[++p] += delta;
					w[++p] += delta;
					w[++p] += delta;
				}
				// different pulsation phase for texture animation
				// const real_t anim_speed = 1/MAX(layer.star_pulsation, 0.2);
				StarTexture new_texture_id = layer.texture_id;
				switch (layer.texture_id) {
					case STAR12_TEXTURE_FRAME1:
						new_texture_id = STAR12_TEXTURE_FRAME2;
						break;
					case STAR12_TEXTURE_FRAME2:
						new_texture_id = STAR12_TEXTURE_FRAME3;
						break;
					case STAR12_TEXTURE_FRAME3:
						new_texture_id = STAR12_TEXTURE_FRAME4;
						break;
					case STAR12_TEXTURE_FRAME4:
						new_texture_id = STAR12_TEXTURE_FRAME5;
						break;
					case STAR12_TEXTURE_FRAME5:
						new_texture_id = STAR12_TEXTURE_FRAME6;
						break;
					case STAR12_TEXTURE_FRAME6:
						new_texture_id = STAR12_TEXTURE_FRAME1;
						break;
					default:
						// not an animated texture
						continue;
				}

#define ANIM_FRAMES 120

				if (new_texture_id != layer.texture_id) {
					const int next_frame = _counter % int(ANIM_FRAMES * (layer.star_pulsation == 0 ? 1 : 1 / layer.star_pulsation));
					if (next_frame == 0) {
						layer.texture_id = new_texture_id;
						// update texture
						PoolVector2Array texture_uv = _texture_cache->rects[new_texture_id];
						uv[quad_origin++] = texture_uv[0];
						uv[quad_origin++] = texture_uv[1];
						uv[quad_origin++] = texture_uv[2];
						uv[quad_origin++] = texture_uv[3];
						uv[quad_origin++] = texture_uv[4];
						uv[quad_origin++] = texture_uv[5];
					}
				}
			}
		}
	}
	_counter++;
	_timer += p_delta;
	_needs_refresh = true;
}

void Starfield::_push_quad(PoolVector2Array &array, Point2 origin, real_t size) {
	const Vector2 sidex(size, 0);
	const Vector2 sidey(0, size);

	// 0--1,2
	// | // |
	// 2,4--3
	array.push_back(origin);
	array.push_back(origin + sidex);
	array.push_back(origin + sidey);

	array.push_back(origin + sidex);
	array.push_back(origin + sidex + sidey);
	array.push_back(origin + sidey);
}

void Starfield::_insert_quad(PoolVector2Array &array, int position, Point2 origin, real_t size) {
	const Vector2 sidex(size, 0);
	const Vector2 sidey(0, size);

	auto w = array.write();

	// 0--1,2
	// | // |
	// 2,4--3
	w[position++] = origin;
	w[position++] = origin + sidex;
	w[position++] = origin + sidey;

	w[position++] = origin + sidex;
	w[position++] = origin + sidex + sidey;
	w[position++] = origin + sidey;
}

void Starfield::regenerate(layerid_t p_layer) {
	ERR_FAIL_COND(p_layer >= _layers.size());

	auto &layer = _layers[p_layer];

	if (!layer._dirty) {
		return;
	}

	PoolPoint2Array positions;
	PoolColorArray colors;

	ERR_FAIL_COND(positions.resize(layer.num_stars) != OK);
	ERR_FAIL_COND(colors.resize(layer.num_stars) != OK);

	auto wp = positions.write();
	auto wc = colors.write();
	for (int p = 0; p < layer.num_stars; ++p) {
		auto &position = wp[p];
		auto &color = wc[p];
		position = { random_value(0, layer.layer_size.x), random_value(0, layer.layer_size.y) };
		color = layer.base_color;
		color.a = RandomAlpha();
	}

	if (layer.star_size == 0 && layer.texture_id == STAR_POINT) {
		layer.vertexes = positions;
		layer.colors = colors;
	} else {
		PoolVector2Array vertexes, uv;
		PoolColorArray vertexes_color;

		const bool textured_star = layer.texture_id >= STAR1_TEXTURE && layer.texture_id <= STAR12_TEXTURE;

#define QUAD_SIZE 6

		ERR_FAIL_COND(vertexes.resize(layer.num_stars * QUAD_SIZE) != OK);
		ERR_FAIL_COND(vertexes_color.resize(layer.num_stars * QUAD_SIZE) != OK);
		ERR_FAIL_COND(uv.resize(textured_star ? layer.num_stars * QUAD_SIZE : 0) != OK);

		for (int p = 0, q = 0; p < layer.num_stars; ++p, q += QUAD_SIZE) {
			_insert_quad(vertexes, q, positions[p], layer.star_size);
			vertexes_color.insert_multi(q, 6, colors[p]);
			if (textured_star) {
				uv.insert_array(q, _texture_cache->rects[layer.texture_id]);
			}
		}
		layer.vertexes = vertexes;
		layer.colors = vertexes_color;
		layer.uv = uv;
	}

	layer._dirty = false;
}

void Starfield::regenerate(layerid_t p_layer, const Size2 p_layer_size) {
	ERR_FAIL_COND(p_layer >= _layers.size());

	auto &layer = _layers[p_layer];

	layer.layer_size = p_layer_size;
	layer._dirty = true;

	_needs_refresh = true;
}

void Starfield::regenerate(layerid_t p_layer, const Size2 p_layer_size, int p_number_of_stars) {
	ERR_FAIL_COND(p_layer >= _layers.size());
	ERR_FAIL_COND(p_number_of_stars <= 0);

	auto &layer = _layers[p_layer];

	layer.num_stars = p_number_of_stars;
	layer.layer_size = p_layer_size;
	layer._dirty = true;

	_needs_refresh = true;
}

void Starfield::regenerate(layerid_t p_layer, int p_number_of_stars) {
	ERR_FAIL_COND(p_layer >= _layers.size());
	ERR_FAIL_COND(p_number_of_stars <= 0);

	auto &layer = _layers[p_layer];

	layer.num_stars = p_number_of_stars;
	layer._dirty = true;

	_needs_refresh = true;
}

void Starfield::set_color(layerid_t p_layer, const Color &p_base_color, real_t p_star_pulsation) {
	ERR_FAIL_COND(p_layer >= _layers.size());

	auto &layer = _layers[p_layer];

	layer.base_color = p_base_color;
	layer.star_pulsation = p_star_pulsation;

	auto w = layer.colors.write();
	for (int c = 0; c < layer.colors.size(); ++c) {
		auto &color = w[c];
		const real_t alpha_depth = layer.star_pulsation > 0 ? _alpha_lookup[c & 0xff] : color.a;
		color = p_base_color;
		color.a = alpha_depth;
	}
}

void Starfield::set_movement(layerid_t p_layer, Vector2 p_movement_scale, bool p_with_alpha) {
	ERR_FAIL_COND(p_layer >= _layers.size());

	auto &layer = _layers[p_layer];

	layer.movement_scale = p_movement_scale;
	layer.movement_scale_with_alpha = p_with_alpha;
}

void Starfield::draw(Node2D *p_canvas) {
	ERR_FAIL_COND(p_canvas == nullptr);

	if (_needs_refresh) {
		_regenerate();
		_update_mesh();
	}
	if (_mesh_textured.is_valid()) {
		p_canvas->draw_mesh(_mesh_textured, _texture_cache->texture, Ref<Texture>());
	}
	if (_mesh_solid.is_valid()) {
		p_canvas->draw_mesh(_mesh_solid, Ref<Texture>(), Ref<Texture>());
	}
}

void Starfield::ready(Node2D *p_owner) {
	ERR_FAIL_COND(p_owner == nullptr);

	if (!_texture_cache.is_valid()) {
		Ref<CacheInfo> cache_info = newref(CacheInfo);

		// build texture atlas from resources
		Vector<Ref<Image>> images;
		Vector<String> names;
		std::vector<EmbedImageItem> embed(embed_starfield_res, embed_starfield_res + embed_starfield_res_count);
		for (const auto &r : embed) {
			ERR_CONTINUE_MSG(r.channels < 3, "Format is not supported, Skipping!");
			Ref<Image> image;
			image.instance();
			PoolByteArray data;
			data.resize(r.size);
			memcpy(data.write().ptr(), r.pixels, r.size);
			image->create(r.width, r.height, false, r.channels == 4 ? Image::FORMAT_RGBA8 : Image::FORMAT_RGB8, data);
			images.push_back(image);
			names.push_back(r.image);
		}
		Dictionary atlas_info = merge_images(images, names);

		ERR_FAIL_COND(atlas_info.empty());

		Array pages = atlas_info["_generated_images"];
		if (pages.size() > 1) {
			WARN_PRINT("Too many texture pages - using only first one for Starfield.");
		}
		Ref<Image> atlas_image = pages[0];

		Vector2 atlas_size(1, 1);
		if (atlas_image.is_valid()) {
			Ref<ImageTexture> texture = newref(ImageTexture);
			texture->create_from_image(atlas_image);
			atlas_size = texture->get_size();
			cache_info->texture = texture;
		} else {
			WARN_PRINT("Atlas image is not valid, Skipping!");
		}

		Dictionary atlas_rects = atlas_info["_rects"];
		for (int j = 0; j < names.size(); ++j) {
			String name = names[j];
			Dictionary entry = atlas_rects[name];
			Rect2 rect = entry["rect"];

			ERR_CONTINUE_MSG(entry.empty(), "Empty atlas entry, Skipping!");

			const Vector2 uv_origin = rect.position / atlas_size;
			const Vector2 uv_size = rect.size / atlas_size;
			static const Vector2 sideu(1, 0);
			static const Vector2 sidev(0, 1);

			// 0--1,2
			// | // |
			// 2,4--3
			PoolVector2Array uv;
			uv.push_back(uv_origin + uv_size * sidev);
			uv.push_back(uv_origin + uv_size);
			uv.push_back(uv_origin);
			uv.push_back(uv_origin + uv_size);
			uv.push_back(uv_origin + uv_size * sideu);
			uv.push_back(uv_origin);

			if (name == "star01.png")
				cache_info->rects[STAR1_TEXTURE] = uv;
			else if (name == "star02.png")
				cache_info->rects[STAR2_TEXTURE] = uv;
			else if (name == "star03.png")
				cache_info->rects[STAR3_TEXTURE] = uv;
			else if (name == "star04.png")
				cache_info->rects[STAR4_TEXTURE] = uv;
			else if (name == "star05.png")
				cache_info->rects[STAR5_TEXTURE] = uv;
			else if (name == "star06.png")
				cache_info->rects[STAR6_TEXTURE] = uv;
			else if (name == "star07.png")
				cache_info->rects[STAR7_TEXTURE] = uv;
			else if (name == "star08.png")
				cache_info->rects[STAR8_TEXTURE] = uv;
			else if (name == "star09.png")
				cache_info->rects[STAR9_TEXTURE] = uv;
			else if (name == "star10.png")
				cache_info->rects[STAR10_TEXTURE] = uv;
			else if (name == "star11.png")
				cache_info->rects[STAR11_TEXTURE] = uv;
			else if (name == "star12.png")
				cache_info->rects[STAR12_TEXTURE] = uv;
			else if (name == "frame0.png") {
				cache_info->rects[STAR12_TEXTURE_FRAME1] = uv;
				cache_info->rects[STAR12_TEXTURE_FRAME6] = uv;
			} else if (name == "frame1.png") {
				cache_info->rects[STAR12_TEXTURE_FRAME2] = uv;
				cache_info->rects[STAR12_TEXTURE_FRAME5] = uv;
			} else if (name == "frame2.png") {
				cache_info->rects[STAR12_TEXTURE_FRAME3] = uv;
				cache_info->rects[STAR12_TEXTURE_FRAME4] = uv;
			}
		}
		_texture_cache = cache_info;

		_register_global_ref(_texture_cache);
	}
}

Starfield::Starfield() {
	_mesh_solid = Ref<ArrayMesh>(NULL);
	_mesh_textured = Ref<ArrayMesh>(NULL);
	_needs_refresh = false;

	random_seed();
}
