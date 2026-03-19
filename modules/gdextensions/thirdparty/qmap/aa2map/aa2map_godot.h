/*
aa2map_godot.h - Godot Engine logging for aa2map

Provides aa2map_log_error() / aa2map_log_info() replacements for fprintf(stderr).
When AA2MAP_GODOT_EMBEDDED is defined, these route through Godot's print_error/print_line.
Otherwise they fall back to fprintf(stderr).
*/
#ifndef AA2MAP_GODOT_H
#define AA2MAP_GODOT_H

#include <stdio.h>
#include <stdarg.h>

#ifdef AA2MAP_GODOT_EMBEDDED
/* Implemented in gd_qmaps.cpp — routes to Godot's print_error() */
extern void aa2map_godot_log_error(const char *fmt, ...);
extern void aa2map_godot_log_info(const char *fmt, ...);
#define AA2MAP_LOG_ERROR(...) aa2map_godot_log_error(__VA_ARGS__)
#define AA2MAP_LOG_INFO(...) aa2map_godot_log_info(__VA_ARGS__)
#else
#define AA2MAP_LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)
#define AA2MAP_LOG_INFO(...) fprintf(stderr, __VA_ARGS__)
#endif

#endif /* AA2MAP_GODOT_H */
