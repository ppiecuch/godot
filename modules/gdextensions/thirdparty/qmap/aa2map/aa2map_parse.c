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
//  unsigned int x = 0, y = 0;
  st_aa2map_parse_t *parsed = NULL;
  int pos = 0;
  char *p = NULL;
  unsigned int file_size = 0;
  unsigned int total_size = 0;
  FILE *fh = NULL;

  // get total_size
  for (; aa2map->input_file[i]; i++)
    {
      if (access (aa2map->input_file[i], R_OK) != 0)
        {
          fprintf (stderr, "ERROR: failed to open input file with ASCII Art (%s)\n", aa2map->input_file[i]);
          continue;
        }

      file_size = fsizeof (aa2map->input_file[i]);
#if 0
      if (file_size < 11) // 9x9 + 2 line feed
        {
          fprintf (stderr, "ERROR: The map is too small (at least 9 x 9 plus 2 line feed)\n"
                           "EXAMPLE: ***(+\\n)\n"
                           "         * *(+\\n)\n"
                           "         ***\n");
        }
#endif
      total_size += file_size;
    }

  if (aa2map->mirror) // when the map is supposed to be mirror'd we need the double
    total_size *= 2;

  // malloc parse buffer
  p = malloc (total_size + 1);
  if (!p)
    {
      fprintf (stderr, "ERROR: aa2map_parse() {p = malloc()} failed\n");
      return NULL;
    }
  memset (p, 0, total_size + 1);

  // malloc st_aa2map_parse_t array
  if (parsed)
    free (parsed);

  if (!(parsed = malloc (sizeof (st_aa2map_parse_t) * (total_size + 1))))
    {
      fprintf (stderr, "ERROR: aa2map_parse() {parsed = malloc()} failed\n");
      free (p);
      return NULL;
    }
  memset (parsed, 0, sizeof (st_aa2map_parse_t) * (total_size + 1));

  // parse
  for (i = 0; aa2map->input_file[i]; i++)
    {
      if (!(fh = fopen (aa2map->input_file[i], "r")))
        {
          fprintf (stderr, "ERROR: failed to open input file with ASCII Art (%s)\n", aa2map->input_file[i]);
          return NULL;
        }

      file_size = fsizeof (aa2map->input_file[i]);

      if (fread (p, 1, file_size, fh) < file_size)
        {
          fprintf (stderr, "ERROR: aa2map_parse() {fread()} failed\n");
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
              st_aa2map_object_t *o = NULL;

              o = aa2map_get_object_by_ascii (aa2map, c);

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

  fprintf (stderr, "Map dimensions: %dx%dx%d\n", aa2map->xsize, aa2map->ysize, aa2map->zsize);

  //mirror
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

  // flip
  for (i = 0; parsed[i].id; i++)
    {
      if (aa2map->hflip == 1)
        {
        }
      else
        parsed[i].x = (aa2map->xsize - 1) - parsed[i].x;

      if (aa2map->vflip == 1)
        parsed[i].y = (aa2map->ysize - 1) - parsed[i].y;
    }

#if 0
  // cube
  for (i = 0; i < pos; i++)
    for (j = 0; j < pos; j++)
      {
        if (parsed[i].z == parsed[j].z - 1) // top
          {
            if (parsed[i].x == parsed[j].x - 1) // 0 - 2
              parsed[i].
          }
        else if (parsed[i].z == parsed[j].z) // center
          {
          }
        else if (parsed[i].z == parsed[j].z + 1) // bottom
          {
          }
      }
#endif

  if (p)
    free (p);

  return parsed;
}


#if 0
static void
aa2map_add_ascii_func (st_aa2map_parse_t *a)
{
}
#endif


void
aa2map_add_ascii (st_aa2map_t *aa2map, st_aa2map_parse_t *a)
{
  (void) aa2map;
  (void) a;
#if 0
  int i = 0;
  int floors = 0;
  int add_ascii_len = strlen (aa2map->add_ascii);
  int pos = 0;   

  for (; a[i].id; i++)
    if (a[i].id == AA2MAP_FLOOR)
      floors++;

  if (floors > add_ascii_len)
    for (i = 0; i < add_ascii_len; i++)
      if (a[i].id == AA2MAP_FLOOR)
        {
        }
#endif
}
