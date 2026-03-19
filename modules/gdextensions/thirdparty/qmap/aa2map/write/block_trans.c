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
#include "idtech3/shader.h"


void
aa2map_block_trans (const st_aa2map_t *aa2map, const st_aa2map_parse_t *a)
{
  st_map_brush_t b = {{IDTECH3_MAP_BLOCK}, "", 1, 1, 0, 0, 0};

  idtech3_map_scale (&b, a->xscale, a->yscale, a->zscale);
  idtech3_map_trans (&b, a->x * a->xscale, a->y * a->yscale, a->z * a->zscale);
  b.xsize = a->xscale;
  b.ysize = a->yscale; 
  b.zsize = a->zscale;
  sprintf (b.texture, "%s/%s", aa2map->path, a->name_s);

  idtech3_map_brush6 (aa2map->map_file, &b);
}


void
aa2map_block_trans_shader (const st_aa2map_t *aa2map, const st_aa2map_parse_t *a)
{
  (void) a;
  char name[MAXBUFSIZE];
  char s[MAXBUFSIZE];

  sprintf (name, "textures/%s/block_trans", aa2map->path);
  sprintf (s, "{\n"
              "surfaceparm nolightmap\n"
              "surfaceparm solid\n"
              "cull twosided\n"
              "{\n"
              "map textures/%s/block_trans.tga\n"
              "tcGen environment\n"
              "blendfunc GL_ONE GL_ONE\n"
              "}\n"
              "}", aa2map->path);

  idtech3_shader6 (aa2map->shader_file, name, s);
}
