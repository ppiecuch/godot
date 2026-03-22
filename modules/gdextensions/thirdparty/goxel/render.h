/* Goxel 3D voxels editor
 *
 * copyright (c) 2019 Guillaume Chereau <guillaume@noctua-software.com>
 *
 * Goxel is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.

 * Goxel is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.

 * You should have received a copy of the GNU General Public License along with
 * goxel.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GOXEL_RENDER_H
#define GOXEL_RENDER_H

#include <stdint.h>
#include <stdbool.h>

enum {
    EFFECT_RENDER_POS       = 1 << 1,
    EFFECT_BORDERS          = 1 << 3,
    EFFECT_SEMI_TRANSPARENT = 1 << 5,
    EFFECT_SEE_BACK         = 1 << 6,
    EFFECT_MARCHING_CUBES   = 1 << 7,
    EFFECT_SHADOW_MAP       = 1 << 8,
    EFFECT_MC_SMOOTH        = 1 << 9,

    // For render box.
    EFFECT_NO_SHADING       = 1 << 10,
    EFFECT_STRIP            = 1 << 11,
    EFFECT_WIREFRAME        = 1 << 12,
    EFFECT_GRID             = 1 << 13,
    EFFECT_EDGES            = 1 << 14,
    EFFECT_GRID_ONLY        = 1 << 15,

    EFFECT_PROJ_SCREEN      = 1 << 16, // Image project in screen.
    EFFECT_ANTIALIASING     = 1 << 17,
    EFFECT_UNLIT            = 1 << 18,

    EFFECT_ARROW            = 1 << 19, // Add an arrow at the end of lines.
};

/* Render functions removed — Godot handles all rendering.
 * Only EFFECT flags above are needed for mesh generation. */

#endif // GOXEL_RENDER_H
