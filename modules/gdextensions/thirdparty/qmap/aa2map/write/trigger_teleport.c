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
#include "block.h"


void
aa2map_entity_trigger_teleport (const st_aa2map_t *aa2map, const st_aa2map_parse_t *a)
{
  st_map_brush_t b = {{IDTECH3_MAP_BLOCK}, "", 1, 1, 0, 0, 0};
  fprintf (aa2map->map_file, "{\n"
                                "  \"classname\" \"trigger_teleport\"\n"
                                "  \"target\" \"t1\"\n");

  idtech3_map_scale (&b, a->xscale, a->yscale, 2.0);
  idtech3_map_trans (&b, a->x * a->xscale, a->y * a->yscale, a->z * a->zscale);
  b.xsize = a->xscale;
  b.ysize = a->yscale;
  b.zsize = 2.0;
  sprintf (b.texture, "%s/%s", aa2map->path, a->name_s);

  idtech3_map_brush6 (aa2map->map_file, &b);

  fprintf (aa2map->map_file, "}\n");
}


void
aa2map_entity_misc_teleporter_dest (const st_aa2map_t *aa2map, const st_aa2map_parse_t *a)
{
  fprintf (aa2map->map_file, "{\n"
//                                "  \"classname\" \"misc_teleporter_dest\"\n"
                                "  \"classname\" \"target_position\"\n"
                                "  \"origin\" \"%d %d %d\"\n"
                                "  \"targetname\" \"t1\"\n"
                                "}\n", (int) (a->x * a->xscale + a->xscale * 0.5),
                                       (int) (a->y * a->yscale + a->yscale * 0.5),
                                       (int) (a->z * a->zscale + a->zscale * 0.5));

}
