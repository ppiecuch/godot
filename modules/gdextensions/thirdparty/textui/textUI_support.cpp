// kate: replace-tabs on; tab-indents on; tab-width 4; indent-width 4; indent-mode cstyle;

#include "textUI_support.h"

#include <fcntl.h>

#include "core/math/vector2.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"

#if defined __MSDOS__ || defined __WIN32__ || defined _Windows
# define DIRSEP_CHAR '\\'
#elif defined macintosh /* only the original Macintosh uses ':', OSX uses the '/' */
# define DIRSEP_CHAR ':'
#else
# define DIRSEP_CHAR '/'
#endif

// BEGIN Host-system integration

BOOL system_keyhit(void) {
    return FALSE;
}

int system_getkey(void) {
    return 0;
}

int system_getshift(void) {
    return 0;
}

void system_resetmouse(void) {
}

int system_mousebuttons(void) {
    return 0;
}

static Point2 _pointer_position;

void system_get_mouseposition(int *x, int *y) {
    *x = _pointer_position.x;
    *y = _pointer_position.y;
}

int system_button_releases(void) {
    return 0;
}

// END Host-system integration

#include <ctype.h>

#ifndef _tolower
#  define _tolower(c) ((c) + 'a' - 'A')
#endif
#ifndef _toupper
#  define _toupper(c) ((c) + 'A' - 'a')
#endif

#ifndef S_ISDIR
#  define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#endif

/* MS-DOS flags equivalent to "dos.h" file attribute definitions */

#define DD_NORMAL   0x00    /* Normal file, no attributes */
#define DD_RDONLY   0x01    /* Read only attribute */
#define DD_HIDDEN   0x02    /* Hidden file */
#define DD_SYSTEM   0x04    /* System file */
#define DD_LABEL    0x08    /* Volume label */
#define DD_DIREC    0x10    /* Directory */
#define DD_ARCH	    0x20    /* Archive */
#define DD_DEVICE   0x40    /* Device */

#define DD_DOSATTRIBS 0x3f          /* DOS ATTRIBUTE MASK */

/*
 *  note that all DOS file attributes defined above do not overlap the
 *  DOS stat definitions, but will conflict will non-DOS machines, so
 *  use following macros to access the flags instead.
 */

#define DD_ISNORMAL(m)   ((m) & S_IFREG)
#ifdef MSDOS
#  define DD_ISRDONLY(m) ((m) & DD_RDONLY)
#  define DD_ISHIDDEN(m) ((m) & DD_HIDDEN)
#  define DD_ISSYSTEM(m) ((m) & DD_SYSTEM)
#  define DD_ISLABEL(m)  ((m) & DD_LABEL)
#  define DD_ISDIREC(m)  ((m) & (DD_DIREC | S_IFDIR))
#  define DD_ISARCH(m)   ((m) & DD_ARCH)
#else
#  define DD_ISRDONLY(m) !((m) & S_IWRITE)
#  define DD_ISHIDDEN(m) (0)
#  define DD_ISSYSTEM(m) (0)
#  define DD_ISLABEL(m)  (0)
#  define DD_ISDIREC(m)  ((m) & S_IFDIR)
#  define DD_ISARCH(m)   (0)
#endif /* ?MSDOS */

#ifdef UNIX
#  include <errno.h>
#  ifndef ENOENT
#    define ENOENT -1
#  endif
#  ifndef ENMFILE
#    define ENMFILE ENOENT
#  endif
#endif /* ?UNIX/VMS */

/* flags used by fnsplit */

#ifndef __TURBOC__
#  define WILDCARDS 0x01
#  define EXTENSION 0x02
#  define FILENAME  0x04
#  define DIRECTORY 0x08
#  define DRIVE     0x10
#endif

// copy_string - copies a string to another
static void copy_string(char *dst, const char *src, unsigned maxlen) {
    if (dst) {
        if (strlen(src) >= maxlen) {
            strncpy(dst, src, maxlen);
            dst[maxlen] = 0;
        } else {
            strcpy(dst, src);
        }
    }
}

// dot_found - checks for special directory names
static  int dot_found(char *pB) {
    if (*(pB-1) == '.') {
        pB--;
    }
    switch (*--pB) {
#ifdef MSDOS
    case ':':
        if (*(pB-2) != '\0') {
            break;
        }
    case '\\':
#endif
    case '/'  :
    case '\0' :
        return 1;
    }
    return 0;
}

