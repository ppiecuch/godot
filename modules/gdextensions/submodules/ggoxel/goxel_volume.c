/**************************************************************************/
/*  goxel_volume.c                                                        */
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

/* Goxel volume backend — minimal tile-based sparse volume implementation.
 *
 * Implements the opaque volume_t type declared in volume.h, plus volume_utils
 * functions needed by the Godot wrapper (blit, merge, op, mesh generation).
 *
 * Each tile is TILE_SIZE^3 voxels × 4 bytes RGBA = 16384 bytes.
 * Tiles are stored as a singly-linked list keyed by integer position.
 *
 * License: GPL-3.0 (matches upstream goxel)
 */

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "goxel/palette.h"
#include "goxel/render.h"
#include "goxel/shape.h"
#include "goxel/volume.h"
#include "goxel/volume_utils.h"

/* ---- system.h stubs --------------------------------------------------- */
#include "goxel/system.h"

sys_callbacks_t sys_callbacks = { 0 };

void sys_log(const char *msg) {
	/* Could redirect to Godot print, but keeping it simple for a C file. */
	fprintf(stderr, "[goxel] %s\n", msg);
}

double sys_get_time(void) {
	/* Minimal stub — not performance-critical for our usage. */
	return 0.0;
}

int sys_list_dir(const char *dir,
		int (*callback)(const char *dir, const char *name, void *user),
		void *user) {
	(void)dir;
	(void)callback;
	(void)user;
	return 0;
}
int sys_delete_file(const char *path) {
	(void)path;
	return -1;
}
const char *sys_get_user_dir(void) { return "/tmp"; }
int sys_make_dir(const char *path) {
	(void)path;
	return 0;
}
void sys_set_window_title(const char *title) { (void)title; }
void sys_show_keyboard(bool has_text) { (void)has_text; }
void sys_save_to_photos(const uint8_t *data, int size,
		void (*on_finished)(int r)) {
	(void)data;
	(void)size;
	(void)on_finished;
}
const char *sys_get_save_path(const char *filters, const char *default_name) {
	(void)filters;
	(void)default_name;
	return NULL;
}
void sys_on_saved(const char *path) { (void)path; }
const char *sys_get_clipboard_text(void *user) {
	(void)user;
	return "";
}
void sys_set_clipboard_text(void *user, const char *text) {
	(void)user;
	(void)text;
}
int sys_get_screen_framebuffer(void) { return 0; }

/* ---- shape functions -------------------------------------------------- */

static float shape_sphere_func(const float p[3], const float s[3],
		float smoothness) {
	(void)smoothness;
	float d = 0;
	for (int i = 0; i < 3; i++) {
		float v = p[i] / s[i];
		d += v * v;
	}
	return sqrtf(d) - 1.0f;
}

static float shape_cube_func(const float p[3], const float s[3],
		float smoothness) {
	float dx = fabsf(p[0]) / s[0];
	float dy = fabsf(p[1]) / s[1];
	float dz = fabsf(p[2]) / s[2];
	if (smoothness > 0) {
		/* Smooth blend between cube and sphere using power norm. */
		float k = 2.0f + smoothness * 8.0f;
		float d = powf(powf(dx, k) + powf(dy, k) + powf(dz, k), 1.0f / k);
		return d - 1.0f;
	}
	float d = dx;
	if (dy > d)
		d = dy;
	if (dz > d)
		d = dz;
	return d - 1.0f;
}

static float shape_cylinder_func(const float p[3], const float s[3],
		float smoothness) {
	float dx = p[0] / s[0];
	float dz = p[2] / s[2];
	float r = sqrtf(dx * dx + dz * dz);
	float h = fabsf(p[1]) / s[1];
	if (smoothness > 0) {
		float k = 2.0f + smoothness * 8.0f;
		float d = powf(powf(r, k) + powf(h, k), 1.0f / k);
		return d - 1.0f;
	}
	float d = r > h ? r : h;
	return d - 1.0f;
}

static float shape_cone_func(const float p[3], const float s[3],
		float smoothness) {
	(void)smoothness;
	/* Cone along Y axis: radius decreases linearly from bottom (y=-s) to
	 * top (y=+s). At y=0 the radius is s[0]. */
	float t = (p[1] / s[1] + 1.0f) * 0.5f; /* 0 at bottom, 1 at top */
	if (t < 0.0f || t > 1.0f)
		return 1.0f; /* outside height range */
	float cone_r = 1.0f - t; /* linearly shrinks to 0 at top */
	float dx = p[0] / s[0];
	float dz = p[2] / s[2];
	float r = sqrtf(dx * dx + dz * dz);
	return r / (cone_r + 1e-6f) - 1.0f;
}

static float shape_torus_func(const float p[3], const float s[3],
		float smoothness) {
	(void)smoothness;
	/* Torus in XZ plane. Major radius = s[0], minor radius = s[1].
	 * s[2] scales along Z like s[0]. */
	float dx = p[0] / s[0];
	float dz = p[2] / s[2];
	float ring_dist = sqrtf(dx * dx + dz * dz) - 1.0f;
	float dy = p[1] / s[1];
	return sqrtf(ring_dist * ring_dist + dy * dy) - 0.5f;
}

shape_t shape_sphere = { "sphere", shape_sphere_func };
shape_t shape_cube = { "cube", shape_cube_func };
shape_t shape_cylinder = { "cylinder", shape_cylinder_func };
shape_t shape_cone = { "cone", shape_cone_func };
shape_t shape_torus = { "torus", shape_torus_func };

void shapes_init(void) {
	/* Shapes are statically initialised above. */
}

/* ---- Tile and volume types -------------------------------------------- */

#define TS TILE_SIZE
#define TILE_VOXELS (TS * TS * TS)
#define TILE_BYTES (TILE_VOXELS * 4)

struct tile {
	uint8_t data[TILE_BYTES];
};

typedef struct tile_node {
	int pos[3]; /* tile origin in voxel coords (always multiples of TS) */
	tile_t *tile;
	struct tile_node *next;
} tile_node_t;

struct volume {
	tile_node_t *tiles;
	int tiles_count;
	uint64_t key; /* monotonically increasing change counter */
};

static uint64_t g_next_key = 1;
static int g_nb_volumes = 0;
static int g_nb_tiles = 0;

/* Align pos to tile boundary. */
static void tile_pos_for(const int pos[3], int out[3]) {
	for (int i = 0; i < 3; i++) {
		out[i] = (pos[i] >= 0) ? (pos[i] / TS) * TS
							   : ((pos[i] - TS + 1) / TS) * TS;
	}
}

/* Find tile node at given tile-aligned position (or NULL). */
static tile_node_t *find_tile(const volume_t *vol, const int tpos[3]) {
	for (tile_node_t *n = vol->tiles; n; n = n->next) {
		if (n->pos[0] == tpos[0] && n->pos[1] == tpos[1] &&
				n->pos[2] == tpos[2])
			return n;
	}
	return NULL;
}

