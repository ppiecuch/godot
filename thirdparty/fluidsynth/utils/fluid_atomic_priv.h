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

#if defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4) || defined(__riscos__)

#define fluid_atomic_int_add(atomic, val) __extension__ ({            \
            STATIC_ASSERT(sizeof((atomic)->value) == sizeof(int),     \
                          "Atomic must be the size of an int");       \
            __sync_fetch_and_add(&(atomic)->value, (val));})

#define fluid_atomic_int_get(atomic) __extension__ ({             \
            STATIC_ASSERT(sizeof((atomic)->value) == sizeof(int), \
                          "Atomic must be the size of an int");   \
            __sync_synchronize();                                 \
            (atomic)->value;})

#define fluid_atomic_int_set(atomic, newval) __extension__ ({         \
                STATIC_ASSERT(sizeof((atomic)->value) == sizeof(int), \
                              "Atomic must be the size of an int");   \
                (atomic)->value = (newval);                           \
                __sync_synchronize();})

#define fluid_atomic_int_inc(atomic) __extension__ ({             \
            STATIC_ASSERT(sizeof((atomic)->value) == sizeof(int), \
                          "Atomic must be the size of an int");   \
            __sync_synchronize();                                 \
            __sync_fetch_and_add(&(atomic)->value, 1);})

#define fluid_atomic_int_compare_and_exchange(atomic, oldval, newval) __extension__ ({ \
            STATIC_ASSERT(sizeof((atomic)->value) == sizeof(int),       \
                          "Atomic must be the size of an int");         \
            __sync_bool_compare_and_swap(&(atomic)->value, (oldval), (newval));})

#define fluid_atomic_float_get(atomic) __extension__ ({   \
  STATIC_ASSERT(sizeof((atomic)->value) == sizeof(float), \
                "Atomic must be the size of a float");    \
  __sync_synchronize();                                   \
  (atomic)->value;})

#define fluid_atomic_float_set(atomic, val) __extension__ ({  \
      STATIC_ASSERT(sizeof((atomic)->value) == sizeof(float), \
                    "Atomic must be the size of a float");    \
      (atomic)->value = (val);                                \
      __sync_synchronize();})

#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define fluid_atomic_int_inc(atomic) InterlockedIncrement((atomic))
#define fluid_atomic_int_add(atomic, val) InterlockedAdd((atomic), (val))
#define fluid_atomic_int_get(atomic) (*(LONG*)(atomic))
#define fluid_atomic_int_set(atomic, val) InterlockedExchange((atomic), (val))
#define fluid_atomic_int_exchange_and_add(atomic, add)  \
    InterlockedExchangeAdd((atomic), (add))

#define fluid_atomic_float_get(atomic) (*(FLOAT*)(atomic))

static inline float
fluid_atomic_float_set(fluid_atomic_float_t *atomic, float val)
{
    LONG ival = *(LONG*)&val;
    LONG rval = InterlockedExchange(atomic, ival);
    return *(float*)&rval;
}

#endif

#endif /* _FLUID_ATOMIC_PRIV_H */
