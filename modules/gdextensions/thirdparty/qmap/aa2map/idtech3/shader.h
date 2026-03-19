/*
shader.c - id Tech 3 shader parser

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

#ifndef IDTECH3_SHADER_H
#define IDTECH3_SHADER_H

#include <stdio.h>

extern const char * strline (const char *s, int line); // returns line from a string with line-feeds

extern const char * idtech3_get_next_shader (FILE *shader_file);

extern void idtech3_shader_start (FILE *output_file);
extern void idtech3_shader (FILE *output_file, const char *name, const char *s);
extern void idtech3_shader6 (FILE *output_file, const char *name, const char *s);

#endif //  IDTECH3_SHADER_H
