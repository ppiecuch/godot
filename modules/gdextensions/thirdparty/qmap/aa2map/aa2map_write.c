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
#include "misc/defines.h"
#include "misc/file.h"
#include "idtech3/map.h"
#include "idtech3/shader.h"
#include "write/world.h"
#include "write/stub.h"
#include "write/trigger_teleport.h"
#include "write/trigger_push.h"
#include "write/trigger_hurt.h"
#include "write/floor.h"
#include "write/floor_landscape.h"
#include "write/stairs.h"
#include "write/block.h"
#include "write/block_trans.h"
#include "write/block_mirror.h"
#include "write/block_water.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


void
aa2map_map_write (st_aa2map_t *aa2map, const st_aa2map_parse_t *a)
{
  fprintf (aa2map->map_file, "// %s\n"
                             "// _t and _b bottom suffices stand for top or bottom surface\n"
                             "// _n, _s, _w, _e suffices refer to the cardinal direction of the surface\n", aa2map->message);

  idtech3_map_start (aa2map->map_file, aa2map->message,
                                       aa2map->light,
                                       (aa2map->gravity ? &aa2map->gravity : NULL),
                                       NULL,
                                       NULL,
                                       NULL);

  // brushes
  for (int i = 0; a[i].id; i++)
    switch (a[i].id)
      {
        case AA2MAP_BLOCK:
          aa2map_block (aa2map, &a[i]);
          break;

        case AA2MAP_BLOCK_TRANS:
          aa2map_block_trans (aa2map, &a[i]);
          break;

        case AA2MAP_BLOCK_MIRROR:
          aa2map_block_mirror (aa2map, &a[i]);
          break;

        case AA2MAP_BLOCK_WATER:
          aa2map_block_water (aa2map, &a[i]);
          break;

        case AA2MAP_TRIGGER_HURT25:
          aa2map_trigger_hurt25 (aa2map, &a[i]);
          break;

        case AA2MAP_TRIGGER_HURT50:
          aa2map_trigger_hurt50 (aa2map, &a[i]);
          break;

        case AA2MAP_TRIGGER_HURT75:
          aa2map_trigger_hurt75 (aa2map, &a[i]);
          break;

        case AA2MAP_TRIGGER_HURT100:
          aa2map_trigger_hurt100 (aa2map, &a[i]);
          break;

        case AA2MAP_FLOOR_LANDSCAPE:
          aa2map_floor_landscape (aa2map, &a[i]);
          break;

        case AA2MAP_STAIRS:
          aa2map_stairs (aa2map, &a[i]);
          break;

        case AA2MAP_FLOOR:
          aa2map_floor (aa2map, &a[i]);
          break;

        // spawn
//        case AA2MAP_INFO_PLAYER_START:
        case AA2MAP_INFO_PLAYER_DEATHMATCH:
          if (a[i].z == 0)
            aa2map_floor (aa2map, &a[i]);
          break;

        default:
          aa2map_stub (aa2map, &a[i]);
          break;
      }

  // scale and draw world box to fit all objects inside
  aa2map_world (aa2map);

  idtech3_map_end (aa2map->map_file);

  // entities
  for (int i = 0; a[i].id; i++)
    switch (a[i].id)
      {
        // teleports
        case AA2MAP_TRIGGER_TELEPORT:
          aa2map_entity_trigger_teleport (aa2map, &a[i]);
          break;

        case AA2MAP_MISC_TELEPORT_DEST:
          aa2map_entity_misc_teleporter_dest (aa2map, &a[i]);
          break;

        // jumppad
        case AA2MAP_TRIGGER_PUSH:
          aa2map_entity_trigger_push (aa2map, &a[i]);
          break;

        case AA2MAP_TARGET_POSITION:
          aa2map_entity_target_position (aa2map, &a[i]);
          break;

        case AA2MAP_TRIGGER_HURT25:
          aa2map_entity_trigger_hurt25 (aa2map, &a[i]);
          break;

        case AA2MAP_TRIGGER_HURT50:
          aa2map_entity_trigger_hurt50 (aa2map, &a[i]);
          break;

        case AA2MAP_TRIGGER_HURT75:
          aa2map_entity_trigger_hurt75 (aa2map, &a[i]);
          break;

        case AA2MAP_TRIGGER_HURT100:
          aa2map_entity_trigger_hurt100 (aa2map, &a[i]);
          break;

        // spawn
        // case AA2MAP_INFO_PLAYER_START:
        case AA2MAP_INFO_PLAYER_DEATHMATCH:

        // weapons
        case AA2MAP_WEAPON_GAUNTLET:
        case AA2MAP_WEAPON_MACHINEGUN:
        case AA2MAP_WEAPON_SHOTGUN:
        case AA2MAP_WEAPON_GRENADELAUNCHER:
        case AA2MAP_WEAPON_ROCKETLAUNCHER:
        case AA2MAP_WEAPON_LIGHTNING:
        case AA2MAP_WEAPON_RAILGUN:
        case AA2MAP_WEAPON_PLASMAGUN:
        case AA2MAP_WEAPON_BFG:
        case AA2MAP_WEAPON_GRAPPLINGHOOK:
        case AA2MAP_WEAPON_NAILGUN:
        case AA2MAP_WEAPON_PROX_LAUNCHER:
        case AA2MAP_WEAPON_CHAINGUN:
        // ammo
        case AA2MAP_AMMO_BULLETS:
        case AA2MAP_AMMO_SHELLS:
        case AA2MAP_AMMO_GRENADES:
        case AA2MAP_AMMO_ROCKETS:
        case AA2MAP_AMMO_LIGHTNING:
        case AA2MAP_AMMO_SLUGS:
        case AA2MAP_AMMO_CELLS:
        case AA2MAP_AMMO_BFG:
        // holdable
        case AA2MAP_HOLDABLE_MEDKIT:
        case AA2MAP_HOLDABLE_TELEPORTER:
        // items
        case AA2MAP_ITEM_ARMOR_BODY:
        case AA2MAP_ITEM_ARMOR_COMBAT:
        case AA2MAP_ITEM_ARMOR_SHARD:
        case AA2MAP_ITEM_ENVIRO:
        case AA2MAP_ITEM_FLIGHT:
        case AA2MAP_ITEM_HASTE:
        case AA2MAP_ITEM_HEALTH:
        case AA2MAP_ITEM_HEALTH_LARGE:
        case AA2MAP_ITEM_HEALTH_MEGA:
        case AA2MAP_ITEM_HEALTH_SMALL:
        case AA2MAP_ITEM_INVIS:
        case AA2MAP_ITEM_QUAD:
        case AA2MAP_ITEM_REGEN:
        // team
        case AA2MAP_TEAM_CTF_BLUEFLAG:
        case AA2MAP_TEAM_CTF_REDFLAG:
        // shooter
        // case AA2MAP_SHOOTER_GRENADE:
        // case AA2MAP_SHOOTER_PLASMA:
        // case AA2MAP_SHOOTER_ROCKET:
          aa2map_entity_stub (aa2map, &a[i]);
          break;

        default:
          break;
      }

  // lights
  if (aa2map->light == AA2MAP_LIGHT_RANDOM)
  for (int i = 0; a[i].id; i++)
    switch (a[i].id)
      {
        case AA2MAP_FLOOR:
        // teleports
        case AA2MAP_TRIGGER_TELEPORT:
        case AA2MAP_MISC_TELEPORT_DEST:
        // jumppad
        case AA2MAP_TRIGGER_PUSH:
        case AA2MAP_TARGET_POSITION:
        case AA2MAP_TRIGGER_HURT25:
        case AA2MAP_TRIGGER_HURT50:
        case AA2MAP_TRIGGER_HURT75:
        case AA2MAP_TRIGGER_HURT100:
        // spawn
        // case AA2MAP_INFO_PLAYER_START:
        case AA2MAP_INFO_PLAYER_DEATHMATCH:
        // weapons
        case AA2MAP_WEAPON_GAUNTLET:
        case AA2MAP_WEAPON_MACHINEGUN:
        case AA2MAP_WEAPON_SHOTGUN:
        case AA2MAP_WEAPON_GRENADELAUNCHER:
        case AA2MAP_WEAPON_ROCKETLAUNCHER:
        case AA2MAP_WEAPON_LIGHTNING:
        case AA2MAP_WEAPON_RAILGUN:
        case AA2MAP_WEAPON_PLASMAGUN:
        case AA2MAP_WEAPON_BFG:
        case AA2MAP_WEAPON_GRAPPLINGHOOK:
        case AA2MAP_WEAPON_NAILGUN:
        case AA2MAP_WEAPON_PROX_LAUNCHER:
        case AA2MAP_WEAPON_CHAINGUN:
        // ammo
        case AA2MAP_AMMO_BULLETS:
        case AA2MAP_AMMO_SHELLS:
        case AA2MAP_AMMO_GRENADES:
        case AA2MAP_AMMO_ROCKETS:
        case AA2MAP_AMMO_LIGHTNING:
        case AA2MAP_AMMO_SLUGS:
        case AA2MAP_AMMO_CELLS:
        case AA2MAP_AMMO_BFG:
        // holdable
        case AA2MAP_HOLDABLE_MEDKIT:
        case AA2MAP_HOLDABLE_TELEPORTER:
        // items
        case AA2MAP_ITEM_ARMOR_BODY:
        case AA2MAP_ITEM_ARMOR_COMBAT:
        case AA2MAP_ITEM_ARMOR_SHARD:
        case AA2MAP_ITEM_ENVIRO:
        case AA2MAP_ITEM_FLIGHT:
        case AA2MAP_ITEM_HASTE:
        case AA2MAP_ITEM_HEALTH:
        case AA2MAP_ITEM_HEALTH_LARGE:
        case AA2MAP_ITEM_HEALTH_MEGA:
        case AA2MAP_ITEM_HEALTH_SMALL:
        case AA2MAP_ITEM_INVIS:
        case AA2MAP_ITEM_QUAD:
        case AA2MAP_ITEM_REGEN:
        // team
        case AA2MAP_TEAM_CTF_BLUEFLAG:
        case AA2MAP_TEAM_CTF_REDFLAG:
        // shooter
        // case AA2MAP_SHOOTER_GRENADE:
        // case AA2MAP_SHOOTER_PLASMA:
        // case AA2MAP_SHOOTER_ROCKET:
          idtech3_map_light (aa2map->map_file,
                             (int) (a[i].x * aa2map->xscale + aa2map->xscale * 0.5),
                             (int) (a[i].y * aa2map->yscale + aa2map->yscale * 0.5),
                             (int) (a[i].z * aa2map->zscale + aa2map->zscale * 0.5),
                             RANDOM (20, 100));
          break;

        default:
          break;
      }
}


void
aa2map_shader_write (st_aa2map_t *aa2map, const st_aa2map_parse_t *a)
{
  static int is_done[AA2MAP_MAX_OBJ] = {0};

  fprintf (aa2map->shader_file, "// %s\n"
                                "// _t and _b bottom suffices stand for top or bottom surface\n"
                                "// _n, _s, _w, _e suffices refer to the cardinal direction of the surface\n", aa2map->message);

  idtech3_shader_start (aa2map->shader_file);

  aa2map_world_shader (aa2map);

  // TODO: ASCII row/col/file specific shaders?

  // shader
  for (int i = 0; a[i].id; i++)
    {
      if (!is_done[a[i].id])
        switch (a[i].id)
          {
            case AA2MAP_BLOCK_TRANS:
              aa2map_block_trans_shader (aa2map, &a[i]);
              break;

            case AA2MAP_BLOCK_MIRROR:
              aa2map_block_mirror_shader (aa2map, &a[i]);
              break;

            case AA2MAP_BLOCK_WATER:
              aa2map_block_water_shader (aa2map, &a[i]);
              break;

            default:
              aa2map_shader_stub (aa2map, &a[i]);
              break;
          }

        is_done[a[i].id] = 1;
    }
}
