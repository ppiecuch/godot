/*
aa2map_write.h - map file writer for aa2map

Copyright (c) 2007 NoisyB


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "aa2map_config.h"
#include <stdio.h>
#include <string.h>
#include "aa2map_defines.h"
#include "aa2map.h"
#include "aa2map_misc.h"
#include "aa2map_parse.h"
#include "aa2map_write.h"
#include "idtech3/map.h"


void
aa2map_stairs (const st_aa2map_t *aa2map, const st_aa2map_parse_t *a)
{
//  int i = 0, j = 0, k = 0;
  st_map_brush_t base = {{IDTECH3_MAP_BLOCK}, "", 1, 1, 0, 0, 0};
  st_map_brush_t b = {
    {
      {{0.75, 0.75, 1}, {0.75, 0, 1 }, {0, 0.75, 1 }}, // top
      {{0.75, 0.75, 1}, {0, 0.75, 1 }, {1, 1, 0 }}, // east
      {{0.75, 0.75, 1}, {1, 1, 0 }, {0.75, 0, 1 }}, // south
      {{0, 0, 0}, {1, 0, 0 }, {0, 1, 0 }}, // bottom
      {{0, 0, 0}, {0.25, 0.25, 1 }, {1, 0, 0 }}, // west
      {{0, 0, 0}, {0, 1, 0 }, {0.25, 0.25, 1 }}  // north
    },
    "", 1, 1, 0, 0, 0};

  idtech3_map_scale (&base, a->xscale, a->yscale, a->zscale * 0.4);
  idtech3_map_trans (&base, a->x * a->xscale, a->y * a->yscale, a->z * a->zscale);
  sprintf (base.texture, "%s/%s", aa2map->path, a->name_s); 
  base.xsize = a->xscale;
  base.ysize = a->yscale; 
  base.zsize = a->zscale * 0.4;

  idtech3_map_brush6 (aa2map->map_file, &base); 

  idtech3_map_scale (&b, a->xscale, a->yscale, a->zscale * 0.25);
  idtech3_map_trans (&b, a->x * a->xscale, a->y * a->yscale, (a->z + 0.4) * a->zscale);
  sprintf (b.texture, "%s/%s", aa2map->path, a->name_s); 
  b.xsize = a->xscale;
  b.ysize = a->yscale; 
  b.zsize = a->zscale * 0.25;

  idtech3_map_brush6 (aa2map->map_file, &b);
}
