/*
aa2map_misc.c - miscellaneous functions for aa2map

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
#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#ifdef USE_ZLIB
#include "misc/unzip2.h"
#endif
#include "misc/string.h"
#include "misc/file.h"
#include "aa2map_defines.h"
#include "aa2map.h"
#include "aa2map_misc.h"
#include "aa2map_parse.h"
#include "aa2map_write.h"
#include "idtech3/map.h"
#include "idtech3/shader.h"
#include "write/world.h"

/* returns milliseconds since midnight */
unsigned long
time_ms (unsigned long *ms)
{
  unsigned long t = 0;
  struct timeval tv;

  if (!gettimeofday (&tv, NULL))
    {
      t = (unsigned long) (tv.tv_usec / 1000);
      t += (unsigned long) ((tv.tv_sec % 86400) * 1000);
    }

  return ms ? *ms = t : t;
}


/*
  '-' horizontal wall
  '|' vertical wall
  '+' a doorway (state is specified in a DOOR declaration)
  'A' open air
  'B' boundary room location (for bounding unwalled irregular regions)
  'C' cloudy air
  'I' ice
  'S' a secret door
  'H' a secret corridor
  '{' a fountain
  '\' a throne
  'K' a sink (if SINKS is defined, else a room location)
  '}' a part of a moat or other deep water
  'P' a pool
  'L' lava
  'W' water (yes, different from a pool)
  'T' a tree
  'F' iron bars
  '#' a corridor
  '.' a normal room location (unlit unless lit in a REGION declaration)
  ' ' stone
*/


st_aa2map_object_t aa2map_object[] = {
  // spawn
  // {3,   AA2MAP_INFO_PLAYER_START,      "info_player_start"},
  {4,   AA2MAP_INFO_PLAYER_DEATHMATCH, "info_player_deathmatch"},

  // weapons
  {5,   AA2MAP_WEAPON_GAUNTLET,        "weapon_gauntlet"}, 
  {6,   AA2MAP_WEAPON_MACHINEGUN,      "weapon_machinegun"}, 
  {7,   AA2MAP_WEAPON_SHOTGUN,         "weapon_shotgun"}, 
  {8,   AA2MAP_WEAPON_GRENADELAUNCHER, "weapon_grenadelauncher"},
  {9,   AA2MAP_WEAPON_ROCKETLAUNCHER,  "weapon_rocketlauncher"},
  {10,  AA2MAP_WEAPON_LIGHTNING,       "weapon_lightning"}, 
  {11,  AA2MAP_WEAPON_RAILGUN,         "weapon_railgun"}, 
  {12,  AA2MAP_WEAPON_PLASMAGUN,       "weapon_plasmagun"}, 
  {13,  AA2MAP_WEAPON_BFG,             "weapon_bfg"},
  {14,  AA2MAP_WEAPON_GRAPPLINGHOOK,   "weapon_grapplinghook"},
  // weapons Quake III: Team Arena (missionpack)
  //  {15,  AA2MAP_WEAPON_NAILGUN,         "weapon_nailgun"},
  //  {16,  AA2MAP_WEAPON_PROX_LAUNCHER,   "weapon_prox_launcher"},
  //  {17,  AA2MAP_WEAPON_CHAINGUN,        "weapon_chaingun"},

  // ammo
  {18,  AA2MAP_AMMO_BULLETS,           "ammo_bullets"},
  {19,  AA2MAP_AMMO_SHELLS,            "ammo_shells"},
  {20,  AA2MAP_AMMO_GRENADES,          "ammo_grenades"},
  {21,  AA2MAP_AMMO_ROCKETS,           "ammo_rockets"},
  {22,  AA2MAP_AMMO_LIGHTNING,         "ammo_lightning"},
  {23,  AA2MAP_AMMO_SLUGS,             "ammo_slugs"},
  {24,  AA2MAP_AMMO_CELLS,             "ammo_cells"},
  {25,  AA2MAP_AMMO_BFG,               "ammo_bfg"},

  // holdable
  {26,  AA2MAP_HOLDABLE_MEDKIT,        "holdable_medkit"},
  {27,  AA2MAP_HOLDABLE_TELEPORTER,    "holdable_teleporter"},

  // items
  {28,  AA2MAP_ITEM_ARMOR_BODY,        "item_armor_body"},
  {29,  AA2MAP_ITEM_ARMOR_COMBAT,      "item_armor_combat"},
  {30,  AA2MAP_ITEM_ARMOR_SHARD,       "item_armor_shard"},
  {31,  AA2MAP_ITEM_ENVIRO,            "item_enviro"},
  {32,  AA2MAP_ITEM_FLIGHT,            "item_flight"},
  {33,  AA2MAP_ITEM_HASTE,             "item_haste"},

  {34,  AA2MAP_ITEM_HEALTH,            "item_health"},
  {35,  AA2MAP_ITEM_HEALTH_LARGE,      "item_health_large"},
  {36,  AA2MAP_ITEM_HEALTH_MEGA,       "item_health_mega"},
  {37,  AA2MAP_ITEM_HEALTH_SMALL,      "item_health_small"},

  {38,  AA2MAP_ITEM_INVIS,             "item_invis"},
  {39,  AA2MAP_ITEM_QUAD,              "item_quad"},
  {40,  AA2MAP_ITEM_REGEN,             "item_regen"},

  // team
  {41,  AA2MAP_TEAM_CTF_BLUEFLAG,      "team_CTF_blueflag"},
  {42,  AA2MAP_TEAM_CTF_REDFLAG,       "team_CTF_redflag"},
  {43,  AA2MAP_CTF_NEUTRALFLAG,        "team_CTF_neutralflag"},

  // shooter
  //  {44,  AA2MAP_SHOOTER_GRENADE,        "shooter_grenade"},
  //  {45,  AA2MAP_SHOOTER_PLASMA,         "shooter_plasma"},
  //  {46,  AA2MAP_SHOOTER_ROCKET,         "shooter_rocket"},

  // teleports
  {47,  AA2MAP_TRIGGER_TELEPORT,       "trigger_teleport"},
  {48,  AA2MAP_MISC_TELEPORT_DEST,     "misc_teleporter_dest"},

  // jumppad
  {49,  AA2MAP_TRIGGER_PUSH,           "trigger_push"},
  {50,  AA2MAP_TARGET_POSITION,        "target_position"},

  // pain
  {51,   AA2MAP_TRIGGER_HURT25,        "trigger_hurt25"},
  {52,   AA2MAP_TRIGGER_HURT50,        "trigger_hurt50"},
  {53,   AA2MAP_TRIGGER_HURT75,        "trigger_hurt75"},
  {54,   AA2MAP_TRIGGER_HURT100,       "trigger_hurt100"},

  // area, world
  {55,   AA2MAP_FLOOR,                 "floor"},
  {56,   AA2MAP_FLOOR_ICE,             "floor_ice"},
  {57,   AA2MAP_FLOOR_LANDSCAPE,       "floor_landscape"},

  {58,   AA2MAP_STAIRS,                "stairs"},

  {59,   AA2MAP_BLOCK,                 "block"},
  {60,   AA2MAP_BLOCK_TRANS,           "block_trans"},
  {61,   AA2MAP_BLOCK_MIRROR,          "block_mirror"},
  {62,   AA2MAP_BLOCK_WATER,           "block_water"},

  {0, 0, NULL}
};


