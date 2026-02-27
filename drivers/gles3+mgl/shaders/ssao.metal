/**************************************************************************/
/*  ssao.metal                                                            */
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

#include "_metal_common.h"
#include "_godot_common.h"

#define TWO_PI 6.283185307179586476925286766559

#ifdef SSAO_QUALITY_HIGH
#define NUM_SAMPLES (16)
#endif

#ifdef SSAO_QUALITY_LOW
#define NUM_SAMPLES (8)
#endif

#if !defined(SSAO_QUALITY_LOW) && !defined(SSAO_QUALITY_HIGH)
#define NUM_SAMPLES (12)
#endif

// If using depth mip levels, the log of the maximum pixel offset before we need to switch to a lower
// miplevel to maintain reasonable spatial locality in the cache
#define LOG_MAX_OFFSET (3)

// This must be less than or equal to the MAX_MIP_LEVEL defined in SSAO.cpp
#define MAX_MIP_LEVEL (4)

// Rotation table - prime numbers to prevent taps from lining up
constant int ROTATIONS[100] = {
	1, 1, 2, 3, 2, 5, 2, 3, 2,
	3, 3, 5, 5, 3, 4, 7, 5, 5, 7,
	9, 8, 5, 5, 7, 7, 7, 8, 5, 8,
	11, 12, 7, 10, 13, 8, 11, 8, 7, 14,
	11, 11, 13, 12, 13, 19, 17, 13, 11, 18,
	19, 11, 11, 14, 17, 21, 15, 16, 17, 18,
	13, 17, 11, 17, 19, 18, 25, 18, 19, 19,
	29, 21, 19, 27, 31, 29, 21, 18, 17, 29,
	31, 31, 23, 18, 25, 26, 25, 23, 19, 34,
	19, 27, 21, 25, 39, 29, 17, 21, 27, 0
};

constant int NUM_SPIRAL_TURNS = ROTATIONS[NUM_SAMPLES - 1];

// SSAO uniforms
struct SSAOUniforms {
	int2 screen_size;
	float camera_z_far;
	float camera_z_near;
	float intensity_div_r6;
	float radius;
#ifdef ENABLE_RADIUS2
	float intensity_div_r62;
	float radius2;
#endif
	float bias;
	float proj_scale;
	float4 proj_info;
};

// Vertex input structure
struct SSAOVertexIn {
	float4 position [[attribute(0)]];
};

// Vertex output / fragment input
struct SSAOVertexOut {
	float4 position [[position]];
};

// ============================================================================
// Vertex Shader
// ============================================================================

vertex SSAOVertexOut vertexFunction(
	SSAOVertexIn in [[stage_in]]
) {
	SSAOVertexOut out;
	out.position = in.position;
	out.position.z = 1.0;
	return out;
}

// ============================================================================
// Helper Functions
// ============================================================================

float3 reconstructCSPosition(float2 S, float z, float4 proj_info) {
#ifdef USE_ORTHOGONAL_PROJECTION
	return float3((S.xy * proj_info.xy + proj_info.zw), z);
#else
	return float3((S.xy * proj_info.xy + proj_info.zw) * z, z);
#endif
}

float3 getPosition(int2 ssP, texture2d<float> source_depth, float camera_z_near, float camera_z_far, float4 proj_info) {
	float3 P;
	P.z = source_depth.read(uint2(ssP), 0).r;

	P.z = P.z * 2.0 - 1.0;
#ifdef USE_ORTHOGONAL_PROJECTION
	P.z = ((P.z + (camera_z_far + camera_z_near) / (camera_z_far - camera_z_near)) * (camera_z_far - camera_z_near)) / 2.0;
#else
	P.z = 2.0 * camera_z_near * camera_z_far / (camera_z_far + camera_z_near - P.z * (camera_z_far - camera_z_near));
#endif
	P.z = -P.z;

	// Offset to pixel center
	P = reconstructCSPosition(float2(ssP) + float2(0.5), P.z, proj_info);
	return P;
}

// Returns a unit vector and a screen-space radius for the tap on a unit disk
float2 tapLocation(int sampleNumber, float spinAngle, thread float& ssR) {
	// Radius relative to ssR
	float alpha = (float(sampleNumber) + 0.5) * (1.0 / float(NUM_SAMPLES));
	float angle = alpha * (float(NUM_SPIRAL_TURNS) * 6.28) + spinAngle;

	ssR = alpha;
	return float2(cos(angle), sin(angle));
}

