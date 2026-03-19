/*
unzip2.h - even simpler zlib wrapper

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
#ifndef UNZIP2_H
#define UNZIP2_H
#include <sys/stat.h>
#include <time.h>
#include "misc/unzip.h"


#define UNZIP2_MAX_ENTRIES 32768
enum {
  UNZIP2_GZIP = 1,
  UNZIP2_PKZIP
};


typedef struct
{
  char name[FILENAME_MAX];
#if 0
  struct stat s;
#else
  mode_t mode;
  unsigned long size;
  time_t mtime;
#endif
  unsigned int crc;
} st_unzip2_entry_t;


typedef struct
{
  int type;    // UNZIP2_PKZIP or UNZIP2_GZIP
  unzFile file;

  int entries;
  int current; 
  st_unzip2_entry_t *entry[UNZIP2_MAX_ENTRIES];
} st_unzip2_t;


/*
  unzip2_open()     open a pkzip or gzip file and read the contents
  unzip2_close()    close a pkzip or gzip

  unzip2_zip()      adds the contents of dir (not dir itself) to zipfile
                     if (compress == 0) store only
                     compress can go up to 9 for best compression

  unzip2_unzip()    extract a file with name from the archive
                      if target == NULL the file will be extracted with its
                      original name into the current workdir
                      target can be e.g. the name of a temp file
                      if successful, the actual name of the file is returned
*/
extern st_unzip2_t *unzip2_open (const char *zipfile, const char *mode);
extern int unzip2_close (st_unzip2_t *z);


//extern int unzip2_zip (st_unzip2_t *z, const char *dir, int compress);
extern const char *unzip2_unzip (st_unzip2_t *z, const char *name, const char *target);


#endif // UNZIP2_H