st_aa2map_object_t *
aa2map_get_object_by_id (int id)
{
  int i = 0;
  for (; aa2map_object[i].name_s; i++)
    if (aa2map_object[i].id == id)
      return &aa2map_object[i];

  return NULL; // default: void
}


int
aa2map_get_object_c_by_id (st_aa2map_t *aa2map, int id)
{
  st_aa2map_object_t *o = aa2map_get_object_by_id (id);
  if (o)
    return aa2map->ascii_chars[o->c];
  return ' '; // default: void
}


st_aa2map_object_t *
aa2map_get_object_by_ascii (st_aa2map_t *aa2map, int c)
{
  int i = 0;
  for (; aa2map_object[i].name_s; i++)
    if (aa2map->ascii_chars[aa2map_object[i].c] == c)
      return &aa2map_object[i];

  return NULL; // default: void
}

int
aa2map_compile (st_aa2map_t *aa2map)
{
  int result = 0;
  char buf[MAXBUFSIZE];

  if (aa2map->map_file == stdout)
    return 0;
    
  sprintf (buf, aa2map->q3map_cmd, aa2map->map_file_s);
  printf ("%s\n", buf);
  result = system (buf);

  sprintf (buf, aa2map->bsp2aas_cmd, aa2map->map_file_s);
  printf ("%s\n", buf);
  result = system (buf);

  return result;
}

/*
  do_set()  return 1 if the locations were the same.  Make them the same.

  Each location in the maze points to some other location connected to it.
  Because how this is constructed, these interrelations form a tree, and
  you can check to see if two nodes are connected by going up to the roots,
  and seeing if the roots are the same.
 
  This function adds one tree to the other if they're not connected.
  As a side effect, it flattens the tree a bit for efficiency.
*/
static int
do_set (int *locations, int loc1, int loc2)
{
  int temp;

  while (locations[loc1] != loc1) // chase loc1 down to the root
    {
      temp = loc1;
      loc1 = locations[loc1];
      locations[temp] = locations[loc1];        // flatten
    }

  while (locations[loc2] != loc2) // chase loc2 down to the root
    {
      temp = loc2;
      loc2 = locations[loc2];
      locations[temp] = locations[loc2];        // flatten
    }

  if (loc1 == loc2) // are they connected
    return 1; // they were connected

  locations[loc2] = loc1; // connect them
  return 0; // they weren't connected
}