/* Get or create tile at tile-aligned position. */
static tile_node_t *get_or_create_tile(volume_t *vol, const int tpos[3]) {
	tile_node_t *n = find_tile(vol, tpos);
	if (n)
		return n;
	n = (tile_node_t *)calloc(1, sizeof(tile_node_t));
	n->tile = (tile_t *)calloc(1, sizeof(tile_t));
	n->pos[0] = tpos[0];
	n->pos[1] = tpos[1];
	n->pos[2] = tpos[2];
	n->next = vol->tiles;
	vol->tiles = n;
	vol->tiles_count++;
	g_nb_tiles++;
	return n;
}

static bool tile_is_empty(const tile_t *t) {
	/* Check only alpha channel. */
	const uint8_t *d = t->data;
	for (int i = 0; i < TILE_VOXELS; i++) {
		if (d[i * 4 + 3] != 0)
			return false;
	}
	return true;
}

static int voxel_offset(int lx, int ly, int lz) {
	return ((lz * TS + ly) * TS + lx) * 4;
}

/* ---- volume lifecycle ------------------------------------------------- */

volume_t *volume_new(void) {
	volume_t *v = (volume_t *)calloc(1, sizeof(volume_t));
	v->key = g_next_key++;
	g_nb_volumes++;
	return v;
}

void volume_delete(volume_t *volume) {
	if (!volume)
		return;
	volume_clear(volume);
	free(volume);
	g_nb_volumes--;
}

void volume_clear(volume_t *volume) {
	tile_node_t *n = volume->tiles;
	while (n) {
		tile_node_t *next = n->next;
		free(n->tile);
		free(n);
		g_nb_tiles--;
		n = next;
	}
	volume->tiles = NULL;
	volume->tiles_count = 0;
	volume->key = g_next_key++;
}

volume_t *volume_dup(const volume_t *volume) {
	if (!volume)
		return NULL;
	volume_t *dup = volume_new();
	for (tile_node_t *n = volume->tiles; n; n = n->next) {
		tile_node_t *nn = get_or_create_tile(dup, n->pos);
		memcpy(nn->tile->data, n->tile->data, TILE_BYTES);
	}
	return dup;
}

volume_t *volume_copy(const volume_t *volume) {
	return volume_dup(volume);
}

void volume_set(volume_t *volume, const volume_t *other) {
	volume_clear(volume);
	if (!other)
		return;
	for (tile_node_t *n = other->tiles; n; n = n->next) {
		tile_node_t *nn = get_or_create_tile(volume, n->pos);
		memcpy(nn->tile->data, n->tile->data, TILE_BYTES);
	}
	volume->key = g_next_key++;
}

/* ---- per-voxel access ------------------------------------------------- */

volume_accessor_t volume_get_accessor(const volume_t *volume) {
	volume_accessor_t acc;
	memset(&acc, 0, sizeof(acc));
	acc.volume = volume;
	return acc;
}

void volume_get_at(const volume_t *volume, volume_iterator_t *it,
		const int pos[3], uint8_t out[4]) {
	(void)it;
	int tpos[3];
	tile_pos_for(pos, tpos);
	tile_node_t *n = find_tile(volume, tpos);
	if (!n) {
		memset(out, 0, 4);
		return;
	}
	int lx = pos[0] - tpos[0];
	int ly = pos[1] - tpos[1];
	int lz = pos[2] - tpos[2];
	memcpy(out, &n->tile->data[voxel_offset(lx, ly, lz)], 4);
}

uint8_t volume_get_alpha_at(const volume_t *volume, volume_iterator_t *it,
		const int pos[3]) {
	uint8_t v[4];
	volume_get_at(volume, it, pos, v);
	return v[3];
}

void volume_set_at(volume_t *volume, volume_iterator_t *it,
		const int pos[3], const uint8_t v[4]) {
	(void)it;
	int tpos[3];
	tile_pos_for(pos, tpos);
	tile_node_t *n = get_or_create_tile(volume, tpos);
	int lx = pos[0] - tpos[0];
	int ly = pos[1] - tpos[1];
	int lz = pos[2] - tpos[2];
	memcpy(&n->tile->data[voxel_offset(lx, ly, lz)], v, 4);
	volume->key = g_next_key++;
}

void volume_clear_tile(volume_t *volume, volume_iterator_t *it,
		const int pos[3]) {
	(void)it;
	int tpos[3];
	tile_pos_for(pos, tpos);
	tile_node_t *n = find_tile(volume, tpos);
	if (n) {
		memset(n->tile->data, 0, TILE_BYTES);
		volume->key = g_next_key++;
	}
}

/* ---- queries ---------------------------------------------------------- */

bool volume_is_empty(const volume_t *volume) {
	if (!volume || !volume->tiles)
		return true;
	for (tile_node_t *n = volume->tiles; n; n = n->next) {
		if (!tile_is_empty(n->tile))
			return false;
	}
	return true;
}

bool volume_get_bbox(const volume_t *volume, int bbox[2][3], bool exact) {
	memset(bbox, 0, sizeof(int) * 6);
	if (!volume || !volume->tiles)
		return false;

	bool found = false;
	int bmin[3] = { INT32_MAX, INT32_MAX, INT32_MAX };
	int bmax[3] = { INT32_MIN, INT32_MIN, INT32_MIN };

	for (tile_node_t *n = volume->tiles; n; n = n->next) {
		if (exact) {
			/* Scan individual voxels for exact bbox. */
			for (int z = 0; z < TS; z++)
				for (int y = 0; y < TS; y++)
					for (int x = 0; x < TS; x++) {
						int off = voxel_offset(x, y, z);
						if (n->tile->data[off + 3] == 0)
							continue;
						int wx = n->pos[0] + x;
						int wy = n->pos[1] + y;
						int wz = n->pos[2] + z;
						if (wx < bmin[0])
							bmin[0] = wx;
						if (wy < bmin[1])
							bmin[1] = wy;
						if (wz < bmin[2])
							bmin[2] = wz;
						if (wx + 1 > bmax[0])
							bmax[0] = wx + 1;
						if (wy + 1 > bmax[1])
							bmax[1] = wy + 1;
						if (wz + 1 > bmax[2])
							bmax[2] = wz + 1;
						found = true;
					}
		} else {
			if (tile_is_empty(n->tile))
				continue;
			for (int i = 0; i < 3; i++) {
				if (n->pos[i] < bmin[i])
					bmin[i] = n->pos[i];
				if (n->pos[i] + TS > bmax[i])
					bmax[i] = n->pos[i] + TS;
			}
			found = true;
		}
	}

	if (found) {
		for (int i = 0; i < 3; i++) {
			bbox[0][i] = bmin[i];
			bbox[1][i] = bmax[i];
		}
	}
	return found;
}

uint64_t volume_get_key(const volume_t *volume) {
	if (!volume)
		return 0;
	return volume->key;
}

