/// Simple error message routines for SDL.

#ifndef _error_h_
#define _error_h_

#include "_stdinc.h"

#include "_begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/* Public functions */


/**
 * Set the SDL error message for the current thread.
 *
 * Calling this function will replace any previous error message that was set.
 *
 * This function always returns -1, since SDL frequently uses -1 to signify an
 * failing result, leading to this idiom:
 *
 * ```c
 * if (error_code) {
 *     return SDL_SetError("This operation has failed: %d", error_code);
 * }
 * ```
 *
 * \param fmt a printf()-style message format string
 * \param ... additional parameters matching % tokens in the `fmt` string, if
 *            any
 * \returns always -1.
 *
 * \since This function is available since SDL 2.0.0.
 *
 * \sa SDL_ClearError
 * \sa SDL_GetError
 */
extern DECLSPEC int SDLCALL SDL_SetError(SDL_PRINTF_FORMAT_STRING const char *fmt, ...) SDL_PRINTF_VARARG_FUNC(1);

/**
 * Retrieve a message about the last error that occurred on the current
 * thread.
 *
 * It is possible for multiple errors to occur before calling SDL_GetError().
 * Only the last error is returned.
 *
 * The message is only applicable when an SDL function has signaled an error.
 * You must check the return values of SDL function calls to determine when to
 * appropriately call SDL_GetError(). You should *not* use the results of
 * SDL_GetError() to decide if an error has occurred! Sometimes SDL will set
 * an error string even when reporting success.
 *
 * SDL will *not* clear the error string for successful API calls. You *must*
 * check return values for failure cases before you can assume the error
 * string applies.
 *
 * Error strings are set per-thread, so an error set in a different thread
 * will not interfere with the current thread's operation.
 *
 * The returned string is internally allocated and must not be freed by the
 * application.
 *
 * \returns a message with information about the specific error that occurred,
 *          or an empty string if there hasn't been an error message set since
 *          the last call to SDL_ClearError(). The message is only applicable
 *          when an SDL function has signaled an error. You must check the
 *          return values of SDL function calls to determine when to
 *          appropriately call SDL_GetError().
 *
 * \since This function is available since SDL 2.0.0.
 *
 * \sa SDL_ClearError
 * \sa SDL_SetError
 */
extern DECLSPEC const char *SDLCALL SDL_GetError(void);

/**
 * Get the last error message that was set for the current thread.
 *
 * This allows the caller to copy the error string into a provided buffer, but
 * otherwise operates exactly the same as SDL_GetError().
 *
 * \param errstr A buffer to fill with the last error message that was set for
 *               the current thread
 * \param maxlen The size of the buffer pointed to by the errstr parameter
 * \returns the pointer passed in as the `errstr` parameter.
 *
 * \since This function is available since SDL 2.0.14.
 *
 * \sa SDL_GetError
 */
extern DECLSPEC char * SDLCALL SDL_GetErrorMsg(char *errstr, int maxlen);

/**
 * Clear any previous error message for this thread.
 *
 * \since This function is available since SDL 2.0.0.
 *
 * \sa SDL_GetError
 * \sa SDL_SetError
 */
extern DECLSPEC void SDLCALL SDL_ClearError(void);

/**
 *  \name Internal error functions
 *
 *  \internal
 *  Private error reporting function - used internally.
 */
/* @{ */
#define SDL_OutOfMemory()   SDL_Error(SDL_ENOMEM)
#define SDL_Unsupported()   SDL_Error(SDL_UNSUPPORTED)
#define SDL_InvalidParamError(param)    SDL_SetError("Parameter '%s' is invalid", (param))
typedef enum
{
    SDL_ENOMEM,
    SDL_EFREAD,
    SDL_EFWRITE,
    SDL_EFSEEK,
    SDL_UNSUPPORTED,
    SDL_LASTERROR
} SDL_errorcode;
/* SDL_Error() unconditionally returns -1. */
extern DECLSPEC int SDLCALL SDL_Error(SDL_errorcode code);
/* @} *//* Internal error functions */

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "_close_code.h"

#endif /* _error_h_ */

/* vi: set ts=4 sw=4 expandtab: */
