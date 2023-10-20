/*
aa2map_parse.h - ASCII parser for aa2map 

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
#ifndef AA2MAP_PARSE_H 
#define AA2MAP_PARSE_H


typedef struct
{
  int id;             // id of object
  const char *name_s; // name of object

  char cube[27]; // 3x9
/*
  top 0  1  2  center  9 10 11  bottom 18 19 20
      3  4  5         12(13)14         21 22 23
      6  7  8         15 16 17         24 25 26

  cube[13] is the ASCII code of the object
*/

  int xscale; // --scale 
  int yscale;
  int zscale;

  int x;
  int y;
  int z;

  int angle;

  const char *map_name; // used for texture and shader path
} st_aa2map_parse_t;


extern st_aa2map_parse_t * aa2map_parse (st_aa2map_t *aa2map);


extern void aa2map_add_ascii (st_aa2map_t *aa2map, st_aa2map_parse_t *a);


#endif  // AA2MAP_PARSE_H
