#include "starfield.h"

#include <random>
#include <functional>

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


namespace
{

	std::mt19937 randomGenerator;
	std::uniform_real_distribution<real_t> randomDistributionAlpha(0.05, 1.0);
	std::function <unsigned short int()> randomAlpha;

	inline void randomSeed() {
		std::random_device rd;
		randomGenerator.seed(rd());
		randomAlpha = std::bind(randomDistributionAlpha, randomGenerator);
	}

	inline float randomValue(const float low, const float high) {
		return std::uniform_real_distribution<float>{low, high}(randomGenerator);
	}

} // namespace


void Starfield::_update_mesh() {
	if (_mesh.is_null())
		_mesh = Ref<ArrayMesh>(memnew(ArrayMesh));

	Array mesh_array;
	for (auto &layer : _layers) {
		mesh_array.clear();
		mesh_array.resize(VS::ARRAY_MAX);
		mesh_array[VS::ARRAY_VERTEX] = layer.positions;

		_mesh->clear_mesh();
		_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_POINTS, mesh_array, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);
		_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, mesh_array, Array(), Mesh::ARRAY_FLAG_USE_2D_VERTICES);
	}
}

void Starfield::move(Vector2 p_movement) {
	for (auto &layer : _layers) {
		auto w = layer.positions.write();
		for (int p=0; p<layer.positions.size(); ++p) {
			auto &position = w[p];
			// move
			position += p_movement * (static_cast<float>(layer.color.a) / 255.f);
			// wrap
			if (position.x < 0)
				position = { layer.size.x, randomValue(0.f, layer.size.y), };
			else if (position.x > layer.size.x)
				position = { 0.f, randomValue(0.f, layer.size.y), };
			if (position.y < 0)
				position = { randomValue(0.f, layer.size.x), layer.size.y, };
			else if (position.y > layer.size.y)
				position = { randomValue(0.f, layer.size.x) , 0.f };
		}
	}
}

void Starfield::regenerate() {
	for (unsigned int l=0; l<_layers.size(); ++l) {
		regenerate(l);
	}
}

void Starfield::regenerate(layerid_t p_layer) {
	ERR_FAIL_COND(p_layer >= _layers.size());

	auto &layer = _layers[p_layer];
	ERR_FAIL_COND(layer.positions.size() != layer.colors.size());

	auto wp = layer.positions.write();
	auto wc = layer.colors.write();
	for (int p=0; p<layer.positions.size(); ++p) {
		auto &position = wp[p];
		auto &color = wc[p];
		position = { randomValue(0.f, layer.size.x), randomValue(0.f, layer.size.y) };
		color = layer.color;
		color.a = randomAlpha();
	}
}

void Starfield::regenerate(layerid_t p_layer, const Vector2 p_size) {
	ERR_FAIL_COND(p_layer >= _layers.size());

	auto &layer = _layers[p_layer];
	layer.size = p_size;
	regenerate(p_layer);
}

void Starfield::regenerate(layerid_t p_layer, const Vector2 p_size, unsigned int p_number_of_stars) {
	ERR_FAIL_COND(p_layer >= _layers.size());

	auto &layer = _layers[p_layer];
	layer.positions.resize(p_number_of_stars);
	layer.colors.resize(p_number_of_stars);
	regenerate(p_layer, layer.size);
}

void Starfield::regenerate(layerid_t p_layer, unsigned int p_number_of_stars) {
	ERR_FAIL_COND(p_layer >= _layers.size());

	auto &layer = _layers[p_layer];
	regenerate(p_layer, layer.size, p_number_of_stars);
}

void Starfield::set_color(layerid_t p_layer, const Color &p_color) {
	ERR_FAIL_COND(p_layer >= _layers.size());

	auto &layer = _layers[p_layer];
	layer.color = p_color;
	auto w = layer.colors.write();
	for (int c=0; c<layer.colors.size(); ++c) {
		auto &color = w[c];
		const real_t alphaDepth { color.a };
		color = p_color;
		color.a = alphaDepth;
	}
}

void Starfield::draw(Node2D &canvas) {
}

Starfield::Starfield() {
	randomSeed();
}
