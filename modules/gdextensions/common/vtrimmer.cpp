
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

#include <stdio.h>
#include <stdlib.h>
#ifndef __APPLE__
# include <malloc.h>
#endif

#include <string>

_FORCE_INLINE_ static real_t distance(const Point2 &pt1, const Point2 &pt2) { return pt1.distance_to(pt2); }

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

class ConvexHull {

	CHNode *m_Root;
	CHNode *m_Curr;
	unsigned m_Count;

public:
	ConvexHull();
	~ConvexHull();

	void Clear();
	bool InsertPoint(const Point2 &point);
	bool RemoveLeastRelevantEdge();
	unsigned FindOptimalPolygon(Point2 *dest, unsigned vertex_count, float *area = nullptr);

	bool GoToFirst() { return (m_Curr = m_Root) != nullptr; }
	bool GoToNext () { return (m_Curr = m_Curr->Next) != m_Root; }

	const Point2 &GetCurrPoint() const { return m_Curr->Point; }
	const Point2 &GetNextPoint() const { return m_Curr->Next->Point; }
	const Point2 &GetPrevPoint() const { return m_Curr->Prev->Point; }

	unsigned GetCount() const { return m_Count; }
	float GetArea() const;
};

struct WorkPacket {
	ConvexHull Hull;
	Point2 Polygon[8];
	real_t Area;
	real_t HullArea;
	unsigned Rotation;
};

_FORCE_INLINE_ static unsigned FindOptimalRotation(Point2 *vertices, unsigned vertex_count, unsigned *indices);

