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

#ifndef _FLUID_ATOMIC_PRIV_H
#define _FLUID_ATOMIC_PRIV_H

#ifndef STATIC_ASSERT
#if __STDC_VERSION__ >= 201112L
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT(expr, msg) typedef char __static_assertion_ ## __COUNTER__[(expr) ? 1 : -1]
#endif
#endif // STATIC_ASSERT

#include "fluid_atomic.h"

#ifndef _FLUID_SYS_H
#warning "Missing fluid_sys.h - some definitions might be missing"
#endif

#if defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4) || defined(__riscos__)

#ifdef __ATOMIC_SEQ_CST // Use GCC's new atomic operations.

#define FLUID_ATOMIC_ORDER __ATOMIC_SEQ_CST
#define fluid_atomic_int_exchange_and_add(atomic, val) (__atomic_add_fetch(atomic, val, FLUID_ATOMIC_ORDER))
#define fluid_atomic_int_get(atomic) __atomic_load_n(atomic, FLUID_ATOMIC_ORDER)
#define fluid_atomic_int_set(atomic, val) __atomic_store_n(atomic, val, FLUID_ATOMIC_ORDER)
#define fluid_atomic_int_dec_and_test(atomic) (__atomic_sub_fetch(atomic, 1, FLUID_ATOMIC_ORDER) == 0)

static FLUID_INLINE int
fluid_atomic_int_compare_and_exchange(volatile int* atomic, int _old, int _new)
{
	return __atomic_compare_exchange_n(atomic, &_old, _new, 0, FLUID_ATOMIC_ORDER, FLUID_ATOMIC_ORDER);
}

#define fluid_atomic_pointer_get(atomic) __atomic_load_n(atomic, FLUID_ATOMIC_ORDER)
#define fluid_atomic_pointer_set(atomic, val) __atomic_store_n(atomic, val, FLUID_ATOMIC_ORDER)

static FLUID_INLINE int
fluid_atomic_pointer_compare_and_exchange(volatile void* atomic, volatile void* _old, void* _new)
{
	return __atomic_compare_exchange_n((volatile void**)atomic, &_old, _new, 0, FLUID_ATOMIC_ORDER, FLUID_ATOMIC_ORDER);
}

#define fluid_atomic_int_add(atomic, val) (void)(fluid_atomic_int_exchange_and_add( atomic, val))
#define fluid_atomic_int_inc(atomic) fluid_atomic_int_add(atomic, 1)

#else // Use older __sync atomics.

#define fluid_atomic_int_add(atomic, val) __extension__ ({            \
            STATIC_ASSERT(sizeof(atomic) == sizeof(int),     \
                          "Atomic must be the size of an int");       \
            __sync_fetch_and_add(&atomic, (val));})

#define fluid_atomic_int_get(atomic) __extension__ ({             \
            STATIC_ASSERT(sizeof(atomic) == sizeof(int), \
                          "Atomic must be the size of an int");   \
            __sync_synchronize();                                 \
            atomic;})

#define fluid_atomic_int_set(atomic, newval) __extension__ ({         \
                STATIC_ASSERT(sizeof(atomic) == sizeof(int), \
                              "Atomic must be the size of an int");   \
                atomic = (newval);                           \
                __sync_synchronize();})

#define fluid_atomic_int_compare_and_exchange(atomic, oldval, newval) __extension__ ({ \
            STATIC_ASSERT(sizeof(atomic) == sizeof(int),       \
                          "Atomic must be the size of an int");         \
            __sync_bool_compare_and_swap(&atomic, (oldval), (newval));})

#define fluid_atomic_int_exchange_and_add(atomic, val) fluid_atomic_int_add(atomic, val)
#define fluid_atomic_int_inc(atomic) fluid_atomic_int_add(atomic, 1)

static FLUID_INLINE void*
fluid_atomic_pointer_get(volatile void* atomic) { __sync_synchronize(); return *(void**)atomic; }

static FLUID_INLINE void
fluid_atomic_pointer_set(volatile void* atomic, void* val) { *(void**)atomic = val; __sync_synchronize(); }

#define fluid_atomic_pointer_compare_and_exchange fluid_atomic_int_compare_and_exchange

#endif // __ATOMIC_SEQ_CST

#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define fluid_atomic_int_inc(atomic) InterlockedIncrement((atomic))
#define fluid_atomic_int_add(atomic, val) InterlockedAdd((atomic), (val))
#define fluid_atomic_int_get(atomic) (*(int*)(atomic))
#define fluid_atomic_int_set(atomic, val) InterlockedExchange((LONG volatile*)(atomic), (LONG)(val))
#define fluid_atomic_int_dec_and_test(atomic) (InterlockedDecrement((LONG volatile*)(atomic)) == 0)
#define fluid_atomic_int_exchange_and_add(atomic, add) InterlockedExchangeAdd((LONG  volatile*)(atomic), (LONG)(add))
#define fluid_atomic_int_compare_and_exchange(atomic, _old, _new) InterlockedCompareExchange((LONG volatile*)(atomic), (LONG)(_new), (LONG)(_old))
#define fluid_atomic_pointer_get(atomic) (*(void**)atomic)
#define fluid_atomic_pointer_set(atomic, val) (void)(InterlockedExchangePointer(atomic, val))

#else

#error "Unsupported platform/compiler"

#endif

static FLUID_INLINE void
fluid_atomic_float_set(fluid_atomic_float_t *fptr, float val)
{
  int32_t ival;
  FLUID_MEMCPY(&ival, &val, 4);
  fluid_atomic_int_set ((fluid_atomic_int_t *)fptr, ival);
}

static FLUID_INLINE float
fluid_atomic_float_get(fluid_atomic_float_t *fptr)
{
  int32_t ival;
  float fval;
  ival = fluid_atomic_int_get ((fluid_atomic_int_t *)fptr);
  FLUID_MEMCPY(&fval, &ival, 4);
  return fval;
}

#endif /* _FLUID_ATOMIC_PRIV_H */
