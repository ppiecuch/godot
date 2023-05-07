#pragma once

#include "core/math/random_pcg.h"
#include "core/math/math_funcs.h"

class DDLSRandGenerator {
	int original_seed;
	int curr_seed;
	int range_min;
	int range_max;

	int num_iter;

	RandomPCG rng;

public:
	void set_seed(int p_value) { original_seed = curr_seed = p_value; }
	void set range_min(int p_value) { range_min = p_value; }
	void set range_max(int p_value) { range_max = p_value; }

	int get_seed() const { return original_seed; }
	int get_range_min() const { return range_min; }
	int get_range_max() const { return range_max; }

	void reset() {
		curr_seed = original_seed;
		num_iter = 0;
	}

	int next() {
		String temp_string = (curr_seed*curr_seed).toString();
		while (temp_string.size() < 8) {
			temp_string = "0" + temp_string;
		}
		curr_seed = int(temp_string.substr(1, 5));
		int res = Math::round(range_min + (curr_seed / 99999) * (range_max - range_min));

		if (curr_seed == 0) {
			curr_seed = original_seed + num_iter;
		}
		num_iter++;

		if (num_iter == 200) {
			reset();
		}
		return res;
	}

	DDLSRandGenerator(int p_seed = 1234, int range_min = 0, int range_max = 1) {
		original_seed = curr_seed = p_seed;
		range_min = p_range_min;
		range_max = p_range_max;

		num_iter = 0;
	}
};
