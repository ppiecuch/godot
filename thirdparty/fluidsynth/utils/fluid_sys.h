/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */


/*
 * @file fluid_sys.h
 *
 * This header contains a bunch of (mostly) system and machine
 * dependent functions:
 *
 * - timers
 * - current time in milliseconds and microseconds
 * - debug logging
 * - profiling
 * - memory locking
 * - checking for floating point exceptions
 *
 * fluidsynth's wrapper OSAL so to say; include it in .c files, be careful to include
 * it in fluidsynth's private header files (see comment in fluid_coreaudio.c)
 */

#ifndef _FLUID_SYS_H
#define _FLUID_SYS_H

#include "fluidsynth_priv.h"

#if HAVE_MATH_H
#include <math.h>
#endif

#if HAVE_ERRNO_H
#include <errno.h>
#endif

#if HAVE_STDARG_H
#include <stdarg.h>
#endif

#if HAVE_STDBOOL_H
#include <stdbool.h>
#endif

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if HAVE_LIMITS_H
#include <limits.h>
#endif

#if HAVE_IO_H
#include <io.h> // _open(), _close(), read(), write() on windows
#endif

#if HAVE_SIGNAL_H
#include <signal.h>
#endif

/** Integer types  */
#if HAVE_STDINT_H
#include <stdint.h>
#else
#include "core/int_types.h"
#endif

/** Boolean types  */
#ifndef _WIN32
#define TRUE true
#define FALSE false
#endif

#if defined(WIN32) && HAVE_WINDOWS_H

/* WIN32 special defines */
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#ifdef _MSC_VER
#pragma warning(disable : 4244)
#pragma warning(disable : 4101)
#pragma warning(disable : 4305)
#pragma warning(disable : 4996)
#endif

#endif

/* Darwin special defines (taken from config_macosx.h) */
#ifdef DARWIN
# define MACINTOSH
# define __Types__
#endif

#ifdef WIN32
char* fluid_get_windows_error(void);
#endif

#define FLUID_INLINE              inline
#define FLUID_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))

/* Integer<->pointer conversion */
#define FLUID_POINTER_TO_UINT(x)  ((unsigned int)(uintptr_t)(x))
#define FLUID_UINT_TO_POINTER(x)  ((void *)(uintptr_t)(x))
#define FLUID_POINTER_TO_INT(x)   ((signed int)(intptr_t)(x))
#define FLUID_INT_TO_POINTER(x)   ((void *)(intptr_t)(x))

/* Endian detection */
#define SWAP32(l) ((((l) >> 24) & 0x000000ff) | (((l) >> 8) & 0x0000ff00) | (((l) << 8) & 0x00ff0000) | (((l) << 24) & 0xff000000))
#define SWAP16(w) ((((w) >> 8) & 0x00ff) | (((w) << 8) & 0xff00))

#ifdef WORDS_BIGENDIAN
#define FLUID_IS_BIG_ENDIAN       (1)
#define INT16_FROM_LE(val)        SWAP16(val)
#define INT32_FROM_LE(val)        SWAP32(val)
#else
#define FLUID_IS_BIG_ENDIAN       (0)
#define INT16_FROM_LE(val)        ((signed short)(val))
#define INT32_FROM_LE(val)        ((signed int)(val))
#endif

#define FLUID_LE32TOH(x)          INT32_FROM_LE(x)
#define FLUID_LE16TOH(x)          INT16_FROM_LE(x)

#if FLUID_IS_BIG_ENDIAN
#define FLUID_FOURCC(_a, _b, _c, _d) \
    (uint32_t)(((uint32_t)(_a) << 24) | ((uint32_t)(_b) << 16) | ((uint32_t)(_c) << 8) | (uint32_t)(_d))
#else
#define FLUID_FOURCC(_a, _b, _c, _d) \
    (uint32_t)(((uint32_t)(_d) << 24) | ((uint32_t)(_c) << 16) | ((uint32_t)(_b) << 8) | (uint32_t)(_a)) 
#endif

/*
 * Utility functions
 */
char *fluid_strtok(char **str, char *delim);

