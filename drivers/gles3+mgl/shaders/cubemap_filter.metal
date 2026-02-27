/**************************************************************************/
/*  cubemap_filter.metal                                                  */
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

#define M_PI 3.14159265359

// Quality settings
#ifdef LOW_QUALITY
#define SAMPLE_COUNT 64
#define SAMPLE_DELTA 0.1
#else
#define SAMPLE_COUNT 512
#define SAMPLE_DELTA 0.03
#endif

// Cubemap filter uniforms
struct CubemapFilterUniforms {
	int face_id;
	float roughness;
	bool z_flip;
#if defined(USE_SOURCE_PANORAMA) || defined(COMPUTE_IRRADIANCE)
	float source_resolution;
#endif
#if defined(USE_SOURCE_DUAL_PARABOLOID) || defined(COMPUTE_IRRADIANCE)
	float source_mip_level;
#endif
#ifdef USE_SOURCE_DUAL_PARABOLOID_ARRAY
	int source_array_index;
#endif
};

// Vertex input structure
struct CubemapFilterVertexIn {
	float4 position [[attribute(0)]];
	float2 uv [[attribute(4)]];
};

// Interpolated vertex output / fragment input
struct CubemapFilterVertexOut {
	float4 position [[position]];
	float2 uv_interp;
};

// ============================================================================
// Vertex Shader
// ============================================================================

vertex CubemapFilterVertexOut vertexFunction(
	CubemapFilterVertexIn in [[stage_in]]
) {
	CubemapFilterVertexOut out;
	out.uv_interp = in.uv;
	out.position = float4(in.position.xy, 0.0, 1.0);
	return out;
}

// ============================================================================
// Helper Functions
// ============================================================================

// Convert texel coordinate to cube direction vector
float3 texelCoordToVec(float2 uv, int faceID) {
	// Face UV vectors for each cubemap face
	// faceUvVectors[face][0] = u direction
	// faceUvVectors[face][1] = v direction
	// faceUvVectors[face][2] = face center direction

	float3 result;

	if (faceID == 0) {
		// -x face
		result = float3(0.0, 0.0, 1.0) * uv.x + float3(0.0, -1.0, 0.0) * uv.y + float3(-1.0, 0.0, 0.0);
	} else if (faceID == 1) {
		// +x face
		result = float3(0.0, 0.0, -1.0) * uv.x + float3(0.0, -1.0, 0.0) * uv.y + float3(1.0, 0.0, 0.0);
	} else if (faceID == 2) {
		// -y face
		result = float3(1.0, 0.0, 0.0) * uv.x + float3(0.0, 0.0, -1.0) * uv.y + float3(0.0, -1.0, 0.0);
	} else if (faceID == 3) {
		// +y face
		result = float3(1.0, 0.0, 0.0) * uv.x + float3(0.0, 0.0, 1.0) * uv.y + float3(0.0, 1.0, 0.0);
	} else if (faceID == 4) {
		// -z face
		result = float3(-1.0, 0.0, 0.0) * uv.x + float3(0.0, -1.0, 0.0) * uv.y + float3(0.0, 0.0, -1.0);
	} else {
		// +z face
		result = float3(1.0, 0.0, 0.0) * uv.x + float3(0.0, -1.0, 0.0) * uv.y + float3(0.0, 0.0, 1.0);
	}

	return normalize(result);
}

// Importance sample GGX distribution
float3 ImportanceSampleGGX(float2 Xi, float Roughness, float3 N) {
	float a = Roughness * Roughness; // Disney's roughness [Burley'12 SIGGRAPH]

	// Compute distribution direction
	float Phi = 2.0 * M_PI * Xi.x;
	float CosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float SinTheta = sqrt(1.0 - CosTheta * CosTheta);

	// Convert to spherical direction
	float3 H;
	H.x = SinTheta * cos(Phi);
	H.y = SinTheta * sin(Phi);
	H.z = CosTheta;

	float3 UpVector = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
	float3 TangentX = normalize(cross(UpVector, N));
	float3 TangentY = cross(N, TangentX);

	// Tangent to world space
	return TangentX * H.x + TangentY * H.y + N * H.z;
}

