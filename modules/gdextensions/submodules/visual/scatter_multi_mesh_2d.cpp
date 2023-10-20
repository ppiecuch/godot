#include "scatter_multi_mesh.h"

#include <vector>

// Reference:
// ----------
// https://blog.demofox.org/2017/05/29/when-random-numbers-are-too-random-low-discrepancy-sequences/
// http://devmag.org.za/2009/05/03/poisson-disk-sampling/

static std::vector<Point2> _get_samples_2d(int num, size_t basex, size_t basey, const Rect2 &view) {
	// calculate the sample points
	std::vector<Point2> samples;
	samples.resize(num);
	for (size_t i = 0; i < num; ++i) {
		// x axis
		samples[i].x = 0;
		{
			real_t denominator = basex;
			size_t n = i;
			while (n > 0) {
				const size_t multiplier = n % basex;
				samples[i].x += multiplier / denominator;
				n = n / basex;
				denominator *= basex;
			}
		}
		// y axis
		samples[i].y = 0;
		{
			real_t denominator = basey;
			size_t n = i;
			while (n > 0) {
				const size_t multiplier = n % basey;
				samples[i].y += multiplier / denominator;
				n = n / basey;
				denominator *= basey;
			}
		}
	}
	for (size_t i = 0; i < num; ++i) {
		samples[i].x = view.position.x + samples[i].x * view.size.width;
		samples[i].y = view.position.y + samples[i].y * view.size.height;
	}
	return samples;
}

static std::vector<Point2> _poisson_disk_sampling_2d(real_t r, int k, Size2 area) {
	std::vector<Point2> res;
	std::vector<int> active_list;

	// Step 0.

	real_t cell_size = r / Math::sqrt(2.0f);

	int cols = Math::floor(area.x / cell_size) + 1;
	int rows = Math::floor(area.y / cell_size) + 1;

	std::vector<std::vector<int>> grid;
	for (int i = 0; i < rows; i++) {
		std::vector<int> arr;
		for (int j = 0; j < cols; j++) {
			arr.push_back(-1);
		}
		grid.push_back(arr);
	}

	// Step 1.

#define F real_t

	real_t initial_x = Math::random(F(0), area.x);
	real_t initial_y = Math::random(F(0), area.y);
	Point2 initial_position = Point2(initial_x, initial_y);

	int initial_x_index = Math::floor(initial_x / cell_size);
	int initial_y_index = Math::floor(initial_y / cell_size);
	grid[initial_y_index][initial_x_index] = 0;

	active_list.push_back(0);
	res.push_back(initial_position);

	// Step 2.

	int current_index = 0;
	while (active_list.size() > 0) {
		auto active_index = active_list[Math::random(0, active_list.size() - 1)];
		auto active_pos = res[active_index];

		bool found = false;
		for (int i = 0; i < k; i++) {
			real_t radian = Math::random(F(0), F(2 * Math_PI));
			real_t radius = Math::random(r, r * 2);
			Point2 p = Point2(Math::cos(radian), Math::sin(radian)) * radius + active_pos;

			int x_index = Math::floor(p.x / cell_size);
			int y_index = Math::floor(p.y / cell_size);
			if (x_index < 0 || y_index <0 || x_index > cols - 1 || y_index > rows - 1) {
				continue;
			}

			if (grid[y_index][x_index] == -1) {
				bool far_from_all = true;
				for (int j = -1;j <= 1; j++) {
					for (int k = -1;k <= 1; k++) {
						const int check_x_index = x_index + k;
						const int check_y_index = y_index + j;
						if (check_x_index < 0 || check_y_index <0 || check_x_index > cols - 1 || check_y_index > rows - 1) {
							continue;
						}
						const int check_index = grid[check_y_index][check_x_index];
						if (check_index == -1) {
							continue;
						}
						const Point2 target_pos = res[check_index];
						if ((target_pos - p).length() < r) {
							far_from_all = false;
						}
					}
				}

				if (far_from_all) {
					current_index++;
					grid[y_index][x_index] = current_index;
					active_list.push_back(current_index);
					res.push_back(p);
					found = true;
					break;
				}
			}
		}
		if (!found) {
			active_list.erase(active_list.begin() + active_index);
		}
	}

	return res;
}

static std::vector<Point2> _get_poisson_disk_sampling_2d_nearly_n(int n, int k, Point2 area) {
	const real_t r = Math::sqrt(area.x * area.y / real_t(n)) * 0.84f;
	return _poisson_disk_sampling_2d(r, k, area);
}

ScatterMultiMesh::ScatterMultiMesh() {
	num_layers = 1;
}

ScatterMultiMesh::~ScatterMultiMesh() {
}