/**
    Time functions

 */

unsigned int fluid_curtime(void);
/**
 * Get time in microseconds to be used in relative timing operations.
 * @return time in microseconds.
 */
double fluid_utime(void);


/**
    Timers

 */

/* if the callback function returns 1 the timer will continue; if it
   returns 0 it will stop */
typedef int (*fluid_timer_callback_t)(void *data, unsigned int msec);

typedef struct _fluid_timer_t fluid_timer_t;

fluid_timer_t *new_fluid_timer(int msec, fluid_timer_callback_t callback,
                               void *data, int new_thread, int auto_destroy,
                               int high_priority);

void delete_fluid_timer(fluid_timer_t *timer);
int fluid_timer_join(fluid_timer_t *timer);
int fluid_timer_stop(fluid_timer_t *timer);
int fluid_timer_is_running(const fluid_timer_t *timer);
long fluid_timer_get_interval(const fluid_timer_t * timer);


/* I/O */

int fluid_istream_readline(fluid_istream_t in, fluid_ostream_t out, char *prompt, char *buf, int len);
int fluid_ostream_printf(fluid_ostream_t out, const char *format, ...);

/* Muteces */

#if HAVE_PTHREAD_H && !defined(_WIN32)

static FLUID_INLINE void
fluid_pthread_mutex_init(pthread_mutex_t *m, int kind)
{
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, kind);
    pthread_mutex_init(m, &attr);
}
/* Regular mutex */
typedef pthread_mutex_t fluid_mutex_t;
#define fluid_mutex_init(_m)      pthread_mutex_init (&(_m), NULL)
#define fluid_mutex_destroy(_m)   pthread_mutex_destroy(&(_m))
#define fluid_mutex_lock(_m)      pthread_mutex_lock(&(_m))
#define fluid_mutex_unlock(_m)    pthread_mutex_unlock(&(_m))

/* Recursive lock capable mutex */
typedef pthread_mutex_t fluid_rec_mutex_t;
#define fluid_rec_mutex_init(_m)      fluid_pthread_mutex_init(&(_m), PTHREAD_MUTEX_RECURSIVE)
#define fluid_rec_mutex_destroy(_m)   pthread_mutex_destroy(&(_m))
#define fluid_rec_mutex_lock(_m)      pthread_mutex_lock(&(_m))
#define fluid_rec_mutex_unlock(_m)    pthread_mutex_unlock(&(_m))

/* Dynamically allocated mutex suitable for fluid_cond_t use */
typedef pthread_mutex_t fluid_cond_mutex_t;
#define fluid_cond_mutex_init(m)      pthread_mutex_init(m, NULL)
#define fluid_cond_mutex_destroy(m)   pthread_mutex_destroy(m)
#define fluid_cond_mutex_lock(m)      pthread_mutex_lock(m)
#define fluid_cond_mutex_unlock(m)    pthread_mutex_unlock(m)

static FLUID_INLINE fluid_cond_mutex_t *
new_fluid_cond_mutex (void)
{
    fluid_cond_mutex_t *mutex = (fluid_cond_mutex_t *)malloc(sizeof(fluid_cond_mutex_t));
    fluid_cond_mutex_init(mutex);
    return mutex;
}

static FLUID_INLINE void
delete_fluid_cond_mutex (fluid_cond_mutex_t *m)
{
    fluid_cond_mutex_destroy(m);
    free(m);
}

/* Thread condition signaling */
typedef pthread_cond_t fluid_cond_t;
#define fluid_cond_init(cond)           pthread_cond_init(cond, NULL)
#define fluid_cond_destroy(cond)        pthread_cond_destroy(cond)
#define fluid_cond_signal(cond)         pthread_cond_signal(cond)
#define fluid_cond_broadcast(cond)      pthread_cond_broadcast(cond)
#define fluid_cond_wait(cond, mutex)    pthread_cond_wait(cond, mutex)

static FLUID_INLINE fluid_cond_t *
new_fluid_cond (void)
{
    fluid_cond_t *cond = (fluid_cond_t *)malloc(sizeof(fluid_cond_t));
    fluid_cond_init(cond);
    return cond;
}

