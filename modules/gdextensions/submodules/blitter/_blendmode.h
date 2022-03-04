/*************************************************************************/
/*  _blendmode.h                                                         */
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

///  Header file declaring the SDL_BlendMode enumeration

#ifndef _blendmode_h_
#define _blendmode_h_

#include "_begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

// The blend mode used in SDL_RenderCopy() and drawing operations.
typedef enum {
	SDL_BLENDMODE_NONE = 0x00000000, // no blending: dstRGBA = srcRGBA
	SDL_BLENDMODE_BLEND = 0x00000001, // alpha blending: dstRGB = (srcRGB * srcA) + (dstRGB * (1-srcA)), dstA = srcA + (dstA * (1-srcA)) */
	SDL_BLENDMODE_ADD = 0x00000002, // additive blending: dstRGB = (srcRGB * srcA) + dstRGB, dstA = dstA
	SDL_BLENDMODE_MOD = 0x00000004, // color modulate: dstRGB = srcRGB * dstRGB, dstA = dstA
	SDL_BLENDMODE_MUL = 0x00000008, // color multiply: dstRGB = (srcRGB * dstRGB) + (dstRGB * (1-srcA)), dstA = (srcA * dstA) + (dstA * (1-srcA))
	SDL_BLENDMODE_INVALID = 0x7FFFFFFF
} SDL_BlendMode;

// The blend operation used when combining source and destination pixel components
typedef enum {
	SDL_BLENDOPERATION_ADD = 0x1, // dst + src: supported by all renderers */
	SDL_BLENDOPERATION_SUBTRACT = 0x2, // dst - src : supported by D3D9, D3D11, OpenGL, OpenGLES */
	SDL_BLENDOPERATION_REV_SUBTRACT = 0x3, // src - dst : supported by D3D9, D3D11, OpenGL, OpenGLES */
	SDL_BLENDOPERATION_MINIMUM = 0x4, // min(dst, src) : supported by D3D11 */
	SDL_BLENDOPERATION_MAXIMUM = 0x5 // max(dst, src) : supported by D3D11 */

} SDL_BlendOperation;

// The normalized factor used to multiply pixel components
typedef enum {
	SDL_BLENDFACTOR_ZERO = 0x1, // 0, 0, 0, 0
	SDL_BLENDFACTOR_ONE = 0x2, // 1, 1, 1, 1
	SDL_BLENDFACTOR_SRC_COLOR = 0x3, // srcR, srcG, srcB, srcA
	SDL_BLENDFACTOR_ONE_MINUS_SRC_COLOR = 0x4, // 1-srcR, 1-srcG, 1-srcB, 1-srcA
	SDL_BLENDFACTOR_SRC_ALPHA = 0x5, // srcA, srcA, srcA, srcA
	SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA = 0x6, // 1-srcA, 1-srcA, 1-srcA, 1-srcA
	SDL_BLENDFACTOR_DST_COLOR = 0x7, // dstR, dstG, dstB, dstA
	SDL_BLENDFACTOR_ONE_MINUS_DST_COLOR = 0x8, // 1-dstR, 1-dstG, 1-dstB, 1-dstA
	SDL_BLENDFACTOR_DST_ALPHA = 0x9, // dstA, dstA, dstA, dstA
	SDL_BLENDFACTOR_ONE_MINUS_DST_ALPHA = 0xA // 1-dstA, 1-dstA, 1-dstA, 1-dstA

} SDL_BlendFactor;

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "_close_code.h"

#endif /* _blendmode_h_ */

/* vi: set ts=4 sw=4 expandtab: */
