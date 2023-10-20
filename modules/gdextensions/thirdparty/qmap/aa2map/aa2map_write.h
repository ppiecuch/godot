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
#ifndef AA2MAP_WRITE_H
#define AA2MAP_WRITE_H


extern void aa2map_map_write (st_aa2map_t *aa2map, const st_aa2map_parse_t *a);
extern void aa2map_shader_write (st_aa2map_t *aa2map, const st_aa2map_parse_t *a);


#endif  // AA2MAP_WRITE_H
