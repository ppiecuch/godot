#ifndef TEXTUI_SUPPORT_H
#define TEXTUI_SUPPORT_H

/* Get types and stat */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

#ifndef TRUE
# define TRUE (1)
#endif
#ifndef FALSE
# define FALSE (0)
#endif
#ifndef YES
# define YES TRUE
#endif
#ifndef NO
# define NO FALSE
#endif
#ifndef BOOL
# define BOOL bool
#endif

#if defined (MSDOS)
# define DD_MAXDRIVE   3
# ifndef __FLAT__
#  define DD_MAXPATH  80
#  define DD_MAXDIR   66
#  define DD_MAXFILE  16
#  define DD_MAXEXT   10 /* allow for wildcards .[ch]*, .etc */
# else
#  define DD_MAXPATH  260
#  define DD_MAXDIR   256
#  define DD_MAXFILE  256
#  define DD_MAXEXT   256
# endif /* ?__FLAT__ */
   typedef long    off_t;
# ifdef __TURBOC__
     typedef short   mode_t;
# else /* ?!__TURBOC__ */
     typedef unsigned short   mode_t;
# endif /* ?__TURBOC__ */
#else /* ?unix */
/* _MAX_PATH is sometimes called differently and it may be in limits.h or stdlib.h instead of stdio.h. */
# if !defined _MAX_PATH
/* not defined, perhaps stdio.h was not included */
#  if !defined PATH_MAX
#   include <stdio.h>
#  endif
#  if !defined _MAX_PATH && !defined PATH_MAX
/* no _MAX_PATH and no MAX_PATH, perhaps it is in limits.h */
#   include <limits.h>
#  endif
#  if !defined _MAX_PATH && !defined PATH_MAX
/* no _MAX_PATH and no MAX_PATH, perhaps it is in stdlib.h */
#   include <stdlib.h>
#  endif
/* if _MAX_PATH is undefined, try common alternative names */
#  if !defined _MAX_PATH
#   if defined MAX_PATH
#    define _MAX_PATH    MAX_PATH
#   elif defined _POSIX_PATH_MAX
#    define _MAX_PATH  _POSIX_PATH_MAX
#   else
/* everything failed, actually we have a problem here... */
#    define _MAX_PATH  1024
#   endif
#  endif
# endif
/* DD_MAXPATH defines the longest permissable path length,
 * including the terminating null. It should be set high
 * enough to allow all legitimate uses, but halt infinite loops
 * reasonably quickly. For now we realy on _MAX_PATH value */
#  define DD_MAXPATH    _MAX_PATH
#  define DD_MAXDRIVE   1
#  define DD_MAXDIR     768
#  define DD_MAXFILE    255
#  define DD_MAXEXT     1
   typedef struct dirent DIR_ENT;
#endif /* ?MSDOS */

#ifndef __TURBOC__
int getdisk(void);
int setdisk(int drive);
#endif /* ?!__TURBOC__ */

int path_split(const char *, char *, char *, char *, char *);
void path_merge(char *, char *, char *, char *, char *);

typedef struct dd_ffblk_tag {
    struct _dd_ffblk *data;
    int position;
} dd_ffblk;

int  dir_findfirst(const char *path, dd_ffblk *fb, int attrib);
int  dir_findnext(dd_ffblk *fb);
int  dir_getattrib(dd_ffblk *fb);
const char* dir_getname(dd_ffblk *fb);

typedef dd_ffblk ffblk;
#define FindFirst(A, B, C) dir_findfirst((A), &(C), (B))
#define FindNext(A)        dir_findnext(&(A))
#define AttribOf(ff)       dir_getattrib(&(ff))
#define NameOf(ff)         dir_getname(&(ff))

typedef struct GFILE GFILE;
GFILE *file_open(const char *name, const char *mode);
int file_stat(const char *path, struct stat *buf);
int file_fclose(GFILE *f);
int file_fseek(GFILE *f, off_t offset, int whence);
off_t file_ftell(GFILE *f);
ssize_t file_fread(void* buf, size_t len, size_t cnt, GFILE *f);
ssize_t file_fwrite(const void* buf, size_t len, size_t cnt, GFILE *f);

#define fnsplit path_split
#define fnmerge path_merge
#define fnstatat file_stat

void _dev_assert(BOOL cond);

BOOL system_keyhit(void);
int system_getkey(void); /* read a keystroke */
int system_getshift(void); /* read the keyboard shift status */
void system_resetmouse(void); /* reset the mouse */
int system_mousebuttons(void); /* return true if mouse buttons are pressed */
void system_get_mouseposition(int *x, int *y); /* return mouse coordinates */
int system_button_releases(void); /* return true if a mouse button has been released */

#endif // TEXTUI_SUPPORT_H