static FLUID_INLINE void
delete_fluid_cond (fluid_cond_t *cond)
{
    fluid_cond_destroy(cond);
    free(cond);
}

/* Thread private data */

typedef pthread_key_t fluid_private_t;
#define fluid_private_init(_priv)                  pthread_key_create(&_priv, NULL)
#define fluid_private_free(_priv)
#define fluid_private_get(_priv)                   pthread_getspecific((_priv))
#define fluid_private_set(_priv, _data)            pthread_setspecific((_priv), (_data))

/* Threads */

/* other thread implementations might change this for their needs */
typedef void *fluid_thread_return_t;
/* static return value for thread functions which requires a return value */
#define fluid_thread_null_value (NULL)

typedef pthread_t fluid_thread_t;
typedef void *(*fluid_thread_func_t)(void* data);

#define FLUID_THREAD_ID_NULL            NULL                   /* A NULL "ID" value */
#define fluid_thread_id_t               pthread_t              /* Data type for a thread ID */
#define fluid_thread_get_id()           pthread_self()         /* Get unique "ID" for current thread */

#define _new_thread(thread, func, data) pthread_create(thread, NULL, func, data);
#define _detach_thread(thread) pthread_detach(*thread)
#define _thread_join(thread) pthread_join(*thread, NULL)

fluid_thread_t* new_fluid_thread(const char *name, fluid_thread_func_t func, void *data, int prio_level, int detach);
void delete_fluid_thread(fluid_thread_t* thread);
void fluid_thread_self_set_prio (int prio_level);
int fluid_thread_join(fluid_thread_t* thread);

#elif defined(_WIN32)

#define fluid_mwinapi_call(_f, _r, ...) FLUID_STMT_START { \
if ((__VA_ARGS__) == (_r)) \
    FLUID_LOG(FLUID_ERR, #_f " failed"); \
} FLUID_STMT_END

static FLUID_INLINE void
fluid_win32_mutex_init(PHANDLE m) { *m = CreateMutex(NULL, TRUE, NULL); }

/* Regular mutex */
typedef SRWLOCK fluid_mutex_t;
#define fluid_mutex_init(_m)      InitializeSRWLock(&(_m))
#define fluid_mutex_destroy(_m)
#define fluid_mutex_lock(_m)      AcquireSRWLockExclusive(&(_m))
#define fluid_mutex_unlock(_m)    ReleaseSRWLockExclusive(&(_m))

/* Recursive lock capable mutex */
typedef SRWLOCK fluid_rec_mutex_t;
#define fluid_rec_mutex_init(_m)      InitializeSRWLock(&(_m))
#define fluid_rec_mutex_destroy(_m)
#define fluid_rec_mutex_lock(_m)      AcquireSRWLockShared(&(_m))
#define fluid_rec_mutex_unlock(_m)    ReleaseSRWLockShared(&(_m))

/* Dynamically allocated mutex suitable for fluid_cond_t use */
typedef CRITICAL_SECTION fluid_cond_mutex_t;
#define fluid_cond_mutex_init(_m)      InitializeCriticalSection(_m)
#define fluid_cond_mutex_destroy(_m)   DeleteCriticalSection(_m)
#define fluid_cond_mutex_lock(_m)      EnterCriticalSection(_m)
#define fluid_cond_mutex_unlock(_m)    LeaveCriticalSection(_m)

static FLUID_INLINE fluid_cond_mutex_t *
new_fluid_cond_mutex (void)
{
    fluid_cond_mutex_t *mutex;
    mutex = malloc(sizeof(fluid_cond_mutex_t));
    fluid_cond_mutex_init(mutex);
    return mutex;
}

static FLUID_INLINE void
delete_fluid_cond_mutex (fluid_cond_mutex_t *m)
{
    fluid_cond_mutex_destroy(m);
    free(m);
}