// GGX Distribution function (Trowbridge-Reitz)
float DistributionGGX(float3 N, float3 H, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = M_PI * denom * denom;

	return nom / denom;
}

// GGX Geometry function (Schlick approximation)
// http://graphicrants.blogspot.com.au/2013/08/specular-brdf-reference.html
float GGX(float NdotV, float a) {
	float k = a / 2.0;
	return NdotV / (NdotV * (1.0 - k) + k);
}

// Smith's geometry function for GGX
// http://graphicrants.blogspot.com.au/2013/08/specular-brdf-reference.html
float G_Smith(float a, float nDotV, float nDotL) {
	return GGX(nDotL, a * a) * GGX(nDotV, a * a);
}

// Radical inverse using bit manipulation (Van der Corput sequence)
float radicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// Hammersley low-discrepancy sequence
float2 Hammersley(uint i, uint N) {
	return float2(float(i) / float(N), radicalInverse_VdC(i));
}

#ifdef USE_SOURCE_PANORAMA
float4 texturePanorama(float3 normal, texture2d<float> pano, sampler panoSampler, float mipLevel) {
	float2 st = float2(
		atan2(normal.x, normal.z),
		acos(normal.y));

	if (st.x < 0.0)
		st.x += M_PI * 2.0;

	st /= float2(M_PI * 2.0, M_PI);

	return pano.sample(panoSampler, st, level(mipLevel));
}
#endif

#ifdef USE_SOURCE_DUAL_PARABOLOID_ARRAY
float4 textureDualParaboloidArray(float3 normal, texture2d_array<float> dpArray, sampler dpSampler, int arrayIndex) {
	float3 norm = normalize(normal);
	norm.xy /= 1.0 + abs(norm.z);
	norm.xy = norm.xy * float2(0.5, 0.25) + float2(0.5, 0.25);
	if (norm.z < 0.0) {
		norm.y = 0.5 - norm.y + 0.5;
	}
	return dpArray.sample(dpSampler, norm.xy, uint(arrayIndex), level(0.0));
}
#endif

#ifdef USE_SOURCE_DUAL_PARABOLOID
float4 textureDualParaboloid(float3 normal, texture2d<float> dp, sampler dpSampler, float mipLevel) {
	float3 norm = normalize(normal);
	norm.xy /= 1.0 + abs(norm.z);
	norm.xy = norm.xy * float2(0.5, 0.25) + float2(0.5, 0.25);
	if (norm.z < 0.0) {
		norm.y = 0.5 - norm.y + 0.5;
	}
	return dp.sample(dpSampler, norm.xy, level(mipLevel));
}
#endif

// ============================================================================
// Fragment Shader
// ============================================================================

