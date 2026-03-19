/*
unzip2.c - even simpler zlib wrapper

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
#include "aa2map_config.h"                             // USE_ZLIB

#ifdef  USE_ZLIB
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "unzip.h"
#include "unzip2.h" 

#ifndef MAXBUFSIZE
#define MAXBUFSIZE 32768
#endif


static int
unzip2_demux (const char *filename)
{
  FILE *file;
  unsigned char magic[4] = { 0 };

  if ((file = fopen (filename, "rb")) == NULL)
    return -1;

  fread (magic, 1, sizeof (magic), file);
  fclose (file);

  if (magic[0] == 0x1f &&
      magic[1] == 0x8b &&
      magic[2] == 0x08) // ID1, ID2 and CM. gzip uses Compression Method 8
    return UNZIP2_GZIP;
  else if (magic[0] == 'P' &&
           magic[1] == 'K' &&
           magic[2] == 0x03 &&
           magic[3] == 0x04)
    return UNZIP2_PKZIP;

  return 0;
}


static int
unzip2_goto_file (unzFile file, int file_index)
{
  int retval = unzGoToFirstFile (file), n = 0;

  if (file_index > 0)
    while (n < file_index)
      {
        retval = unzGoToNextFile (file);
        n++;
      }
  return retval;
}


st_unzip2_t *
unzip2_open (const char *zipfile, const char *mode)
{
  (void) mode; // read-only for now
  int i = 0;
  static st_unzip2_t z;

  memset (&z, 0, sizeof (st_unzip2_t));

  z.type = unzip2_demux (zipfile);

  if (!z.type) // not a zip
    {
      AA2MAP_LOG_ERROR ( "ERROR: unzip2_demux() failed (%s)\n", zipfile);
      return NULL; 
    }

  if (z.type == UNZIP2_GZIP)
    z.file = gzopen (zipfile, "rb");
  else if (z.type == UNZIP2_PKZIP)  
    z.file = unzOpen (zipfile);

  if (!z.file)
    {
      AA2MAP_LOG_ERROR ( "ERROR: unzip2_open() failed (%s)\n", zipfile);
      return NULL;
    }

  if (z.type == UNZIP2_GZIP)
    {
      int size = 0;
#if 1
      // This is not much faster than the other method
      while (!gzeof (z.file))
        gzseek (z.file, 1024 * 1024, SEEK_CUR);
      size = gztell (z.file);
#else
      // Is there a more efficient way to determine the uncompressed size?
      while ((bytesread = gzread (z.file, buf, MAXBUFSIZE)) > 0)
        size += bytesread;
#endif
//      printf ("%s %d %d %x\n", name, size, info.dosDate, info.crc);
    }
  else if (z.type == UNZIP2_PKZIP)
    {
      unz_global_info ginfo;

      if (unzGetGlobalInfo (z.file, &ginfo) != UNZ_OK)
        return NULL;

      z.entries = ginfo.number_entry;
      for (i = 0; i < z.entries && i < UNZIP2_MAX_ENTRIES; i++)
        {
          unz_file_info info;
          char name[FILENAME_MAX];

          unzip2_goto_file (z.file, i);
          unzGetCurrentFileInfo (z.file, &info, name, FILENAME_MAX, NULL, 0, NULL, 0);

          if (!(z.entry[i] = (st_unzip2_entry_t *) malloc (sizeof (st_unzip2_entry_t))))
            return NULL;

          strncpy (z.entry[i]->name, name, FILENAME_MAX)[FILENAME_MAX - 1] = 0;
          z.entry[i]->size = info.uncompressed_size;
          z.entry[i]->crc = info.crc;
          z.entry[i]->mtime = info.dosDate;
//          z.entry[i]->mode = ;

#ifdef  DEBUG
          printf ("%s %d %d %x\n", z.entry[i]->name, z.entry[i]->size, z.entry[i]->mtime, z.entry[i]->crc);
          fflush (stdout);
#endif
        }
    }

  return &z;
}


int
unzip2_close (st_unzip2_t *z)
{
  int i = 0;

  if (z->type == UNZIP2_GZIP)
    {
      gzclose (z->file);
    }
  else if (z->type == UNZIP2_PKZIP)
    {
      unzClose (z->file);
    }

  for (; i < z->entries && i < UNZIP2_MAX_ENTRIES; i++)
    {
      free (z->entry[i]);
      z->entry[i] = NULL;
    }

  return 0;
}


static size_t
unzip2_read (void *buffer, size_t size, size_t number, st_unzip2_t *z)
{  
  if (size == 0 || number == 0)
    return 0;

  if (z->type == UNZIP2_GZIP)
    {
      int n = gzread (z->file, buffer, number * size);
      return n / size;
    }
  else if (z->type == UNZIP2_PKZIP)
    {
      int n = unzReadCurrentFile (z->file, buffer, number * size);
      return n / size;
    }
  return 0;
}


const char *
unzip2_unzip (st_unzip2_t *z, const char *name, const char *target)
{
  int i = 0;
  int found = 0;
  int result = 0;
  char buf[MAXBUFSIZE];
  FILE *fh = NULL;
  char tname[FILENAME_MAX];

  for (; i < z->entries && i < UNZIP2_MAX_ENTRIES; i++)
    if (!strcmp (z->entry[i]->name, name))
      {
        found = 1;
        break;
      }

  if (!found)
    return NULL;

  strncpy (tname, target ? target : z->entry[i]->name, FILENAME_MAX)[FILENAME_MAX - 1] = 0;

  if (!(fh = fopen (tname, "wb")))
    {
      AA2MAP_LOG_ERROR ( "ERROR: unzip2_unzip() could not open %s for writing\n", tname);
      return NULL;
    }

  unzip2_goto_file (z->file, i);
  unzOpenCurrentFile (z->file);

  while ((result = unzip2_read (buf, 1, MAXBUFSIZE, z)) > 0)
    fwrite (buf, 1, result, fh);

  unzCloseCurrentFile (z->file);

  fclose (fh);

  return target ? target : z->entry[i]->name;
}

#endif // USE_ZLIB
