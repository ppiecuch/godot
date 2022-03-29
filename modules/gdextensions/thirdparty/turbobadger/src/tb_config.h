// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#ifndef TB_CONFIG_H
#define TB_CONFIG_H

// Enable for some handy runtime debugging, enabled by modifying
// the various settings in g_tb_debug. A settings window can be
// shown by calling ShowDebugInfoSettingsWindow. */
#if !defined(NDEBUG) || defined(DEBUG_ENABLED)
#define TB_RUNTIME_DEBUG_INFO
#endif

// Enable if the focus state should automatically be set on edit fields even
// when using the pointer. It is normally set only while moving focus by keyboard.
// #define TB_ALWAYS_SHOW_EDIT_FOCUS

// Enable to use premultiplied alpha. Warning: This is not handled everywhere in
// the default backends, so consider it an experimental and unfinished feature!
// #define TB_PREMULTIPLIED_ALPHA

// Enable to support TBBF fonts (Turbo Badger Bitmap Fonts)
#define TB_FONT_RENDERER_TBBF

// Enable to get TBRendererBatcher, an helper class for renderers that
// implements batching of draw operations. Subclasses of TBRendererBatcher
// can be done super easily, and still do batching.
#define TB_RENDERER_BATCHER

// The width of the font glyph cache. Must be a power of two.
#define TB_GLYPH_CACHE_WIDTH 512

// The height of the font glyph cache. Must be a power of two.
#define TB_GLYPH_CACHE_HEIGHT 512

// == Optional features ===========================================================

// Enable support for TBImage, TBImageManager, TBImageWidget.
#define TB_IMAGE

// == Setting some defaults for platform implementations ==========================

#if defined(ANDROID) || defined(__ANDROID__)
#define TB_SYSTEM_ANDROID
#define TB_CLIPBOARD_DUMMY
#elif defined(__linux) || defined(__linux__)
#define TB_FILE_POSIX
#define TB_TARGET_LINUX
#define TB_SYSTEM_LINUX
#define TB_CLIPBOARD_GD
#elif MACOSX
#define TB_FILE_POSIX
#define TB_TARGET_MACOSX
#define TB_SYSTEM_LINUX
#define TB_CLIPBOARD_GD
#elif defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#define TB_FILE_POSIX
#define TB_TARGET_WINDOWS
#define TB_CLIPBOARD_WINDOWS
#define TB_SYSTEM_WINDOWS
#endif

#endif // TB_CONFIG_H
