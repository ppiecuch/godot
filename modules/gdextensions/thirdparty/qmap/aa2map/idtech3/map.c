/*
map.c - id Tech 3 map code

Copyright (c) 2008 NoisyB


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
#include "misc/string.h"
#include "misc/file.h"
#include "misc/math_vector.h"
#include "map.h"

#include <stdio.h>
#include <math.h>

static int idtech3_brush_count = 0;  


const st_entity_names_t idtech3_entity_names[] = {
  {"info_player_deathmatch", NULL},
  {"info_player_start", NULL},
  {"trigger_teleport", NULL},
  {"misc_teleporter_dest", NULL},

  {"ammo_belt", "icona_chaingun"},
  {"ammo_bfg", "icona_bfg"},
  {"ammo_bullets", "icona_machinegun"},
  {"ammo_cells", "icona_plasma"},
  {"ammo_grenades", "icona_grenade"},
  {"ammo_lightning", "icona_lightning"},
  {"ammo_mines", "icona_proxlauncher"},
  {"ammo_nails", "icona_nailgun"},
  {"ammo_rockets", "icona_rocket"},
  {"ammo_shells", "icona_shotgun"},
  {"ammo_slugs", "icona_railgun"},

  {"holdable_invulnerability", "invulnerability"},
  {"holdable_kamikaze", "kamikaze"},
  {"holdable_medkit", "medkit"},
  {"holdable_portal", "portal"},
  {"holdable_teleporter", "teleporter"},

  {"item_ammoregen", "ammo_regen"},
  {"item_armor_body", "iconr_red"},
  {"item_armor_combat", "iconr_yellow"},
  {"item_armor_shard", "iconr_shard"},
  {"item_bluecube", "iconh_borb"},
  {"item_doubler", "doubler"},
  {"item_enviro", "envirosuit"},
  {"item_flight", "flight"},
  {"item_guard", "guard"},
  {"item_haste", "haste"},
  {"item_health_large", "iconh_red"},
  {"item_health_mega", "iconh_mega"},
  {"item_health_small", "iconh_green"},
  {"item_health", "iconh_yellow"},
  {"item_invis", "invis"},
  {"item_quad", "quad"},
  {"item_redcube", "iconh_rorb"},
  {"item_regen", "regen"},
  {"item_scout", "scout"},

  {"shooter_grenade", "shootergl"},
  {"shooter_plasma", "shooterpg"},
  {"shooter_rocket", "shooterrl"},

  {"team_CTF_blueflag", "iconf_blu1"},
  {"team_CTF_neutralflag", "iconf_neutral1"},
  {"team_CTF_redflag", "iconf_red1"},

  {"weapon_bfg", "iconw_bfg"},
  {"weapon_chaingun", "chaingun128"},
  {"weapon_gauntlet", "iconw_gauntlet"},
  {"weapon_grapplinghook", "iconw_grapple"},
  {"weapon_grenadelauncher", "iconw_grenade"},
  {"weapon_lightning", "iconw_lightning"},
  {"weapon_machinegun", "iconw_machinegun"},
  {"weapon_nailgun", "nailgun128"},
  {"weapon_plasmagun", "iconw_plasma"},
  {"weapon_prox_launcher", "proxmine"},          
  {"weapon_railgun", "iconw_railgun"},
  {"weapon_rocketlauncher", "iconw_rocket"},
  {"weapon_shotgun", "iconw_shotgun"},

  // defrag
  {"target_startTimer", "defrag"},
  {"target_stopTimer", "defrag"},
  {"target_checkpoint", "defrag"},
  {"target_fragsFilter", "defrag"},
  {"target_init", "defrag"},
  {"target_smallprint", "defrag"},

  {"shooter_grenade_targetplayer", "shootergl"},
  {"shooter_plasma_targetplayer", "shooterpg"},
  {"shooter_rocket_targetplayer", "shooterrl"},
  {NULL, NULL}
};


const char *
idtech3_entity_to_icon (const char *entity)
{
  int i = 0;
  for (; idtech3_entity_names[i].entity_s; i++)
    if (!stricmp (idtech3_entity_names[i].entity_s, entity))
      return idtech3_entity_names[i].icon_s;
  return NULL;
}


void
aa2map_defi_write (FILE *output_file)
{
  (void) output_file;
#if 0
  char buf[32768];

  sprintf (buf, "scripts/%s.defi", mapname);
  fprintf (output_file, "{\n");
  fprintf (output_file, "  map \"%s\"\n", mapname);
  fprintf (output_file, "  longname \"%s\"\n", mapname);
  fprintf (output_file, "  style \"run\"\n");
  fprintf (output_file, "  cpm \"1\"\n");
  fprintf (output_file, "  author \"%s\"\n", author);
  fprintf (output_file, "}\n");
#endif
}


void
aa2map_arena_write (FILE *output_file)
{
  (void) output_file;
#if 0
  char buf[32768];
 
  sprintf (buf, "scripts/%s.arena", mapname);
  fprintf (output_file, "{\n");
  fprintf (output_file, "  map \"%s\"\n", mapname);
  fprintf (output_file, "  bots \"%s\"\n", bots);       
  fprintf (output_file, "  longname \"%s\"\n", mapname);
  fprintf (output_file, "  fraglimit %d\n", fraglimit);
  fprintf (output_file, "  type \"%s\"\n", type);
  fprintf (output_file, "}\n");
#endif
}


void
idtech3_map_start (FILE *output_file, const char *message, int light_type, int *gravity, int *dust, int *breath, const char *music)
{
  fprintf (output_file, "{\n"
                        "  \"classname\" \"worldspawn\"\n");
  if (music)
    fprintf (output_file, "  \"music\" \"music/%s\"\n", music);
  if (dust)
    fprintf (output_file, "  \"enableDust\" \"%d\"\n", *dust);
  if (breath)
    fprintf (output_file, "  \"enableBreath\" \"%d\"\n", *breath); 
  if (gravity)
    fprintf (output_file, "  \"gravity\" \"%d\"\n", *gravity);
//  if (angle)
//    fprintf (output_file, "  \"angle\" \"%d\"\n", angle);
  if (message)
    fprintf (output_file, "  \"message\" \"%s\"\n", message);

//  if (fog)
//    fprintf (output_file, "\"_farplanedist\" \"2048\"\n"
//                          "\"_foghull\" \"cosmo_skies/foghullsky1\"\n"
//                          "\"_blocksize\" \"512\"\n"
//                          "\"enabledust\" \"1\"\n");

  if (light_type == IDTECH3_LIGHT_AMBIENT)
    fprintf (output_file, "  \"_ambient\" \"50\"\n");
  else  if (light_type == IDTECH3_LIGHT_SUN)
    fprintf (output_file, "  \"_sun_light\" \"280\"\n"
                          "  \"_sun_ambient\" \"20\"\n"
                          "  \"_sun_color\" \"1 1 .8\"\n"
                          "  \"_sun_diffuse\" \"150\"\n"
                          "  \"_sun_diffade\" \"0.5\"\n"
                          "  \"_sun_angle\" \"90 -45\"\n");
}


/*
2.1.2 Brushes:

Brushes are one of the two primary components of a MAP file. Each brush
defines a solid region. Brushes define this region as the intersection of
four or more planes. Each plane is defined by three noncolinear points.
These points must go in a clockwise orientation:

1--2-------->
|
3
|
|
,

Each brush statement looks like this:

 {
  ( 128 0 0 ) ( 128 1 0 ) ( 128 0 1 ) GROUND1_6 0 0 0 1.0 1.0
  ( 256 0 0 ) ( 256 0 1 ) ( 256 1 0 ) GROUND1_6 0 0 0 1.0 1.0
  ( 0 128 0 ) ( 0 128 1 ) ( 1 128 0 ) GROUND1_6 0 0 0 1.0 1.0
  ( 0 384 0 ) ( 1 384 0 ) ( 0 384 1 ) GROUND1_6 0 0 0 1.0 1.0
  ( 0 0 64 )  ( 1 0 64 )  ( 0 1 64 )  GROUND1_6 0 0 0 1.0 1.0
  ( 0 0 128 ) ( 0 1 128 ) ( 1 0 128 ) GROUND1_6 0 0 0 1.0 1.0
 }

That's probably just a bit confusing when you first see it. It defines a
rectangular region that extends from (128,128,64) to (256,384,128). Here's
what a single line means:

 ( 128 0 0 ) ( 128 1 0 ) ( 128 0 1 ) GROUND1_6   0     0       0      1.0    1.0
  1st Point   2nd Point   3rd Point   Texture   x_off y_off rotation x_scale y_scale
					       

Here are more details about those fields:

1st Point \   Those three points define a plane, so they must not be colinear.
2nd Point  >  Each plane should only be defined once.
3rd Point /   Plane normal is oriented toward the cross product of (P1 - P2) and (P3 -P2)
Texture   - The name of the MIP texture (without quotes). 
x_off     - Texture x-offset (must be multiple of 16)
y_off     - Texture y-offset (must be multiple of 16)
rotation  - The texture rotation angle, in degree.
x_scale   - scales x-dimension of texture
y_scale   - scales y-dimension of texture
*/


