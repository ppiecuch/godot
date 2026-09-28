#ifndef TEXTUI_SUPPORT_H
#define TEXTUI_SUPPORT_H

/* Get types and stat */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

/* Bug fix: textUI.c (a plain C translation unit) calls the functions
** declared below expecting plain C linkage/symbol names (e.g. the
** exported symbol "_system_getkey"). Without this guard,
** textUI_support.cpp (compiled as C++) mangles them instead (e.g.
** "__Z13system_getkeyv"), so the two translation units silently disagree
** on every symbol in this header -- confirmed with `nm` on the current
** .o files: textUI.c.o references undefined "_system_getkey" while
** textUI_support.cpp.o only exports the mangled "__Z13system_getkeyv".
** The "textui" Godot module fails to link with "undefined symbol" for
** every system_*()/file/dir helper the instant it's actually built
** (is_enabled("textui")) -- presumably never caught because nothing has
** linked this module yet. Mirrors the extern "C" guard textUI.h already
** has. */
#ifdef __cplusplus
extern "C" {
#endif

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
# define DF_MAXDRIVE   3
# ifndef __FLAT__
#  define DF_MAXPATH  80
#  define DF_MAXDIR   66
#  define DF_MAXFILE  16
#  define DF_MAXEXT   10 /* allow for wildcards .[ch]*, .etc */
# else
#  define DF_MAXPATH  260
#  define DF_MAXDIR   256
#  define DF_MAXFILE  256
#  define DF_MAXEXT   256
# endif /* ?__FLAT__ */
   typedef long off_t;
# ifdef __TURBOC__
     typedef short mode_t;
# else /* ?!__TURBOC__ */
     typedef unsigned short mode_t;
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
#  define DF_MAXPATH    _MAX_PATH
#  define DF_MAXDRIVE   1
#  define DF_MAXDIR     768
#  define DF_MAXFILE    255
#  define DF_MAXEXT     1
   typedef struct dirent DIR_ENT;
#endif /* ?MSDOS */

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
const char* dir_getname(const dir_ffblk *fb);

#define FindFirst(A, B, C) dir_findfirst((A), &(C), (B))
#define FindNext(A)        dir_findnext(&(A))
#define AttribOf(ff)       dir_getattrib(&(ff))
#define NameOf(ff)         dir_getname(&(ff))

#define _A_NORMAL 0x00

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

/* ---------------------------------------------------------------------
** Godot integration surface.
**
** The functions above are polled *from* textUI.c (keyhit()/getkey()/
** getshift()/mousebuttons()/get_mouseposition()/button_releases(), see
** textUI.c's "console.c"/"mouse.c" sections) every dispatch_message()
** call. They read from the small event state kept in textUI_support.cpp,
** which a Godot host (e.g. scene/debugconsole/'s ConsoleInstance, or any
** other Control/CanvasItem) feeds by calling the functions below from its
** own _gui_input()/_input() handler. Nothing in textUI.c/textUI.h needs
** to know Godot exists; nothing here needs to know about WINDOW/MESSAGE.
**
** Shift-mask bits match RIGHTSHIFT/LEFTSHIFT/CTRLKEY/ALTKEY (textUI.h);
** mouse masks match the bit0=left/bit1=right convention of
** mousebuttons()/leftbutton()/rightbutton() (also textUI.h). Screen
** coordinates are in character cells, matching PutWindowChar/
** MouseWindow()'s (x,y), not pixels.
** --------------------------------------------------------------------- */

/* Sets the *live* modifier-key state (RIGHTSHIFT|LEFTSHIFT|CTRLKEY|ALTKEY),
** queried back by system_getshift(). Call this every time a modifier key
** (or a key combined with one) changes state -- in particular, on every
** InputEventKey, not just on the modifier keys themselves, so a keystroke
** queued by TextUI_FeedKey() right after this call is seen by textUI.c's
** collect_events() with the correct shift state already in effect (it
** polls getshift() and keyhit()/getkey() together, in that order, once
** per dispatch_message() call). */
void TextUI_SetShiftState(int shift_mask);

/* Queues one keystroke for system_getkey()/system_keyhit(). `code` is
** either a plain ASCII character (e.g. 'f') -- including for Alt+letter
** combinations, exactly like a real keyboard/terminal driver would
** report, relying on TextUI_SetShiftState()'s ALTKEY bit plus textUI.c's
** own AltConvert() to recognize the shortcut -- or an extended/function
** key code already in DFlat's own FKEY-offset encoding (see textUI.h's
** HOME/UP/DN/DEL/F1../ALT_A.. macros), for keys with no ASCII
** representation at all (arrows, function keys, Home/End, ...). Silently
** drops the keystroke if the internal queue is full; real typing is far
** slower than this queue drains. */
void TextUI_FeedKey(int code);

/* Reports a mouse button transition. `button_mask` is 1 for the left
** button, 2 for the right button (matching mousebuttons()'s bit
** layout); `pressed` is true on button-down, false on button-up. Button
** state is level-tracked (system_mousebuttons() reports "currently
** held"); releases are additionally latched until the next
** system_button_releases() call, matching what collect_events() expects
** (it polls button_releases() as a one-shot "was released since I last
** asked" flag, separately from the held-state mask). */
void TextUI_FeedMouseButton(int button_mask, BOOL pressed);

/* Updates the mouse position returned by system_get_mouseposition(), in
** character cells (not pixels) -- convert from the Control's local pixel
** position using its cell size before calling this. */
void TextUI_FeedMousePosition(int x, int y);

#ifndef _MSC_VER
# define stricmp strcasecmp
#endif

#ifdef __cplusplus
}
#endif

#endif // TEXTUI_SUPPORT_H
