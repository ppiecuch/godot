
// simple real_t ALU heavy benchmark
// no dependency on any math library

#include "core/math/math_funcs.h"

static uint32_t EvaluateJuliaFractalAt(real_t x, real_t y) {
	uint32_t ret = 0;

	real_t dist2 = 0;
	const real_t maxdist2 = 1600;
	uint32_t maxiter = 300;

	// pretty Julia fractal
	real_t cx = -0.73;
	real_t cy = 0.176;

	while (dist2 <= maxdist2 && ret < maxiter) {
		const real_t x2 = x * x - y * y + cx;
		const real_t y2 = 2 * x * y + cy;

		x = x2;
		y = y2;

		++ret;
		dist2 = x * x + y * y;
	}

	return ret;
}

real_t FractalBenchmark() {
	const int32_t extent = 256; // scales quadratic with the time

	real_t sum = 0;

	for (int y = 0; y < extent; ++y) {
		for (int x = 0; x < extent; ++x) {
			sum += EvaluateJuliaFractalAt(x / (real_t)extent * 2 - 1, y / (real_t)extent * 2 - 1);
		}
	}

	// Aggressive math optimizations and inline expansion can optimize this benchmark's return value into a constant
	// Introduce some randomness to avoid this, noting the return value of this function is not used, rather the time it takes to complete
	return sum / (real_t)(extent * extent) + Math::rand();
}
