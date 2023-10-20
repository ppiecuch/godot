/*
aa2map_misc.h - miscellaneous functions for aa2map 

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
#ifndef AA2MAP_MISC_H
#define AA2MAP_MISC_H


extern unsigned long time_ms (unsigned long *ms);


typedef struct
{
  int c;   // the ASCII character
  int id;  // the internal id
  const char *name_s;  // name_s is also used in the usage as description
} st_aa2map_object_t;


extern st_aa2map_object_t *aa2map_get_object_by_ascii (st_aa2map_t *aa2map, int c);

extern void aa2map_mazegen (st_aa2map_t *aa2map, int xsize, int ysize);
extern void aa2map_strafe (st_aa2map_t *aa2map, int pads, int gap_initial, int gap_increment, int height_increment);
extern void aa2map_run (st_aa2map_t *aa2map, int run_len, int checkpoints);
extern void aa2map_museum (st_aa2map_t *aa2map, const char *path);

extern int aa2map_compile (st_aa2map_t *aa2map);


#endif  // AA2MAP_MISC_H