int volume_get_tiles_count(const volume_t *volume) {
	if (!volume)
		return 0;
	return volume->tiles_count;
}

void *volume_get_tile_data(const volume_t *volume,
		volume_accessor_t *accessor,
		const int bpos[3], uint64_t *id) {
	(void)accessor;
	int tpos[3];
	tile_pos_for(bpos, tpos);
	tile_node_t *n = find_tile(volume, tpos);
	if (!n) {
		if (id)
			*id = 0;
		return NULL;
	}
	if (id)
		*id = volume->key;
	return n->tile->data;
}

void volume_copy_tile(const volume_t *src, const int src_pos[3],
		volume_t *dst, const int dst_pos[3]) {
	int stpos[3], dtpos[3];
	tile_pos_for(src_pos, stpos);
	tile_pos_for(dst_pos, dtpos);
	tile_node_t *sn = find_tile(src, stpos);
	if (!sn)
		return;
	tile_node_t *dn = get_or_create_tile(dst, dtpos);
	memcpy(dn->tile->data, sn->tile->data, TILE_BYTES);
	dst->key = g_next_key++;
}

void volume_remove_empty_tiles(volume_t *volume, bool fast) {
	(void)fast;
	tile_node_t **pp = &volume->tiles;
	while (*pp) {
		if (tile_is_empty((*pp)->tile)) {
			tile_node_t *del = *pp;
			*pp = del->next;
			free(del->tile);
			free(del);
			volume->tiles_count--;
			g_nb_tiles--;
		} else {
			pp = &(*pp)->next;
		}
	}
}

void volume_read(const volume_t *volume,
		const int pos[3], const int size[3],
		uint8_t *data) {
	memset(data, 0, (size_t)size[0] * size[1] * size[2] * 4);

	if (!volume)
		return;

	for (tile_node_t *n = volume->tiles; n; n = n->next) {
		/* Compute overlap between tile and read region. */
		int omin[3], omax[3];
		bool overlap = true;
		for (int i = 0; i < 3; i++) {
			omin[i] = n->pos[i] > pos[i] ? n->pos[i] : pos[i];
			int tend = n->pos[i] + TS;
			int rend = pos[i] + size[i];
			omax[i] = tend < rend ? tend : rend;
			if (omin[i] >= omax[i]) {
				overlap = false;
				break;
			}
		}
		if (!overlap)
			continue;

		for (int z = omin[2]; z < omax[2]; z++)
			for (int y = omin[1]; y < omax[1]; y++)
				for (int x = omin[0]; x < omax[0]; x++) {
					int lx = x - n->pos[0];
					int ly = y - n->pos[1];
					int lz = z - n->pos[2];
					int rx = x - pos[0];
					int ry = y - pos[1];
					int rz = z - pos[2];
					int src_off = voxel_offset(lx, ly, lz);
					int dst_off = ((rz * size[1] + ry) * size[0] + rx) * 4;
					memcpy(&data[dst_off],
							&n->tile->data[src_off], 4);
				}
	}
}

void volume_get_global_stats(volume_global_stats_t *stats) {
	stats->nb_volumes = g_nb_volumes;
	stats->nb_tiles = g_nb_tiles;
	stats->mem = (uint64_t)g_nb_tiles * sizeof(tile_t);
}

/* ---- iterators -------------------------------------------------------- */

volume_iterator_t volume_get_iterator(const volume_t *volume, int flags) {
	volume_iterator_t it;
	memset(&it, 0, sizeof(it));
	it.volume = volume;
	it.flags = flags;
	/* Point to the first tile. */
	if (volume && volume->tiles) {
		it.tile = volume->tiles->tile;
		it.tile_pos[0] = volume->tiles->pos[0];
		it.tile_pos[1] = volume->tiles->pos[1];
		it.tile_pos[2] = volume->tiles->pos[2];
	}
	/* Set initial position before start so first volume_iter advances. */
	it.pos[0] = -1;
	it.pos[1] = 0;
	it.pos[2] = 0;
	return it;
}

volume_iterator_t volume_get_box_iterator(const volume_t *volume,
		const float box[4][4], int flags) {
	volume_iterator_t it;
	memset(&it, 0, sizeof(it));
	it.volume = volume;
	it.flags = flags;
	/* Compute AABB from box matrix — box is a 4×4 transform from unit cube. */
	for (int i = 0; i < 3; i++) {
		float center = box[3][i];
		float extent = fabsf(box[0][i]) + fabsf(box[1][i]) + fabsf(box[2][i]);
		it.bbox[0][i] = (int)floorf(center - extent);
		it.bbox[1][i] = (int)ceilf(center + extent);
	}
	memcpy(it.box, box, sizeof(float) * 16);
	it.pos[0] = it.bbox[0][0] - 1;
	it.pos[1] = it.bbox[0][1];
	it.pos[2] = it.bbox[0][2];
	return it;
}

volume_iterator_t volume_get_union_iterator(
		const volume_t *m1, const volume_t *m2, int flags) {
	volume_iterator_t it;
	memset(&it, 0, sizeof(it));
	it.volume = m1;
	it.volume2 = m2;
	it.flags = flags;
	/* Use bbox union of both volumes. */
	int b1[2][3], b2[2][3];
	bool h1 = volume_get_bbox(m1, b1, false);
	bool h2 = volume_get_bbox(m2, b2, false);
	if (h1 && h2) {
		for (int i = 0; i < 3; i++) {
			it.bbox[0][i] = b1[0][i] < b2[0][i] ? b1[0][i] : b2[0][i];
			it.bbox[1][i] = b1[1][i] > b2[1][i] ? b1[1][i] : b2[1][i];
		}
	} else if (h1) {
		memcpy(it.bbox, b1, sizeof(b1));
	} else if (h2) {
		memcpy(it.bbox, b2, sizeof(b2));
	}
	it.pos[0] = it.bbox[0][0] - 1;
	it.pos[1] = it.bbox[0][1];
	it.pos[2] = it.bbox[0][2];
	return it;
}

/* Iterate tiles: walk the linked list. */
static int iter_tiles(volume_iterator_t *it, int pos[3]) {
	const volume_t *vol = it->volume;
	if (!vol)
		return 0;

	/* Find next tile after current tile_pos. */
	tile_node_t *found = NULL;

	if (it->tile == NULL && vol->tiles) {
		/* First call — return first tile. */
		found = vol->tiles;
	} else {
		/* Find the node matching current tile_pos, then advance. */
		for (tile_node_t *n = vol->tiles; n; n = n->next) {
			if (n->pos[0] == it->tile_pos[0] &&
					n->pos[1] == it->tile_pos[1] &&
					n->pos[2] == it->tile_pos[2]) {
				found = n->next;
				break;
			}
		}
	}

	while (found) {
		if ((it->flags & VOLUME_ITER_SKIP_EMPTY) &&
				tile_is_empty(found->tile)) {
			found = found->next;
			continue;
		}
		it->tile = found->tile;
		it->tile_pos[0] = found->pos[0];
		it->tile_pos[1] = found->pos[1];
		it->tile_pos[2] = found->pos[2];
		pos[0] = found->pos[0];
		pos[1] = found->pos[1];
		pos[2] = found->pos[2];
		return 1;
	}
	return 0;
}