/* Thread condition signaling */
typedef CONDITION_VARIABLE fluid_cond_t;
#define fluid_cond_init(cond)           InitializeConditionVariable(cond)
#define fluid_cond_destroy(cond)
#define fluid_cond_signal(cond)         WakeConditionVariable(cond)
#define fluid_cond_broadcast(cond)      WakeAllConditionVariable(cond)
#define fluid_cond_wait(cond, mutex)    SleepConditionVariableCS(cond, mutex, INFINITE)

static FLUID_INLINE fluid_cond_t *
new_fluid_cond (void)
{
    fluid_cond_t *cond = (fluid_cond_t *)malloc(sizeof(fluid_cond_t));
    fluid_cond_init(cond);
    return cond;
}

static FLUID_INLINE void
delete_fluid_cond (fluid_cond_t *cond)
{
    fluid_cond_destroy(cond);
    free(cond);
}

/* Thread private data */

typedef DWORD fluid_private_t;
#define fluid_private_init(_priv)                  (_priv = TlsAlloc())
#define fluid_private_free(_priv)                  TlsFree(_priv);
#define fluid_private_get(_priv)                   TlsGetValue(_priv)
#define fluid_private_set(_priv, _data)            TlsSetValue(_priv, _data)

/* Threads */

typedef DWORD fluid_thread_return_t;
#define fluid_thread_null_value (0)

typedef HANDLE fluid_thread_t;
typedef DWORD (WINAPI *fluid_thread_func_t)(void* data);

#define FLUID_THREAD_ID_NULL            0                      /* A NULL "ID" value */
#define fluid_thread_id_t               DWORD                  /* Data type for a thread ID */
#define fluid_thread_get_id()           GetCurrentThreadId()   /* Get unique "ID" for current thread */

#define _new_thread(thread, func, data) *thread = CreateThread(NULL, 0, func, data, 0, NULL)
#define _detach_thread(thread) CloseHandle(*thread);
#define _thread_join(thread) { WaitForSingleObject(*thread, INFINITE); CloseHandle(*thread); }

fluid_thread_t* new_fluid_thread(const char *name, fluid_thread_func_t func, void *data, int prio_level, int detach);
void delete_fluid_thread(fluid_thread_t* thread);
void fluid_thread_self_set_prio (int prio_level);
int fluid_thread_join(fluid_thread_t* thread);

#else

#error "Unsupported platform/compiler"

#endif /* HAVE_PTHREAD_H, _WIN32 */


/* Atomic operations */

#include "fluid_atomic_priv.h"


/* File access */
#ifdef HAVE_SYS_STAT_H
typedef struct stat fluid_stat_buf_t;
#else
typedef struct _fluid_stat_buf_t{ int st_mtime; } fluid_stat_buf_t;
#endif
bool fluid_stat(const char *filename, fluid_stat_buf_t *statbuf);

enum {
	FILE_TEST_EXISTS,
	FILE_TEST_IS_REGULAR,
};
bool fluid_file_test(const char *path, int test_op);
FILE* fluid_file_open(const char* filename, const char** errMsg);
fluid_long_long_t fluid_file_tell(FILE* f);


/* Profiling */
#if WITH_PROFILING
/** profiling interface between Profiling command shell and Audio
    rendering  API (FluidProfile_0004.pdf- 3.2.2)
*/

/*
  -----------------------------------------------------------------------------
  Shell task side |    Profiling interface              |  Audio task side
  -----------------------------------------------------------------------------
  profiling       |    Internal    |      |             |      Audio
  command   <---> |<-- profling -->| Data |<--macros -->| <--> rendering
  shell           |    API         |      |             |      API

*/

/* default parameters for shell command "prof_start" in fluid_sys.c */
#define FLUID_PROFILE_DEFAULT_BANK 0       /* default bank */
#define FLUID_PROFILE_DEFAULT_PROG 16      /* default prog (organ) */
#define FLUID_PROFILE_FIRST_KEY 12         /* first key generated */
#define FLUID_PROFILE_LAST_KEY 108         /* last key generated */
#define FLUID_PROFILE_DEFAULT_VEL 64       /* default note velocity */
#define FLUID_PROFILE_VOICE_ATTEN -0.04f   /* gain attenuation per voice (dB) */


