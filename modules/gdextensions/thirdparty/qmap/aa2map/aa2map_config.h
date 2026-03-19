#ifndef AA2MAP_CONFIG_H
#define AA2MAP_CONFIG_H

// Platform detection for aa2map running inside Godot Engine.
// Only enable features available on the host platform.

#if defined(_WIN32) || defined(_WIN64)
// Windows: no dirent.h or unistd.h in MSVC
#define HAVE_INTTYPES_H
#elif defined(__APPLE__) || defined(__linux__) || defined(__unix__)
#define HAVE_DIRENT_H
#define HAVE_UNISTD_H
#define HAVE_INTTYPES_H
#else
// Conservative fallback for retro/embedded platforms (3DS, PSP, Vita)
#define HAVE_INTTYPES_H
#endif

// Godot Engine logging: include after platform detection
#include "aa2map_godot.h"

#endif /* AA2MAP_CONFIG_H */