/*---------------------------------------------------------------------*
Name            path_split - splits a full path name into its components

Description     path_split takes a file's full path name (path) as a
                string in the form

    /DIR/SUBDIR/FILENAME                (UNIX)
    X:\DIR\SUBDIR\NAME.EXT              (MS-DOS)

    and splits path into its four components. It then stores
    those components in the strings pointed to by dir and
    ext.  (Each component is required but can be a NULL,
    which means the corresponding component will be parsed
    but not stored.)

    The maximum sizes for these strings are given by the
    constants MAXDRIVE, MAXDIR, MAXPATH, MAXNAME and MAXEXT,
    (defined in dosdir.h) and each size includes space for
    the null-terminator.

    Constant        String

    DD_MAXPATH      path
    DD_MAXDRIVE     drive; includes colon; not used by UNIX
    DD_MAXDIR       dir; includes leading and trailing
                    backslashes for DOS or slashes for UNIX
    DD_MAXFILE      filename
    DD_MAXEXT       ext; includes leading dot (.)
                    (not used by UNIX)

    path_split assumes that there is enough space to store each
    non-NULL component. fnmerge assumes that there is enough
    space for the constructed path name. The maximum constructed
    length is DD_MAXPATH.

    When path_split splits path, it treats the punctuation as
    follows:

    * drive keeps the colon attached (C:, A:, etc.).
      It is not applicable to unix file system.

    * dir keeps the leading and trailing slashes
      (/usr/local/bin/, /src/, etc.)

    * ext keeps the dot preceding the extension (.c, .doc, etc.)
      It is not applicable to unix file system.

Return value    path_split returns an integer (composed of five flags,
        defined in dosdir.h) indicating which of the full path name
        components were present in path; these flags and the components
        they represent are:

        EXTENSION       an extension
        FILENAME        a filename
        DIRECTORY       a directory (and possibly sub-directories)
        DRIVE           a drive specification (see dir.h)
        WILDCARDS       wildcards (* or ? cards)

*---------------------------------------------------------------------*/

int path_split(const char *pathP, char *driveP, char *dirP, char *nameP, char *extP) {
    char buf[ DD_MAXPATH+2 ];

    /* Set all string to default value zero */
    int Ret = 0;
    if (driveP) *driveP = 0;
    if (dirP) *dirP = 0;
    if (nameP) *nameP = 0;
    if (extP) *extP = 0;

    /* Copy filename into template up to DD_MAXPATH characters */
    int Wrk;
    char *pB = buf;
    while (*pathP == ' ') pathP++;
    if ((Wrk = strlen(pathP)) > DD_MAXPATH)
        Wrk = DD_MAXPATH;
    *pB++ = 0;
    strncpy(pB, pathP, Wrk);
    *(pB += Wrk) = 0;

    /* Split the filename and fill corresponding nonzero pointers */
    Wrk = 0;
    for (; ; ) {
        switch (*--pB) {
        case '.':
            if (!Wrk && (*(pB+1) == '\0')) Wrk = dot_found(pB);
#ifdef MSDOS
            if ((!Wrk) && ((Ret & EXTENSION) == 0)) {
                Ret |= EXTENSION;
                copy_string(extP, pB, DD_MAXEXT - 1);
                *pB = 0;
            }
#endif
            continue;
#if defined(MSDOS)
        case ':'  :
            if (pB != &buf[2])
                continue;
#elif defined(UNIX)
        case '~' :
            if (pB != &buf[1])
                continue;
            else {
                /* expand path as appropriate */
                struct passwd *pw = NULL;
                char* tail = strchr (pB, '/');
                int len;
                if (tail != NULL) {
                    len = tail - (pB+1);
                    if (len > 0) {
                        char username[DD_MAXUSERNAME];
                        if (len <= DD_MAXUSERNAME) {
                            strncpy(username, pB+1, len);
                            username[len] = 0;
                            pw = getpwnam(username);
                        }
                    } else {
                        pw = getpwuid (getuid());
                    }
                    if (pw != NULL && (len=strlen(pw->pw_dir)) < DD_MAXDIR) {
                        strcpy(dirP, pw->pw_dir);
                        dirP += len;
                    } else
                        strcpy (dirP++, "?");
                    copy_string(dirP, tail, DD_MAXDIR - len - 1);
                    dirP += strlen(dirP);
                } else {
                    Wrk = 1;
                    if (pB[1] != 0)
                        pw = getpwnam (pB + 1);
                    else
                        pw = getpwuid (getuid());

                    if (pw != NULL && (len=strlen(pw->pw_dir)) < DD_MAXDIR) {
                        strcpy(dirP, pw->pw_dir);
                        dirP += len;
                    } else
                        strcpy (dirP++, "?");
                }
                *pB-- = 0;
                Ret |= DIRECTORY;
            }
#endif /* ?MSDOS */
        case '\0' :
            if (Wrk) {
                if (*++pB)
                    Ret |= DIRECTORY;
                copy_string(dirP, pB, DD_MAXDIR - 1);
#ifdef MSDOS
                *pB-- = 0;
#endif
                break;
            }
#ifdef MSDOS
        case '\\' :
#endif
#if (defined(MSDOS) || defined(UNIX))
        case '/':
#endif
            if (!Wrk) {
                Wrk++;
                if (*++pB) Ret |= FILENAME;
                copy_string(nameP, pB, DD_MAXFILE - 1);
                *pB-- = 0;
#ifdef MSDOS
                if (*pB == 0 || (*pB == ':' && pB == &buf[2]))
#else
                if (*pB == 0)
#endif
                break;
            }
            continue;
        case '[' :
        case '*' :
        case '?' :
            if (!Wrk) Ret |= WILDCARDS;
                default :
            continue;
        }
        break;
    }

#ifdef MSDOS
    if (*pB == ':') {
        if (buf[1]) Ret |= DRIVE;
        copy_string(driveP, &buf[1], DD_MAXDRIVE - 1);
    }
#endif

    return (Ret);
}

