/**************************************************************************/
/*  vtrimmer.cpp                                                          */
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

/*
 * This file is a part of the work done by Humus. You are free to
 * use the code in any way you like, modified, unmodified or copied
 * into your own work. However, I expect you to respect these points:
 *  - If you use this file and its contents unmodified, or use a major
 *    part of this file, please credit the author and leave this note.
 *  - For use in anything commercial, please request my approval.
 *  - Share your work and ideas too as much as you can.
 */
#include "vtrimmer.h"

#if TOOLS_ENABLED

#include "core/math/vector2.h"

_FORCE_INLINE_ static real_t distance(const Point2 &pt1, const Point2 &pt2) {
	return pt1.distance_to(pt2);
}
_FORCE_INLINE_ static real_t dot(const Point2 &pt1, const Point2 &pt2) {
	return pt1.dot(pt2);
}

vertex_trimmer_opt_t vertex_trimmer_default_opt = {
	4,
	0,
	{ 1, 1 },
	{ 0, 0 },
	1,
	1,
	50,
	16,
	0,
	{ 0 },
	false
};

struct CHNode {
	CHNode *Prev;
	CHNode *Next;
	Point2 Point;
};

static real_t AreaX2Of(const Point2 &v0, const Point2 &v1, const Point2 &v2) {
	Point2 u = v1 - v0;
	Point2 v = v2 - v0;
	return /* Math::absf */ (u.y * v.x - u.x * v.y);
}

class ConvexHull {
	CHNode *m_Root;
	CHNode *m_Curr;
	unsigned m_Count;

public:
	ConvexHull() {
		m_Root = m_Curr = nullptr;
		m_Count = 0;
	}
	~ConvexHull() { Clear(); }

	void Clear() {
		if (m_Root) {
			CHNode *node = m_Root;
			CHNode *next;
			do {
				next = node->Next;
				memdelete(node);
				node = next;
			} while (node != m_Root);

			m_Root = nullptr;
			m_Count = 0;
		}
		m_Curr = nullptr;
	}

	bool InsertPoint(const Point2 &point);
	bool RemoveLeastRelevantEdge();
	unsigned FindOptimalPolygon(Point2 *dest, unsigned vertex_count, real_t *area = nullptr);

	bool GoToFirst() { return (m_Curr = m_Root) != nullptr; }
	bool GoToNext() { return (m_Curr = m_Curr->Next) != m_Root; }

	const Point2 &GetCurrPoint() const { return m_Curr->Point; }
	const Point2 &GetNextPoint() const { return m_Curr->Next->Point; }
	const Point2 &GetPrevPoint() const { return m_Curr->Prev->Point; }

	unsigned GetCount() const { return m_Count; }
	real_t GetArea() const {
		if (m_Count < 3) {
			return 0;
		}
		real_t area = 0;
		const Point2 &v0 = m_Root->Point;
		CHNode *node = m_Root->Next;
		do {
			const Point2 &v1 = node->Point;
			node = node->Next;
			const Point2 &v2 = node->Point;
			area += AreaX2Of(v0, v1, v2);
		} while (node != m_Root);
		return 0.5 * area;
	}
};

struct WorkPacket {
	ConvexHull Hull;
	Point2 Polygon[8];
	real_t Area;
	real_t HullArea;
	unsigned Rotation;
};

static int FindOptimalRotation(Point2 *vertices, int vertex_count, unsigned *indices);

/// Public interface:

