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

#ifndef _FLUID_AUDRIVER_H
#define _FLUID_AUDRIVER_H

#include "fluidsynth_priv.h"

/*
 * fluid_audio_driver_t
 */

typedef struct _fluid_audriver_definition_t fluid_audriver_definition_t;

struct _fluid_audio_driver_t
{
    const fluid_audriver_definition_t *define;
};

void fluid_audio_driver_settings(fluid_settings_t *settings);

/* Defined in fluid_filerenderer.c */
void fluid_file_renderer_settings(fluid_settings_t *settings);
#if AUFILE_SUPPORT
fluid_audio_driver_t *new_fluid_file_audio_driver(fluid_settings_t *settings,
        fluid_synth_t *synth);
void delete_fluid_file_audio_driver(fluid_audio_driver_t *p);
#endif



#endif  /* _FLUID_AUDRIVER_H */
