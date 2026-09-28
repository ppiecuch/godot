/**************************************************************************/
/*  rockcluster.h                                                         */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/**************************************************************************/

#ifndef PROC_ROCKS_ROCKCLUSTER_H
#define PROC_ROCKS_ROCKCLUSTER_H

#include "core/array.h"
#include "core/math/math_defs.h"
#include "scene/resources/curve.h"

// Method 4: RockCluster — scatters and combines small pre-authored "cell" meshes
// (boulder chunks or crystal shards, generators/rockcluster/rockcluster_cells_data.gen.h)
// into a single mesh, purely via position/scale/rotation math — no per-cell
// deformation. See memo.md's "Method 4: RockCluster" for the algorithm writeup.

// Boulder and Sharp share identical placement math — only the cell mesh pack
// differs. Ends with an (uncapped) plane clip at y=0, so the result is meant
// to sit in terrain, not be viewed from below.
Array rock_cluster_gen_boulder(
		bool p_sharp, // false = Boulder cell pack, true = Sharp cell pack
		int p_density,
		real_t p_radius,
		real_t p_asymmetry,
		real_t p_wave,
		real_t p_decentralize,
		real_t p_scale_local,
		const Ref<Curve> &p_scale_by_distance,
		real_t p_tallness,
		real_t p_flatness,
		real_t p_wideness,
		real_t p_rotation_degrees,
		real_t p_rotation_local_degrees,
		real_t p_rotation_rnd,
		int p_randseed);

// Grid-cluster placement with Perlin-noise-driven scale and a "bloom" param
// that blends each cell's orientation between "pointing away from center" and
// "pointing straight up". No plane clip (crystals aren't meant to embed flush
// in terrain the way boulders are).
// p_scale_local: the source algorithm has no absolute size control for
// Crystal at all — cell scale there is purely *relative* (distance falloff *
// noise), because the original relied on scaling its own parent scene node
// to fit a scene. Crystal cells are natively huge on their own (one sample
// cell's own bounding box is ~1x10.9x1) with nothing here to shrink that, so
// this multiplier was added — same field as Boulder/Sharp's, applied as a
// flat multiplier on top of Crystal's own relative scale math.
Array rock_cluster_gen_crystal(
		int p_density,
		real_t p_scale_local,
		real_t p_scale_by_angle,
		real_t p_scale_random_offset,
		real_t p_scale_bias,
		real_t p_bloom,
		int p_randseed);

// Testing-only: clips and caps ONE specific cell (by style/index, at the origin, identity
// rotation, unit scale) in isolation, returning its non-indexed VERTEX array. Lets a test
// check every individual cell's winding directly (rock_cluster_gen_boulder's own p_density
// loop always starts at cell index 0, so it can't reach the others in isolation).
// p_style: 0=Boulder, 1=Sharp, 2=Crystal (Crystal is never clipped -- returns the raw
// transformed cell instead, matching rock_cluster_gen_crystal's own no-clip behavior).
Array rock_cluster_debug_clip_single_cell(int p_style, int p_cell_index);

#endif // PROC_ROCKS_ROCKCLUSTER_H