int volume_iter(volume_iterator_t *it, int pos[3]) {
	if (it->flags & VOLUME_ITER_TILES) {
		return iter_tiles(it, pos);
	}

	/* Voxel iteration over bbox or full volume. */
	bool has_bbox = (it->bbox[0][0] != 0 || it->bbox[0][1] != 0 ||
			it->bbox[0][2] != 0 || it->bbox[1][0] != 0 ||
			it->bbox[1][1] != 0 || it->bbox[1][2] != 0);

	if (!has_bbox) {
		/* Compute bbox from volume. */
		int bb[2][3];
		if (!volume_get_bbox(it->volume, bb, false))
			return 0;
		memcpy(it->bbox, bb, sizeof(bb));
		has_bbox = true;
	}

	/* Advance position. */
	it->pos[0]++;
	if (it->pos[0] >= it->bbox[1][0]) {
		it->pos[0] = it->bbox[0][0];
		it->pos[1]++;
		if (it->pos[1] >= it->bbox[1][1]) {
			it->pos[1] = it->bbox[0][1];
			it->pos[2]++;
			if (it->pos[2] >= it->bbox[1][2]) {
				return 0;
			}
		}
	}

	if (it->flags & VOLUME_ITER_SKIP_EMPTY) {
		uint8_t v[4];
		volume_get_at(it->volume, NULL, it->pos, v);
		while (v[3] == 0) {
			it->pos[0]++;
			if (it->pos[0] >= it->bbox[1][0]) {
				it->pos[0] = it->bbox[0][0];
				it->pos[1]++;
				if (it->pos[1] >= it->bbox[1][1]) {
					it->pos[1] = it->bbox[0][1];
					it->pos[2]++;
					if (it->pos[2] >= it->bbox[1][2])
						return 0;
				}
			}
			volume_get_at(it->volume, NULL, it->pos, v);
		}
	}

	pos[0] = it->pos[0];
	pos[1] = it->pos[1];
	pos[2] = it->pos[2];
	return 1;
}

/* ---- volume_utils implementations ------------------------------------- */

void volume_get_box(const volume_t *volume, bool exact, float box[4][4]) {
	int bbox[2][3];
	memset(box, 0, sizeof(float) * 16);
	if (!volume_get_bbox(volume, bbox, exact))
		return;
	for (int i = 0; i < 3; i++) {
		box[3][i] = (bbox[0][i] + bbox[1][i]) / 2.0f;
		box[i][i] = (bbox[1][i] - bbox[0][i]) / 2.0f;
	}
	box[3][3] = 1.0f;
}

void volume_blit(volume_t *volume, const uint8_t *data,
		int x, int y, int z, int w, int h, int d,
		volume_iterator_t *iter) {
	(void)iter;
	for (int dz = 0; dz < d; dz++)
		for (int dy = 0; dy < h; dy++)
			for (int dx = 0; dx < w; dx++) {
				int pos[3] = { x + dx, y + dy, z + dz };
				const uint8_t *v =
						&data[((dz * h + dy) * w + dx) * 4];
				volume_set_at(volume, NULL, pos, v);
			}
}

static void blend_voxel(uint8_t dst[4], const uint8_t src[4], int mode) {
	switch (mode) {
		case MODE_OVER:
			if (src[3] > 0)
				memcpy(dst, src, 4);
			break;
		case MODE_SUB: {
			int a = (int)dst[3] - src[3];
			dst[3] = (uint8_t)(a > 0 ? a : 0);
		} break;
		case MODE_SUB_CLAMP: {
			int a = 255 - src[3];
			if (a < dst[3])
				dst[3] = (uint8_t)a;
		} break;
		case MODE_PAINT:
			if (dst[3] > 0) {
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
			}
			break;
		case MODE_MAX:
			if (src[3] > dst[3]) {
				memcpy(dst, src, 4);
			}
			break;
		case MODE_INTERSECT: {
			uint8_t a = src[3] < dst[3] ? src[3] : dst[3];
			dst[3] = a;
		} break;
		case MODE_INTERSECT_FILL:
			if (src[3] > 0 && dst[3] > 0) {
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				if (src[3] < dst[3])
					dst[3] = src[3];
			} else {
				dst[3] = 0;
			}
			break;
		case MODE_MULT_ALPHA: {
			int a = ((int)dst[3] * src[3]) / 255;
			dst[3] = (uint8_t)a;
		} break;
		case MODE_REPLACE:
			memcpy(dst, src, 4);
			break;
		default:
			if (src[3] > 0)
				memcpy(dst, src, 4);
			break;
	}
}

void volume_merge(volume_t *volume, const volume_t *other, int mode,
		const uint8_t color[4]) {
	if (!other)
		return;
	for (tile_node_t *n = other->tiles; n; n = n->next) {
		for (int z = 0; z < TS; z++)
			for (int y = 0; y < TS; y++)
				for (int x = 0; x < TS; x++) {
					int off = voxel_offset(x, y, z);
					uint8_t src[4];
					memcpy(src, &n->tile->data[off], 4);
					if (color) {
						src[0] = color[0];
						src[1] = color[1];
						src[2] = color[2];
						/* Keep source alpha. */
					}
					if (src[3] == 0 && mode != MODE_REPLACE)
						continue;
					int pos[3] = { n->pos[0] + x, n->pos[1] + y,
						n->pos[2] + z };
					uint8_t dst[4];
					volume_get_at(volume, NULL, pos, dst);
					blend_voxel(dst, src, mode);
					volume_set_at(volume, NULL, pos, dst);
				}
	}
}

