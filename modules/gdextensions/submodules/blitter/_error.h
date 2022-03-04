/*************************************************************************/
/*  _error.h                                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

/// Simple error message routines for SDL.

#ifndef _error_h_
#define _error_h_

#include "_stdinc.h"

#include "_begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

// Set the SDL error message for the current thread.
//
// Calling this function will replace any previous error message that was set.
// This function always returns -1, since SDL frequently uses -1 to signify an
// failing result, leading to this idiom:
//
// ```c
// if (error_code) {
//   return SDL_SetError("This operation has failed: %d", error_code);
// }
//```
extern DECLSPEC int SDLCALL SDL_SetError(SDL_PRINTF_FORMAT_STRING const char *fmt, ...) SDL_PRINTF_VARARG_FUNC(1);

// Retrieve a message about the last error that occurred on the current
// thread.
//
// It is possible for multiple errors to occur before calling SDL_GetError().
// Only the last error is returned.
//
// The message is only applicable when an SDL function has signaled an error.
// You must check the return values of SDL function calls to determine when to
// appropriately call SDL_GetError(). You should *not* use the results of
// SDL_GetError() to decide if an error has occurred! Sometimes SDL will set
// an error string even when reporting success.
//
// SDL will *not* clear the error string for successful API calls. You *must*
// check return values for failure cases before you can assume the error
// string applies.
//
// Error strings are set per-thread, so an error set in a different thread
// will not interfere with the current thread's operation.
//
// The returned string is internally allocated and must not be freed by the
// application.
extern DECLSPEC const char *SDLCALL SDL_GetError(void);

// Get the last error message that was set for the current thread.
//
// This allows the caller to copy the error string into a provided buffer, but
// otherwise operates exactly the same as SDL_GetError().
extern DECLSPEC char *SDLCALL SDL_GetErrorMsg(char *errstr, int maxlen);

// Clear any previous error message for this thread.
extern DECLSPEC void SDLCALL SDL_ClearError(void);

// Private error reporting function - used internally.
#define SDL_OutOfMemory() SDL_Error(SDL_ENOMEM)
#define SDL_Unsupported() SDL_Error(SDL_UNSUPPORTED)
#define SDL_InvalidParamError(param) SDL_SetError("Parameter '%s' is invalid", (param))
typedef enum {
	SDL_ENOMEM,
	SDL_EFREAD,
	SDL_EFWRITE,
	SDL_EFSEEK,
	SDL_UNSUPPORTED,
	SDL_LASTERROR
} SDL_errorcode;

// SDL_Error() unconditionally returns -1.
extern DECLSPEC int SDLCALL SDL_Error(SDL_errorcode code);

// Basic assertion support
#ifdef _MSC_VER  // stupid /W4 warnings.
#define SDL_NULL_WHILE_LOOP_CONDITION (0,0)
#else
#define SDL_NULL_WHILE_LOOP_CONDITION (0)
#endif

#define SDL_disabled_assert(condition) \
    do { (void) sizeof ((condition)); } while (SDL_NULL_WHILE_LOOP_CONDITION)

#if defined(DEBUG_ENABLED) || defined(TOOLS_ENABLED)
extern DECLSPEC void SDL_assert(SDL_bool cond);
#else
#define SDL_assert(cond) SDL_disabled_assert(cond)
#endif

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "_close_code.h"

#endif /* _error_h_ */

/* vi: set ts=4 sw=4 expandtab: */