#if 0
// TODO: make this replace xsize, ysize and zsize in st_map_brush_t 
static void
idtech3_map_brush_AABB (const st_map_brush_t *b, float *xmin, float *ymin, float *zmin,
                                                 float *xmax, float *ymax, float *zmax)
{
}
#endif


void
idtech3_map_brush6 (FILE *output_file, const st_map_brush_t *b)
{
#define WRITE_FORMAT_S "( %5d %5d %5d )"
  int i = 0;
  float xsize, ysize;
//  float min[3], max[3];
  char name[1024];
  char suffix[1024];
  const char *suffix_s[6] = {"_t", "_s", "_w", "_b", "_n", "_e"};
  const char *p = NULL;

  strcpy (name, b->texture);
  set_suffix (name, "");
  strcpy (suffix, get_suffix (b->texture));

  fprintf (output_file, "  { // brush %d\n", idtech3_brush_count++);

//  idtech3_map_brush_AABB (b, &min[0], &min[1], &min[2], &max[0], &max[1], &max[2]);

  for (i = 0; i < 6; i++)
    {
      // scale the texture to the whole size of the surface
      switch (i)
        {
          case 0:
          case 3:
            xsize = b->xsize;
            ysize = b->ysize;
            break;
          case 1:
          case 4:
            xsize = b->xsize;
            ysize = b->zsize; 
            break;
          case 2:
          case 5:
            xsize = b->ysize;
            ysize = b->zsize; 
            break;
        }

      p = suffix_s[i];
#if 1
      // HACK: use top texture for 1 unit high sides
      if (b->zsize <= 2.0) // 2.0 units, the idtech3 minimum brush height for triggers
        if (strchr ("nwse", p[1]))
          p = "_t";
#endif
      fprintf (output_file, "    " WRITE_FORMAT_S " " WRITE_FORMAT_S " " WRITE_FORMAT_S " %s%s%s 0 0 0 %f %f 0 0 0\n",
               (int) b->b[i][0][0],
               (int) b->b[i][0][1],
               (int) b->b[i][0][2],

               (int) b->b[i][1][0],
               (int) b->b[i][1][1],
               (int) b->b[i][1][2],

               (int) b->b[i][2][0],
               (int) b->b[i][2][1],
               (int) b->b[i][2][2],

               name,
               p,
               suffix,

               (xsize * b->xscale) * 0.015625 * ((i == 0 || i == 1 || i == 3 || i == 5) ? -1 : 1),
               (ysize * b->yscale) * 0.015625 * ((i == 0) ? -1 : 1)); 
    }

  fprintf (output_file, "  }\n"); 
}


