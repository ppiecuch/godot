/*
aa2map.h - ASCII Art to (id Tech 3/Quake3:Arena) map converter

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

#ifndef  AA2MAP_H
#define  AA2MAP_H

#include <stdio.h>

typedef struct
{
  const char *input_file[ARGS_MAX];

  char path[FILENAME_MAX];

  char map_file_s[FILENAME_MAX];
  FILE *map_file;
  char shader_file_s[FILENAME_MAX];
  FILE *shader_file;

  char configfile[FILENAME_MAX];
  char q3map_cmd[FILENAME_MAX];
  char bsp2aas_cmd[FILENAME_MAX];
  char ascii_chars[AA2MAP_MAX_ASCII_CHARS];  // from config

  int type;  // AA2MAP_IDTECH2, AA2MAP_IDTECH3, AA2MAP_IDTECH4, etc
  int compile;
  int pack;

  char message[MAXBUFSIZE];
  int gravity; // gravitation in map
  unsigned long fog; // fog in map 0xRRGGBBAA
  int light; // set light type 0 == ambient, 1 == random, 2 == sun

  int xscale; // default 128
  int yscale; // default 128
  int zscale; // default 128

  int xrot; // default 0
  int yrot; // default 0

  int mirror; // 'n', 'e', 's' or 'w'
  int hflip;
  int vflip;
  const char *add_ascii; // add new ASCII randomly to map

  // --strafe
  int strafe; // DO generate strafe pad map
  int pads;
  int gap_initial;
  int gap_increment;
  int height_increment;

  // --run
  int run;
  int run_len; // has to be !0
  int checkpoints;

  // --maze
  int maze; // DO generate maze
  int xmaze; // xy size of maze
  int ymaze;

  // --museum
  const char *museum_path; // path to pk3 or files with models and textures
                           // if (museum_path != NULL) DO generate museum

  // --heightmap
  int heightmap_only;
  const char *megatexture;

  int xsize; // total xsize of map (in blocks)
  int ysize; // total ysize of map (in blocks)
  int zsize; // total zsize of map (in blocks)
} st_aa2map_t;

extern int aa2map_set_opt (st_aa2map_t *aa2map, int c, const char *optarg);
extern int aa2map_gen (st_aa2map_t *aa2map);

#endif  // AA2MAP_H

