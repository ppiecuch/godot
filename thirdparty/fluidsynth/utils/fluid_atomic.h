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

#ifndef _FLUID_ATOMIC_H
#define _FLUID_ATOMIC_H

#if defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4) || defined(__riscos__)

typedef struct {
    volatile int value;
} fluid_atomic_int_t;
typedef struct {
    volatile unsigned value;
} fluid_atomic_uint_t;
typedef struct {
    volatile float value;
} fluid_atomic_float_t;

#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef volatile LONG fluid_atomic_int_t;
typedef volatile ULONG fluid_atomic_uint_t;
typedef volatile LONG fluid_atomic_float_t;

#endif

#endif /* _FLUID_ATOMIC_H */