void volume_op(volume_t *volume, const painter_t *painter,
		const float box[4][4]) {
	if (!painter || !painter->shape)
		return;

	/* Compute AABB of the box transform. */
	int amin[3], amax[3];
	for (int i = 0; i < 3; i++) {
		float center = box[3][i];
		float extent = fabsf(box[0][i]) + fabsf(box[1][i]) + fabsf(box[2][i]);
		amin[i] = (int)floorf(center - extent) - 1;
		amax[i] = (int)ceilf(center + extent) + 1;
	}

	/* Invert the box matrix to get world→local transform. */
	float inv[4][4];
	/* Use Cramer's rule for 3×3 rotation part + translation. */
	float det = box[0][0] * (box[1][1] * box[2][2] - box[1][2] * box[2][1]) - box[0][1] * (box[1][0] * box[2][2] - box[1][2] * box[2][0]) + box[0][2] * (box[1][0] * box[2][1] - box[1][1] * box[2][0]);

	if (fabsf(det) < 1e-10f)
		return;
	float inv_det = 1.0f / det;

	inv[0][0] = (box[1][1] * box[2][2] - box[1][2] * box[2][1]) * inv_det;
	inv[0][1] = (box[0][2] * box[2][1] - box[0][1] * box[2][2]) * inv_det;
	inv[0][2] = (box[0][1] * box[1][2] - box[0][2] * box[1][1]) * inv_det;
	inv[1][0] = (box[1][2] * box[2][0] - box[1][0] * box[2][2]) * inv_det;
	inv[1][1] = (box[0][0] * box[2][2] - box[0][2] * box[2][0]) * inv_det;
	inv[1][2] = (box[0][2] * box[1][0] - box[0][0] * box[1][2]) * inv_det;
	inv[2][0] = (box[1][0] * box[2][1] - box[1][1] * box[2][0]) * inv_det;
	inv[2][1] = (box[0][1] * box[2][0] - box[0][0] * box[2][1]) * inv_det;
	inv[2][2] = (box[0][0] * box[1][1] - box[0][1] * box[1][0]) * inv_det;
	/* Translation. */
	inv[3][0] = -(inv[0][0] * box[3][0] + inv[1][0] * box[3][1] +
			inv[2][0] * box[3][2]);
	inv[3][1] = -(inv[0][1] * box[3][0] + inv[1][1] * box[3][1] +
			inv[2][1] * box[3][2]);
	inv[3][2] = -(inv[0][2] * box[3][0] + inv[1][2] * box[3][1] +
			inv[2][2] * box[3][2]);
	inv[0][3] = inv[1][3] = inv[2][3] = 0.0f;
	inv[3][3] = 1.0f;

	/* Size for shape function — extract column lengths from box. */
	float s[3];
	for (int i = 0; i < 3; i++) {
		s[i] = sqrtf(box[i][0] * box[i][0] + box[i][1] * box[i][1] +
				box[i][2] * box[i][2]);
		if (s[i] < 1e-10f)
			s[i] = 1.0f;
	}

	for (int z = amin[2]; z <= amax[2]; z++)
		for (int y = amin[1]; y <= amax[1]; y++)
			for (int x = amin[0]; x <= amax[0]; x++) {
				/* Transform to local space. */
				float wx = (float)x + 0.5f;
				float wy = (float)y + 0.5f;
				float wz = (float)z + 0.5f;
				float lx = wx - box[3][0];
				float ly = wy - box[3][1];
				float lz = wz - box[3][2];
				float p[3];
				p[0] = inv[0][0] * lx + inv[1][0] * ly + inv[2][0] * lz;
				p[1] = inv[0][1] * lx + inv[1][1] * ly + inv[2][1] * lz;
				p[2] = inv[0][2] * lx + inv[1][2] * ly + inv[2][2] * lz;

				float d = painter->shape->func(p, s, painter->smoothness);
				if (d > 0)
					continue; /* Outside shape. */

				/* Compute alpha based on SDF distance. */
				uint8_t alpha = 255;
				if (painter->smoothness > 0 && d > -painter->smoothness) {
					float t = (-d) / painter->smoothness;
					/* Smooth hermite interpolation. */
					t = t * t * (3.0f - 2.0f * t);
					alpha = (uint8_t)(255.0f * t);
				}

				uint8_t src[4] = { painter->color[0], painter->color[1],
					painter->color[2], alpha };
				int pos[3] = { x, y, z };
				uint8_t dst[4];
				volume_get_at(volume, NULL, pos, dst);
				blend_voxel(dst, src, painter->mode);
				volume_set_at(volume, NULL, pos, dst);
			}
}

void volume_extrude(volume_t *volume, const float plane[4][4],
		const float box[4][4]) {
	(void)volume;
	(void)plane;
	(void)box;
	/* Stub — not needed for Godot integration. */
}

void volume_move(volume_t *volume, const float mat[4][4]) {
	if (!volume)
		return;

	/* Collect all non-empty voxels, then clear and re-write at
	 * transformed positions. Using inverse-transform sampling:
	 * for each destination voxel, sample from source. */
	int bbox[2][3];
	if (!volume_get_bbox(volume, bbox, true))
		return;

	/* Copy source volume. */
	volume_t *src = volume_dup(volume);
	volume_clear(volume);

	/* Invert the matrix. */
	float inv[4][4];
	float det = mat[0][0] * (mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1]) - mat[0][1] * (mat[1][0] * mat[2][2] - mat[1][2] * mat[2][0]) + mat[0][2] * (mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0]);
	if (fabsf(det) < 1e-10f) {
		volume_delete(src);
		return;
	}
	float inv_det = 1.0f / det;
	inv[0][0] = (mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1]) * inv_det;
	inv[0][1] = (mat[0][2] * mat[2][1] - mat[0][1] * mat[2][2]) * inv_det;
	inv[0][2] = (mat[0][1] * mat[1][2] - mat[0][2] * mat[1][1]) * inv_det;
	inv[1][0] = (mat[1][2] * mat[2][0] - mat[1][0] * mat[2][2]) * inv_det;
	inv[1][1] = (mat[0][0] * mat[2][2] - mat[0][2] * mat[2][0]) * inv_det;
	inv[1][2] = (mat[0][2] * mat[1][0] - mat[0][0] * mat[1][2]) * inv_det;
	inv[2][0] = (mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0]) * inv_det;
	inv[2][1] = (mat[0][1] * mat[2][0] - mat[0][0] * mat[2][1]) * inv_det;
	inv[2][2] = (mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0]) * inv_det;
	inv[3][0] = -(inv[0][0] * mat[3][0] + inv[1][0] * mat[3][1] +
			inv[2][0] * mat[3][2]);
	inv[3][1] = -(inv[0][1] * mat[3][0] + inv[1][1] * mat[3][1] +
			inv[2][1] * mat[3][2]);
	inv[3][2] = -(inv[0][2] * mat[3][0] + inv[1][2] * mat[3][1] +
			inv[2][2] * mat[3][2]);
	inv[0][3] = inv[1][3] = inv[2][3] = 0.0f;
	inv[3][3] = 1.0f;

	/* Compute destination bbox by transforming source bbox corners. */
	int dmin[3] = { INT32_MAX, INT32_MAX, INT32_MAX };
	int dmax[3] = { INT32_MIN, INT32_MIN, INT32_MIN };
	for (int c = 0; c < 8; c++) {
		float sx = (c & 1) ? (float)bbox[1][0] : (float)bbox[0][0];
		float sy = (c & 2) ? (float)bbox[1][1] : (float)bbox[0][1];
		float sz = (c & 4) ? (float)bbox[1][2] : (float)bbox[0][2];
		float dx = mat[0][0] * sx + mat[1][0] * sy + mat[2][0] * sz + mat[3][0];
		float dy = mat[0][1] * sx + mat[1][1] * sy + mat[2][1] * sz + mat[3][1];
		float dz = mat[0][2] * sx + mat[1][2] * sy + mat[2][2] * sz + mat[3][2];
		int ix = (int)floorf(dx), iy = (int)floorf(dy), iz = (int)floorf(dz);
		if (ix < dmin[0])
			dmin[0] = ix;
		if (iy < dmin[1])
			dmin[1] = iy;
		if (iz < dmin[2])
			dmin[2] = iz;
		ix = (int)ceilf(dx);
		iy = (int)ceilf(dy);
		iz = (int)ceilf(dz);
		if (ix > dmax[0])
			dmax[0] = ix;
		if (iy > dmax[1])
			dmax[1] = iy;
		if (iz > dmax[2])
			dmax[2] = iz;
	}

	/* For each destination voxel, find source via inverse transform. */
	for (int z = dmin[2]; z <= dmax[2]; z++)
		for (int y = dmin[1]; y <= dmax[1]; y++)
			for (int x = dmin[0]; x <= dmax[0]; x++) {
				float fx = (float)x + 0.5f;
				float fy = (float)y + 0.5f;
				float fz = (float)z + 0.5f;
				float sx = inv[0][0] * fx + inv[1][0] * fy +
						inv[2][0] * fz + inv[3][0];
				float sy = inv[0][1] * fx + inv[1][1] * fy +
						inv[2][1] * fz + inv[3][1];
				float sz = inv[0][2] * fx + inv[1][2] * fy +
						inv[2][2] * fz + inv[3][2];
				int isx = (int)floorf(sx);
				int isy = (int)floorf(sy);
				int isz = (int)floorf(sz);
				int spos[3] = { isx, isy, isz };
				uint8_t v[4];
				volume_get_at(src, NULL, spos, v);
				if (v[3] > 0) {
					int dpos[3] = { x, y, z };
					volume_set_at(volume, NULL, dpos, v);
				}
			}

	volume_delete(src);
}

