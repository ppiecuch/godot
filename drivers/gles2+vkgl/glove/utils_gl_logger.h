/**************************************************************************/
/*  utils_gl_logger.h                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

/**
 * Copyright (C) 2015-2018 Think Silicon S.A. (https://think-silicon.com/)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public v3
 * License as published by the Free Software Foundation;
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 */

/// A Logging Library

#ifndef UTILS_GL_LOGGER_H
#define UTILS_GL_LOGGER_H

#include "utils_gl_logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// Code
#ifdef NDEBUG
#define NOT_REACHED() printf("You shouldn't be here. (function %s at line %d of file %s)\n", __func__, __LINE__, __FILE__)
#define NOT_FOUND_ENUM(inv_enum) printf("Invalid enum: %#04x. (function %s at line %d of file %s)\n", inv_enum, __func__, __LINE__, __FILE__)
#define NOT_IMPLEMENTED() printf("Function %s (line %d of file %s) not implemented yet.\n", __func__, __LINE__, __FILE__)
#else
#define NOT_REACHED() assert(0 && "You shouldn't be here.")
#define NOT_FOUND_ENUM(inv_enum) assert(0 && "Invalid enum")
#define NOT_IMPLEMENTED() assert(0 && "Function not implemented yet.")
#endif // NDEBUG

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>

#ifdef __cplusplus
}
#endif

#ifdef TRACE_BUILD
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define PROJECT_LENGTH strlen(STR(PROJECT_PATH)) - 1
#define GL_SOURCE_FILE_NAME &__FILE__[PROJECT_LENGTH]
#define FUN_ENTRY(__lvl__) GLLogger::FunEntry(__lvl__, GL_SOURCE_FILE_NAME, __func__, __LINE__)
#define GLOVE_PRINT(__lvl__, ...) GLLogger::Log(__lvl__, __VA_ARGS__)
#define GLOVE_PRINT_ERR(...) fprintf(stderr, __VA_ARGS__);
#else
#define FUN_ENTRY(__lvl__)
#define GLOVE_PRINT(...)
#define GLOVE_PRINT_ERR(...)
#endif

class GLLogger {
	static const int funcWidth = 55;
	static GLLogger *mInstance;
	static GLLoggerImpl *mLoggerImpl;

	static void SetLoggerImpl();

	static GLLogger *GetInstance();
	static void DestroyInstance();

	int CalculateSpacesBefore(glLogLevel_e level);
	int CalculateSpacesAfter(const char *func, glLogLevel_e level);
	void WriteFunEntry(glLogLevel_e level, const char *filename, const char *func, int line);

	GLLogger();
	virtual ~GLLogger();

public:
	static void Shutdown();
	static void FunEntry(glLogLevel_e level, const char *filename, const char *func, int line);
	static void Log(glLogLevel_e level, const char *format, ...);
};

#endif // UTILS_GL_LOGGER_H
