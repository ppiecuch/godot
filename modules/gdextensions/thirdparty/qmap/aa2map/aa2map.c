/*
aa2map->c - ASCII Art to (id Tech 3/Quake3:Arena) map converter

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
#include <stdbool.h>
#include <unistd.h>  // access()
#include "misc/defines.h"
#include "misc/file.h"
#include "misc/property.h"
#include "misc/string.h"
#include "aa2map_defines.h"
#include "aa2map.h"
#include "aa2map_misc.h"
#include "aa2map_parse.h"
#include "aa2map_write.h"

void
aa2map_pre_exit (st_aa2map_t *aa2map)
{
  if (aa2map->map_file)
    if (aa2map->map_file != stdout)
      {
        fclose (aa2map->map_file);
        aa2map->map_file = NULL;
      }

  if (aa2map->shader_file)
    if (aa2map->shader_file != stdout)
      { 
        fclose (aa2map->shader_file);
        aa2map->shader_file = NULL;
      }

  if (aa2map->compile)
    if (aa2map->type == AA2MAP_IDTECH2 ||
        aa2map->type == AA2MAP_IDTECH3)
      aa2map_compile (aa2map);
}


void  
aa2map_exit (st_aa2map_t *aa2map)
{
  if (aa2map->map_file)
    if (aa2map->map_file != stdout)
      {
        fclose (aa2map->map_file);
        aa2map->map_file = NULL;
      }

  if (aa2map->shader_file)
    if (aa2map->shader_file != stdout)
      { 
        fclose (aa2map->shader_file);
        aa2map->shader_file = NULL;
      }
}


int
aa2map_set_opt (st_aa2map_t *aa2map, int c, const char *optarg)
{
  const char *p = NULL;
  switch (c)
    {
      case AA2MAP_VER:
          printf ("aa2map version: %s\n", AA2MAP_VERSION_S);
          break;

      case AA2MAP_IDTECH2:
      case AA2MAP_IDTECH3:
      case AA2MAP_IDTECH4:
          aa2map->type = AA2MAP_IDTECH3;
          break;

      case AA2MAP_PATH:
          p = optarg;
          if (p)
          strncpy (aa2map->path, p, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
          break;

      case AA2MAP_STRAFE:
          p = optarg;
          if (p)
          sscanf (p, "%d:%d:%d:%d", &aa2map->pads, &aa2map->gap_initial, &aa2map->gap_increment, &aa2map->height_increment);
          aa2map->strafe = 1;
          break;

      case AA2MAP_RUN:
          p = optarg;
          if (p)
          sscanf (p, "%d:%d", &aa2map->run_len, &aa2map->checkpoints);
          aa2map->run = 1;
          break;

      case AA2MAP_MUSEUM:
          p = optarg;
          if (p)
          aa2map->museum_path = p;
          break;

      case AA2MAP_MAZE:
          p = optarg;
          if (p)
          sscanf (p, "%dx%d", &aa2map->xmaze, &aa2map->ymaze);
          aa2map->maze = 1;
          break;

      case AA2MAP_ADD:
          p = optarg;
          if (p)
          aa2map->add_ascii = p;
          break;

      case AA2MAP_SCALE:
          p = optarg;
          if (p)
          sscanf (p, "%dx%dx%d", &aa2map->xscale, &aa2map->yscale, &aa2map->zscale);
          break;

      case AA2MAP_ROT:
          p = optarg;
          if (p)
          sscanf (p, "%d:%d", &aa2map->xrot, &aa2map->yrot);
          break;

      case AA2MAP_MIRROR:
          p = optarg;
          if (p)
          if (strchr ("nesw", *p))
              aa2map->mirror = *p;
          break;

      case AA2MAP_HFLIP:
          aa2map->hflip = 1;
          break;

      case AA2MAP_VFLIP:
          aa2map->vflip = 1;
          break;

      case AA2MAP_COMPILE:
          aa2map->compile = 1;
          break;

      case AA2MAP_PACK:
          aa2map->pack = 1;
          break;

      case AA2MAP_MESSAGE:
          p = optarg;
          if (p)
          strncpy (aa2map->message, p, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;
          break;

      case AA2MAP_GRAVITY:
          p = optarg;
          if (p)
          aa2map->gravity = strtol (p, NULL, 10);
          break;

      case AA2MAP_LIGHT:
          p = optarg;
          if (p)
          aa2map->light = strtol (p, NULL, 10);
          break;

      case AA2MAP_FOG:
          p = optarg;
          if (p)
          aa2map->fog = strtol (p, NULL, 16);
          break;

      case AA2MAP_O:
          p = optarg;
          if (p)
          {
              strncpy (aa2map->map_file_s, p, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
              set_suffix (aa2map->shader_file_s, ".map");
              aa2map->map_file = NULL;
              aa2map->map_file = fopen (aa2map->map_file_s, "w");
              if (!aa2map->map_file)
              {
                  fprintf (stderr, "ERROR: could not open output file %s (using stdout)\n", aa2map->map_file_s);
                  aa2map->map_file = stdout;
              }

              strncpy (aa2map->shader_file_s, p, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
              set_suffix (aa2map->shader_file_s, ".shader");
              aa2map->shader_file = NULL;
              aa2map->shader_file = fopen (aa2map->shader_file_s, "w");
              if (!aa2map->shader_file)
              {
                  fprintf (stderr, "ERROR: could not open output file %s (using stdout)\n", aa2map->shader_file_s);
                  aa2map->shader_file = stdout;
              }
          }
          break;

      default:
          fputs ("Command unknown", stdout);
          return -1;
    }
  return 0;
}

int
aa2map_gen (st_aa2map_t *aa2map)
{
  st_aa2map_parse_t *parsed = NULL;
  int result = 0;
  char buf[MAXBUFSIZE];
  const char *p = NULL;
  const st_property_t props[] =
    {
      {
        "ascii_chars", AA2MAP_DEFAULT_ASCII_CHARS_S,
        "configurable definition of ASCII art usage/translation"
      },
      {
        "q2map_cmd", "q2map.x86",
        "command for id Tech 2 map to bsp compiler"
      },
      {
        "q3map_cmd", "q3map2.x86 -v -meta -fs_basepath ~/.q3a/baseq3 -game quake3 %s",
        "command for id Tech 3 map to bsp compiler"
      },
      {
        "bsp2aas_cmd", "bspc -forcesidesvisible -optimize -bsp2aas %s",
        "command for id Tech 3 bsp to aas compiler"
      },
      {NULL, NULL, NULL}
    };

  memset (aa2map, 0, sizeof (st_aa2map_t));

  realpath2 (PROPERTY_HOME_RC ("aa2map"), aa2map->configfile);

  result = property_check (aa2map->configfile, AA2MAP_CONFIG_VERSION, 1);
  if (result == 1) // update needed
    result = set_property_array (aa2map->configfile, props);
  if (result == -1) // property_check() or update failed
    { 
      fprintf (stderr, "ERROR: could not create %s\n", aa2map->configfile);
      fprintf (stderr, "       using default: %s", AA2MAP_DEFAULT_ASCII_CHARS_S);
    }

  srand (time_ms (0)); // seed random
  aa2map->map_file = stdout;
  aa2map->shader_file = stdout;
  aa2map->xscale = aa2map->yscale = aa2map->zscale = 128; // default ASCII block size
  aa2map->type = AA2MAP_IDTECH3;
  // default path used in/for texture, map and shader
  strcpy (aa2map->path, AA2MAP_PATH_DEFAULT);
  // sprintf (aa2map->path, "%08x", RANDOM (0x10000000, 0xffffffff)); // TODO: recalculate per generation (if more than one map at once is supported)
  // maze defaults
  aa2map->xmaze = 10;
  aa2map->ymaze = 20;
  // strafe defaults
  aa2map->pads = 10;
  aa2map->gap_initial = 220;
  aa2map->gap_increment = 40;
  // run defaults
  aa2map->run_len = 20000;
  aa2map->checkpoints = 6;
  // heightmap defaults
  aa2map->heightmap_only = 0;
  p = get_property (aa2map->configfile, "ascii_chars", PROPERTY_MODE_TEXT);
  if (p)
    strncpy (aa2map->ascii_chars, p, AA2MAP_MAX_ASCII_CHARS)[AA2MAP_MAX_ASCII_CHARS - 1] = 0;
  p = get_property (aa2map->configfile, "q3map_cmd", PROPERTY_MODE_TEXT);
  if (p)
    strncpy (aa2map->q3map_cmd, p, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
  p = get_property (aa2map->configfile, "bsp2aas_cmd", PROPERTY_MODE_TEXT);
  if (p)
    strncpy (aa2map->bsp2aas_cmd, p, FILENAME_MAX)[FILENAME_MAX - 1] = 0;

  if (aa2map->maze)
    {
      aa2map_mazegen (aa2map, aa2map->xmaze, aa2map->ymaze); // writes ASCII file instead of .map file
      return 0;
    }

  if (aa2map->strafe)
    {
      aa2map_strafe (aa2map, aa2map->pads, aa2map->gap_initial, aa2map->gap_increment, aa2map->height_increment);
      aa2map_pre_exit (aa2map);
      return 0;
    }

  if (aa2map->run)
    {
      aa2map_run (aa2map, aa2map->run_len, aa2map->checkpoints);
      aa2map_pre_exit (aa2map);
      return 0;
    }

  if (aa2map->museum_path)
    {
      aa2map_museum (aa2map, aa2map->museum_path);
      aa2map_pre_exit (aa2map);
      return 0;
    }

  if (!aa2map->input_file[0])
    {
      fprintf (stderr, "ERROR: no ASCIIFILE specified\n");
      return -1;
    }

  if (!aa2map->message[0])
    {
      strncpy (buf, aa2map->input_file[0], FILENAME_MAX)[FILENAME_MAX - 1] = 0;
      set_suffix (buf, "");
      sprintf (aa2map->message, "%s created with aa2map " AA2MAP_VERSION_S, (char *) basename2 (buf));
    }

  parsed = aa2map_parse (aa2map);

  if (!parsed)
    {
      fprintf (stderr, "ERROR: aa2map_parse() failed\n");
      return -1;
    }

  if (aa2map->add_ascii)
    aa2map_add_ascii (aa2map, parsed);

  aa2map_map_write (aa2map, parsed);

  aa2map_shader_write (aa2map, parsed);

  if (parsed)
    free (parsed);

  aa2map_pre_exit (aa2map);

  return 0;
}
