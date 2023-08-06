/**************************************************************************/
/*  fast_noise.h                                                          */
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

#ifndef FASTNOISE_NOISE_H
#define FASTNOISE_NOISE_H

#include "core/image.h"
#include "core/resource.h"
#include "thirdparty/fastnoise/FastNoise.h"

typedef fastnoise::FastNoise _FastNoise;

class FastNoise : public Resource {
	GDCLASS(FastNoise, Resource)
	OBJ_SAVE_TYPE(FastNoise);

	_FastNoise _noise;
	Ref<FastNoise> _cellular_lookup_ref;

protected:
	static void _bind_methods();

public:
	// Enums Godot-style (same values)

	enum Type {
		TYPE_VALUE = _FastNoise::Value,
		TYPE_VALUE_FRACTAL = _FastNoise::ValueFractal,
		TYPE_PERLIN = _FastNoise::Perlin,
		TYPE_PERLIN_FRACTAL = _FastNoise::PerlinFractal,
		TYPE_SIMPLEX = _FastNoise::Simplex,
		TYPE_SIMPLEX_FRACTAL = _FastNoise::SimplexFractal,
		TYPE_CELLULAR = _FastNoise::Cellular,
		TYPE_WHITE_NOISE = _FastNoise::WhiteNoise,
		TYPE_CUBIC = _FastNoise::Cubic,
		TYPE_CUBIC_FRACTAL = _FastNoise::CubicFractal
	};

	enum Interpolation {
		INTERP_LINEAR = _FastNoise::Linear,
		INTERP_QUINTIC = _FastNoise::Quintic,
		INTERP_HERMITE = _FastNoise::Hermite
	};

	enum FractalType {
		FRACTAL_FBM = _FastNoise::FBM,
		FRACTAL_BILLOW = _FastNoise::Billow,
		FRACTAL_RIGID_MULTI = _FastNoise::RigidMulti
	};

	enum CellularDistanceFunction {
		DISTANCE_EUCLIDEAN = _FastNoise::Euclidean,
		DISTANCE_MANHATTAN = _FastNoise::Manhattan,
		DISTANCE_NATURAL = _FastNoise::Natural
	};

	enum CellularReturnType {
		RETURN_CELL_VALUE = _FastNoise::CellValue,
		RETURN_NOISE_LOOKUP = _FastNoise::NoiseLookup,
		RETURN_DISTANCE = _FastNoise::Distance,
		RETURN_DISTANCE_2 = _FastNoise::Distance2,
		RETURN_DISTANCE_2_ADD = _FastNoise::Distance2Add,
		RETURN_DISTANCE_2_SUB = _FastNoise::Distance2Sub,
		RETURN_DISTANCE_2_MUL = _FastNoise::Distance2Mul,
		RETURN_DISTANCE_2_DIV = _FastNoise::Distance2Div
	};

	int get_seed() const { return _noise.GetSeed(); }
	void set_seed(int seed) { _noise.SetSeed(seed); }

	void set_noise_type(Type noise_type) { _noise.SetNoiseType((_FastNoise::NoiseType)noise_type); }
	Type get_noise_type() const { return (Type)_noise.GetNoiseType(); }

	void set_interpolation(Interpolation interp) { _noise.SetInterp((_FastNoise::Interp)interp); }
	Interpolation get_interpolation() const { return (Interpolation)_noise.GetInterp(); }

	void set_frequency(real_t freq) { _noise.SetFrequency(freq); }
	real_t get_frequency() const { return _noise.GetFrequency(); }

	void set_fractal_octaves(unsigned int octave_count) { _noise.SetFractalOctaves(octave_count); }
	int get_fractal_octaves() const { return _noise.GetFractalOctaves(); }

	void set_fractal_lacunarity(real_t lacunarity) { _noise.SetFractalLacunarity(lacunarity); }
	real_t get_fractal_lacunarity() const { return _noise.GetFractalLacunarity(); }

	void set_fractal_gain(real_t gain) { _noise.SetFractalGain(gain); }
	real_t get_fractal_gain() const { return _noise.GetFractalGain(); }

	void set_fractal_type(FractalType type) { _noise.SetFractalType((_FastNoise::FractalType)type); }
	FractalType get_fractal_type() const { return (FractalType)_noise.GetFractalType(); }

	void set_cellular_distance_function(CellularDistanceFunction func) { _noise.SetCellularDistanceFunction((_FastNoise::CellularDistanceFunction)func); }
	CellularDistanceFunction get_cellular_distance_function() const { return (CellularDistanceFunction)_noise.GetCellularDistanceFunction(); }

	void set_cellular_return_type(CellularReturnType ret) { _noise.SetCellularReturnType((_FastNoise::CellularReturnType)ret); }
	CellularReturnType get_cellular_return_type() const { return (CellularReturnType)_noise.GetCellularReturnType(); }

	void set_cellular_noise_lookup(Ref<FastNoise> other_noise);
	Ref<FastNoise> get_cellular_noise_lookup() const { return _cellular_lookup_ref; }

	void set_cellular_distance_2_indices(int index0, int index1);
	PoolIntArray get_cellular_distance_2_indices() const;

	void set_cellular_jitter(real_t jitter) { _noise.SetCellularJitter(jitter); }
	real_t get_cellular_jitter() const { return _noise.GetCellularJitter(); }

	void set_gradient_perturbation_amplitude(real_t amp) { _noise.SetGradientPerturbAmp(amp); }
	real_t get_gradient_perturbation_amplitude() const { return _noise.GetGradientPerturbAmp(); }

	float get_noise_2d(float x, float y) const { return _noise.GetNoise(x, y); } // 2D
	float get_noise_3d(float x, float y, float z) const { return _noise.GetNoise(x, y, z); } // 3D
	float get_simplex_4d(float x, float y, float z, float w) const { return _noise.GetSimplex(x, y, z, w); } // 4D
	float get_white_noise_4d(float x, float y, float z, float w) const { return _noise.GetWhiteNoise(x, y, z, w); } // 4D

	Ref<Image> get_image(int p_width, int p_height, const Vector2 &p_noise_offset = Vector2()) const;
	Ref<Image> get_seamless_image(int p_size, bool p_white_noise = false) const;

	// Convenience

	float get_noise_2dv(Vector2 pos) { return _noise.GetNoise(pos.x, pos.y); }
	float get_noise_3dv(Vector3 pos) { return _noise.GetNoise(pos.x, pos.y, pos.z); }

	FastNoise();
};

// Make Variant happy with custom enums
VARIANT_ENUM_CAST(FastNoise::Type)
VARIANT_ENUM_CAST(FastNoise::FractalType)
VARIANT_ENUM_CAST(FastNoise::Interpolation)
VARIANT_ENUM_CAST(FastNoise::CellularDistanceFunction)
VARIANT_ENUM_CAST(FastNoise::CellularReturnType)

#endif // FASTNOISE_NOISE_H