// Read the camera-space position of the point at screen-space pixel ssP + unitOffset * ssR
float3 getOffsetPosition(int2 ssC, float2 unitOffset, float ssR,
						 texture2d<float> source_depth, texture2d<uint> source_depth_mipmaps,
						 constant SSAOUniforms& uniforms) {
	// Derivation:
	//  mipLevel = floor(log(ssR / MAX_OFFSET));
	int mipLevel = clamp(int(floor(log2(ssR))) - LOG_MAX_OFFSET, 0, MAX_MIP_LEVEL);

	int2 ssP = int2(ssR * unitOffset) + ssC;

	float3 P;

	// We need to divide by 2^mipLevel to read the appropriately scaled coordinate from a MIP-map.
	// Manually clamp to the texture size because texelFetch bypasses the texture unit
	int2 mipP = clamp(ssP >> mipLevel, int2(0), (uniforms.screen_size >> mipLevel) - int2(1));

	if (mipLevel < 1) {
		// read from depth buffer
		P.z = source_depth.read(uint2(mipP), 0).r;
		P.z = P.z * 2.0 - 1.0;
#ifdef USE_ORTHOGONAL_PROJECTION
		P.z = ((P.z + (uniforms.camera_z_far + uniforms.camera_z_near) / (uniforms.camera_z_far - uniforms.camera_z_near)) * (uniforms.camera_z_far - uniforms.camera_z_near)) / 2.0;
#else
		P.z = 2.0 * uniforms.camera_z_near * uniforms.camera_z_far / (uniforms.camera_z_far + uniforms.camera_z_near - P.z * (uniforms.camera_z_far - uniforms.camera_z_near));
#endif
		P.z = -P.z;
	} else {
		// read from mipmaps
		uint d = source_depth_mipmaps.read(uint2(mipP), mipLevel - 1).r;
		P.z = -(float(d) / 65535.0) * uniforms.camera_z_far;
	}

	// Offset to pixel center
	P = reconstructCSPosition(float2(ssP) + float2(0.5), P.z, uniforms.proj_info);

	return P;
}

// Compute the occlusion due to sample with index i about the pixel at ssC
float sampleAO(int2 ssC, float3 C, float3 n_C, float ssDiskRadius, float p_radius,
			   int tapIndex, float randomPatternRotationAngle,
			   texture2d<float> source_depth, texture2d<uint> source_depth_mipmaps,
			   constant SSAOUniforms& uniforms) {
	// Offset on the unit disk, spun for this pixel
	float ssR;
	float2 unitOffset = tapLocation(tapIndex, randomPatternRotationAngle, ssR);
	ssR *= ssDiskRadius;

	// The occluding point in camera space
	float3 Q = getOffsetPosition(ssC, unitOffset, ssR, source_depth, source_depth_mipmaps, uniforms);

	float3 v = Q - C;

	float vv = dot(v, v);
	float vn = dot(v, n_C);

	const float epsilon = 0.01;
	float radius2 = p_radius * p_radius;

	// Smoother transition to zero (lowers contrast, smoothing out corners). [Recommended]
	float f = max(radius2 - vv, 0.0);
	return f * f * f * max((vn - uniforms.bias) / (epsilon + vv), 0.0);
}

// ============================================================================
// Fragment Shader
// ============================================================================

fragment float fragmentFunction(
	SSAOVertexOut in [[stage_in]],
	constant SSAOUniforms& uniforms [[buffer(1)]],
	texture2d<float> source_depth [[texture(0)]],
	texture2d<uint> source_depth_mipmaps [[texture(1)]],
	texture2d<float> source_normal [[texture(2)]]
) {
	// Pixel being shaded
	int2 ssC = int2(in.position.xy);

	// World space point being shaded
	float3 C = getPosition(ssC, source_depth, uniforms.camera_z_near, uniforms.camera_z_far, uniforms.proj_info);

	// Reconstruct normals from positions using derivatives
	float3 n_C = normalize(cross(dfdx(C), dfdy(C)));
	n_C = -n_C;

	// Hash function used in the HPG12 AlchemyAO paper
	float randomPatternRotationAngle = fmod(float((3 * ssC.x ^ ssC.y + ssC.x * ssC.y) * 10), TWO_PI);

	// Choose the screen-space sample radius
	// proportional to the projected area of the sphere
#ifdef USE_ORTHOGONAL_PROJECTION
	float ssDiskRadius = -uniforms.proj_scale * uniforms.radius;
#else
	float ssDiskRadius = -uniforms.proj_scale * uniforms.radius / C.z;
#endif

	float sum = 0.0;
	for (int i = 0; i < NUM_SAMPLES; ++i) {
		sum += sampleAO(ssC, C, n_C, ssDiskRadius, uniforms.radius, i, randomPatternRotationAngle,
						source_depth, source_depth_mipmaps, uniforms);
	}

	float A = max(0.0, 1.0 - sum * uniforms.intensity_div_r6 * (5.0 / float(NUM_SAMPLES)));

#ifdef ENABLE_RADIUS2
	// go again for radius2
	randomPatternRotationAngle = fmod(float((5 * ssC.x ^ ssC.y + ssC.x * ssC.y) * 11), TWO_PI);

	ssDiskRadius = -uniforms.proj_scale * uniforms.radius2 / C.z;

	sum = 0.0;
	for (int i = 0; i < NUM_SAMPLES; ++i) {
		sum += sampleAO(ssC, C, n_C, ssDiskRadius, uniforms.radius2, i, randomPatternRotationAngle,
						source_depth, source_depth_mipmaps, uniforms);
	}

	A = min(A, max(0.0, 1.0 - sum * uniforms.intensity_div_r62 * (5.0 / float(NUM_SAMPLES))));
#endif

	// Bilateral box-filter over a quad for free, respecting depth edges
	if (abs(dfdx(C.z)) < 0.02) {
		A -= dfdx(A) * (float(ssC.x & 1) - 0.5);
	}
	if (abs(dfdy(C.z)) < 0.02) {
		A -= dfdy(A) * (float(ssC.y & 1) - 0.5);
	}

	return A;
}