static int
maze_calc (int xsize, int ysize, char *hedges, char *vedges)
{
  int x, y;
  int *squares = NULL, *hedgelist = NULL, *vedgelist = NULL;
  int hedgecount = 0, vedgecount = 0, index, curedge;

  if (!(squares = malloc (xsize * ysize * sizeof (int))))
    return -1;

  if (!(hedgelist = malloc (xsize * ysize * sizeof (int))))
    {
      free (squares);
      return -1;
    }

  if (!(vedgelist = malloc (xsize * ysize * sizeof (int))))
    {
      free (squares);
      free (hedgelist);
      return -1;
    }

  // init arrays of all horizontal edges left to check
  for (x = 0; x < xsize; x++)
    for (y = 1; y < ysize; y++)
      hedgelist[hedgecount++] = x * ysize + y;

  // init arrays of all vertical edges left to check
  for (x = 1; x < xsize; x++)
    for (y = 0; y < ysize; y++)
      vedgelist[vedgecount++] = x * ysize + y;

  // label squares by what they're connected to: just self
  for (x = 0; x < xsize; x++)
    for (y = 0; y < ysize; y++)
      squares[x * ysize + y] = x * ysize + y;

  // punch holes as necessary
  while ((hedgecount > 0) || (vedgecount > 0))  // all edges checked?
    {
      // do a horizontal edge if ...
      if ((vedgecount == 0) ||  // that's all that's left
          (RANDOM (0, 1) &&     // or 50/50 chance and
          (hedgecount > 0)))    // there are some left
        {
          /*
            horizontal edge
            pick one at random from the unchecked
          */
          index = RANDOM (0, hedgecount);
          curedge = hedgelist[index];

          // and remove it from the "to check" list
          hedgelist[index] = hedgelist[--hedgecount];

          /*
            if the stuff on either side of it is already
            connected, leave it alone.  If they're not,
            then do_set connectes them, and we punch a hole
            in that wall
          */
          hedges[curedge] = do_set (squares, curedge, curedge - 1);
        }
      else
        {
          /*
            vertical edge
            pick one at random from the unchecked
          */
          index = RANDOM (0, vedgecount);
          curedge = vedgelist[index];

          // and remove it from the "to check" list
          vedgelist[index] = vedgelist[--vedgecount]; 

          /*
            if the stuff on either side of it is already
            connected, leave it alone.  If they're not,
            then do_set connectes them, and we punch a hole
            in that wall
          */
          vedges[curedge] = do_set (squares, curedge, curedge - ysize);
        }
    }
  // Finish up the horizontal edges of the maze
  for (x = 0; x < xsize; x++)
    hedges[x * ysize] = 1;

  // and the vertical edges too
  for (y = 0; y < ysize; y++)
    vedges[y] = 1;

  // and make one entrance/exit
  x = rand () % xsize;
  hedges[x * ysize] = 0;

  free (squares);
  free (hedgelist);    
  free (vedgelist);

  return 0;
}


void
aa2map_mazegen (st_aa2map_t *aa2map, int xsize, int ysize)
{
  int x, y;
  int maze_block = aa2map_get_object_c_by_id (aa2map, AA2MAP_BLOCK);
  int maze_floor = aa2map_get_object_c_by_id (aa2map, AA2MAP_FLOOR);
  int maze_start = aa2map_get_object_c_by_id (aa2map, AA2MAP_INFO_PLAYER_DEATHMATCH);
  int maze_stop = aa2map_get_object_c_by_id (aa2map, AA2MAP_FLOOR);
  char *hedges = NULL, *vedges = NULL;

  if (!(hedges = malloc (xsize * ysize)))
    {
      fprintf (stderr, "ERROR: aa2map_mazegen() failed\n");
      return;
    }

  if (!(vedges = malloc (xsize * ysize)))
    {
      fprintf (stderr, "ERROR: aa2map_mazegen() failed\n");
      free (hedges);
      return;
    }

  if (maze_calc (xsize, ysize, hedges, vedges) != 0)
    {
      fprintf (stderr, "ERROR: randomize() failed\n");
      free (hedges);
      free (vedges);
      return; 
    }

  for (y = 0; y < ysize; y++)
    {
      for (x = 0; x < xsize; x++)
        fprintf (aa2map->map_file, "%c%c", maze_block, hedges[x * ysize + y] ? maze_block : (!y ? maze_stop : maze_floor));

      fprintf (aa2map->map_file, "%c\n", maze_block);

      for (x = 0; x < xsize; x++)
        fprintf (aa2map->map_file, "%c%c", vedges[x * ysize + y] ? maze_block : maze_floor, maze_floor);

      fprintf (aa2map->map_file, "%c\n", maze_block);
    }

  // bottom
  for (x = 0; x < xsize; x++)
    fprintf (aa2map->map_file, "%c%c", maze_block, hedges[x * ysize] ? maze_block : maze_start);  

  fprintf (aa2map->map_file, "%c\n", maze_block);

  free (hedges);
  free (vedges);
}