void
idtech3_map_scale (st_map_brush_t *b, float x, float y, float z)
{
  int i = 0;

  for (; i < 6; i++)
    {
      b->b[i][0][0] *= x;
      b->b[i][0][1] *= y;
      b->b[i][0][2] *= z;

      b->b[i][1][0] *= x;
      b->b[i][1][1] *= y;
      b->b[i][1][2] *= z;

      b->b[i][2][0] *= x;
      b->b[i][2][1] *= y;
      b->b[i][2][2] *= z;
    }
}   


void
idtech3_map_trans (st_map_brush_t *b, float x, float y, float z)
{
  int i = 0;

  for (; i < 6; i++)
    {
      b->b[i][0][0] += x;
      b->b[i][0][1] += y;
      b->b[i][0][2] += z;

      b->b[i][1][0] += x;
      b->b[i][1][1] += y;
      b->b[i][1][2] += z;
 
      b->b[i][2][0] += x;
      b->b[i][2][1] += y;
      b->b[i][2][2] += z;
    }
}


void
idtech3_map_rot (st_map_brush_t *b, float x, float y, float z)
{
#if 1
  (void) b;
  (void) x;
  (void) y;
  (void) z;
#else
  int i = 0, j = 0;
  float old[3];
  float theta = 10;
  float cost = (float) cos (MATH_DEG2RAD (theta));
  float sint = (float) sin (MATH_DEG2RAD (theta));
 
  for (; i < 6; i++)
    for (j = 0; j < 3; j++)
    {
      old[0] = b->b[i][j][0];
      old[1] = b->b[i][j][1];
      old[2] = b->b[i][j][2];
      b->b[i][j][0] = (cost + (1 - cost) * x * x) * old[0]
                    + ((1 - cost) * x * y - z * sint) * old[1];
                    + ((1 - cost) * x * z + y * sint) * old[2];

      b->b[i][j][1] = ((1 - cost) * x * y + z * sint) * old[0]
                    + (cost + (1 - cost) * y * y) * old[1];
                    + ((1 - cost) * y * z - x * sint) * old[2];

      b->b[i][j][2] = ((1 - cost) * x * z - y * sint) * old[0]
                    + ((1 - cost) * y * z + x * sint) * old[1];
                    + (cost + (1 - cost) * z * z) * old[2];
    }
#endif
}


void
idtech3_map_end (FILE *output_file)
{
  fprintf (output_file, "}\n");
}


void
idtech3_map_light (FILE *output_file, float x, float y, float z, float brightness)
{
  fprintf (output_file, "{\n"
                        "  \"classname\" \"light\"\n"
                        "  \"origin\" \"%d %d %d\"\n"
                        "  \"light\" \"%d\"\n"
                        "  \"_color\" \"1 1 1\"\n"
                        "}\n", (int) x, (int) y, (int) z, (int) brightness);
}


#if 0
void
idtech3_map_entity (FILE *output_file, const st_map_entity_t *e)
{
  (void) output_file;
  (void) e;
}
#endif


