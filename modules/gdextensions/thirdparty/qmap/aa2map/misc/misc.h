/*
misc.h - miscellaneous functions

Copyright (c) 1999 - 2008 NoisyB
Copyright (c) 2001 - 2005 dbjh


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
#ifndef MISC_H
#define MISC_H


/*
  tmpnam3()   replacement for tmpnam() temp must have the size of FILENAME_MAX
  bytes_per_second() returns bytes per second (useful in combination with
                    gauge())
  misc_percent()  returns percentage of progress (useful in combination with
                    gauge())
  wait2()         wait (sleep) a specified number of milliseconds
  getenv2()       getenv() clone for enviroments w/o HOME, TMP or TEMP variables
  strptime2()     parse dates with different formats to time_t
*/
extern char *tmpnam3 (char *temp, int dir);
extern int bytes_per_second (time_t start_time, int nbytes);
extern int misc_percent (unsigned long pos, unsigned long len);
extern void wait2 (int nmillis);
extern char *getenv2 (const char *variable);
extern int misc_digits (unsigned long value);
extern time_t strptime2 (const char *s);


#endif // MISC_H
