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
#include "floor.h"
#include "block.h"


void
aa2map_entity_trigger_push (const st_aa2map_t *aa2map, const st_aa2map_parse_t *a)
{
  st_map_brush_t b = {{IDTECH3_MAP_BLOCK}, "", 1, 1, 0, 0, 0};
  fprintf (aa2map->map_file, "{\n"
                             "  \"classname\" \"trigger_push\"\n"
//                             "  \"origin\" \"%d %d %d\"\n"
                             "  \"speed\" \"160\"\n"
                             "  \"angle\" \"-1\"\n"  // Direction player is pushed towards (-1=up -2=down, other=normal)
                             "  \"target\" \"j1\"\n"//,
//                             (int) (a->x * a->xscale + a->xscale * 0.5),
//                             (int) (a->y * a->yscale + a->yscale * 0.5), 
//                             (int) (a->z * a->zscale + a->zscale * 0.5)
);

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
aa2map_entity_target_position (const st_aa2map_t *aa2map, const st_aa2map_parse_t *a)
{
  fprintf (aa2map->map_file, "{\n"
                                "  \"classname\" \"target_position\"\n"
                                "  \"origin\" \"%d %d %d\"\n"
                                "  \"targetname\" \"j1\"\n"
                                "}\n", (int) (a->x * a->xscale + a->xscale * 0.5),
                                       (int) (a->y * a->yscale + a->yscale * 0.5),
                                       (int) (a->z * a->zscale + 3.0));

}

#if 0
// entity 0
{
"classname" "worldspawn"
// brush 0
{
( 64 60 8 ) ( 64 0 8 ) ( 0 60 8 ) common/oldstone2 0 0 0 0.5 0.5 0 0 0
( 64 64 128 ) ( 0 64 128 ) ( 64 64 0 ) common/oldstone2 0 0 0 0.5 0.5 0 0 0
( 64 60 128 ) ( 64 60 0 ) ( 64 0 128 ) common/oldstone2 0 0 0 0.5 0.5 0 0 0
( 0 0 0 ) ( 64 0 0 ) ( 0 60 0 ) common/oldstone2 0 0 0 0.5 0.5 0 0 0
( 0 0 0 ) ( 0 0 128 ) ( 64 0 0 ) common/oldstone2 0 0 0 0.5 0.5 0 0 0
( 0 0 0 ) ( 0 60 0 ) ( 0 0 128 ) common/oldstone2 0 0 0 0.5 0.5 0 0 0
}
}
// entity 1
{
"classname" "trigger_push"
"target" "target_position1"
// brush 0
{
( 64 64 10 ) ( 64 0 10 ) ( 0 64 10 ) common/caulk 0 0 0 0.5 0.5 0 0 0
( 64 64 16 ) ( 0 64 16 ) ( 64 64 8 ) common/caulk 0 0 0 0.5 0.5 0 0 0
( 64 64 16 ) ( 64 64 8 ) ( 64 0 16 ) common/caulk 0 0 0 0.5 0.5 0 0 0
( 0 0 8 ) ( 64 0 8 ) ( 0 64 8 ) common/caulk 0 0 0 0.5 0.5 0 0 0
( 0 0 8 ) ( 0 0 16 ) ( 64 0 8 ) common/caulk 0 0 0 0.5 0.5 0 0 0
( 0 0 8 ) ( 0 64 8 ) ( 0 0 16 ) common/caulk 0 0 0 0.5 0.5 0 0 0
}
}
// entity 2
{
"classname" "target_position"
"origin" "32 32 128"
"targetname" "target_position1"
}
#endif