#define FLUID_PROFILE_DEFAULT_PRINT 0      /* default print mode */
#define FLUID_PROFILE_DEFAULT_N_PROF 1     /* default number of measures */
#define FLUID_PROFILE_DEFAULT_DURATION 500 /* default duration (ms)  */


extern unsigned short fluid_profile_notes; /* number of generated notes */
extern unsigned char fluid_profile_bank;   /* bank,prog preset used by */
extern unsigned char fluid_profile_prog;   /* generated notes */
extern unsigned char fluid_profile_print;  /* print mode */

extern unsigned short fluid_profile_n_prof;/* number of measures */
extern unsigned short fluid_profile_dur;   /* measure duration in ms */
extern fluid_atomic_int_t fluid_profile_lock ; /* lock between multiple shell */
/**/

/*----------------------------------------------
  Internal profiling API (in fluid_sys.c)
-----------------------------------------------*/
/* Starts a profiling measure used in shell command "prof_start" */
void fluid_profile_start_stop(unsigned int end_ticks, short clear_data);

/* Returns status used in shell command "prof_start" */
int fluid_profile_get_status(void);

/* Prints profiling data used in shell command "prof_start" */
void fluid_profiling_print_data(double sample_rate, fluid_ostream_t out);

/* Returns True if profiling cancellation has been requested */
int fluid_profile_is_cancel_req(void);

/* For OS that implement <ENTER> key for profile cancellation:
 1) Adds #define FLUID_PROFILE_CANCEL
 2) Adds the necessary code inside fluid_profile_is_cancel() see fluid_sys.c
*/
#if defined(WIN32)      /* Profile cancellation is supported for Windows */
#define FLUID_PROFILE_CANCEL

#else   /* POSIX stuff */
#define FLUID_PROFILE_CANCEL /* Profile cancellation is supported for linux */
#include <unistd.h> /* STDIN_FILENO */
#include <sys/select.h> /* select() */
#endif /* posix */

/* logging profiling data (used on synthesizer instance deletion) */
void fluid_profiling_print(void);

/*----------------------------------------------
  Profiling Data (in fluid_sys.c)
-----------------------------------------------*/
/** Profiling data. Keep track of min/avg/max values to profile a
    piece of code. */
typedef struct _fluid_profile_data_t
{
    const char *description;        /* name of the piece of code under profiling */
    double min, max, total;   /* duration (microsecond) */
    unsigned int count;       /* total count */
    unsigned int n_voices;    /* voices number */
    unsigned int n_samples;   /* audio samples number */
} fluid_profile_data_t;

enum
{
    /* commands/status  (profiling interface) */
    PROFILE_STOP,    /* command to stop a profiling measure */
    PROFILE_START,   /* command to start a profile measure */
    PROFILE_READY,   /* status to signal that a profiling measure has finished
	                    and ready to be printed */
    /*- State returned by fluid_profile_get_status() -*/
    /* between profiling commands and internal profiling API */
    PROFILE_RUNNING, /* a profiling measure is running */
    PROFILE_CANCELED,/* a profiling measure has been canceled */
};

/* Data interface */
extern unsigned char fluid_profile_status ;       /* command and status */
extern unsigned int fluid_profile_end_ticks;      /* ending position (in ticks) */
extern fluid_profile_data_t fluid_profile_data[]; /* Profiling data */

/*----------------------------------------------
  Probes macros
-----------------------------------------------*/
/** Macro to obtain a time reference used for the profiling */
#define fluid_profile_ref() fluid_utime()

/** Macro to create a variable and assign the current reference time for profiling.
 * So we don't get unused variable warnings when profiling is disabled. */
#define fluid_profile_ref_var(name)     double name = fluid_utime()

/**
 * Profile identifier numbers. List all the pieces of code you want to profile
 * here. Be sure to add an entry in the fluid_profile_data table in
 * fluid_sys.c
 */
