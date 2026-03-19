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

#include "aa2map_config.h"
#include "misc/string.h"
#include "shader.h"

#include <stdio.h>
#include <string.h>

#define MAXBUFSIZE 32768

const char *
strline (const char *s, int line)
{
  int l = 0;
  static char buf[MAXBUFSIZE];
  char *p = NULL;

  if (!s) 
    return NULL;

  if (!(*s))
    return NULL;

  for (l = 0; l < line; l++)
    {
      if (!(s = strchr (s, '\n')))
        return NULL;
      s++;
    }

  if (l < line)
    return NULL;

  strncpy (buf, s, MAXBUFSIZE)[MAXBUFSIZE - 1] = 0;
  if ((p = strchr (buf, '\n')))
    *p = 0;

#ifdef  DEBUG
  printf ("[%s]\n", buf);
  fflush (stdout);
#endif

  return buf;
}


const char *
idtech3_get_next_shader (FILE * shader_file)
{
  char buf[MAXBUFSIZE];
  static char shader[MAXBUFSIZE * 2]; // 64k
  int obrack = 0;
  int found = 0;
  char *p = NULL;

  *shader = 0;
  while (fgets (buf, MAXBUFSIZE, shader_file))
    {
      // sanitize line
      if ((p = strpbrk (buf, "\n")))
        *p = 0;

//      strrep (buf, "\r", "\n");
      strrep (buf, "\r", "");

#if 0
      // replace wrong backslashes with slashes
      p = buf;
      while ((p = strchr (p, '\\')))
        {
          if (*(p + 1) != 'n' &&
              *(p + 1) != '"')
            *p = '/';
          p++;
        }
#endif

      if (!strncmp (buf, "//", 2))
        *buf = 0;
      p = strstr (buf, "//");
      if (p)
        *p = 0;

      p = strtriml (strtrimr (buf));
      if (!(*p))
        continue;

      // count brakets
      p = buf;
      while ((p = strchr (p, '{')))
        {
          obrack++;
          found = 1;
          p++;
        }

      sprintf (strchr (shader, 0), "%s\n", buf);

      p = buf;
      while ((p = strchr (p, '}')))
        {
          obrack--;
          p++;
        }

      if (!obrack && found)
        break;
    }

  if (*shader)
    { 
#ifdef  DEBUG
      printf ("[%s]\n", shader);
      fflush (stdout);
#endif
      return shader;
    }

  return NULL;
}


void
idtech3_shader_start (FILE *output_file)
{
  (void) output_file;
}


void
idtech3_shader (FILE *output_file, const char *name, const char *s)
{
  fprintf (output_file, "%s\n%s\n", name, s);
}


void
idtech3_shader6 (FILE *output_file, const char *name, const char *s)
{
  int i = 0;
  const char *suffix_s[6] = {"_t", "_s", "_e", "_b", "_n", "_w"};

  for (; i < 6; i++)
    fprintf (output_file, "%s%s\n%s\n", name, suffix_s[i], s);
}
