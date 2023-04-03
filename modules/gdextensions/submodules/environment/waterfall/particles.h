/**************************************************************************/
/*  particles.h                                                           */
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

extern "C" const unsigned char *particles_size_s[];
extern "C" const unsigned char *particles_size_m[];
extern "C" const unsigned char *particles_size_l[];

#include "common/enum_string.h"

enum WaterfallParticles {
	WATERFALL_PARTICLE_DROP1,
	WATERFALL_PARTICLE_DROP2,
	WATERFALL_PARTICLE_DROP3,
	WATERFALL_PARTICLE_DROP4,
	WATERFALL_PARTICLE_CLOUD1,
	WATERFALL_PARTICLE_CLOUD2,
	WATERFALL_PARTICLE_COUNT,
};

BeginEnumString(WaterfallParticles) {
	EnumString(WATERFALL_PARTICLE_DROP1);
	EnumString(WATERFALL_PARTICLE_DROP2);
	EnumString(WATERFALL_PARTICLE_DROP3);
	EnumString(WATERFALL_PARTICLE_DROP4);
	EnumString(WATERFALL_PARTICLE_CLOUD1);
	EnumString(WATERFALL_PARTICLE_CLOUD2);
}
EndEnumString;

enum WaterfallParticlesSize {
	WATERFALL_PARTICLE_SIZE_S,
	WATERFALL_PARTICLE_SIZE_M,
	WATERFALL_PARTICLE_SIZE_L,
};

const bool _available_particles[] = {
	particles_size_s[0] != nullptr,
	particles_size_m[0] != nullptr,
	particles_size_l[0] != nullptr,
};