bool vertex_trimmer(Ref<Image> &image, vertex_trimmer_opt_t *opt) {
	unsigned threshold = 0;
	unsigned vertex_count = 4;
	Point2 scale(1, 1);
	Point2 bias(0, 0);
	unsigned atlas_x = 1;
	unsigned atlas_y = 1;
	unsigned max_hull_size = 50;
	int sub_pixel = 16;
	unsigned dilate_count = 0;

	unsigned indices[(8 - 2) * 3];
	bool use_index_buffer = false;

	if (opt) {
		vertex_count = opt->vertex_count;
		threshold = opt->treshold;
		scale = Point2(opt->scale.x, opt->scale.y);
		bias = Point2(opt->bias.x, opt->bias.y);
		atlas_x = opt->atlas_x;
		atlas_y = opt->atlas_y;
		max_hull_size = opt->max_hull_size;
		sub_pixel = opt->sub_pixel;
		dilate_count = opt->dilate_count;
		use_index_buffer = opt->use_index_buffer;
	}

	const real_t threshold_f = (real_t)threshold;

	if (threshold > 255) {
		ERR_FAIL_V_MSG(false, vformat("(vertex_trimmer) Error: Threshold %d out of range", threshold));
	}

	if (vertex_count < 3 || vertex_count > 8) {
		ERR_FAIL_V_MSG(false, vformat("(vertex_trimmer) Error: Vertex count %d out of range", vertex_count));
	}

	// const bool using_atlas = (atlas_x > 1 || atlas_y > 1);

	Ref<Image> img = image->converted(Image::FORMAT_L8);
	ERR_FAIL_NULL_V(img, false);

	const int w = img->get_width();
	const int h = img->get_height();

	if (dilate_count) {
		const int data_size = img->get_data().size();
		unsigned char *copy = (unsigned char *)memalloc(data_size);
		while (dilate_count--) {
			unsigned char *dst = img->get_data().write().ptr();
			memcpy(copy, dst, data_size);
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++) {
					const int startX = MAX(x - 1, 0);
					const int endX = MIN(x + 2, w);
					const int startY = MAX(y - 1, 0);
					const int endY = MIN(y + 2, h);
					int max = 0;
					for (int iy = startY; iy < endY; iy++) {
						for (int ix = startX; ix < endX; ix++) {
							const int c = copy[iy * w + ix];
							if (c > max) {
								max = c;
							}
						}
					}
					*dst++ = max;
				}
			}
		}
		memfree(copy);
	}

	const unsigned char *pixels = img->get_data().read().ptr();
	const int tile_w = w / atlas_x;
	const int tile_h = h / atlas_y;

	scale.x *= 2.0 / tile_w;
	scale.y *= 2.0 / tile_h;

	// Set up convex hulls
	WorkPacket *packets = new WorkPacket[atlas_x * atlas_y];
	for (unsigned ay = 0; ay < atlas_y; ay++) {
		for (unsigned ax = 0; ax < atlas_x; ax++) {
			ConvexHull &hull = packets[ay * atlas_x + ax].Hull;

			const int start_x = ax * tile_w;
			const int start_y = ay * tile_h;
			const int end_x = start_x + tile_w - 1;
			const int end_y = start_y + tile_h - 1;

			const real_t off_x = 0.5 * (tile_w - 1);
			const real_t off_y = 0.5 * (tile_h - 1);
			const real_t corner_off_x = 0.5 * tile_w;
			const real_t corner_off_y = 0.5 * tile_h;

			// Corner cases
			if (pixels[start_y * w + start_x] > threshold) {
				hull.InsertPoint(Point2(-corner_off_x, -corner_off_y));
			}

			if (pixels[start_y * w + end_x] > threshold) {
				hull.InsertPoint(Point2(corner_off_x, -corner_off_y));
			}

			if (pixels[end_y * w + start_x] > threshold) {
				hull.InsertPoint(Point2(-corner_off_x, corner_off_y));
			}

			if (pixels[end_y * w + end_x] > threshold) {
				hull.InsertPoint(Point2(corner_off_x, corner_off_y));
			}

			// Edge cases
			const unsigned char *row = pixels + start_y * w;
			for (int x = start_x; x < end_x; x++) {
				const unsigned c0 = row[x + 0];
				const unsigned c1 = row[x + 1];

				if ((c0 > threshold) != (c1 > threshold)) {
					const real_t d0 = c0;
					const real_t d1 = c1;

					const real_t sub_pixel_x = (threshold_f - d0) / (d1 - d0);
					hull.InsertPoint(Point2(x - start_x - off_x + sub_pixel_x, -corner_off_y));
				}
			}

			row = pixels + end_y * w;
			for (int x = start_x; x < end_x; x++) {
				const unsigned c0 = row[x + 0];
				const unsigned c1 = row[x + 1];

				if ((c0 > threshold) != (c1 > threshold)) {
					const real_t d0 = c0;
					const real_t d1 = c1;

					const real_t sub_pixel_x = (threshold_f - d0) / (d1 - d0);
					hull.InsertPoint(Point2(x - start_x - off_x + sub_pixel_x, corner_off_y));
				}
			}

			const unsigned char *col = pixels + start_x;
			for (int y = start_y; y < end_y; y++) {
				const unsigned c0 = col[(y + 0) * w];
				const unsigned c1 = col[(y + 1) * w];

				if ((c0 > threshold) != (c1 > threshold)) {
					const real_t d0 = c0;
					const real_t d1 = c1;

					const real_t sub_pixel_y = (threshold_f - d0) / (d1 - d0);
					hull.InsertPoint(Point2(-corner_off_x, y - start_y - off_y + sub_pixel_y));
				}
			}

			col = pixels + end_x;
			for (int y = start_y; y < end_y; y++) {
				const unsigned c0 = col[(y + 0) * w];
				const unsigned c1 = col[(y + 1) * w];

				if ((c0 > threshold) != (c1 > threshold)) {
					const real_t d0 = c0;
					const real_t d1 = c1;

					const real_t sub_pixel_y = (threshold_f - d0) / (d1 - d0);
					hull.InsertPoint(Point2(corner_off_x, y - start_y - off_y + sub_pixel_y));
				}
			}

			// The interior pixels
			for (int y = start_y; y < end_y; y++) {
				const unsigned char *row0 = pixels + (y + 0) * w;
				const unsigned char *row1 = pixels + (y + 1) * w;

				for (int x = start_x; x < end_x; x++) {
					const unsigned c00 = row0[x + 0];
					const unsigned c01 = row0[x + 1];
					const unsigned c10 = row1[x + 0];
					const unsigned c11 = row1[x + 1];

					int count = 0;
					if (c00 > threshold) {
						++count;
					}
					if (c01 > threshold) {
						++count;
					}
					if (c10 > threshold) {
						++count;
					}
					if (c11 > threshold) {
						++count;
					}

					if (count > 0 && count < 4) {
						const real_t d00 = c00;
						const real_t d01 = c01;
						const real_t d10 = c10;
						const real_t d11 = c11;

						for (int n = 0; n <= sub_pixel; n++) {
							// Lerping factors
							const real_t f0 = real_t(n) / real_t(sub_pixel);
							const real_t f1 = 1 - f0;

							const real_t x0 = d00 * f1 + d10 * f0;
							const real_t x1 = d01 * f1 + d11 * f0;

							if ((x0 > threshold_f) != (x1 > threshold_f)) {
								const real_t sub_pixel_x = (threshold_f - x0) / (x1 - x0);
								hull.InsertPoint(Point2(x - start_x - off_x + sub_pixel_x, y - start_y - off_y + f0));
							}

							const real_t y0 = d00 * f1 + d01 * f0;
							const real_t y1 = d10 * f1 + d11 * f0;

							if ((y0 > threshold_f) != (y1 > threshold_f)) {
								const real_t sub_pixel_y = (threshold_f - y0) / (y1 - y0);
								hull.InsertPoint(Point2(x - start_x - off_x + f0, y - start_y - off_y + sub_pixel_y));
							}
						}
					}
				}
			}

			print_verbose(vformat("Convex hull has %d vertices", hull.GetCount()));
			if (hull.GetCount() > max_hull_size) {
				const real_t area_before = hull.GetArea();
				do {
					if (!hull.RemoveLeastRelevantEdge()) {
						break;
					}
				} while (hull.GetCount() > max_hull_size);
				real_t area_after = hull.GetArea();
				print_verbose(vformat("Convex hull was reduced to %d vertices. Area expanded from %.2f%% to %.2f%%", hull.GetCount(), 100.0 * area_before / (tile_w * tile_h), 100.0 * area_after / (tile_w * tile_h)));
			}
		}
	}

	// Do the heavy work
	for (unsigned n = 0; n < atlas_x * atlas_y; n++) {
		WorkPacket &packet = packets[n];
		const int count = packet.Hull.FindOptimalPolygon(packet.Polygon, vertex_count, &packet.Area);
		packet.HullArea = packet.Hull.GetArea();
		// Scale-bias the results
		for (int i = 0; i < count; i++) {
			packet.Polygon[i] = packet.Polygon[i] * scale + bias;
		}
		// If fewer vertices were returned than asked for, just repeat the last vertex
		for (unsigned i = count; i < vertex_count; i++) {
			packet.Polygon[i] = packet.Polygon[count - 1];
		}
		// Optimize vertex ordering
		if (use_index_buffer) {
			packet.Rotation = FindOptimalRotation(packet.Polygon, vertex_count, indices);
		} else {
			packet.Rotation = 0;
		}
	}

	delete[] packets;

	return true;
}