/*---------------------------------------------------------------------*
Name            path_merge - portable replacement for fnmerge(), _makepath(), etc

Description     forms a full DOS pathname from drive, path, file, and extension
                specifications.

Arguments:      1 - Buffer to receive full pathname
                2 - Drive
                3 - Path
                4 - Name
                5 - Extension

Returns:        nothing
*---------------------------------------------------------------------*/

#define LAST_CHAR(s) ((s)[strlen(s) - 1])

void path_merge(char *path, char *drive, char *dir, char *fname, char *ext) {
    *path = '\0';

    if (drive && *drive) {
        strcat(path, drive);
        if (':' != LAST_CHAR(path))
            strcat(path, ":");
    }

    if (dir && *dir) {
        strcat(path, dir);
        for (char *p = path; *p; ++p)
            if ('/' == *p)
                *p = '\\';
        if ('\\' != LAST_CHAR(path))
            strcat(path, "\\");
    }

    if (fname && *fname) {
        strcat(path, fname);

        if (ext && *ext) {
            if ('.' != *ext)
                strcat(path, ".");
            strcat(path, ext);
        }
    }
}


/*---------------------------------------------------------------------*

Name        IO file access layer.

*---------------------------------------------------------------------*/
enum ExModeFlags {
    EX_MODE_CREATE = 256, // highe values than FileAccess::ModeFlags
    EX_MODE_APPEND = 512,
};

struct _dd_ffblk {
    DirAccess *list_dir;
};

static DirAccess *_current_dir = nullptr;

// Convert file permissions

static int _conv_perm(const String &p_path, int mode) {
    int flags = 0;
    if (mode & O_RDONLY) {
        flags = FileAccess::READ;
    } else if (mode & O_WRONLY) {
        flags = FileAccess::WRITE;
    }

    if (mode & O_APPEND) {
        if (FileAccess::exists(p_path)) {
            flags = FileAccess::READ_WRITE | EX_MODE_APPEND;
        } else {
            flags = FileAccess::WRITE_READ;
        }
    }

    if (flags == 0) {
        flags = FileAccess::READ; // default
    }

    return flags;
}

static int _conv_perm(const String &p_path, const String &mode) {
#define check(C) (mode.has(C))
    // Convert permissions
    int flags = 0;
    if (check("r+"))
            flags = FileAccess::READ_WRITE;
    else if (check("w+"))
            flags = FileAccess::WRITE_READ;
    else if (check("r"))
            flags = FileAccess::READ;
    else if (check("w"))
            flags = FileAccess::WRITE;

    if (check("a")) {
        if (FileAccess::exists(p_path)) {
            flags = FileAccess::READ_WRITE | EX_MODE_APPEND;
        } else {
            flags = FileAccess::WRITE_READ;
        }
    }

    if (flags == 0)
        flags = FileAccess::READ; // default
#undef check
    return flags;
}

struct PYFILE {
    FileAccess *fa;
    static String fixpath(const String &p_path) {
        if (_current_dir && !p_path.is_abs_path() && !FileAccess::exists(p_path)) {
            return _current_dir->get_current_dir().append_path(p_path);
        }
        return p_path;
    }
    static PYFILE *fopen(const String &p_path, int p_mode_flags) {
        if (!p_path.empty()) {
            const String real_path = fixpath(p_path);
            const int flags = _conv_perm(real_path, p_mode_flags);
            if (FileAccess *_fa = FileAccess::open(real_path, flags&0xff)) {
                if (flags & EX_MODE_CREATE) {
                    _fa->seek_end();
                }
                return memnew(PYFILE(_fa));
            } else {
                return nullptr;
            }
        }
        return nullptr;
    }
    static PYFILE *fopen(const String &p_path, const String &p_mode_flags) {
        if (!p_path.empty()) {
            const String real_path = fixpath(p_path);
            const int flags = _conv_perm(real_path, p_mode_flags);
            if (FileAccess *_fa = FileAccess::open(real_path, flags&0xff)) {
                if (flags & EX_MODE_APPEND) {
                    _fa->seek_end();
                }
                return memnew(PYFILE(_fa));
            } else {
                return nullptr;
            }
        }
        return nullptr;
    }
    PYFILE(FileAccess *p_fa) { fa = p_fa; }
    PYFILE() { fa = nullptr; }
    ~PYFILE() {
        if (fa) {
            memdelete(fa);
        }
    }
};
