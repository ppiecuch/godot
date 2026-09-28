#ifndef TEXTUI_SUPPORT_H
#define TEXTUI_SUPPORT_H

/* Get types and stat */
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif
#ifndef YES
#define YES TRUE
#endif
#ifndef NO
#define NO FALSE
#endif
#ifndef BOOL
#define BOOL bool
#endif

#if defined(MSDOS)
#define DF_MAXDRIVE 3
#ifndef __FLAT__
#define DF_MAXPATH 80
#define DF_MAXDIR 66
#define DF_MAXFILE 16
#define DF_MAXEXT 10 /* allow for wildcards .[ch]*, .etc */
#else
#define DF_MAXPATH 260
#define DF_MAXDIR 256
#define DF_MAXFILE 256
#define DF_MAXEXT 256
#endif /* ?__FLAT__ */
typedef long off_t;
#ifdef __TURBOC__
typedef short mode_t;
#else /* ?!__TURBOC__ */
typedef unsigned short mode_t;
#endif /* ?__TURBOC__ */
#else /* ?unix */
/* _MAX_PATH is sometimes called differently and it may be in limits.h or stdlib.h instead of stdio.h. */
#if !defined _MAX_PATH
/* not defined, perhaps stdio.h was not included */
#if !defined PATH_MAX
#include <stdio.h>
#endif
#if !defined _MAX_PATH && !defined PATH_MAX
/* no _MAX_PATH and no MAX_PATH, perhaps it is in limits.h */
#include <limits.h>
#endif
#if !defined _MAX_PATH && !defined PATH_MAX
/* no _MAX_PATH and no MAX_PATH, perhaps it is in stdlib.h */
#include <stdlib.h>
#endif
/* if _MAX_PATH is undefined, try common alternative names */
#if !defined _MAX_PATH
#if defined MAX_PATH
#define _MAX_PATH MAX_PATH
#elif defined _POSIX_PATH_MAX
#define _MAX_PATH _POSIX_PATH_MAX
#else
/* everything failed, actually we have a problem here... */
#define _MAX_PATH 1024
#endif
#endif
#endif
/* DD_MAXPATH defines the longest permissable path length,
 * including the terminating null. It should be set high
 * enough to allow all legitimate uses, but halt infinite loops
 * reasonably quickly. For now we realy on _MAX_PATH value */
#define DF_MAXPATH _MAX_PATH
#define DF_MAXDRIVE 1
#define DF_MAXDIR 768
#define DF_MAXFILE 255
#define DF_MAXEXT 1
typedef struct dirent DIR_ENT;
#endif /* ?MSDOS */

/* ---------------------------------------------------------------------
** Portable DOS-style path/drive/directory-enumeration shim, implemented
** in textUI_support.c with no dependency on Godot: getdisk()/setdisk(),
** path_split()/path_merge() (fnsplit/fnmerge), and the FindFirst/
** FindNext/AttribOf/NameOf dir_ffblk family. Called directly by
** textUI.c/textUI_edit.c.
** --------------------------------------------------------------------- */

#ifndef __TURBOC__
int getdisk(void);
int setdisk(int drive);
#endif /* ?!__TURBOC__ */

int path_split(const char *, char *, char *, char *, char *);
void path_merge(char *, char *, char *, char *, char *);

typedef struct _dir_ffblk {
	struct _ffblk *data;
	int position;
} dir_ffblk;

int dir_findfirst(const char *path, dir_ffblk *fb, int attrib);
int dir_findnext(dir_ffblk *fb);
int dir_getattrib(const dir_ffblk *fb);
const char *dir_getname(const dir_ffblk *fb);

#define FindFirst(A, B, C) dir_findfirst((A), &(C), (B))
#define FindNext(A) dir_findnext(&(A))
#define AttribOf(ff) dir_getattrib(&(ff))
#define NameOf(ff) dir_getname(&(ff))

#define _A_NORMAL 0x00

#define fnsplit path_split
#define fnmerge path_merge

/* file_stat() (via fnstatat()) backs _findnext()'s S_ISDIR() check, itself built on plain
** POSIX opendir()/readdir_r() (textUI.c) rather than any Godot API, so it's a plain stat()
** wrapper here, not a Godot bridge -- unlike the real Godot file/event glue (TextUI_Feed*,
** GFILE, ...) declared in scene/debugconsole/textUI_support.cpp, which textUI.c never calls
** directly and so doesn't need to live in this vendor-facing header. */
int file_stat(const char *path, struct stat *buf);
#define fnstatat file_stat

void _dev_assert(BOOL cond);

BOOL system_keyhit(void);
int system_getkey(void); /* read a keystroke */
int system_getshift(void); /* read the keyboard shift status */
void system_resetmouse(void); /* reset the mouse */
int system_mousebuttons(void); /* return true if mouse buttons are pressed */
void system_get_mouseposition(int *x, int *y); /* return mouse coordinates */
int system_button_releases(void); /* return true if a mouse button has been released */

#ifndef _MSC_VER
#define stricmp strcasecmp
#endif

#ifdef __cplusplus
}
#endif

#endif // TEXTUI_SUPPORT_H