static int FindOptimalRotation(Point2 *vertices, int vertex_count, unsigned *indices) {
	const int index_count = (vertex_count - 2) * 3;

	int optimal = 0;
	real_t min_length = FLT_MAX;

	for (int i = 0; i < vertex_count; i++) {
		real_t sum = 0;
		for (int k = 0; k < index_count; k += 3) {
			const int i0 = (indices[k + 0] + i) % vertex_count;
			const int i1 = (indices[k + 1] + i) % vertex_count;
			const int i2 = (indices[k + 2] + i) % vertex_count;
			const Point2 &v0 = vertices[i0];
			const Point2 &v1 = vertices[i1];
			const Point2 &v2 = vertices[i2];
			sum += distance(v0, v1);
			sum += distance(v1, v2);
			sum += distance(v2, v0);
		}
		if (sum < min_length) {
			optimal = i;
			min_length = sum;
		}
	}
	return optimal;
}

/// ConvexHull

struct Line {
	Point2 v;
	Point2 d;
};

#define perp(u, v) ((u).x * (v).y - (u).y * (v).x)

static bool Intersect(Point2 &point, const Line &line0, const Line &line1) {
#if 0
	const real_t d = perp(line0.d, line1.d);
	if (d > -0.000000000001) { // Parallel lines
		return false;
}
	Point2 diff = line0.v - line1.v;

	const real_t t = perp(line1.d, diff);

	if (t > 0.0) { // Intersects on the wrong side
		return false;
}
	point = line0.v + (t / d) * line0.d;
	return true;
#else
	const real_t d = perp(line0.d, line1.d);
	if (Math::abs(d) < 0.000000000001) { // Parallel lines
		return false;
	}
	const real_t t = perp(line1.d, line0.v - line1.v) / d;
	if (t < 0.5) { // Intersects on the wrong side
		return false;
	}
	point = line0.v + t * line0.d;
	return true;
#endif
}