enum
{
    FLUID_PROF_WRITE,
    FLUID_PROF_ONE_BLOCK,
    FLUID_PROF_ONE_BLOCK_CLEAR,
    FLUID_PROF_ONE_BLOCK_VOICE,
    FLUID_PROF_ONE_BLOCK_VOICES,
    FLUID_PROF_ONE_BLOCK_REVERB,
    FLUID_PROF_ONE_BLOCK_CHORUS,
    FLUID_PROF_VOICE_NOTE,
    FLUID_PROF_VOICE_RELEASE,
    FLUID_PROFILE_NBR	/* number of profile probes */
};
/** Those macros are used to calculate the min/avg/max. Needs a profile number, a
    time reference, the voices and samples number. */

/* local macro : acquiere data */
#define fluid_profile_data(_num, _ref, voices, samples)\
{\
	double _now = fluid_utime();\
	double _delta = _now - _ref;\
	fluid_profile_data[_num].min = _delta < fluid_profile_data[_num].min ?\
                                   _delta : fluid_profile_data[_num].min; \
	fluid_profile_data[_num].max = _delta > fluid_profile_data[_num].max ?\
                                   _delta : fluid_profile_data[_num].max;\
	fluid_profile_data[_num].total += _delta;\
	fluid_profile_data[_num].count++;\
	fluid_profile_data[_num].n_voices += voices;\
	fluid_profile_data[_num].n_samples += samples;\
	_ref = _now;\
}

/** Macro to collect data, called from inner functions inside audio
    rendering API */
#define fluid_profile(_num, _ref, voices, samples)\
{\
	if ( fluid_profile_status == PROFILE_START)\
	{	/* acquires data */\
		fluid_profile_data(_num, _ref, voices, samples)\
	}\
}

/** Macro to collect data, called from audio rendering API (fluid_write_xxxx()).
 This macro control profiling ending position (in ticks).
*/
#define fluid_profile_write(_num, _ref, voices, samples)\
{\
	if (fluid_profile_status == PROFILE_START)\
	{\
		/* acquires data first: must be done before checking that profile is
           finished to ensure at least one valid data sample.
		*/\
		fluid_profile_data(_num, _ref, voices, samples)\
		if (fluid_synth_get_ticks(synth) >= fluid_profile_end_ticks)\
		{\
			/* profiling is finished */\
			fluid_profile_status = PROFILE_READY;\
		}\
	}\
}

#else

/* No profiling */
#define fluid_profiling_print()
#define fluid_profile_ref()  0
#define fluid_profile_ref_var(name)
#define fluid_profile(_num,_ref,voices, samples)
#define fluid_profile_write(_num,_ref, voices, samples)
#endif /* WITH_PROFILING */

/**

    Memory locking

    Memory locking is used to avoid swapping of the large block of
    sample data.
 */

#if defined(HAVE_SYS_MMAN_H)
#define fluid_mlock(_p,_n)      mlock(_p, _n)
#define fluid_munlock(_p,_n)    munlock(_p,_n)
#else
#define fluid_mlock(_p,_n)      0
#define fluid_munlock(_p,_n)
#endif


/* System control */
void fluid_msleep(unsigned int msecs);

/**
 * Advances the given \c ptr to the next \c alignment byte boundary.
 * Make sure you've allocated an extra of \c alignment bytes to avoid a buffer overflow.
 *
 * @note \c alignment must be a power of two
 * @return Returned pointer is guaranteed to be aligned to \c alignment boundary and in range \f[ ptr <= returned_ptr < ptr + alignment \f].
 */
static FLUID_INLINE void *fluid_align_ptr(const void *ptr, unsigned int alignment)
{
    uintptr_t ptr_int = (uintptr_t)ptr;
    unsigned int offset = ptr_int & (alignment - 1);
    unsigned int add = (alignment - offset) & (alignment - 1); // advance the pointer to the next alignment boundary
    ptr_int += add;

    /* assert alignment is power of two */
    FLUID_ASSERT(!(alignment == 0) && !(alignment & (alignment - 1)));

    return (void *)ptr_int;
}

#define FLUID_DEFAULT_ALIGNMENT (64U)

#endif /* _FLUID_SYS_H */