fragment float4 fragmentFunction(
	CubemapFilterVertexOut in [[stage_in]],
	constant CubemapFilterUniforms& uniforms [[buffer(1)]]
#ifdef USE_SOURCE_PANORAMA
	, texture2d<float> source_panorama [[texture(0)]]
	, sampler panoramaSampler [[sampler(0)]]
#elif defined(USE_SOURCE_DUAL_PARABOLOID_ARRAY)
	, texture2d_array<float> source_dual_paraboloid_array [[texture(0)]]
	, sampler dpArraySampler [[sampler(0)]]
#elif defined(USE_SOURCE_DUAL_PARABOLOID)
	, texture2d<float> source_dual_paraboloid [[texture(0)]]
	, sampler dpSampler [[sampler(0)]]
#else
	, texturecube<float> source_cube [[texture(0)]]
	, sampler cubeSampler [[sampler(0)]]
#endif
) {
	float3 N;

#ifdef USE_DUAL_PARABOLOID
	N = float3(in.uv_interp * 2.0 - 1.0, 0.0);
	N.z = 0.5 - 0.5 * ((N.x * N.x) + (N.y * N.y));
	N = normalize(N);

	if (uniforms.z_flip) {
		N.y = -N.y; // y is flipped to improve blending between both sides
		N.z = -N.z;
	}
#else
	float2 uv = (in.uv_interp * 2.0) - 1.0;
	N = texelCoordToVec(uv, uniforms.face_id);
#endif

#ifdef USE_DIRECT_WRITE
	// Direct copy mode - no filtering
#ifdef USE_SOURCE_PANORAMA
	return float4(texturePanorama(N, source_panorama, panoramaSampler, 0.0).rgb, 1.0);
#elif defined(USE_SOURCE_DUAL_PARABOLOID_ARRAY)
	return float4(textureDualParaboloidArray(N, source_dual_paraboloid_array, dpArraySampler, uniforms.source_array_index).rgb, 1.0);
#elif defined(USE_SOURCE_DUAL_PARABOLOID)
	return float4(textureDualParaboloid(N, source_dual_paraboloid, dpSampler, uniforms.source_mip_level).rgb, 1.0);
#else
	float3 N_cube = N;
	N_cube.y = -N_cube.y;
	return float4(source_cube.sample(cubeSampler, N_cube).rgb, 1.0);
#endif

#else // USE_DIRECT_WRITE

#ifdef COMPUTE_IRRADIANCE
	// Irradiance convolution for diffuse IBL
	float3 irradiance = float3(0.0);

	// Tangent space calculation from origin point
	float3 UpVector = float3(0.0, 1.0, 0.0);
	float3 TangentX = cross(UpVector, N);
	float3 TangentY = cross(N, TangentX);

	float num_samples = 0.0;

	for (float phi = 0.0; phi < 2.0 * M_PI; phi += SAMPLE_DELTA) {
		for (float theta = 0.0; theta < 0.5 * M_PI; theta += SAMPLE_DELTA) {
			// Calculate sample positions
			float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// Find world vector of sample position
			float3 H = tangentSample.x * TangentX + tangentSample.y * TangentY + tangentSample.z * N;

			float2 st = float2(atan2(H.x, H.z), acos(H.y));
			if (st.x < 0.0) {
				st.x += M_PI * 2.0;
			}
			st /= float2(M_PI * 2.0, M_PI);

#ifdef USE_SOURCE_PANORAMA
			irradiance += source_panorama.sample(panoramaSampler, st, level(uniforms.source_mip_level)).rgb * cos(theta) * sin(theta);
#endif
			num_samples += 1.0;
		}
	}
	irradiance = M_PI * irradiance * (1.0 / num_samples);

	return float4(irradiance, 1.0);

#else
	// Importance sampling GGX for specular IBL (prefiltered environment map)
	float4 sum = float4(0.0);

	for (uint sampleNum = 0u; sampleNum < SAMPLE_COUNT; sampleNum++) {
		float2 xi = Hammersley(sampleNum, SAMPLE_COUNT);

		float3 H = normalize(ImportanceSampleGGX(xi, uniforms.roughness, N));
		float3 V = N;
		float3 L = normalize(2.0 * dot(V, H) * H - V);

		float ndotl = max(dot(N, L), 0.0);

		if (ndotl > 0.0) {
#ifdef USE_SOURCE_PANORAMA
			float D = DistributionGGX(N, H, uniforms.roughness);
			float ndoth = max(dot(N, H), 0.0);
			float hdotv = max(dot(H, V), 0.0);
			float pdf = D * ndoth / (4.0 * hdotv) + 0.0001;

			float saTexel = 4.0 * M_PI / (6.0 * uniforms.source_resolution * uniforms.source_resolution);
			float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

			float mipLevel = uniforms.roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

			sum.rgb += texturePanorama(L, source_panorama, panoramaSampler, mipLevel).rgb * ndotl;
#elif defined(USE_SOURCE_DUAL_PARABOLOID_ARRAY)
			sum.rgb += textureDualParaboloidArray(L, source_dual_paraboloid_array, dpArraySampler, uniforms.source_array_index).rgb * ndotl;
#elif defined(USE_SOURCE_DUAL_PARABOLOID)
			sum.rgb += textureDualParaboloid(L, source_dual_paraboloid, dpSampler, uniforms.source_mip_level).rgb * ndotl;
#else
			float3 L_cube = L;
			L_cube.y = -L_cube.y;
			sum.rgb += source_cube.sample(cubeSampler, L_cube, level(0.0)).rgb * ndotl;
#endif
			sum.a += ndotl;
		}
	}
	sum /= sum.a;

	return float4(sum.rgb, 1.0);

#endif // COMPUTE_IRRADIANCE
#endif // USE_DIRECT_WRITE
}