void
aa2map_strafe (st_aa2map_t *aa2map, int pads, int gap_initial, int gap_increment, int height_increment)
{
#define FIRSTPAD_LEN 256
#define SIDE_SPACE 128
  int i = 0;
  char message[MAXBUFSIZE];
  char name[MAXBUFSIZE];
  char s[MAXBUFSIZE];
  int xpos = 0;
  int zpos = 0;
  int gap = 0;
  int xscale = aa2map->xscale;
  int yscale = aa2map->yscale;
  int zscale = aa2map->zscale;
  st_map_brush_t b = {{IDTECH3_MAP_BLOCK}, "", 1, 1, 0, 0, 0};
  st_map_brush_t bak = {{IDTECH3_MAP_BLOCK}, "", 1, 1, 0, 0, 0};

  sprintf (message, "Map: %d pads, %dx%dx%d+%d pad size, %d+%d gaps",
           pads,
           aa2map->xscale,
           aa2map->yscale,
           aa2map->zscale, height_increment, gap_initial, gap_increment);

  fprintf (stderr, "%s\n", message);

  idtech3_map_start (aa2map->map_file, message,
                                       aa2map->light,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL);

  idtech3_map_scale (&b, FIRSTPAD_LEN, yscale + SIDE_SPACE * 2, zscale);
  sprintf (b.texture, "%s/block", aa2map->path);
  b.xsize = FIRSTPAD_LEN;
  b.ysize = yscale + SIDE_SPACE * 2;
  b.zsize = zscale;
  idtech3_map_brush6 (aa2map->map_file, &b);

  xpos = FIRSTPAD_LEN;
  gap = gap_initial;
  zpos = zscale;
  for (i = 0; i < pads - 1; i++)
    {
      memcpy (&b, &bak, sizeof (st_map_brush_t));
      idtech3_map_scale (&b, xscale, yscale, zpos);
      idtech3_map_trans (&b, xpos + gap, SIDE_SPACE, 0);
      sprintf (b.texture, "%s/block_trans", aa2map->path);
      b.xsize = xscale;
      b.ysize = yscale;
      b.zsize = zpos;
      idtech3_map_brush6 (aa2map->map_file, &b);
      xpos += (xscale + gap);
      gap += gap_increment;
      zpos += height_increment;
    }

  memcpy (&b, &bak, sizeof (st_map_brush_t));
  idtech3_map_scale (&b, FIRSTPAD_LEN, yscale + SIDE_SPACE * 2, zpos);
  idtech3_map_trans (&b, xpos + gap, 0, 0);
  sprintf (b.texture, "%s/block", aa2map->path);
  b.xsize = FIRSTPAD_LEN;
  b.ysize = yscale + SIDE_SPACE * 2;
  b.zsize = zpos;  
  idtech3_map_brush6 (aa2map->map_file, &b);
  xpos += (FIRSTPAD_LEN + gap);

  // HACK
  aa2map->xsize = aa2map->ysize = aa2map->zsize = 1;
  aa2map->xscale = xpos;
  aa2map->yscale = yscale + SIDE_SPACE * 2;
  aa2map->zscale = zscale;

  // scale and draw world box to fit all objects inside
  aa2map_world (aa2map);

  idtech3_map_end (aa2map->map_file);

  // spawn point
  fprintf (aa2map->map_file, "{\n"
                             "  \"classname\" \"%s\"\n"
                             "  \"origin\" \"%d %d %d\"\n"
                             "  \"angle\" \"%d\"\n"
                             "}\n", 
                             "info_player_start",
                             (int) (FIRSTPAD_LEN * 0.5),
                             (int) (yscale * 0.5) + SIDE_SPACE,
                             (int) (zscale * 1.5),
                             0);

  // teleport back to start
  fprintf (aa2map->map_file, "{\n"
                             "  \"target\" \"t1\"\n"
                             "  \"classname\" \"trigger_teleport\"\n");

  memcpy (&b, &bak, sizeof (st_map_brush_t));
  idtech3_map_scale (&b, xpos, yscale + SIDE_SPACE * 2, zscale * 0.5);
  sprintf (b.texture, "%s/world_b", aa2map->path);  // "common/trigger"
  idtech3_map_brush6 (aa2map->map_file, &b);

  fprintf (aa2map->map_file, "}\n");

  // start
  fprintf (aa2map->map_file, "{\n"
                             "  \"targetname\" \"t1\"\n"
                             "  \"origin\" \"%d %d %d\"\n"
                             "  \"classname\" \"target_position\"\n"
                             "}\n",
                             (int) (FIRSTPAD_LEN * 0.5),
                             (int) (yscale * 0.5) + SIDE_SPACE,
                             zscale + 32);

  // message trigger
  fprintf (aa2map->map_file, "{\n"
                             "  \"target\" \"t2\"\n"
                             "  \"classname\" \"trigger_multiple\"\n");

  memcpy (&b, &bak, sizeof (st_map_brush_t));
  idtech3_map_scale (&b, FIRSTPAD_LEN + gap, yscale + SIDE_SPACE * 2, zscale * 0.5);
  sprintf (b.texture, "%s/world_b", aa2map->path); // "common/trigger"
  idtech3_map_brush6 (aa2map->map_file, &b);

  fprintf (aa2map->map_file, "}\n");

  // message
  fprintf (aa2map->map_file, "{\n"
                             "  \"targetname\" \"t2\"\n"  
                             "  \"origin\" \"%d %d %d\"\n"
                             "  \"classname\" \"target_print\"\n"
                             "  \"message\" \"%s\"\n"
                             "}\n",
                             (int) (FIRSTPAD_LEN * 0.5),
                             (int) (yscale * 0.5) + SIDE_SPACE,
                             zscale + 32,
                             message);
  // shader
  idtech3_shader_start (aa2map->shader_file);  

  aa2map_world_shader (aa2map);

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


static void
aa2map_run_brush (st_aa2map_t *aa2map,
                  int x1, int x2, int x3, int x4,
                  int y1, int y2,
                  int z1, int z2, int z3,
                  const char *texture_s,
                  float xscale,
                  float yscale,
                  float zscale)
{
#define WRITE_FORMAT_S "( %5d %5d %5d )"
  const char *f = "    " WRITE_FORMAT_S " " WRITE_FORMAT_S " " WRITE_FORMAT_S " %s/%s.jpg %d %d 0 %f %f 0 0 0\n";

  fprintf (aa2map->map_file, "  {\n");
  fprintf (aa2map->map_file, f, x1, y1, z2, x3, y2, z3, x2, y1, z2, aa2map->path, texture_s, 0, 0, xscale * 0.015625, yscale * 0.015625); // top
  fprintf (aa2map->map_file, f, x2, y1, z1, x2, y1, z2, x4, y2, z1, aa2map->path, texture_s, 0, 0, xscale * 0.015625, zscale * 0.015625); // east
  fprintf (aa2map->map_file, f, x3, y2, z1, x4, y2, z1, x3, y2, z2, aa2map->path, texture_s, 0, 0, yscale * 0.015625, zscale * 0.015625); // south
  fprintf (aa2map->map_file, f, x1, y1, z1, x2, y1, z1, x2, y2, z1, aa2map->path, texture_s, 0, 0, xscale * 0.015625, yscale * 0.015625); // bottom
  fprintf (aa2map->map_file, f, x1, y1, z1, x3, y2, z1, x1, y1, z2, aa2map->path, texture_s, 0, 0, xscale * 0.015625, zscale * 0.015625); // west
  fprintf (aa2map->map_file, f, x1, y1, z1, x1, y1, z2, x2, y1, z1, aa2map->path, texture_s, 0, 0, yscale * 0.015625, zscale * 0.015625); // north
  fprintf (aa2map->map_file, "  }\n");
}


void
aa2map_run (st_aa2map_t *aa2map, int run_len, int checkpoints)
{
  int i = 0;
  char message[MAXBUFSIZE];
//  char name[MAXBUFSIZE];
//  char s[MAXBUFSIZE];
  const char *start_stop, *trigger, *floor, *floor_edge;
  const char *wall_n, *wall_s, *wall_w, *wall_e;
  const char *wall_edge_n, *wall_edge_s, *wall_edge_w, *wall_edge_e;
  const char *world_t_b, *world_w_e, *world_e_w, *world_n_s, *world_s_n;
  int stepsize;
  int oldxr, newxr;
  int oldxl, newxl;
  int oldy, newy;
  int oldz, newz;
  int bottomlevel, starty, startz;

  start_stop = "floor_landscape_t";
  trigger = "floor_landscape_t";
  wall_n = "block_s";
  wall_s = "block_n"; 
  wall_w = "block_e";
  wall_e = "block_w";
  wall_edge_n = "block_s";
  wall_edge_s = "block_n";
  wall_edge_w = "block_e";
  wall_edge_e = "block_w";
  floor = "floor_t"; 
  floor_edge = "floor_landscape_t";
  world_t_b = "world_t_b";
  world_e_w = "world_e_w";
  world_w_e = "world_w_e";
  world_n_s = "world_n_s";
  world_s_n = "world_s_n";

  bottomlevel = 0;
  oldxr = 0;
  oldxl = 256;
  oldy = -65536+256+64+256;
  oldz = 256;
  startz = oldz;
  starty = oldy;

  fprintf (aa2map->map_file, "// inspired by RMG v0.003, by Daniel 'Space' Lundgren\n");

  sprintf (message, "Map: %d units length, %d checkpoints, %d downhill factor",
           run_len,
           checkpoints,
           aa2map->xrot);

  fprintf (stderr, "%s\n", message);

  idtech3_map_start (aa2map->map_file, message,
                                       aa2map->light,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL);

  // start
  aa2map_run_brush (aa2map, 0, 256, 0, 256, oldy - 256, oldy, oldz - 64, oldz, oldz, start_stop, 256, 256, 256);
  aa2map_run_brush (aa2map, 0, 256, 0, 256, oldy - 256, oldy, 1024, 1024 + 32, 1024 + 32, world_t_b, 256, 256, 1024);

  aa2map_run_brush (aa2map, 0, 256, 0, 256, oldy - 256 - 32, oldy - 256, oldz, 512, 512, wall_s, 256, 256, 512);
  aa2map_run_brush (aa2map, 0, 256, 0, 256, oldy - 256 - 32, oldy - 256, 512, 640, 640, wall_edge_s, 256, 256, 128);
  aa2map_run_brush (aa2map, 0, 256, 0, 256, oldy - 256 - 32, oldy - 256, 640, 1024, 1024, world_s_n, 256, 256, 384);

  aa2map_run_brush (aa2map, -32, 0, -32, 0, oldy - 256, oldy, oldz, 512, 512, wall_e, 256, 256, 512);
  aa2map_run_brush (aa2map, -32, 0, -32, 0, oldy - 256, oldy, 512, 640, 640, wall_edge_e, 256, 256, 128);
  aa2map_run_brush (aa2map, -32, 0, -32, 0, oldy - 256, oldy, 640, 1024, 1024, world_e_w, 256, 256, 384);

  aa2map_run_brush (aa2map, 256, 256 + 32, 256, 256 + 32, oldy - 256, oldy, oldz, 512, 512, wall_w, 256, 256, 512);
  aa2map_run_brush (aa2map, 256, 256 + 32, 256, 256 + 32, oldy - 256, oldy, 512, 640, 640, wall_edge_w, 256, 256, 128);
  aa2map_run_brush (aa2map, 256, 256 + 32, 256, 256 + 32, oldy - 256, oldy, 640, 1024, 1024, world_w_e, 256, 256, 384);
     
  while (1)
    {
      // length
      newy = oldy + RANDOM(200, 800);
      if (abs (newy - starty) > run_len)
        break;

      // floor height
      newz = RANDOM(1, 300);
      if (abs (newz - oldz) < 64)
        newz = oldz;

      // walls
      newxr = RANDOM(-1000, 1000); // right
      newxl = RANDOM(-1000, 1000); // left

      if (abs (newxr - oldxr) < 128)
        newxr = oldxr;

      if (abs (newxl - oldxl) < 128)
        newxl = oldxl;
  
      // dead end?
      if (newxl < newxr)
        {
          int t = newxr;
          newxr = newxl;
          newxl = t;
        }
    
      if (abs (newxr - newxl) < 5000)
        {
          newxr -= 300;
          newxl += 300;
        }
  
      aa2map_run_brush (aa2map, oldxr + 64, oldxl - 64, newxr + 64, newxl - 64, oldy, newy, bottomlevel, oldz, newz, floor, 256, 256, 256);

      aa2map_run_brush (aa2map, oldxr, oldxr + 64, newxr, newxr + 64, oldy, newy, bottomlevel, oldz, newz, floor_edge, 256, 256, 256);
      aa2map_run_brush (aa2map, oldxl - 64, oldxl, newxl - 64, newxl, oldy, newy, bottomlevel, oldz, newz, floor_edge, 256, 256, 256);
  
      aa2map_run_brush (aa2map, oldxr, oldxl, newxr, newxl, oldy, newy, 1024, 1024 + 32, 1024 + 32, world_t_b, 256, 256, 1024);
  
      aa2map_run_brush (aa2map, oldxl, oldxl + 64, newxl, newxl + 64, oldy, newy, 0, 512, 512, wall_w, 256, 256, 512);
      aa2map_run_brush (aa2map, oldxl, oldxl + 64, newxl, newxl + 64, oldy, newy, 512, 640, 640, wall_edge_w, 256, 256, 128);
      aa2map_run_brush (aa2map, oldxl, oldxl + 64, newxl, newxl + 64, oldy, newy, 640, 1024, 1024, world_w_e, 256, 256, 384);
    
      aa2map_run_brush (aa2map, oldxr - 64, oldxr, newxr - 64, newxr, oldy, newy, 0, 512, 512, wall_e, 256, 256, 512);
      aa2map_run_brush (aa2map, oldxr - 64, oldxr, newxr - 64, newxr, oldy, newy, 512, 640, 640, wall_edge_e, 256, 256, 128);
      aa2map_run_brush (aa2map, oldxr - 64, oldxr, newxr - 64, newxr, oldy, newy, 640, 1024, 1024, world_e_w, 256, 256, 384);
  
      // length
      oldy = newy;

      // walls
      oldxr = newxr;
      oldxl = newxl;

      // floor height
      oldz = newz;
    }
  
  // stop
  aa2map_run_brush (aa2map, oldxr, oldxl, oldxr, oldxl, oldy, oldy + 256, bottomlevel, oldz, oldz, start_stop, 256, 256, 256);
  aa2map_run_brush (aa2map, oldxr, oldxl, oldxr, oldxl, oldy, oldy + 256, 1024, 1024 + 32, 1024 + 32, world_t_b, 256, 256, 256);

  aa2map_run_brush (aa2map, oldxl, oldxl + 32, oldxl, oldxl + 32, oldy, oldy + 256, 0, 512, 512, wall_w, 256, 256, 512);
  aa2map_run_brush (aa2map, oldxr - 32, oldxr, oldxr - 32, oldxr, oldy, oldy + 256, 0, 512, 512, wall_e, 256, 256, 512);
  aa2map_run_brush (aa2map, oldxr, oldxl, oldxr, oldxl, oldy + 256, oldy + 256 + 32, 0, 512, 512, wall_n, 256, 256, 512);

  aa2map_run_brush (aa2map, oldxl, oldxl + 32, oldxl, oldxl + 32, oldy, oldy + 256, 512, 640, 640, wall_edge_w, 256, 256, 128);
  aa2map_run_brush (aa2map, oldxr - 32, oldxr, oldxr - 32, oldxr, oldy, oldy + 256, 512, 640, 640, wall_edge_e, 256, 256, 128);
  aa2map_run_brush (aa2map, oldxr, oldxl, oldxr, oldxl, oldy + 256, oldy + 256 + 32, 512, 640, 640, wall_edge_n, 256, 256, 128);

  aa2map_run_brush (aa2map, oldxl, oldxl + 32, oldxl, oldxl + 32, oldy, oldy + 256, 640, 1024, 1024, world_w_e, 256, 256, 384);
  aa2map_run_brush (aa2map, oldxr - 32, oldxr, oldxr - 32, oldxr, oldy, oldy + 256, 640, 1024, 1024, world_e_w, 256, 256, 384);
  aa2map_run_brush (aa2map, oldxr, oldxl, oldxr, oldxl, oldy + 256, oldy + 256 + 32, 640, 1024, 1024, world_n_s, 256, 256, 384);

  // close worldspawn
  fprintf (aa2map->map_file, "}\n");

  // add player
  fprintf (aa2map->map_file, "{\n");
  fprintf (aa2map->map_file, "  \"classname\" \"info_player_start\"\n");
  fprintf (aa2map->map_file, "  \"angle\" \"90\"\n");
  fprintf (aa2map->map_file, "  \"origin\" \"128 %d %d\"\n", starty - 64, startz);
  fprintf (aa2map->map_file, "}\n");

  // defrag start timer
  fprintf (aa2map->map_file, "{\n");
  fprintf (aa2map->map_file, "  \"classname\" \"trigger_multiple\"\n");
  fprintf (aa2map->map_file, "  \"target\" \"startTimer\"\n");

  // TODO: fix height thingy
  aa2map_run_brush (aa2map, 0, 256, 0, 256, starty - 32, starty, 64, 1024, 1024, trigger, 256, 256, 256);

  fprintf (aa2map->map_file, "}\n");
  fprintf (aa2map->map_file, "{\n");
  fprintf (aa2map->map_file, "  \"classname\" \"target_startTimer\"\n");
  fprintf (aa2map->map_file, "  \"targetname\" \"startTimer\"\n");
  fprintf (aa2map->map_file, "  \"origin\" \"128 %d 512\"\n", starty);
  fprintf (aa2map->map_file, "}\n");

  // defrag stop timer
  fprintf (aa2map->map_file, "{\n");
  fprintf (aa2map->map_file, "  \"classname\" \"trigger_multiple\"\n");
  fprintf (aa2map->map_file, "  \"target\" \"stopTimer\"\n");

  aa2map_run_brush (aa2map, oldxr, oldxl, oldxr, oldxl, oldy, oldy + 32, 0, 1024, 1024, trigger, 256, 256, 256);

  fprintf (aa2map->map_file, "}\n");
  fprintf (aa2map->map_file, "{\n");
  fprintf (aa2map->map_file, "  \"classname\" \"target_stopTimer\"\n");
  fprintf (aa2map->map_file, "  \"targetname\" \"stopTimer\"\n");
  fprintf (aa2map->map_file, "  \"origin\" \"128 %d 988\"\n", oldy);
  fprintf (aa2map->map_file, "}\n");

  // defrag checkpoint
  stepsize = abs (oldy - starty) / (checkpoints + 1);
  for (i = 1; i < checkpoints + 1; i++)
    {
      fprintf (aa2map->map_file, "{\n");
      fprintf (aa2map->map_file, "  \"classname\" \"trigger_multiple\"\n");
      fprintf (aa2map->map_file, "  \"target\" \"check%d\"\n", i);

      aa2map_run_brush (aa2map, -2000, 2000, 2000, 2000, (starty + stepsize * i) - 32, starty + stepsize * i, 64, 1024, 1024, trigger, 256, 256, 256);

      fprintf (aa2map->map_file, "}\n");
      fprintf (aa2map->map_file, "{\n");
      fprintf (aa2map->map_file, "  \"classname\" \"target_checkpoint\"\n");
      fprintf (aa2map->map_file, "  \"targetname\" \"check%d\"\n", i);
      fprintf (aa2map->map_file, "  \"origin\" \"128 %d 512\"\n", starty);
      fprintf (aa2map->map_file, "}\n");
  }

  // shader
  idtech3_shader_start (aa2map->shader_file);  

  aa2map_world_shader (aa2map);
}


void
aa2map_museum (st_aa2map_t *aa2map, const char *pk3_file)
{
#ifdef USE_ZLIB
  int i = 0;
  char message[MAXBUFSIZE];
  char name[MAXBUFSIZE];
  char s[MAXBUFSIZE];
#define AA2MAP_MAX_ASSETS 64
#define AA2MAP_MAX_ASSET_NAME 96
  int assets = 0;
#define SPACE_RADIUS 96
  int xpos = SPACE_RADIUS;
  int ypos = SPACE_RADIUS;
  char asset[AA2MAP_MAX_ASSETS][AA2MAP_MAX_ASSET_NAME];
  int xscale = aa2map->xscale;
  int yscale = aa2map->yscale;
  int zscale = aa2map->zscale;
  st_map_brush_t b = {{IDTECH3_MAP_BLOCK}, "", 1, 1, 0, 0, 0};
  st_map_brush_t bak = {{IDTECH3_MAP_BLOCK}, "", 1, 1, 0, 0, 0};
  st_unzip2_t *pk3 = NULL;

  if (!(pk3 = unzip2_open (pk3_file, "rb")))
    {
      fprintf (stderr, "ERROR: could not open %s\n", pk3_file);
      return;
    }

  for (i = 0; i < pk3->entries && i < UNZIP2_MAX_ENTRIES && assets < AA2MAP_MAX_ASSETS; i++)
    if (stristr (get_suffix (pk3->entry[i]->name), ".md3") //||
//        !strnicmp (pk3->entry[i]->name, "levelshot", 9)
)
      if (strlen (pk3->entry[i]->name) < AA2MAP_MAX_ASSET_NAME)
        {
          strncpy (asset[assets], pk3->entry[i]->name, AA2MAP_MAX_ASSET_NAME)[AA2MAP_MAX_ASSET_NAME - 1] = 0;
          assets++;
        }

  unzip2_close (pk3);

  sprintf (message, "Map: Museum, %d models", assets);

  fprintf (stderr, "%s\n", message);

  idtech3_map_start (aa2map->map_file, message,
                                       aa2map->light,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL);

  // HACK
  aa2map->xsize = aa2map->ysize = aa2map->zsize = 1;
  aa2map->xscale = (((int) sqrt (assets)) + 2) * SPACE_RADIUS;
  aa2map->yscale = aa2map->xscale;
  aa2map->zscale = SPACE_RADIUS;

  memcpy (&b, &bak, sizeof (st_map_brush_t));
  idtech3_map_scale (&b, aa2map->xscale - 2, aa2map->yscale - 2, 32);
  // TODO: cull
  idtech3_map_trans (&b, 1, 1, 1);
  sprintf (b.texture, "%s/block_trans", aa2map->path);
  b.xsize = aa2map->xscale - 2;
  b.ysize = aa2map->yscale - 2;
  b.zsize = 32;
  idtech3_map_brush6 (aa2map->map_file, &b);

  // scale and draw world box to fit all objects inside
  aa2map_world (aa2map);

  idtech3_map_end (aa2map->map_file);

  // spawn point
  fprintf (aa2map->map_file, "{\n"
                             "  \"classname\" \"%s\"\n"
                             "  \"origin\" \"%d %d %d\"\n"
                             "  \"angle\" \"%d\"\n"
                             "}\n", 
                             "info_player_start",
                             (int) (xscale * 0.5),
                             (int) (yscale * 0.5),
                             (int) (zscale * 1.5),
                             0);

  for (i = 0; i < assets; i++)
    if (!stricmp (get_suffix (asset[i]), ".md3"))
      {
        fprintf (aa2map->map_file, "{\n"
                                   "  \"classname\" \"misc_model\"\n"
                                   "  \"origin\" \"%d %d 64\"\n"
                                   "  \"model\" \"%s\"\n"
                                   "}\n", xpos, ypos, asset[i]);

        xpos += SPACE_RADIUS;
        if (xpos + SPACE_RADIUS > aa2map->xscale)
          {
            xpos = SPACE_RADIUS;
            ypos += SPACE_RADIUS;
          }
      }

  // shader
  idtech3_shader_start (aa2map->shader_file);  

  aa2map_world_shader (aa2map);

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
#endif
}
