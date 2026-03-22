/* Goxel volume format import/export — portable C implementations.
 *
 * Adapted from the upstream goxel repository format handlers.
 * Each format is a self-contained reader/writer operating on volume_t*.
 *
 * License: GPL-3.0 (matches upstream goxel)
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "goxel/volume.h"
#include "goxel/volume_utils.h"

/* ---- helpers ---------------------------------------------------------- */

static uint32_t read_le32(const uint8_t *p) {
	return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
			((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void write_le32(uint8_t *p, uint32_t v) {
	p[0] = (uint8_t)(v);
	p[1] = (uint8_t)(v >> 8);
	p[2] = (uint8_t)(v >> 16);
	p[3] = (uint8_t)(v >> 24);
}

/* ======================================================================= */
/*  VOX (MagicaVoxel) format                                               */
/* ======================================================================= */

/*
 * MagicaVoxel .vox file format (simplified):
 *   4 bytes: "VOX " magic
 *   4 bytes: version (150)
 *   Then chunks:
 *     4 bytes: chunk ID
 *     4 bytes: content size N
 *     4 bytes: children size M
 *     N bytes: content
 *     M bytes: children
 *
 * We care about: SIZE, XYZI, RGBA chunks.
 */

volume_t *goxel_import_vox(const uint8_t *data, int data_size) {
	if (data_size < 12)
		return NULL;
	if (memcmp(data, "VOX ", 4) != 0)
		return NULL;

	volume_t *vol = volume_new();

	/* Default MagicaVoxel palette. */
	uint8_t palette[256][4];
	memset(palette, 0, sizeof(palette));
	/* Fill with default MV palette — all white for now, RGBA chunk overrides. */
	for (int i = 1; i < 256; i++) {
		palette[i][0] = palette[i][1] = palette[i][2] = 255;
		palette[i][3] = 255;
	}

	int sx = 0, sy = 0, sz = 0;
	bool has_rgba = false;

	/* Skip past MAIN chunk header to reach children.
	 * File layout: magic(4) + version(4) + MAIN_header(12) + children... */
	int main_off = 8; /* offset of MAIN chunk */
	if (main_off + 12 > data_size) {
		volume_delete(vol);
		return NULL;
	}
	/* uint32_t main_content = read_le32(data + main_off + 4); */
	/* uint32_t main_children = read_le32(data + main_off + 8); */
	int children_start = main_off + 12; /* skip MAIN header to children */

	/* First pass: find RGBA chunk if present. */
	int off = children_start;
	while (off + 12 <= data_size) {
		const uint8_t *chunk = &data[off];
		uint32_t content_size = read_le32(chunk + 4);
		uint32_t children_size = read_le32(chunk + 8);

		if (memcmp(chunk, "RGBA", 4) == 0 && content_size >= 1024) {
			for (int i = 0; i < 255; i++) {
				palette[i + 1][0] = chunk[12 + i * 4 + 0];
				palette[i + 1][1] = chunk[12 + i * 4 + 1];
				palette[i + 1][2] = chunk[12 + i * 4 + 2];
				palette[i + 1][3] = chunk[12 + i * 4 + 3];
			}
			has_rgba = true;
		}
		off += 12 + content_size + children_size;
	}
	(void)has_rgba;

	/* Second pass: read SIZE and XYZI chunks. */
	off = children_start;
	while (off + 12 <= data_size) {
		const uint8_t *chunk = &data[off];
		uint32_t content_size = read_le32(chunk + 4);
		uint32_t children_size = read_le32(chunk + 8);
		const uint8_t *content = chunk + 12;

		if (memcmp(chunk, "SIZE", 4) == 0 && content_size >= 12) {
			sx = (int)read_le32(content + 0);
			sy = (int)read_le32(content + 4);
			sz = (int)read_le32(content + 8);
			(void)sx;
			(void)sy;
			(void)sz;
		} else if (memcmp(chunk, "XYZI", 4) == 0 && content_size >= 4) {
			uint32_t num_voxels = read_le32(content);
			if (content_size >= 4 + num_voxels * 4) {
				for (uint32_t i = 0; i < num_voxels; i++) {
					int vx = content[4 + i * 4 + 0];
					int vy = content[4 + i * 4 + 1];
					int vz = content[4 + i * 4 + 2];
					int ci = content[4 + i * 4 + 3];
					/* MagicaVoxel uses Y-up, Z-forward.
					 * We map: x→x, z→y, y→z */
					int pos[3] = { vx, vz, vy };
					volume_set_at(vol, NULL, pos, palette[ci]);
				}
			}
		}

		off += 12 + content_size + children_size;
	}

	return vol;
}

int goxel_export_vox(const volume_t *volume, uint8_t **out_data,
		int *out_size) {
	if (!volume)
		return -1;

	int bbox[2][3];
	if (!volume_get_bbox(volume, bbox, true))
		return -1;

	/* Collect voxels. */
	int cap = 1024;
	int count = 0;
	uint8_t *voxels = (uint8_t *)malloc((size_t)cap * 4);
	uint8_t palette[256][4];
	int palette_count = 1; /* index 0 is empty */
	memset(palette, 0, sizeof(palette));

	for (int z = bbox[0][2]; z < bbox[1][2]; z++)
		for (int y = bbox[0][1]; y < bbox[1][1]; y++)
			for (int x = bbox[0][0]; x < bbox[1][0]; x++) {
				int pos[3] = { x, y, z };
				uint8_t v[4];
				volume_get_at(volume, NULL, pos, v);
				if (v[3] == 0)
					continue;

				/* Find or add color in palette. */
				int ci = -1;
				for (int i = 1; i < palette_count; i++) {
					if (palette[i][0] == v[0] && palette[i][1] == v[1] &&
							palette[i][2] == v[2]) {
						ci = i;
						break;
					}
				}
				if (ci < 0 && palette_count < 256) {
					ci = palette_count++;
					palette[ci][0] = v[0];
					palette[ci][1] = v[1];
					palette[ci][2] = v[2];
					palette[ci][3] = 255;
				}
				if (ci < 0)
					ci = 1; /* fallback */

				if (count >= cap) {
					cap *= 2;
					voxels = (uint8_t *)realloc(voxels, (size_t)cap * 4);
				}
				/* Map back: x→x, y→z, z→y for MagicaVoxel coords */
				voxels[count * 4 + 0] = (uint8_t)(x - bbox[0][0]);
				voxels[count * 4 + 1] = (uint8_t)(z - bbox[0][2]);
				voxels[count * 4 + 2] = (uint8_t)(y - bbox[0][1]);
				voxels[count * 4 + 3] = (uint8_t)ci;
				count++;
			}

	/* Build file. */
	int size_w = bbox[1][0] - bbox[0][0];
	int size_h = bbox[1][2] - bbox[0][2];
	int size_d = bbox[1][1] - bbox[0][1];

	/* Calculate total size:
	 * header(8) + MAIN(12) + SIZE(12+12) + XYZI(12+4+count*4) + RGBA(12+1024) */
	int total = 8 + 12 + 24 + 12 + 4 + count * 4 + 12 + 1024;
	uint8_t *buf = (uint8_t *)calloc(1, total);
	int p = 0;

	/* File header. */
	memcpy(buf + p, "VOX ", 4);
	p += 4;
	write_le32(buf + p, 150);
	p += 4;

	/* MAIN chunk. */
	memcpy(buf + p, "MAIN", 4);
	p += 4;
	write_le32(buf + p, 0);
	p += 4; /* content size */
	write_le32(buf + p, total - 20);
	p += 4; /* children size */

	/* SIZE chunk. */
	memcpy(buf + p, "SIZE", 4);
	p += 4;
	write_le32(buf + p, 12);
	p += 4;
	write_le32(buf + p, 0);
	p += 4;
	write_le32(buf + p, size_w);
	p += 4;
	write_le32(buf + p, size_h);
	p += 4;
	write_le32(buf + p, size_d);
	p += 4;

	/* XYZI chunk. */
	memcpy(buf + p, "XYZI", 4);
	p += 4;
	write_le32(buf + p, 4 + count * 4);
	p += 4;
	write_le32(buf + p, 0);
	p += 4;
	write_le32(buf + p, count);
	p += 4;
	memcpy(buf + p, voxels, count * 4);
	p += count * 4;

	/* RGBA chunk. */
	memcpy(buf + p, "RGBA", 4);
	p += 4;
	write_le32(buf + p, 1024);
	p += 4;
	write_le32(buf + p, 0);
	p += 4;
	for (int i = 0; i < 255; i++) {
		buf[p + i * 4 + 0] = palette[i + 1][0];
		buf[p + i * 4 + 1] = palette[i + 1][1];
		buf[p + i * 4 + 2] = palette[i + 1][2];
		buf[p + i * 4 + 3] = palette[i + 1][3];
	}
	p += 1024;

	free(voxels);
	*out_data = buf;
	*out_size = p;
	return 0;
}

/* ======================================================================= */
/*  GOX (native goxel) format                                              */
/* ======================================================================= */

/*
 * GOX file format:
 *   4 bytes: "GOX " magic
 *   4 bytes: version
 *   Then chunks:
 *     4 bytes: type
 *     4 bytes: data length
 *     N bytes: data
 *     4 bytes: CRC (ignored)
 *
 * BL16 chunk: 16×16×16 tile data (compressed or raw)
 * LAYR chunk: layer with tile positions
 */

volume_t *goxel_import_gox(const uint8_t *data, int data_size) {
	if (data_size < 8)
		return NULL;
	if (memcmp(data, "GOX ", 4) != 0)
		return NULL;

	volume_t *vol = volume_new();
	int off = 8;

	/* Collect BL16 data blocks first. */
	int bl16_cap = 64;
	int bl16_count = 0;
	uint8_t **bl16_data = (uint8_t **)malloc(
			(size_t)bl16_cap * sizeof(uint8_t *));

	while (off + 8 <= data_size) {
		uint32_t chunk_len = read_le32((const uint8_t *)data + off + 4);
		if (off + 8 + (int)chunk_len + 4 > data_size)
			break;

		if (memcmp(data + off, "BL16", 4) == 0) {
			if (bl16_count >= bl16_cap) {
				bl16_cap *= 2;
				bl16_data = (uint8_t **)realloc(bl16_data,
						(size_t)bl16_cap * sizeof(uint8_t *));
			}
			uint8_t *block = (uint8_t *)malloc(16 * 16 * 16 * 4);
			int bdata_size = (int)chunk_len;
			if (bdata_size >= 16 * 16 * 16 * 4) {
				memcpy(block, data + off + 8, 16 * 16 * 16 * 4);
			} else {
				/* Data might be raw or truncated — zero-fill rest. */
				memset(block, 0, 16 * 16 * 16 * 4);
				memcpy(block, data + off + 8,
						bdata_size < 16 * 16 * 16 * 4 ? (size_t)bdata_size : 16 * 16 * 16 * 4);
			}
			bl16_data[bl16_count++] = block;
		}

		off += 8 + (int)chunk_len + 4; /* +4 for CRC */
	}

	/* Second pass: find LAYR chunks and place blocks. */
	off = 8;
	while (off + 8 <= data_size) {
		uint32_t chunk_len = read_le32((const uint8_t *)data + off + 4);
		if (off + 8 + (int)chunk_len + 4 > data_size)
			break;

		if (memcmp(data + off, "LAYR", 4) == 0) {
			const uint8_t *layr = data + off + 8;
			int layr_off = 0;
			/* LAYR format: repeated (block_index:4, pos_x:4, pos_y:4,
			 * pos_z:4, flags:4) = 20 bytes per entry, then metadata. */
			while (layr_off + 20 <= (int)chunk_len) {
				int block_idx = (int)read_le32(layr + layr_off);
				int px = (int)(int32_t)read_le32(layr + layr_off + 4);
				int py = (int)(int32_t)read_le32(layr + layr_off + 8);
				int pz = (int)(int32_t)read_le32(layr + layr_off + 12);
				/* uint32_t flags = read_le32(layr + layr_off + 16); */
				layr_off += 20;

				if (block_idx < 0 || block_idx >= bl16_count)
					continue;

				/* Blit the 16×16×16 block at position. */
				volume_blit(vol, bl16_data[block_idx],
						px, py, pz, 16, 16, 16, NULL);
			}
		}

		off += 8 + (int)chunk_len + 4;
	}

	for (int i = 0; i < bl16_count; i++)
		free(bl16_data[i]);
	free(bl16_data);

	return vol;
}

int goxel_export_gox(const volume_t *volume, uint8_t **out_data,
		int *out_size) {
	if (!volume)
		return -1;

	int bbox[2][3];
	if (!volume_get_bbox(volume, bbox, false))
		return -1;

	/* Iterate tiles. */
	int tile_count = volume_get_tiles_count(volume);
	if (tile_count <= 0)
		return -1;

	/* Estimate max size. */
	int max_size = 8 + /* header */
			(tile_count * (12 + 16 * 16 * 16 * 4 + 4)) + /* BL16 chunks */
			(12 + tile_count * 20 + 4) + /* LAYR chunk */
			1024; /* padding */
	uint8_t *buf = (uint8_t *)calloc(1, max_size);
	int p = 0;

	memcpy(buf, "GOX ", 4);
	p += 4;
	write_le32(buf + p, 2);
	p += 4; /* version 2 */

	/* Emit BL16 and collect positions. */
	int *positions = (int *)malloc((size_t)tile_count * 3 * sizeof(int));
	int ti = 0;

	volume_iterator_t it = volume_get_iterator(volume,
			VOLUME_ITER_TILES | VOLUME_ITER_SKIP_EMPTY);
	int tpos[3];
	while (volume_iter(&it, tpos)) {
		/* Read tile data. */
		uint8_t tile_data[16 * 16 * 16 * 4];
		int rsize[3] = { 16, 16, 16 };
		volume_read(volume, tpos, rsize, tile_data);

		/* BL16 chunk. */
		memcpy(buf + p, "BL16", 4);
		p += 4;
		write_le32(buf + p, 16 * 16 * 16 * 4);
		p += 4;
		memcpy(buf + p, tile_data, 16 * 16 * 16 * 4);
		p += 16 * 16 * 16 * 4;
		write_le32(buf + p, 0);
		p += 4; /* CRC placeholder */

		positions[ti * 3 + 0] = tpos[0];
		positions[ti * 3 + 1] = tpos[1];
		positions[ti * 3 + 2] = tpos[2];
		ti++;
	}

	/* LAYR chunk. */
	int layr_size = ti * 20;
	memcpy(buf + p, "LAYR", 4);
	p += 4;
	write_le32(buf + p, layr_size);
	p += 4;
	for (int i = 0; i < ti; i++) {
		write_le32(buf + p, i);
		p += 4;
		write_le32(buf + p, (uint32_t)positions[i * 3 + 0]);
		p += 4;
		write_le32(buf + p, (uint32_t)positions[i * 3 + 1]);
		p += 4;
		write_le32(buf + p, (uint32_t)positions[i * 3 + 2]);
		p += 4;
		write_le32(buf + p, 0);
		p += 4; /* flags */
	}
	write_le32(buf + p, 0);
	p += 4; /* CRC placeholder */

	free(positions);
	*out_data = buf;
	*out_size = p;
	return 0;
}

/* ======================================================================= */
/*  Qubicle QB format                                                      */
/* ======================================================================= */

volume_t *goxel_import_qb(const uint8_t *data, int data_size) {
	if (data_size < 24)
		return NULL;

	/* QB header: version(4), colorFormat(4), zAxisOrientation(4),
	 * compressed(4), visibilityMaskEncoded(4), numMatrices(4) */
	/* uint32_t version = read_le32(data); */
	uint32_t color_format = read_le32(data + 4);
	/* uint32_t z_orient = read_le32(data + 8); */
	uint32_t compressed = read_le32(data + 12);
	/* uint32_t vis_mask = read_le32(data + 16); */
	uint32_t num_matrices = read_le32(data + 20);
	(void)compressed; /* We only support uncompressed for now. */

	volume_t *vol = volume_new();
	int off = 24;

	for (uint32_t m = 0; m < num_matrices; m++) {
		if (off >= data_size)
			break;

		/* Name length + name. */
		uint8_t name_len = data[off++];
		off += name_len;
		if (off + 12 > data_size)
			break;

		uint32_t sx = read_le32(data + off);
		off += 4;
		uint32_t sy = read_le32(data + off);
		off += 4;
		uint32_t sz = read_le32(data + off);
		off += 4;

		if (off + 12 > data_size)
			break;
		int32_t px = (int32_t)read_le32(data + off);
		off += 4;
		int32_t py = (int32_t)read_le32(data + off);
		off += 4;
		int32_t pz = (int32_t)read_le32(data + off);
		off += 4;

		/* Uncompressed: sx*sy*sz voxels of 4 bytes each. */
		for (uint32_t z = 0; z < sz; z++)
			for (uint32_t y = 0; y < sy; y++)
				for (uint32_t x = 0; x < sx; x++) {
					if (off + 4 > data_size)
						goto done_qb;
					uint8_t r, g, b, a;
					if (color_format == 0) {
						/* RGBA */
						r = data[off + 0];
						g = data[off + 1];
						b = data[off + 2];
						a = data[off + 3];
					} else {
						/* BGRA */
						b = data[off + 0];
						g = data[off + 1];
						r = data[off + 2];
						a = data[off + 3];
					}
					off += 4;
					if (a == 0)
						continue;
					uint8_t v[4] = { r, g, b, a };
					int pos[3] = { px + (int)x, py + (int)y, pz + (int)z };
					volume_set_at(vol, NULL, pos, v);
				}
	}
done_qb:
	return vol;
}

int goxel_export_qb(const volume_t *volume, uint8_t **out_data,
		int *out_size) {
	if (!volume)
		return -1;

	int bbox[2][3];
	if (!volume_get_bbox(volume, bbox, true))
		return -1;

	int sx = bbox[1][0] - bbox[0][0];
	int sy = bbox[1][1] - bbox[0][1];
	int sz = bbox[1][2] - bbox[0][2];

	int total = 24 + 1 + 0 + 12 + 12 + sx * sy * sz * 4;
	uint8_t *buf = (uint8_t *)calloc(1, total);
	int p = 0;

	/* Header. */
	write_le32(buf + p, 0x00000101);
	p += 4; /* version 1.1.0.0 */
	write_le32(buf + p, 0);
	p += 4; /* RGBA format */
	write_le32(buf + p, 1);
	p += 4; /* right-handed */
	write_le32(buf + p, 0);
	p += 4; /* uncompressed */
	write_le32(buf + p, 0);
	p += 4; /* no visibility mask */
	write_le32(buf + p, 1);
	p += 4; /* 1 matrix */

	/* Matrix name. */
	buf[p++] = 0; /* empty name */

	/* Size. */
	write_le32(buf + p, sx);
	p += 4;
	write_le32(buf + p, sy);
	p += 4;
	write_le32(buf + p, sz);
	p += 4;

	/* Position. */
	write_le32(buf + p, (uint32_t)bbox[0][0]);
	p += 4;
	write_le32(buf + p, (uint32_t)bbox[0][1]);
	p += 4;
	write_le32(buf + p, (uint32_t)bbox[0][2]);
	p += 4;

	/* Voxels. */
	for (int z = 0; z < sz; z++)
		for (int y = 0; y < sy; y++)
			for (int x = 0; x < sx; x++) {
				int pos[3] = { bbox[0][0] + x, bbox[0][1] + y,
					bbox[0][2] + z };
				uint8_t v[4];
				volume_get_at(volume, NULL, pos, v);
				buf[p++] = v[0];
				buf[p++] = v[1];
				buf[p++] = v[2];
				buf[p++] = v[3];
			}

	*out_data = buf;
	*out_size = p;
	return 0;
}

/* ======================================================================= */
/*  Text XYZ+RGB format                                                    */
/* ======================================================================= */

volume_t *goxel_import_txt(const uint8_t *data, int data_size) {
	volume_t *vol = volume_new();

	const char *ptr = (const char *)data;
	const char *end = ptr + data_size;

	while (ptr < end) {
		int x, y, z, r, g, b;
		int n = 0;
		/* Try to parse: x y z r g b */
		if (sscanf(ptr, "%d %d %d %d %d %d%n", &x, &y, &z, &r, &g, &b,
					&n) >= 6 &&
				n > 0) {
			uint8_t v[4] = { (uint8_t)r, (uint8_t)g, (uint8_t)b, 255 };
			int pos[3] = { x, y, z };
			volume_set_at(vol, NULL, pos, v);
		}
		/* Skip to next line. */
		while (ptr < end && *ptr != '\n')
			ptr++;
		if (ptr < end)
			ptr++;
	}

	return vol;
}

int goxel_export_txt(const volume_t *volume, uint8_t **out_data,
		int *out_size) {
	if (!volume)
		return -1;

	int bbox[2][3];
	if (!volume_get_bbox(volume, bbox, true))
		return -1;

	/* Estimate max size — 30 chars per voxel. */
	int max_voxels = (bbox[1][0] - bbox[0][0]) *
			(bbox[1][1] - bbox[0][1]) *
			(bbox[1][2] - bbox[0][2]);
	int buf_size = max_voxels * 30 + 1;
	char *buf = (char *)malloc(buf_size);
	int p = 0;

	for (int z = bbox[0][2]; z < bbox[1][2]; z++)
		for (int y = bbox[0][1]; y < bbox[1][1]; y++)
			for (int x = bbox[0][0]; x < bbox[1][0]; x++) {
				int pos[3] = { x, y, z };
				uint8_t v[4];
				volume_get_at(volume, NULL, pos, v);
				if (v[3] == 0)
					continue;
				p += snprintf(buf + p, buf_size - p,
						"%d %d %d %d %d %d\n",
						x, y, z, v[0], v[1], v[2]);
			}

	*out_data = (uint8_t *)buf;
	*out_size = p;
	return 0;
}

/* ======================================================================= */
/*  VXL (Ace of Spades) format — import only                               */
/* ======================================================================= */

volume_t *goxel_import_vxl(const uint8_t *data, int data_size) {
	/* VXL is a 512×512×64 column-based format.
	 * Each column has spans of solid and air. */
	volume_t *vol = volume_new();
	int map_x = 512, map_y = 512, map_z = 64;
	int off = 0;

	for (int y = 0; y < map_y; y++) {
		for (int x = 0; x < map_x; x++) {
			int z = 0;
			while (z < map_z) {
				if (off + 4 > data_size)
					goto done_vxl;
				int span_start = data[off + 1];
				int span_end = data[off + 2];
				int span_len = data[off + 3];
				int air_start = data[off + 0];

				/* Skip air. */
				z = air_start;

				/* Read surface voxels. */
				off += 4;
				int top_count = span_end - span_start + 1;
				for (int i = 0; i < top_count; i++) {
					if (off + 4 > data_size)
						goto done_vxl;
					uint8_t b = data[off + 0];
					uint8_t g = data[off + 1];
					uint8_t r = data[off + 2];
					/* data[off+3] is unused/alpha */
					off += 4;
					uint8_t v[4] = { r, g, b, 255 };
					int pos[3] = { x, z + i, y };
					volume_set_at(vol, NULL, pos, v);
				}

				z = span_end + 1;

				if (span_len == 0) {
					/* Fill remaining column as solid ground. */
					for (; z < map_z; z++) {
						uint8_t v[4] = { 128, 128, 128, 255 };
						int pos[3] = { x, z, y };
						volume_set_at(vol, NULL, pos, v);
					}
					break;
				}

				/* Bottom colors. */
				int bottom_count = span_len - 1 - top_count;
				for (int i = 0; i < bottom_count; i++) {
					if (off + 4 > data_size)
						goto done_vxl;
					off += 4; /* Skip bottom colors for simplicity. */
				}
			}
		}
	}
done_vxl:
	return vol;
}

/* ======================================================================= */
/*  KVX (Voxlap) format — import only                                      */
/* ======================================================================= */

volume_t *goxel_import_kvx(const uint8_t *data, int data_size) {
	if (data_size < 28)
		return NULL;

	uint32_t sx = read_le32(data + 4);
	uint32_t sy = read_le32(data + 8);
	uint32_t sz = read_le32(data + 12);
	/* Skip pivot (12 bytes). */

	volume_t *vol = volume_new();

	/* After header (28 bytes) comes the xoffset table (sx+1 uint32),
	 * then xyoffset table (sx * sy * 2 uint16),
	 * then palette (768 bytes at end),
	 * then voxel data. This is a simplified reader. */

	/* Read palette from end of file. */
	uint8_t pal[256][4];
	memset(pal, 0, sizeof(pal));
	if (data_size >= 768) {
		const uint8_t *pdata = data + data_size - 768;
		for (int i = 0; i < 256; i++) {
			/* KVX palette is 6-bit RGB. */
			pal[i][0] = pdata[i * 3 + 0] * 4;
			pal[i][1] = pdata[i * 3 + 1] * 4;
			pal[i][2] = pdata[i * 3 + 2] * 4;
			pal[i][3] = 255;
		}
	}

	/* Read voxel slab data. */
	int hdr_size = 28 + (int)(sx + 1) * 4 + (int)(sx * sy) * 2;
	if (hdr_size >= data_size) {
		volume_delete(vol);
		return NULL;
	}

	int off = hdr_size;
	for (uint32_t x = 0; x < sx; x++) {
		for (uint32_t y = 0; y < sy; y++) {
			while (off + 4 <= data_size - 768) {
				uint8_t ztop = data[off + 0];
				uint8_t zlength = data[off + 1];
				uint8_t vis = data[off + 2];
				(void)vis;
				/* uint8_t dir = data[off + 3]; */
				off += 4;

				if (zlength == 0)
					break;

				for (int zz = 0; zz < zlength; zz++) {
					if (off >= data_size - 768)
						break;
					uint8_t ci = data[off++];
					uint8_t v[4];
					memcpy(v, pal[ci], 4);
					int pos[3] = { (int)x, (int)(ztop + zz), (int)y };
					volume_set_at(vol, NULL, pos, v);
				}
			}
		}
	}

	return vol;
}

/* ======================================================================= */
/*  Wavefront OBJ — export only                                            */
/* ======================================================================= */

int goxel_export_obj(const volume_t *volume, char **out_data, int *out_size) {
	if (!volume)
		return -1;

	volume_mesh_t *mesh = volume_generate_mesh(volume, 0, NULL, 0);
	if (!mesh || mesh->vertices_count == 0) {
		if (mesh)
			volume_mesh_free(mesh);
		return -1;
	}

	/* Estimate buffer size. */
	int buf_size = mesh->vertices_count * 80 + mesh->indices_count * 20 + 256;
	char *buf = (char *)malloc(buf_size);
	int p = 0;

	p += snprintf(buf + p, buf_size - p, "# Goxel OBJ export\n");

	/* Vertices. */
	for (int i = 0; i < mesh->vertices_count; i++) {
		p += snprintf(buf + p, buf_size - p, "v %f %f %f %f %f %f\n",
				mesh->vertices[i].pos[0],
				mesh->vertices[i].pos[1],
				mesh->vertices[i].pos[2],
				mesh->vertices[i].color[0],
				mesh->vertices[i].color[1],
				mesh->vertices[i].color[2]);
	}

	/* Normals. */
	for (int i = 0; i < mesh->vertices_count; i++) {
		p += snprintf(buf + p, buf_size - p, "vn %f %f %f\n",
				mesh->vertices[i].normal[0],
				mesh->vertices[i].normal[1],
				mesh->vertices[i].normal[2]);
	}

	/* Faces. */
	for (int i = 0; i + 2 < mesh->indices_count; i += 3) {
		int a = mesh->indices[i] + 1;
		int b = mesh->indices[i + 1] + 1;
		int c = mesh->indices[i + 2] + 1;
		p += snprintf(buf + p, buf_size - p, "f %d//%d %d//%d %d//%d\n",
				a, a, b, b, c, c);
	}

	volume_mesh_free(mesh);
	*out_data = buf;
	*out_size = p;
	return 0;
}
