/**************************************************************************/
/*  scatter_multi_mesh_2d.cpp                                             */
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

#include "scatter_multi_mesh_2d.h"

#include "common/gd_core.h"
#include "scene/resources/primitive_meshes.h"

#include <vector>

// Reference:
// ----------
// https://blog.demofox.org/2017/05/29/when-random-numbers-are-too-random-low-discrepancy-sequences/
// http://devmag.org.za/2009/05/03/poisson-disk-sampling/
// https://github.com/arcaneenergy/godot-multimesh-scatter/tree/3.5

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

	const real_t cell_size = r / Math::sqrt(2.0f);

	const int cols = Math::floor(area.x / cell_size) + 1;
	const int rows = Math::floor(area.y / cell_size) + 1;

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

	const real_t initial_x = Math::random(F(0), area.x);
	const real_t initial_y = Math::random(F(0), area.y);
	const Point2 initial_position = Point2(initial_x, initial_y);

	const int initial_x_index = Math::floor(initial_x / cell_size);
	const int initial_y_index = Math::floor(initial_y / cell_size);
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
			if (x_index < 0 || y_index < 0 || x_index > cols - 1 || y_index > rows - 1) {
				continue;
			}

			if (grid[y_index][x_index] == -1) {
				bool far_from_all = true;
				for (int j = -1; j <= 1; j++) {
					for (int k = -1; k <= 1; k++) {
						const int check_x_index = x_index + k;
						const int check_y_index = y_index + j;
						if (check_x_index < 0 || check_y_index < 0 || check_x_index > cols - 1 || check_y_index > rows - 1) {
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
	const real_t r = Math::sqrt(area.x * area.y / real_t(n)) * 0.84;
	return _poisson_disk_sampling_2d(r, k, area);
}

void ScatterMultiMesh2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			if (Engine::get_singleton()->is_editor_hint()) {
				if (show_debug_area) {
					_create_debug_area();
				} else {
					_delete_debug_area();
				}
			} else {
				set_notify_transform(false);
				// set_ignore_transform_notification(true);
			}
			if (!get_multimesh()) {
				set_multimesh(newref(MultiMesh));
				get_multimesh()->set_transform_format(MultiMesh::TRANSFORM_3D);
			}
		} break;
	}
}

void ScatterMultiMesh2D::scatter() {
	ERR_FAIL_NULL(get_multimesh());

	get_multimesh()->set_instance_count(0);
	int count = 0;
	for (const auto &c : layers_info) {
		count += c.num_instances;
	}
	get_multimesh()->set_instance_count(count);
}

ScatterMultiMesh2D::ScatterMultiMesh2D() {
	num_layers = 1;
	layers_info.resize(1);
	offset_position = Vector3(0.0, 0.0, 0.0);
	offset_rotation = Vector3(0.0, 0.0, 0.0);
	base_scale = Vector3(1.0, 1.0, 1.0);
	min_random_size = Vector3(0.75, 0.75, 0.75);
	max_random_size = Vector3(1.25, 1.25, 1.25);
	random_rotation = Vector3(0.0, 0.0, 0.0);
	show_debug_area = false;
}

ScatterMultiMesh2D::~ScatterMultiMesh2D() {
}