static bool IntersectNoParallelCheck(Point2 &point, const Line &line0, const Line &line1) {
	const real_t d = perp(line0.d, line1.d);
	const real_t t = perp(line1.d, line0.v - line1.v) / d;
	if (t < 0.5) { // Intersects on the wrong side
		return false;
	}
	point = line0.v + t * line0.d;
	return true;
}

bool ConvexHull::InsertPoint(const Point2 &point) {
	if (m_Count < 2) {
		CHNode *node = new CHNode;
		node->Point = point;
		if (m_Root == nullptr) {
			m_Root = node;
		} else {
			node->Prev = m_Root;
			node->Next = m_Root;
		}
		m_Root->Next = node;
		m_Root->Prev = node;
		++m_Count;
		return true;
	}

	CHNode *node = m_Root;

	const Point2 &root_v0 = node->Prev->Point;
	const Point2 &root_v1 = node->Point;

	Point2 root_dir = root_v1 - root_v0;
	Point2 root_nrm(-root_dir.y, root_dir.x);

	if (dot(point - root_v0, root_nrm) > 0) {
		do {
			node = node->Prev;
			const Point2 &v0 = node->Prev->Point;
			const Point2 &v1 = node->Point;

			Point2 dir = v1 - v0;
			Point2 nrm(-dir.y, dir.x);

			if (dot(point - v0, nrm) <= 0) {
				node = node->Next;
				break;
			}
		} while (true);
	} else {
		do {
			const Point2 &v0 = node->Point;
			node = node->Next;
			const Point2 &v1 = node->Point;

			Point2 dir = v1 - v0;
			Point2 nrm(-dir.y, dir.x);

			if (dot(point - v0, nrm) > 0) {
				break;
			}
			if (node == m_Root) {
				return false;
			}
		} while (true);
	}
	do {
		const Point2 &v0 = node->Point;
		const Point2 &v1 = node->Next->Point;

		Point2 dir = v1 - v0;
		Point2 nrm(-dir.y, dir.x);

		if (dot(point - v0, nrm) <= 0) {
			break;
		}

		// Delete this node
		node->Prev->Next = node->Next;
		node->Next->Prev = node->Prev;

		CHNode *del = node;
		node = node->Next;
		memdelete(del);
		--m_Count;

	} while (true);

	CHNode *new_node = memnew(CHNode);
	new_node->Point = point;
	++m_Count;

	new_node->Prev = node->Prev;
	new_node->Next = node;

	node->Prev->Next = new_node;
	node->Prev = new_node;

	m_Root = new_node;

	return true;
}