void volume_shift_alpha(volume_t *volume, int v) {
	if (!volume)
		return;
	for (tile_node_t *n = volume->tiles; n; n = n->next) {
		for (int i = 0; i < TILE_VOXELS; i++) {
			int a = (int)n->tile->data[i * 4 + 3] + v;
			if (a < 0)
				a = 0;
			if (a > 255)
				a = 255;
			n->tile->data[i * 4 + 3] = (uint8_t)a;
		}
	}
	volume->key = g_next_key++;
}

int volume_select(const volume_t *volume, const int start_pos[3],
		int (*cond)(void *user, const volume_t *volume,
				const int base_pos[3], const int new_pos[3],
				volume_accessor_t *volume_accessor),
		void *user, volume_t *selection) {
	if (!volume || !selection)
		return 0;

	/* Flood-fill BFS from start_pos. */
	int bbox[2][3];
	if (!volume_get_bbox(volume, bbox, false))
		return 0;

	/* Check start voxel is non-empty. */
	uint8_t sv[4];
	volume_get_at(volume, NULL, start_pos, sv);
	if (sv[3] == 0)
		return 0;

	/* Mark start voxel in selection. */
	uint8_t white[4] = { 255, 255, 255, 255 };
	volume_set_at(selection, NULL, start_pos, white);

	/* Simple stack-based flood fill. */
	int cap = 4096;
	int count = 1;
	int *stack = (int *)malloc((size_t)cap * 3 * sizeof(int));
	stack[0] = start_pos[0];
	stack[1] = start_pos[1];
	stack[2] = start_pos[2];
	int sp = 1;

	static const int dirs[6][3] = {
		{ 1, 0, 0 }, { -1, 0, 0 }, { 0, 1, 0 }, { 0, -1, 0 }, { 0, 0, 1 }, { 0, 0, -1 }
	};

	volume_accessor_t acc = volume_get_accessor(volume);

	while (sp > 0) {
		sp--;
		int bx = stack[sp * 3 + 0];
		int by = stack[sp * 3 + 1];
		int bz = stack[sp * 3 + 2];
		int base_pos[3] = { bx, by, bz };

		for (int d = 0; d < 6; d++) {
			int nx = bx + dirs[d][0];
			int ny = by + dirs[d][1];
			int nz = bz + dirs[d][2];

			/* Bounds check. */
			if (nx < bbox[0][0] || nx >= bbox[1][0] ||
					ny < bbox[0][1] || ny >= bbox[1][1] ||
					nz < bbox[0][2] || nz >= bbox[1][2])
				continue;

			/* Already selected? */
			int npos[3] = { nx, ny, nz };
			uint8_t sel[4];
			volume_get_at(selection, NULL, npos, sel);
			if (sel[3] > 0)
				continue;

			/* Check condition. */
			if (cond && !cond(user, volume, base_pos, npos, &acc))
				continue;

			/* Check non-empty. */
			uint8_t nv[4];
			volume_get_at(volume, NULL, npos, nv);
			if (nv[3] == 0)
				continue;

			volume_set_at(selection, NULL, npos, white);
			count++;

			if (sp >= cap) {
				cap *= 2;
				stack = (int *)realloc(stack, (size_t)cap * 3 * sizeof(int));
			}
			stack[sp * 3 + 0] = nx;
			stack[sp * 3 + 1] = ny;
			stack[sp * 3 + 2] = nz;
			sp++;
		}
	}

	free(stack);
	return count;
}

void volume_crop(volume_t *volume, const float box[4][4]) {
	if (!volume)
		return;

	/* Compute AABB from box matrix. */
	int cmin[3], cmax[3];
	for (int i = 0; i < 3; i++) {
		float center = box[3][i];
		float extent = fabsf(box[0][i]) + fabsf(box[1][i]) + fabsf(box[2][i]);
		cmin[i] = (int)floorf(center - extent);
		cmax[i] = (int)ceilf(center + extent);
	}

	/* Zero out all voxels outside the crop region. */
	uint8_t zero[4] = { 0, 0, 0, 0 };
	for (tile_node_t *n = volume->tiles; n; n = n->next) {
		for (int z = 0; z < TS; z++)
			for (int y = 0; y < TS; y++)
				for (int x = 0; x < TS; x++) {
					int wx = n->pos[0] + x;
					int wy = n->pos[1] + y;
					int wz = n->pos[2] + z;
					if (wx >= cmin[0] && wx < cmax[0] &&
							wy >= cmin[1] && wy < cmax[1] &&
							wz >= cmin[2] && wz < cmax[2])
						continue;
					int off = voxel_offset(x, y, z);
					memcpy(&n->tile->data[off], zero, 4);
				}
	}
	volume_remove_empty_tiles(volume, false);
	volume->key = g_next_key++;
}

