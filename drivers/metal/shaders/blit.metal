/*************************************************************************/
/*  blit.metal                                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

// Copyright 2019 The ANGLE Project. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// blit.metal: Implements blitting texture content to current frame buffer.

#include "metal_common.h"


struct BlitParams
{
    // 0: lower left, 1: lower right, 2: upper left
    float2 srcTexCoords[3];
    int srcLevel;
    bool srcLuminance; // source texture is luminance texture
    bool dstFlipViewportY;
    bool dstLuminance; // destination texture is luminance;
};

struct BlitVSOut
{
    float4 position [[position]];
    float2 texCoords [[user(locn1)]];
};

vertex BlitVSOut blitVS(unsigned int vid [[ vertex_id ]],
                         constant BlitParams &options [[buffer(0)]])
{
    BlitVSOut output;
    output.position  = float4(gCorners[vid], 0.0, 1.0);
    output.texCoords = options.srcTexCoords[vid];

    if (!options.dstFlipViewportY)
    {
        // If viewport is not flipped, we have to flip Y in normalized device coordinates.
        // Since NDC has Y is opposite direction of viewport coodrinates.
        output.position.y = -output.position.y;
    }

    return output;
}

float4 blitSampleTexture(texture2d<float> srcTexture,
                     float2 texCoords,
                     constant BlitParams &options)
{
    constexpr sampler textureSampler(mag_filter::linear,
                                     min_filter::linear);
    float4 output = srcTexture.sample(textureSampler, texCoords, level(options.srcLevel));

    if (options.srcLuminance)
    {
        output.gb = float2(output.r, output.r);
    }

    return output;
}

float4 blitOutput(float4 color, constant BlitParams &options)
{
    float4 ret = color;

    if (options.dstLuminance)
    {
        ret.r = ret.g = ret.b = color.r;
    }

    return ret;
}

fragment float4 blitFS(BlitVSOut input [[stage_in]],
                       texture2d<float> srcTexture [[texture(0)]],
                       constant BlitParams &options [[buffer(0)]])
{
    return blitOutput(blitSampleTexture(srcTexture, input.texCoords, options), options);
}

fragment float4 blitPremultiplyAlphaFS(BlitVSOut input [[stage_in]],
                                       texture2d<float> srcTexture [[texture(0)]],
                                       constant BlitParams &options [[buffer(0)]])
{
    float4 output = blitSampleTexture(srcTexture, input.texCoords, options);
    output.xyz *= output.a;
    return blitOutput(output, options);
}

fragment float4 blitUnmultiplyAlphaFS(BlitVSOut input [[stage_in]],
                                      texture2d<float> srcTexture [[texture(0)]],
                                      constant BlitParams &options [[buffer(0)]])
{
    float4 output = blitSampleTexture(srcTexture, input.texCoords, options);
    if (output.a != 0.0)
    {
        output.xyz *= 1.0 / output.a;
    }
    return blitOutput(output, options);
}
