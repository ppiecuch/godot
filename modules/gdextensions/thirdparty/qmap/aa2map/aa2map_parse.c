/*
aa2map_parse.c - ASCII parser for aa2map

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
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "misc/defines.h"
#include "misc/file.h"
#include "aa2map_defines.h"
#include "aa2map.h"
#include "aa2map_misc.h"
#include "aa2map_parse.h"
#include "aa2map_write.h"


st_aa2map_parse_t *
aa2map_parse (st_aa2map_t *aa2map)
{
  unsigned int i = 0, j = 0;
  int xpos = 0, ypos = 0, zpos = 0;
  st_aa2map_parse_t *parsed = NULL;
  int pos = 0;
  char *p = NULL;
  unsigned int file_size = 0;
  unsigned int total_size = 0;
  FILE *fh = NULL;

  for (; aa2map->input_file[i]; i++)
    {
      if (access (aa2map->input_file[i], R_OK) != 0)
        {
          AA2MAP_LOG_ERROR ( "ERROR: failed to open input file with ASCII Art (%s)\n", aa2map->input_file[i]);
          continue;
        }

      file_size = fsizeof (aa2map->input_file[i]);
      total_size += file_size;
    }

  if (aa2map->mirror)
    total_size *= 2;

  p = malloc (total_size + 1);
  if (!p)
    {
      AA2MAP_LOG_ERROR ( "ERROR: aa2map_parse() {p = malloc()} failed\n");
      return NULL;
    }
  memset (p, 0, total_size + 1);

  if (!(parsed = malloc (sizeof (st_aa2map_parse_t) * (total_size + 1))))
    {
      AA2MAP_LOG_ERROR ( "ERROR: aa2map_parse() {parsed = malloc()} failed\n");
      free (p);
      return NULL;
    }
  memset (parsed, 0, sizeof (st_aa2map_parse_t) * (total_size + 1));

  for (i = 0; aa2map->input_file[i]; i++)
    {
      if (!(fh = fopen (aa2map->input_file[i], "r")))
        {
          AA2MAP_LOG_ERROR ( "ERROR: failed to open input file with ASCII Art (%s)\n", aa2map->input_file[i]);
          return NULL;
        }

      file_size = fsizeof (aa2map->input_file[i]);

      if (fread (p, 1, file_size, fh) < file_size)
        {
          AA2MAP_LOG_ERROR ( "ERROR: aa2map_parse() {fread()} failed\n");
          return NULL;
        }

      fclose (fh);

      for (j = 0; j < file_size; j++)
        {
          char c = p[j];

          if (c == '\n')
            {
              aa2map->xsize = MAX (aa2map->xsize, xpos);
              xpos = 0;
              ypos++;
            }
          else
            {
              st_aa2map_object_t *o = aa2map_get_object_by_ascii (aa2map, c);

              if (o)
                {
                  parsed[pos].id = o->id;
                  parsed[pos].name_s = o->name_s;
                  parsed[pos].xscale = aa2map->xscale;
                  parsed[pos].yscale = aa2map->yscale;
                  parsed[pos].zscale = aa2map->zscale;
                  parsed[pos].angle = 0;

                  parsed[pos].x = xpos;
                  parsed[pos].y = ypos;
                  parsed[pos].z = zpos;

                  pos++;
                }

              xpos++;
            }
        }

      aa2map->ysize = MAX (aa2map->ysize, ypos);
      ypos = 0;
      zpos++;
    }

  aa2map->zsize = MAX (aa2map->zsize, zpos);

  AA2MAP_LOG_INFO ("Map dimensions: %dx%dx%d\n", aa2map->xsize, aa2map->ysize, aa2map->zsize);

  // Mirror: duplicate all parsed elements and reflect along the specified
  // cardinal direction ('n'/'s' mirrors Y, 'e'/'w' mirrors X), doubling
  // the map size on the mirrored axis.
  if (aa2map->mirror)
    if (strchr ("nesw", aa2map->mirror))
    {
      j = pos;
      for (i = 0; i < j; i++)
        {
          parsed[pos].id = parsed[i].id;
          parsed[pos].name_s = parsed[i].name_s;
          parsed[pos].xscale = parsed[i].xscale;
          parsed[pos].yscale = parsed[i].yscale;
          parsed[pos].zscale = parsed[i].zscale;
          parsed[pos].angle = parsed[i].angle;

          parsed[pos].x = parsed[i].x;
          parsed[pos].y = parsed[i].y;
          parsed[pos].z = parsed[i].z;

          switch (aa2map->mirror)
            {
              case 'n':
                parsed[pos].y = (aa2map->ysize - 1) - parsed[pos].y;
                parsed[i].y = aa2map->ysize + parsed[i].y;
                break;
              case 's':
                parsed[i].y = (aa2map->ysize * 2 - 1) - parsed[i].y;
                break;
              case 'w':
                parsed[pos].x = (aa2map->xsize - 1) - parsed[pos].x;
                parsed[i].x = aa2map->xsize + parsed[i].x;
                break;
              case 'e':
                parsed[i].x = (aa2map->xsize * 2 - 1) - parsed[i].x;
                break;
            }

          pos++;
        }

      if (strchr ("ns", aa2map->mirror))
        aa2map->ysize *= 2;
      else if (strchr ("we", aa2map->mirror))
        aa2map->xsize *= 2;
    }

  // Flip: by default X is mirrored (ASCII left-to-right → map right-to-left).
  // --hflip disables the default X mirror. --vflip enables Y mirror.
  for (i = 0; parsed[i].id; i++)
    {
      if (!aa2map->hflip)
        parsed[i].x = (aa2map->xsize - 1) - parsed[i].x;

      if (aa2map->vflip)
        parsed[i].y = (aa2map->ysize - 1) - parsed[i].y;
    }

  if (p)
    free (p);

  return parsed;
}


// Randomly scatter ASCII characters from aa2map->add_ascii onto floor tiles.
// Each character in add_ascii is looked up via aa2map_get_object_by_ascii
// and placed at a random floor tile position (replacing the floor with the
// new object type). Characters are distributed round-robin across floors.
void
aa2map_add_ascii (st_aa2map_t *aa2map, st_aa2map_parse_t *a)
{
  int add_len = 0;
  int floor_count = 0;
  int i = 0;

  if (!aa2map->add_ascii)
    return;

  add_len = strlen (aa2map->add_ascii);
  if (add_len == 0)
    return;

  // Count floor tiles
  for (i = 0; a[i].id; i++)
    if (a[i].id == AA2MAP_FLOOR)
      floor_count++;

  if (floor_count == 0)
    return;

  // Scatter each add_ascii character onto a random floor tile
  for (i = 0; i < add_len; i++)
    {
      st_aa2map_object_t *o = aa2map_get_object_by_ascii (aa2map, aa2map->add_ascii[i]);
      if (!o)
        continue;

      // Pick a random floor tile
      int target = RANDOM (0, floor_count - 1);
      int floor_idx = 0;
      int j;

      for (j = 0; a[j].id; j++)
        {
          if (a[j].id == AA2MAP_FLOOR)
            {
              if (floor_idx == target)
                {
                  a[j].id = o->id;
                  a[j].name_s = o->name_s;
                  floor_count--;
                  break;
                }
              floor_idx++;
            }
        }

      if (floor_count == 0)
        break;
    }
}

