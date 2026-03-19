/*
map.h - id Tech 3 map code    

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

#ifndef IDTECH3_MAP_H
#define IDTECH3_MAP_H

#include <stdio.h>

typedef struct
{
  const char *entity_s;
  const char *icon_s;
} st_entity_names_t;


extern const st_entity_names_t idtech3_entity_names[];

extern const char *idtech3_entity_to_icon (const char *entity);


// 1 brush x 6 surfaces x 3 points x 3 top, east, south, bottom, west, north
//  {{1, 1, 1 }, {1, 0, 1 }, {0, 1, 1 }}, // top 
//  {{1, 1, 1 }, {0, 1, 1 }, {1, 1, 0 }}, // east 
//  {{1, 1, 1 }, {1, 1, 0 }, {1, 0, 1 }}, // south 
//  {{0, 0, 0 }, {1, 0, 0 }, {0, 1, 0 }}, // bottom
//  {{0, 0, 0 }, {0, 0, 1 }, {1, 0, 0 }}, // west  
//  {{0, 0, 0 }, {0, 1, 0 }, {0, 0, 1 }}  // north
#define IDTECH3_MAP_BLOCK \
  {{1, 1, 1 }, {1, 0, 1 }, {0, 1, 1 }}, \
  {{1, 1, 1 }, {0, 1, 1 }, {1, 1, 0 }}, \
  {{1, 1, 1 }, {1, 1, 0 }, {1, 0, 1 }}, \
  {{0, 0, 0 }, {1, 0, 0 }, {0, 1, 0 }}, \
  {{0, 0, 0 }, {0, 0, 1 }, {1, 0, 0 }}, \
  {{0, 0, 0 }, {0, 1, 0 }, {0, 0, 1 }}

typedef struct
{
  float b[6][3][3]; // brush[surfaces][points][xyz]
  char texture[64]; // texture is always scaled to fit the size of the surface
  float xscale;     // scale texture in X and Y axis
  float yscale;
  // TODO: calculate these values from the brush itself using a bounding box
  float xsize;
  float ysize;
  float zsize;
} st_map_brush_t;


/*
  id Tech 3 *.map layout

  { // idtech3_map_start()
    "classname" "worldspawn"

    // brush n
    {
      ...
    }
  } // idtech3_map_end()
  // entity n
  {
    ...
  }
*/  

// light types   
enum {
  IDTECH3_LIGHT_AMBIENT = 0,
  IDTECH3_LIGHT_RANDOM,
  IDTECH3_LIGHT_SUN
};

extern void idtech3_map_start (FILE *output_file, const char *message, int light_type, int *gravity, int *dust, int *breath, const char *music);
extern void idtech3_map_scale (st_map_brush_t *b, float x, float y, float z);
extern void idtech3_map_trans (st_map_brush_t *b, float x, float y, float z);
extern void idtech3_map_rot (st_map_brush_t *b, float x, float y, float z);
extern void idtech3_map_brush6 (FILE *output_file, const st_map_brush_t *b);
extern void idtech3_map_end (FILE *output_file);

extern void idtech3_map_light (FILE *output_file, float x, float y, float z, float brightness);

#endif  // IDTECH3_MAP_H