uint32_t volume_crc32(const volume_t *volume) {
	if (!volume)
		return 0;
	/* Simple CRC32 over all non-empty voxels in bbox order. */
	int bbox[2][3];
	if (!volume_get_bbox(volume, bbox, true))
		return 0;

	uint32_t crc = 0xFFFFFFFF;
	for (int z = bbox[0][2]; z < bbox[1][2]; z++)
		for (int y = bbox[0][1]; y < bbox[1][1]; y++)
			for (int x = bbox[0][0]; x < bbox[1][0]; x++) {
				uint8_t v[4];
				int pos[3] = { x, y, z };
				volume_get_at(volume, NULL, pos, v);
				for (int i = 0; i < 4; i++) {
					crc ^= v[i];
					for (int b = 0; b < 8; b++) {
						if (crc & 1)
							crc = (crc >> 1) ^ 0xEDB88320;
						else
							crc >>= 1;
					}
				}
			}
	return crc ^ 0xFFFFFFFF;
}

/* ---- mesh generation -------------------------------------------------- */

/* Declared in marchingcube.c — generates MC vertices for one tile. */
int volume_generate_vertices_mc(const volume_t *volume,
		const int block_pos[3], int effects,
		voxel_vertex_t *out, int *size, int *subdivide);

/* Generate cube-style vertices for one tile (simple box per voxel). */
static int volume_generate_vertices_cubes(const volume_t *volume,
		const int block_pos[3],
		voxel_vertex_t *out, int *size, int *subdivide) {
	*size = 4; /* quads */
	*subdivide = 1;

	uint8_t data[TS * TS * TS * 4];
	volume_read(volume, block_pos, (const int[3]){ TS, TS, TS }, data);

	int nb = 0;
	static const int FACE_NORMALS[6][3] = {
		{ 0, -1, 0 }, { 0, 1, 0 }, { 0, 0, -1 },
		{ 0, 0, 1 }, { 1, 0, 0 }, { -1, 0, 0 }
	};
	static const int FACE_VERTS[6][4][3] = {
		/* -Y */ { { 0, 0, 0 }, { 1, 0, 0 }, { 1, 0, 1 }, { 0, 0, 1 } },
		/* +Y */ { { 0, 1, 0 }, { 0, 1, 1 }, { 1, 1, 1 }, { 1, 1, 0 } },
		/* -Z */ { { 0, 0, 0 }, { 0, 1, 0 }, { 1, 1, 0 }, { 1, 0, 0 } },
		/* +Z */ { { 0, 0, 1 }, { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 } },
		/* +X */ { { 1, 0, 0 }, { 1, 1, 0 }, { 1, 1, 1 }, { 1, 0, 1 } },
		/* -X */ { { 0, 0, 0 }, { 0, 0, 1 }, { 0, 1, 1 }, { 0, 1, 0 } },
	};

	for (int z = 0; z < TS; z++)
		for (int y = 0; y < TS; y++)
			for (int x = 0; x < TS; x++) {
				int off = ((z * TS + y) * TS + x) * 4;
				if (data[off + 3] == 0)
					continue;

				for (int f = 0; f < 6; f++) {
					/* Check neighbour. */
					int nx = x + FACE_NORMALS[f][0];
					int ny = y + FACE_NORMALS[f][1];
					int nz = z + FACE_NORMALS[f][2];
					if (nx >= 0 && nx < TS && ny >= 0 && ny < TS &&
							nz >= 0 && nz < TS) {
						int noff = ((nz * TS + ny) * TS + nx) * 4;
						if (data[noff + 3] > 0)
							continue;
					}
					/* Emit quad. */
					for (int v = 0; v < 4; v++) {
						voxel_vertex_t *vt = &out[nb * 4 + v];
						memset(vt, 0, sizeof(*vt));
						vt->pos[0] = x + FACE_VERTS[f][v][0];
						vt->pos[1] = y + FACE_VERTS[f][v][1];
						vt->pos[2] = z + FACE_VERTS[f][v][2];
						vt->normal[0] = FACE_NORMALS[f][0] * 127;
						vt->normal[1] = FACE_NORMALS[f][1] * 127;
						vt->normal[2] = FACE_NORMALS[f][2] * 127;
						vt->color[0] = data[off + 0];
						vt->color[1] = data[off + 1];
						vt->color[2] = data[off + 2];
						vt->color[3] = data[off + 3];
					}
					nb++;
				}
			}
	return nb;
}

int volume_generate_vertices(const volume_t *volume, const int block_pos[3],
		int effects, voxel_vertex_t *out,
		int *size, int *subdivide) {
	if (effects & EFFECT_MARCHING_CUBES) {
		return volume_generate_vertices_mc(
				volume, block_pos, effects, out, size, subdivide);
	}
	return volume_generate_vertices_cubes(
			volume, block_pos, out, size, subdivide);
}

volume_mesh_t *volume_generate_mesh(const volume_t *volume, int effects,
		const palette_t *palette, int options) {
	(void)palette;
	(void)options;
	if (!volume)
		return NULL;

	volume_mesh_t *mesh = (volume_mesh_t *)calloc(1, sizeof(volume_mesh_t));
	int cap_verts = 0;
	int cap_idx = 0;

	/* Iterate tiles. */
	volume_iterator_t it = volume_get_iterator(volume,
			VOLUME_ITER_TILES | VOLUME_ITER_SKIP_EMPTY);
	int tpos[3];
	while (iter_tiles(&it, tpos)) {
		/* Generous buffer — worst case each voxel has 6 faces × 4 verts
		 * for cubes or many triangles for MC. */
		int buf_size = TS * TS * TS * 6 * 4;
		voxel_vertex_t *verts =
				(voxel_vertex_t *)malloc(buf_size * sizeof(voxel_vertex_t));
		int prim_size, subdivide;
		int nb = volume_generate_vertices(volume, tpos, effects,
				verts, &prim_size, &subdivide);

		float scale = 1.0f / (float)subdivide;
		int total_verts = nb * prim_size;

		/* Grow output arrays. */
		int new_cap = cap_verts + total_verts;
		mesh->vertices = (void *)realloc(mesh->vertices,
				(size_t)new_cap * sizeof(*mesh->vertices));

		/* Triangulate. */
		int new_indices;
		if (prim_size == 3) {
			new_indices = total_verts;
		} else {
			/* Quads → 2 triangles per quad = 6 indices per quad. */
			new_indices = nb * 6;
		}
		int new_idx_cap = cap_idx + new_indices;
		mesh->indices = (unsigned int *)realloc(mesh->indices,
				(size_t)new_idx_cap * sizeof(unsigned int));

		for (int i = 0; i < total_verts; i++) {
			int vi = cap_verts + i;
			mesh->vertices[vi].pos[0] =
					tpos[0] + verts[i].pos[0] * scale;
			mesh->vertices[vi].pos[1] =
					tpos[1] + verts[i].pos[1] * scale;
			mesh->vertices[vi].pos[2] =
					tpos[2] + verts[i].pos[2] * scale;
			mesh->vertices[vi].normal[0] = verts[i].normal[0] / 127.0f;
			mesh->vertices[vi].normal[1] = verts[i].normal[1] / 127.0f;
			mesh->vertices[vi].normal[2] = verts[i].normal[2] / 127.0f;
			mesh->vertices[vi].color[0] = verts[i].color[0] / 255.0f;
			mesh->vertices[vi].color[1] = verts[i].color[1] / 255.0f;
			mesh->vertices[vi].color[2] = verts[i].color[2] / 255.0f;
			mesh->vertices[vi].color[3] = verts[i].color[3] / 255.0f;
		}

		if (prim_size == 3) {
			for (int i = 0; i < total_verts; i++) {
				mesh->indices[cap_idx + i] = cap_verts + i;
			}
		} else {
			/* Quads. */
			for (int q = 0; q < nb; q++) {
				int base = cap_verts + q * 4;
				int idx = cap_idx + q * 6;
				mesh->indices[idx + 0] = base + 0;
				mesh->indices[idx + 1] = base + 1;
				mesh->indices[idx + 2] = base + 2;
				mesh->indices[idx + 3] = base + 0;
				mesh->indices[idx + 4] = base + 2;
				mesh->indices[idx + 5] = base + 3;
			}
		}

		cap_verts = new_cap;
		cap_idx = new_idx_cap;
		free(verts);
	}

	mesh->vertices_count = cap_verts;
	mesh->indices_count = cap_idx;

	/* Compute bounds. */
	if (cap_verts > 0) {
		for (int i = 0; i < 3; i++) {
			mesh->pos_min[i] = mesh->vertices[0].pos[i];
			mesh->pos_max[i] = mesh->vertices[0].pos[i];
		}
		for (int i = 1; i < cap_verts; i++) {
			for (int j = 0; j < 3; j++) {
				if (mesh->vertices[i].pos[j] < mesh->pos_min[j])
					mesh->pos_min[j] = mesh->vertices[i].pos[j];
				if (mesh->vertices[i].pos[j] > mesh->pos_max[j])
					mesh->pos_max[j] = mesh->vertices[i].pos[j];
			}
		}
	}

	return mesh;
}

