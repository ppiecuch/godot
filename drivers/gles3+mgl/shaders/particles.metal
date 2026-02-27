/**************************************************************************/
/*  particles.metal                                                       */
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

// GPU Particles compute shader - converts Transform Feedback to compute
// This shader simulates particle behavior

#define MAX_ATTRACTORS 64

// Attractor structure
struct Attractor {
	float3 pos;
	float radius;
	float3 dir;
	float eat_radius;
	float strength;
	float attenuation;
	float2 _pad;
};

// Particle uniforms
struct ParticleUniforms {
	float4x4 emission_transform;
	float system_phase;
	float prev_system_phase;
	float time;
	float delta;
	float lifetime;
	float explosiveness;
	float randomness;
	int total_particles;
	int attractor_count;
	uint cycle;
	uint random_seed;
	bool emitting;
	bool clear;
};

// Particle state (input/output)
struct ParticleData {
	float4 color;
	float4 velocity_active;  // xyz = velocity, w = active flag
	float4 custom;
	float4 xform_1;
	float4 xform_2;
	float4 xform_3;
};

// ============================================================================
// Helper Functions
// ============================================================================

uint hash(uint x) {
	x = ((x >> uint(16)) ^ x) * uint(0x45d9f3b);
	x = ((x >> uint(16)) ^ x) * uint(0x45d9f3b);
	x = (x >> uint(16)) ^ x;
	return x;
}

// Build 4x4 matrix from rows
float4x4 make_xform(float4 r0, float4 r1, float4 r2) {
	return transpose(float4x4(r0, r1, r2, float4(0.0, 0.0, 0.0, 1.0)));
}

// ============================================================================
// Compute Kernel
// ============================================================================

kernel void particleKernel(
	constant ParticleUniforms& uniforms [[buffer(0)]],
	constant Attractor* attractors [[buffer(1)]],
	const device ParticleData* particles_in [[buffer(2)]],
	device ParticleData* particles_out [[buffer(3)]],
#if defined(USE_MATERIAL)
	constant void* material_uniforms [[buffer(4)]],
#endif
	uint gid [[thread_position_in_grid]]
) {
	if (int(gid) >= uniforms.total_particles) {
		return;
	}

	int index = int(gid);
	ParticleData particle = particles_in[gid];

	bool apply_forces = true;
	bool apply_velocity = true;
	float local_delta = uniforms.delta;
	float mass = 1.0;

	// Calculate restart phase for this particle
	float restart_phase = float(index) / float(uniforms.total_particles);

	if (uniforms.randomness > 0.0) {
		uint seed = uniforms.cycle;
		if (restart_phase >= uniforms.system_phase) {
			seed -= uint(1);
		}
		seed *= uint(uniforms.total_particles);
		seed += uint(index);
		float random = float(hash(seed) % uint(65536)) / 65536.0;
		restart_phase += uniforms.randomness * random * 1.0 / float(uniforms.total_particles);
	}

	restart_phase *= (1.0 - uniforms.explosiveness);
	bool restart = false;
	bool shader_active = particle.velocity_active.w > 0.5;

	if (uniforms.system_phase > uniforms.prev_system_phase) {
		// Normal phase progression
		if (restart_phase >= uniforms.prev_system_phase && restart_phase < uniforms.system_phase) {
			restart = true;
#ifdef USE_FRACTIONAL_DELTA
			local_delta = (uniforms.system_phase - restart_phase) * uniforms.lifetime;
#endif
		}
	} else if (uniforms.delta > 0.0) {
		// Phase wrapped around
		if (restart_phase >= uniforms.prev_system_phase) {
			restart = true;
#ifdef USE_FRACTIONAL_DELTA
			local_delta = (1.0 - restart_phase + uniforms.system_phase) * uniforms.lifetime;
#endif
		} else if (restart_phase < uniforms.system_phase) {
			restart = true;
#ifdef USE_FRACTIONAL_DELTA
			local_delta = (uniforms.system_phase - restart_phase) * uniforms.lifetime;
#endif
		}
	}

	uint current_cycle = uniforms.cycle;
	if (uniforms.system_phase < restart_phase) {
		current_cycle -= uint(1);
	}

	uint particle_number = current_cycle * uint(uniforms.total_particles) + uint(index);

	if (restart) {
		shader_active = uniforms.emitting;
	}

	float4x4 xform;
	float4 out_color;
	float4 out_velocity_active;
	float4 out_custom;

#if defined(ENABLE_KEEP_DATA)
	if (uniforms.clear) {
#else
	if (uniforms.clear || restart) {
#endif
		out_color = float4(1.0);
		out_velocity_active = float4(0.0);
		out_custom = float4(0.0);
		if (!restart) {
			shader_active = false;
		}

		xform = float4x4(
			float4(1.0, 0.0, 0.0, 0.0),
			float4(0.0, 1.0, 0.0, 0.0),
			float4(0.0, 0.0, 1.0, 0.0),
			float4(0.0, 0.0, 0.0, 1.0));
	} else {
		out_color = particle.color;
		out_velocity_active = particle.velocity_active;
		out_custom = particle.custom;
		xform = make_xform(particle.xform_1, particle.xform_2, particle.xform_3);
	}

	if (shader_active) {
		// User-defined particle shader code would be inserted here
		// via code generation, similar to how VERTEX_SHADER_CODE works in GLSL
		// For the base shader, we just do the default simulation

		// VERTEX_SHADER_CODE placeholder - generated code goes here

#if !defined(DISABLE_FORCE)
		// Apply attractors (currently disabled in GLSL with "if (false)")
		// Keeping the structure for potential future use
		if (false) {
			float3 force = float3(0.0);
			for (int i = 0; i < uniforms.attractor_count; i++) {
				float3 rel_vec = xform[3].xyz - attractors[i].pos;
				float dist = length(rel_vec);
				if (attractors[i].radius < dist) {
					continue;
				}
				if (attractors[i].eat_radius > 0.0 && attractors[i].eat_radius > dist) {
					out_velocity_active.w = 0.0;
				}

				rel_vec = normalize(rel_vec);
				float attenuation = pow(dist / attractors[i].radius, attractors[i].attenuation);

				if (all(attractors[i].dir == float3(0.0))) {
					// Towards center
					force += attractors[i].strength * rel_vec * attenuation * mass;
				} else {
					force += attractors[i].strength * attractors[i].dir * attenuation * mass;
				}
			}

			out_velocity_active.xyz += force * local_delta;
		}
#endif

#if !defined(DISABLE_VELOCITY)
		if (true) {
			xform[3].xyz += out_velocity_active.xyz * local_delta;
		}
#endif
	} else {
		xform = float4x4(0.0);
	}

	// Transpose back for storage
	xform = transpose(xform);

	out_velocity_active.w = float(shader_active);

	// Write output
	ParticleData out_particle;
	out_particle.color = out_color;
	out_particle.velocity_active = out_velocity_active;
	out_particle.custom = out_custom;
	out_particle.xform_1 = xform[0];
	out_particle.xform_2 = xform[1];
	out_particle.xform_3 = xform[2];

	particles_out[gid] = out_particle;
}

// ============================================================================
// Alternative kernel for materials with user-defined code
// This would be generated with VERTEX_SHADER_CODE inserted
// ============================================================================

// Note: In practice, the particle material system would generate
// specialized kernels with the user's particle shader code inserted.
// The structure above provides the framework for that code generation.
