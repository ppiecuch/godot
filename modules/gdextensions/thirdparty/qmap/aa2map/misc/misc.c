/*
misc.c - miscellaneous functions

Copyright (c) 1999 - 2005 NoisyB
Copyright (c) 2001 - 2005 dbjh
Copyright (c) 2002 - 2004 Jan-Erik Karlsson (Amiga code)


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
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef  HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>                             // va_arg()
#include <sys/stat.h>                           // for S_IFLNK
#include "defines.h"
#ifdef  HAVE_INTTYPES_H
#include "inttypes.h"
#else
#include "itypes.h"
#endif
#include "misc.h"


#ifdef  MAXBUFSIZE
#undef  MAXBUFSIZE
#endif  // MAXBUFSIZE
#define MAXBUFSIZE 32768


#ifndef _WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif


int
misc_digits (unsigned long v)
{
  int ret = 1;

  if ( v >= 100000000 ) { ret += 8; v /= 100000000; }
  if ( v >=     10000 ) { ret += 4; v /=     10000; }
  if ( v >=       100 ) { ret += 2; v /=       100; }
  if ( v >=        10 ) { ret += 1;                 }

  return ret;
}


int
bytes_per_second (time_t start_time, int nbytes)
{
  int curr = time (NULL) - start_time;

  if (curr < 1)
    curr = 1;                                   // "round up" to at least 1 sec (no division
                                                //  by zero below)
  return nbytes / curr;                         // # bytes/second (average transfer speed)
}


int
misc_percent (unsigned long pos, unsigned long len)
{
  if (len < 1)
    len = 1;

  return (int) ((((int64_t) 100) * pos) / len);
}


void
wait2 (int nmillis)
{
#ifdef  __MSDOS__
  delay (nmillis);
#elif   defined __unix__ || defined __APPLE__   // Mac OS X actually
  usleep (nmillis * 1000);
#elif   defined __BEOS__
  snooze (nmillis * 1000);
#elif   defined AMIGA
  Delay (nmillis * 1000);
#elif   defined _WIN32
  Sleep (nmillis);
#else
#ifdef  __GNUC__
#warning Please provide a wait2() implementation
#else
#pragma message ("Please provide a wait2() implementation")
#endif
  volatile int n;
  for (n = 0; n < nmillis * 65536; n++)
    ;
#endif
}


char *
tmpnam3 (char *temp, int dir)
{
  char *t = NULL, *p = NULL;

  if (!temp)
    return NULL;
  
  t = getenv2 ("TEMP");

  if (!(p = malloc (strlen (t) + strlen (temp) + 12)))
    return NULL;

  sprintf (p, "%s" FILE_SEPARATOR_S "%st_XXXXXX", t, temp);
  strcpy (temp, p);
  free (p);

  if (!dir)
    if (mkstemp (temp) != -1)
      return temp;

  if (dir)
    if (mkdtemp (temp))
      return temp;

  return NULL;
}


time_t
strptime2 (const char *s)
{
  int i = 0;
  char y[100], m[100], d[100];
  char h[100], min[100];
//  char sec[100];
  struct tm time_tag;
  time_t t = time (0);
  const char *month_s[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", NULL};

  *y = *m = *d = *h = *min = 0;

  if (s[10] == 'T')                     // YYYY-MM-DDT00:00+00:00
    {
      sscanf (s, " %4s-%2s-%2sT%2s:%2s", y, m, d, h, min);
    }
  else if (s[3] == ',' && s[4] == ' ')  // Mon, 31 Jul 2006 15:05:00 GMT
    {
      sscanf (s + 5, "%2s %s %4s %2s:%2s", d, m, y, h, min);

      for (i = 0; month_s[i]; i++)
        if (!stricmp (m, month_s[i]))
          {
            sprintf (m, "%d", i + 1);
            break;
          }
    }
  else if (s[4] == '-' && s[7] == '-')  // 2006-07-19
    {
      sscanf (s, "%4s-%2s-%2s", y, m, d);
    }
  else                                  // YYYYMMDDTHHMMSS
    {
//      sscanf (s, " %4s%2s%2sT", y, m, d);
    }

  memset (&time_tag, 0, sizeof (struct tm));

  if (*y)
    time_tag.tm_year = strtol (y, NULL, 10) - 1900;
  if (*m)
    time_tag.tm_mon = strtol (m, NULL, 10) - 1;
  if (*d)
    time_tag.tm_mday = strtol (d, NULL, 10);
  if (*h)
    time_tag.tm_hour = strtol (h, NULL, 10);
  if (*min)
    time_tag.tm_min = strtol (min, NULL, 10);

  t = mktime (&time_tag);

  return t;
}


char *
getenv2 (const char *variable)
/*
  getenv() suitable for enviroments w/o HOME, TMP or TEMP variables.
  The caller should copy the returned string to it's own memory, because this
  function will overwrite that memory on the next call.
  Note that this function never returns NULL.
*/
{
  char *tmp;
  static char value[MAXBUFSIZE];
#if     defined __CYGWIN__ || defined __MSDOS__
/*
  Under DOS and Windows the environment variables are not stored in a case
  sensitive manner. The run-time systems of DJGPP and Cygwin act as if they are
  stored in upper case. Their getenv() however *is* case sensitive. We fix this
  by changing all characters of the search string (variable) to upper case.

  Note that under Cygwin's Bash environment variables *are* stored in a case
  sensitive manner.
*/
  char tmp2[MAXBUFSIZE];

  strcpy (tmp2, variable);
  variable = strupr (tmp2);                     // DON'T copy the string into variable
#endif                                          //  (variable itself is local)

  *value = 0;

  if ((tmp = getenv (variable)) != NULL)
    strcpy (value, tmp);
  else
    {
      if (!strcmp (variable, "HOME"))
        {
          if ((tmp = getenv ("USERPROFILE")) != NULL)
            strcpy (value, tmp);
          else if ((tmp = getenv ("HOMEDRIVE")) != NULL)
            {
              strcpy (value, tmp);
              tmp = getenv ("HOMEPATH");
              strcat (value, tmp ? tmp : FILE_SEPARATOR_S);
            }
          else
            /*
              Don't just use C:\\ under DOS, the user might not have write access
              there (Windows NT DOS-box). Besides, it would make uCON64 behave
              differently on DOS than on the other platforms.
              Returning the current directory when none of the above environment
              variables are set can be seen as a feature. A frontend could execute
              uCON64 with an environment without any of the environment variables
              set, so that the directory from where uCON64 starts will be used.
            */
            {
              char c;
              getcwd (value, FILENAME_MAX);
              c = toupper (*value);
              // if current dir is root dir strip problematic ending slash (DJGPP)
              if (c >= 'A' && c <= 'Z' &&
                  value[1] == ':' && value[2] == '/' && value[3] == 0)
                value[2] = 0;
            }
         }

      if (!strcmp (variable, "TEMP") || !strcmp (variable, "TMP"))
        {
#if     defined __MSDOS__ || defined __CYGWIN__
          /*
            DJGPP and (yet another) Cygwin quirck
            A trailing backslash is used to check for a directory. Normally
            DJGPP's run-time system is able to handle forward slashes in paths,
            but access() won't differentiate between files and dirs if a
            forward slash is used. Cygwin's run-time system seems to handle
            paths with forward slashes quite different from paths with
            backslashes. This trick seems to work only if a backslash is used.
          */
          if (access ("\\tmp\\", R_OK | W_OK) == 0)
#else
          // trailing file separator to force it to be a directory
          if (access (FILE_SEPARATOR_S"tmp"FILE_SEPARATOR_S, R_OK | W_OK) == 0)
#endif
            strcpy (value, FILE_SEPARATOR_S"tmp");
          else
            getcwd (value, FILENAME_MAX);
        }
    }

#ifdef  __CYGWIN__
  /*
    Under certain circumstances Cygwin's run-time system returns "/" as value
    of HOME while that var has not been set. To specify a root dir a path like
    /cygdrive/<drive letter> or simply a drive letter should be used.
  */
  if (!strcmp (variable, "HOME") && !strcmp (value, "/"))
    getcwd (value, FILENAME_MAX);

  return fix_character_set (value);
#else
  return value;
#endif
}