void volume_mesh_free(volume_mesh_t *mesh) {
	if (!mesh)
		return;
	free(mesh->vertices);
	free(mesh->indices);
	free(mesh);
}

/* ---- quantization (median cut) ---------------------------------------- */

typedef struct {
	uint8_t color[4];
	int count;
} color_entry_t;

static int color_cmp_r(const void *a, const void *b) {
	return ((const color_entry_t *)a)->color[0] -
			((const color_entry_t *)b)->color[0];
}
static int color_cmp_g(const void *a, const void *b) {
	return ((const color_entry_t *)a)->color[1] -
			((const color_entry_t *)b)->color[1];
}
static int color_cmp_b(const void *a, const void *b) {
	return ((const color_entry_t *)a)->color[2] -
			((const color_entry_t *)b)->color[2];
}

/* Median cut: recursively split the color list along the axis with
 * greatest range, until we have the desired number of buckets. */
static void median_cut(color_entry_t *colors, int count, int depth,
		int target_depth, uint8_t (*out_palette)[4], int *out_idx) {
	if (depth >= target_depth || count <= 0) {
		/* Average all colors in this bucket. */
		long r = 0, g = 0, b = 0, total = 0;
		for (int i = 0; i < count; i++) {
			int w = colors[i].count;
			r += colors[i].color[0] * w;
			g += colors[i].color[1] * w;
			b += colors[i].color[2] * w;
			total += w;
		}
		if (total > 0) {
			out_palette[*out_idx][0] = (uint8_t)(r / total);
			out_palette[*out_idx][1] = (uint8_t)(g / total);
			out_palette[*out_idx][2] = (uint8_t)(b / total);
			out_palette[*out_idx][3] = 255;
		}
		(*out_idx)++;
		return;
	}

	/* Find axis with greatest range. */
	int min_r = 255, max_r = 0;
	int min_g = 255, max_g = 0;
	int min_b = 255, max_b = 0;
	for (int i = 0; i < count; i++) {
		if (colors[i].color[0] < min_r)
			min_r = colors[i].color[0];
		if (colors[i].color[0] > max_r)
			max_r = colors[i].color[0];
		if (colors[i].color[1] < min_g)
			min_g = colors[i].color[1];
		if (colors[i].color[1] > max_g)
			max_g = colors[i].color[1];
		if (colors[i].color[2] < min_b)
			min_b = colors[i].color[2];
		if (colors[i].color[2] > max_b)
			max_b = colors[i].color[2];
	}
	int range_r = max_r - min_r;
	int range_g = max_g - min_g;
	int range_b = max_b - min_b;

	if (range_r >= range_g && range_r >= range_b)
		qsort(colors, count, sizeof(color_entry_t), color_cmp_r);
	else if (range_g >= range_r && range_g >= range_b)
		qsort(colors, count, sizeof(color_entry_t), color_cmp_g);
	else
		qsort(colors, count, sizeof(color_entry_t), color_cmp_b);

	int mid = count / 2;
	median_cut(colors, mid, depth + 1, target_depth, out_palette, out_idx);
	median_cut(colors + mid, count - mid, depth + 1, target_depth,
			out_palette, out_idx);
}

void quantization_gen_palette(const volume_t *volume, int nb,
		uint8_t (*palette)[4]) {
	if (!volume || nb <= 0)
		return;

	/* Collect unique colors. */
	int bbox[2][3];
	if (!volume_get_bbox(volume, bbox, true))
		return;

	int cap = 256;
	int unique_count = 0;
	color_entry_t *colors = (color_entry_t *)malloc(
			(size_t)cap * sizeof(color_entry_t));

	for (int z = bbox[0][2]; z < bbox[1][2]; z++)
		for (int y = bbox[0][1]; y < bbox[1][1]; y++)
			for (int x = bbox[0][0]; x < bbox[1][0]; x++) {
				int pos[3] = { x, y, z };
				uint8_t v[4];
				volume_get_at(volume, NULL, pos, v);
				if (v[3] == 0)
					continue;

				/* Check if already in list. */
				bool found = false;
				for (int i = 0; i < unique_count; i++) {
					if (colors[i].color[0] == v[0] &&
							colors[i].color[1] == v[1] &&
							colors[i].color[2] == v[2]) {
						colors[i].count++;
						found = true;
						break;
					}
				}
				if (!found) {
					if (unique_count >= cap) {
						cap *= 2;
						colors = (color_entry_t *)realloc(colors,
								(size_t)cap * sizeof(color_entry_t));
					}
					colors[unique_count].color[0] = v[0];
					colors[unique_count].color[1] = v[1];
					colors[unique_count].color[2] = v[2];
					colors[unique_count].color[3] = 255;
					colors[unique_count].count = 1;
					unique_count++;
				}
			}

	if (unique_count == 0) {
		free(colors);
		return;
	}

	/* Determine tree depth for median cut. */
	int depth = 0;
	int buckets = 1;
	while (buckets < nb) {
		depth++;
		buckets *= 2;
	}

	memset(palette, 0, (size_t)nb * 4);
	int out_idx = 0;
	median_cut(colors, unique_count, 0, depth, palette, &out_idx);

	free(colors);
}
