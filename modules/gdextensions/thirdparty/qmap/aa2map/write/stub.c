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
#include "aa2map_defines.h"
#include "aa2map.h"
#include "aa2map_misc.h"
#include "aa2map_parse.h"
#include "aa2map_write.h"
#include "floor.h"
#include "idtech3/map.h"
#include "idtech3/shader.h"

#include <stdio.h>
#include <string.h>


void
aa2map_stub (const st_aa2map_t *aa2map, const st_aa2map_parse_t *a)
{
  if (a->z == 0)
    aa2map_floor (aa2map, a);
}


void
aa2map_entity_stub (const st_aa2map_t *aa2map, const st_aa2map_parse_t *a)
{
//  const char *icon_s = idtech3_entity_to_icon (a->name_s);

//  if (icon_s) // IS an entity
    fprintf (aa2map->map_file, "{\n"
                               "  \"classname\" \"%s\"\n"
                               "  \"origin\" \"%d %d %d\"\n"
                               "  \"angle\" \"%d\"\n"
                               "}\n", 
                               a->name_s,
                               (int) (a->x * a->xscale + a->xscale * 0.5),
                               (int) (a->y * a->yscale + a->yscale * 0.5),
                               (int) (a->z * a->zscale + a->zscale * 0.5),
                               a->angle - 90);
}


void
aa2map_shader_stub (const st_aa2map_t *aa2map, const st_aa2map_parse_t *a)
{
  int i = 0;
  const char *suffix_s[6] = {"_t", "_s", "_e", "_b", "_n", "_w"};
  char name[256];
  char s[1024];
  const char *icon_s = idtech3_entity_to_icon (a->name_s);

  for (i = 0; i < 6; i++)
    {
      sprintf (name, "textures/%s/%s%s", aa2map->path, a->name_s, suffix_s[i]);
      sprintf (s, "{\n"
                  "{\n"
                  "map textures/%s/%s%s.jpg\n"  
                  "}\n"
                  "}", aa2map->path, icon_s ? "floor" : a->name_s, suffix_s[i]);
      idtech3_shader (aa2map->shader_file, name, s);
    }
}