bool ConvexHull::RemoveLeastRelevantEdge() {
	CHNode *min_node = nullptr;
	Point2 min_pos;
	real_t min_area = 1e10;

	CHNode *node = m_Root;
	do {
		const Point2 &v0 = node->Prev->Point;
		const Point2 &v1 = node->Point;
		const Point2 &v2 = node->Next->Point;
		const Point2 &v3 = node->Next->Next->Point;

		Line line0 = { v0, v1 - v0 };
		Line line1 = { v2, v3 - v2 };

		Point2 v;
		if (IntersectNoParallelCheck(v, line0, line1)) {
			const real_t area = AreaX2Of(v1, v, v2);
			if (area < min_area) {
				min_node = node;
				min_pos = v;
				min_area = area;
			}
		}
		node = node->Next;
	} while (node != m_Root);

	if (min_node) {
		min_node->Point = min_pos;

		CHNode *del = min_node->Next;
		min_node->Next->Next->Prev = min_node;
		min_node->Next = min_node->Next->Next;

		if (del == m_Root) {
			m_Root = min_node;
		}
		memdelete(del);
		--m_Count;
		return true;
	}
	return false;
}

unsigned ConvexHull::FindOptimalPolygon(Point2 *dest, unsigned vertex_count, real_t *area) {
	if (vertex_count > m_Count) {
		vertex_count = m_Count;
	}
	if (vertex_count < 3) {
		if (area) {
			*area = 0;
		}
		return 0;
	}
	if (vertex_count > 8) {
		vertex_count = 8;
	}
	// Allocate memory on stack
	Line *lines = (Line *)alloca(m_Count * sizeof(Line));

	CHNode *node = m_Root;

	// Precompute lines
	unsigned n = 0;
	do {
		lines[n].v = node->Point;
		lines[n].d = node->Next->Point - node->Point;
		// lines[n].v += 0.5 * lines[n].d; // Move origin to center of line
		node = node->Next;
		++n;
	} while (node != m_Root);

	ERR_FAIL_COND_V(n == m_Count, 0);

	real_t min_area = 1e10;

	Point2 v[8];
	Point2 &v0 = v[0];
	Point2 &v1 = v[1];
	Point2 &v2 = v[2];
	Point2 &v3 = v[3];
	Point2 &v4 = v[4];
	Point2 &v5 = v[5];
	Point2 &v6 = v[6];
	Point2 &v7 = v[7];

	// This can probably be made a lot prettier and generic
	switch (vertex_count) {
		case 3: {
			for (unsigned x = 0; x < n; x++) {
				for (unsigned y = x + 1; y < n; y++) {
					if (Intersect(v0, lines[x], lines[y])) {
						for (unsigned z = y + 1; z < n; z++) {
							if (Intersect(v1, lines[y], lines[z])) {
								if (Intersect(v2, lines[z], lines[x])) {
									Point2 u0 = v1 - v0;
									Point2 u1 = v2 - v0;
									const real_t _area = (u0.y * u1.x - u0.x * u1.y);
									if (_area < min_area) {
										min_area = _area;
										dest[0] = v0;
										dest[1] = v1;
										dest[2] = v2;
									}
								}
							}
						}
					}
				}
			}
		} break;
		case 4: {
			for (unsigned x = 0; x < n; x++) {
				for (unsigned y = x + 1; y < n; y++) {
					if (Intersect(v0, lines[x], lines[y])) {
						for (unsigned z = y + 1; z < n; z++) {
							if (Intersect(v1, lines[y], lines[z])) {
								for (unsigned w = z + 1; w < n; w++) {
									if (Intersect(v2, lines[z], lines[w])) {
										if (Intersect(v3, lines[w], lines[x])) {
											Point2 u0 = v1 - v0;
											Point2 u1 = v2 - v0;
											Point2 u2 = v3 - v0;
											const real_t _area =
													(u0.y * u1.x - u0.x * u1.y) +
													(u1.y * u2.x - u1.x * u2.y);
											if (_area < min_area) {
												min_area = _area;
												dest[0] = v0;
												dest[1] = v1;
												dest[2] = v2;
												dest[3] = v3;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		} break;
		case 5: {
			for (unsigned x = 0; x < n; x++) {
				for (unsigned y = x + 1; y < n; y++) {
					if (Intersect(v0, lines[x], lines[y])) {
						for (unsigned z = y + 1; z < n; z++) {
							if (Intersect(v1, lines[y], lines[z])) {
								for (unsigned w = z + 1; w < n; w++) {
									if (Intersect(v2, lines[z], lines[w])) {
										for (unsigned r = w + 1; r < n; r++) {
											if (Intersect(v3, lines[w], lines[r])) {
												if (Intersect(v4, lines[r], lines[x])) {
													Point2 u0 = v1 - v0;
													Point2 u1 = v2 - v0;
													Point2 u2 = v3 - v0;
													Point2 u3 = v4 - v0;
													const real_t _area =
															(u0.y * u1.x - u0.x * u1.y) +
															(u1.y * u2.x - u1.x * u2.y) +
															(u2.y * u3.x - u2.x * u3.y);
													if (_area < min_area) {
														min_area = _area;
														dest[0] = v0;
														dest[1] = v1;
														dest[2] = v2;
														dest[3] = v3;
														dest[4] = v4;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		} break;
		case 6: {
			for (unsigned x = 0; x < n; x++) {
				for (unsigned y = x + 1; y < n; y++) {
					if (Intersect(v0, lines[x], lines[y])) {
						for (unsigned z = y + 1; z < n; z++) {
							if (Intersect(v1, lines[y], lines[z])) {
								for (unsigned w = z + 1; w < n; w++) {
									if (Intersect(v2, lines[z], lines[w])) {
										for (unsigned r = w + 1; r < n; r++) {
											if (Intersect(v3, lines[w], lines[r])) {
												for (unsigned s = r + 1; s < n; s++) {
													if (Intersect(v4, lines[r], lines[s])) {
														if (Intersect(v5, lines[s], lines[x])) {
															Point2 u0 = v1 - v0;
															Point2 u1 = v2 - v0;
															Point2 u2 = v3 - v0;
															Point2 u3 = v4 - v0;
															Point2 u4 = v5 - v0;
															const real_t _area =
																	(u0.y * u1.x - u0.x * u1.y) +
																	(u1.y * u2.x - u1.x * u2.y) +
																	(u2.y * u3.x - u2.x * u3.y) +
																	(u3.y * u4.x - u3.x * u4.y);
															if (_area < min_area) {
																min_area = _area;
																dest[0] = v0;
																dest[1] = v1;
																dest[2] = v2;
																dest[3] = v3;
																dest[4] = v4;
																dest[5] = v5;
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		} break;
		case 7: {
			for (unsigned x = 0; x < n; x++) {
				for (unsigned y = x + 1; y < n; y++) {
					if (Intersect(v0, lines[x], lines[y])) {
						for (unsigned z = y + 1; z < n; z++) {
							if (Intersect(v1, lines[y], lines[z])) {
								for (unsigned w = z + 1; w < n; w++) {
									if (Intersect(v2, lines[z], lines[w])) {
										for (unsigned r = w + 1; r < n; r++) {
											if (Intersect(v3, lines[w], lines[r])) {
												for (unsigned s = r + 1; s < n; s++) {
													if (Intersect(v4, lines[r], lines[s])) {
														for (unsigned t = s + 1; t < n; t++) {
															if (Intersect(v5, lines[s], lines[t])) {
																if (Intersect(v6, lines[t], lines[x])) {
																	Point2 u0 = v1 - v0;
																	Point2 u1 = v2 - v0;
																	Point2 u2 = v3 - v0;
																	Point2 u3 = v4 - v0;
																	Point2 u4 = v5 - v0;
																	Point2 u5 = v6 - v0;
																	const real_t _area =
																			(u0.y * u1.x - u0.x * u1.y) +
																			(u1.y * u2.x - u1.x * u2.y) +
																			(u2.y * u3.x - u2.x * u3.y) +
																			(u3.y * u4.x - u3.x * u4.y) +
																			(u4.y * u5.x - u4.x * u5.y);
																	if (_area < min_area) {
																		min_area = _area;
																		dest[0] = v0;
																		dest[1] = v1;
																		dest[2] = v2;
																		dest[3] = v3;
																		dest[4] = v4;
																		dest[5] = v5;
																		dest[6] = v6;
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		} break;
		case 8: {
			for (unsigned x = 0; x < n; x++) {
				for (unsigned y = x + 1; y < n; y++) {
					if (Intersect(v0, lines[x], lines[y])) {
						for (unsigned z = y + 1; z < n; z++) {
							if (Intersect(v1, lines[y], lines[z])) {
								for (unsigned w = z + 1; w < n; w++) {
									if (Intersect(v2, lines[z], lines[w])) {
										for (unsigned r = w + 1; r < n; r++) {
											if (Intersect(v3, lines[w], lines[r])) {
												for (unsigned s = r + 1; s < n; s++) {
													if (Intersect(v4, lines[r], lines[s])) {
														for (unsigned t = s + 1; t < n; t++) {
															if (Intersect(v5, lines[s], lines[t])) {
																for (unsigned u = t + 1; u < n; u++) {
																	if (Intersect(v6, lines[t], lines[u])) {
																		if (Intersect(v7, lines[u], lines[x])) {
																			Point2 u0 = v1 - v0;
																			Point2 u1 = v2 - v0;
																			Point2 u2 = v3 - v0;
																			Point2 u3 = v4 - v0;
																			Point2 u4 = v5 - v0;
																			Point2 u5 = v6 - v0;
																			Point2 u6 = v7 - v0;
																			const real_t _area =
																					(u0.y * u1.x - u0.x * u1.y) +
																					(u1.y * u2.x - u1.x * u2.y) +
																					(u2.y * u3.x - u2.x * u3.y) +
																					(u3.y * u4.x - u3.x * u4.y) +
																					(u4.y * u5.x - u4.x * u5.y) +
																					(u5.y * u6.x - u5.x * u6.y);
																			if (_area < min_area) {
																				min_area = _area;
																				dest[0] = v0;
																				dest[1] = v1;
																				dest[2] = v2;
																				dest[3] = v3;
																				dest[4] = v4;
																				dest[5] = v5;
																				dest[6] = v6;
																				dest[7] = v7;
																			}
																		}
																	}
																}
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		} break;
	}

	if (area != nullptr) {
		*area = 0.5 * min_area;
	}

	return vertex_count;
}

#endif // TOOLS_ENABLED