bool vertex_trimmer(const Ref<Image> &image, const std::string base_file_name, vertex_trimmer_opt_t *opt) {
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

	const real_t threshold_f = (real_t) threshold;

	if (threshold > 255) {
		WARN_PRINT(vformat("(vertex_trimmer) Error: Threshold %d out of range", threshold));
		return false;
	}

	if (vertex_count < 3 || vertex_count > 8) {
		WARN_PRINT(vformat("(vertex_trimmer) Error: Vertex count %d out of range", vertex_count));
		return false;
	}

	const bool using_atlas = (atlas_x > 1 || atlas_y > 1);

	Ref<Image> img = image->converted(Image::FORMAT_L8);
	ERR_FAIL_NULL_V(img, false);

	while (dilate_count--) {
		img = img->dilate();
	}

	const int w = img->get_width();
	const int h = img->get_height();
	const unsigned char *pixels = img->get_data().read().ptr();

	// Threshold the pixels
#if 0
	int set_pixels = 0;
	for (int i = 0; i < w * h; i++) {
		unsigned c = (pixels[i] > threshold)? 1 : 0;
		pixels[i] = c;
		set_pixels += c;
	}
	print_verbose(vformat("Set pixels: %.2f%%\n", real_t(100 * set_pixels) / real_t(w * h)));
#endif
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
				int c0 = row[x + 0];
				int c1 = row[x + 1];

				if ((c0 > threshold) != (c1 > threshold)) {
					real_t d0 = (real_t) c0;
					real_t d1 = (real_t) c1;

					real_t sub_pixel_x = (threshold_f - d0) / (d1 - d0);
					hull.InsertPoint(Point2(x - start_x - off_x + sub_pixel_x, -corner_off_y));
				}
			}

			row = pixels + end_y * w;
			for (int x = start_x; x < end_x; x++) {
				int c0 = row[x + 0];
				int c1 = row[x + 1];

				if ((c0 > threshold) != (c1 > threshold)) {
					real_t d0 = (real_t) c0;
					real_t d1 = (real_t) c1;

					real_t sub_pixel_x = (threshold_f - d0) / (d1 - d0);
					hull.InsertPoint(Point2(x - start_x - off_x + sub_pixel_x, corner_off_y));
				}
			}

			const unsigned char *col = pixels + start_x;
			for (int y = start_y; y < end_y; y++) {
				int c0 = col[(y + 0) * w];
				int c1 = col[(y + 1) * w];

				if ((c0 > threshold) != (c1 > threshold)) {
					real_t d0 = (real_t) c0;
					real_t d1 = (real_t) c1;

					real_t sub_pixel_y = (threshold_f - d0) / (d1 - d0);
					hull.InsertPoint(Point2(-corner_off_x, y - start_y - off_y + sub_pixel_y));
				}
			}

			col = pixels + end_x;
			for (int y = start_y; y < end_y; y++) {
				int c0 = col[(y + 0) * w];
				int c1 = col[(y + 1) * w];

				if ((c0 > threshold) != (c1 > threshold)) {
					real_t d0 = (real_t) c0;
					real_t d1 = (real_t) c1;

					real_t sub_pixel_y = (threshold_f - d0) / (d1 - d0);
					hull.InsertPoint(Point2(corner_off_x, y - start_y - off_y + sub_pixel_y));
				}
			}

			// The interior pixels
			for (int y = start_y; y < end_y; y++) {
				const unsigned char *row0 = pixels + (y + 0) * w;
				const unsigned char *row1 = pixels + (y + 1) * w;

				for (int x = start_x; x < end_x; x++) {
					int c00 = row0[x + 0];
					int c01 = row0[x + 1];
					int c10 = row1[x + 0];
					int c11 = row1[x + 1];

					int count = 0;
					if (c00 > threshold) { ++count; }
					if (c01 > threshold) { ++count; }
					if (c10 > threshold) { ++count; }
					if (c11 > threshold) { ++count; }

					if (count > 0 && count < 4) {
						real_t d00 = (real_t) c00;
						real_t d01 = (real_t) c01;
						real_t d10 = (real_t) c10;
						real_t d11 = (real_t) c11;

						for (int n = 0; n <= sub_pixel; n++) {
							// Lerping factors
							real_t f0 = real_t(n) / real_t(sub_pixel);
							real_t f1 = 1.0f - f0;

							real_t x0 = d00 * f1 + d10 * f0;
							real_t x1 = d01 * f1 + d11 * f0;

							if ((x0 > threshold_f) != (x1 > threshold_f)) {
								real_t sub_pixel_x = (threshold_f - x0) / (x1 - x0);
								hull.InsertPoint(Point2(x - start_x - off_x + sub_pixel_x, y - start_y - off_y + f0));
							}

							real_t y0 = d00 * f1 + d01 * f0;
							real_t y1 = d10 * f1 + d11 * f0;

							if ((y0 > threshold_f) != (y1 > threshold_f)) {
								real_t sub_pixel_y = (threshold_f - y0) / (y1 - y0);
								hull.InsertPoint(Point2(x - start_x - off_x + f0, y - start_y - off_y + sub_pixel_y));
							}
						}
					}

				}

			}

			print_verbose(vformat("Convex hull has %d vertices", hull.GetCount()));
			if (hull.GetCount() > max_hull_size) {
				real_t area_before = hull.GetArea();

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

		const unsigned count = packet.Hull.FindOptimalPolygon(packet.Polygon, vertex_count, &packet.Area);
		packet.HullArea = packet.Hull.GetArea();

		// Scale-bias the results
		for (unsigned i = 0; i < count; i++) {
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

	delete [] packets;

	return true;
}

static unsigned FindOptimalRotation(Point2 *vertices, unsigned vertex_count, unsigned *indices) {
	const unsigned index_count = (vertex_count - 2) * 3;

	unsigned optimal = 0;
	real_t min_length = FLT_MAX;

	for (unsigned i = 0; i < vertex_count; i++) {
		real_t sum = 0;
		for (unsigned k = 0; k < index_count; k += 3) {
			unsigned i0 = (indices[k + 0] + i) % vertex_count;
			unsigned i1 = (indices[k + 1] + i) % vertex_count;
			unsigned i2 = (indices[k + 2] + i) % vertex_count;

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
