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
#include "misc/defines.h"
#include "aa2map_defines.h"
#include "aa2map.h"
#include "aa2map_misc.h"
#include "aa2map_parse.h"
#include "aa2map_write.h"
#include "idtech3/map.h"
#include "idtech3/shader.h"


void
aa2map_world (const st_aa2map_t *aa2map)
{
  int xsize = 0, ysize = 0, zsize = 0;
  st_map_brush_t b = {{IDTECH3_MAP_BLOCK}, "", 1, 1, 0, 0, 0};
  st_map_brush_t bak = {{IDTECH3_MAP_BLOCK}, "", 1, 1, 0, 0, 0};

  xsize = aa2map->xsize * aa2map->xscale;
  ysize = aa2map->ysize * aa2map->yscale;
  zsize = (aa2map->zsize + (512 / MAX (aa2map->zscale, 1.0)) + 1) * aa2map->zscale; // + 512 for rocketjumps (391 units) without bumping the head

  sprintf (b.texture, "%s/world_t", aa2map->path);
  b.xsize = xsize;
  b.ysize = ysize;
  b.zsize = 1.0;
  idtech3_map_scale (&b, xsize, ysize, 1.0);
  idtech3_map_trans (&b, 0.0, 0.0, zsize);
  idtech3_map_brush6 (aa2map->map_file, &b);

  memcpy (&b, &bak, sizeof (st_map_brush_t));
  sprintf (b.texture, "%s/world_s", aa2map->path);
  b.xsize = xsize;
  b.ysize = 1.0;
  b.zsize = zsize;  
  idtech3_map_scale (&b, xsize, 1.0, zsize);      
  idtech3_map_trans (&b, 0.0, ysize, 0.0);      
  idtech3_map_brush6 (aa2map->map_file, &b);      

  memcpy (&b, &bak, sizeof (st_map_brush_t));
  sprintf (b.texture, "%s/world_w", aa2map->path);
  b.xsize = 1.0;
  b.ysize = ysize;
  b.zsize = zsize;
  idtech3_map_scale (&b, 1.0, ysize, zsize);
  idtech3_map_trans (&b, xsize, 0.0, 0.0);
  idtech3_map_brush6 (aa2map->map_file, &b);

  memcpy (&b, &bak, sizeof (st_map_brush_t));
  sprintf (b.texture, "%s/world_b", aa2map->path);
  b.xsize = xsize;
  b.ysize = ysize;
  b.zsize = 1.0;
  idtech3_map_scale (&b, xsize, ysize, 1.0);
  idtech3_map_trans (&b, 0.0, 0.0, -1.0);
  idtech3_map_brush6 (aa2map->map_file, &b);      

  memcpy (&b, &bak, sizeof (st_map_brush_t));
  sprintf (b.texture, "%s/world_n", aa2map->path);
  b.xsize = xsize;
  b.ysize = 1.0;
  b.zsize = zsize;
  idtech3_map_scale (&b, xsize, 1.0, zsize);      
  idtech3_map_trans (&b, 0.0, -1.0, 0.0);
  idtech3_map_brush6 (aa2map->map_file, &b);      

  memcpy (&b, &bak, sizeof (st_map_brush_t));
  sprintf (b.texture, "%s/world_e", aa2map->path);
  b.xsize = 1.0;
  b.ysize = ysize;
  b.zsize = zsize;
  idtech3_map_scale (&b, 1.0, ysize, zsize);      
  idtech3_map_trans (&b, -1.0, 0.0, 0.0);
  idtech3_map_brush6 (aa2map->map_file, &b);      
}


void
aa2map_world_shader (const st_aa2map_t *aa2map)
{
  char name[MAXBUFSIZE];
  char s[MAXBUFSIZE];
  char *f = "{\n"
            "{\n"
            "map textures/%s/%s.jpg\n"
            "}\n"
            "}";

  sprintf (name, "textures/%s/world_t_b", aa2map->path);
  sprintf (s, f, aa2map->path, "world_t_b");
  idtech3_shader (aa2map->shader_file, name, s);
  sprintf (name, "textures/%s/world_e_w", aa2map->path);
  sprintf (s, f, aa2map->path, "world_e_w");
  idtech3_shader (aa2map->shader_file, name, s);
  sprintf (name, "textures/%s/world_s_n", aa2map->path);
  sprintf (s, f, aa2map->path, "world_s_n");
  idtech3_shader (aa2map->shader_file, name, s);
  sprintf (name, "textures/%s/world_b_t", aa2map->path);
  sprintf (s, f, aa2map->path, "world_b_t");
  idtech3_shader (aa2map->shader_file, name, s);
  sprintf (name, "textures/%s/world_w_e", aa2map->path);
  sprintf (s, f, aa2map->path, "world_w_e");
  idtech3_shader (aa2map->shader_file, name, s);
  sprintf (name, "textures/%s/world_n_s", aa2map->path);
  sprintf (s, f, aa2map->path, "world_n_s");
  idtech3_shader (aa2map->shader_file, name, s);
}
