/**************************************************************************/
/*  procedural_kit_generator.cpp                                         */
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

#include "procedural_kit_generator.h"

#include "trackergen_instrument.h"

#include "core/error_macros.h"
#include "core/math/math_funcs.h"

// Deterministic LCG so noise tables are identical across variants (phase aligned).
static _FORCE_INLINE_ float lcg_bipolar(uint32_t &s) {
	s = s * 1664525u + 1013904223u;
	return ((s >> 8) & 0xFFFFFF) / 8388608.0f - 1.0f; // ~[-1,1)
}

float ProceduralKitGenerator::osc(int p_waveform, double p_phase, int p_harmonics, double p_falloff, double p_pulse_width, int p_fm_ratio, double p_fm_amount, uint32_t &r_rng) {
	const double tau = Math_PI * 2.0;

	// 2-op FM overrides the additive waveform: carrier sine phase-modulated by a
	// sine at an integer ratio (stays periodic within one cycle). Bells/e-pianos.
	if (p_fm_amount > 0.0) {
		const double mod = p_fm_amount * Math::sin(tau * p_fm_ratio * p_phase);
		return (float)Math::sin(tau * p_phase + mod);
	}

	switch (p_waveform) {
		case TrackerInstrument::WAVE_SINE:
			return (float)Math::sin(tau * p_phase);

		case TrackerInstrument::WAVE_SAW: {
			double acc = 0.0;
			for (int k = 1; k <= p_harmonics; k++) {
				acc += Math::sin(tau * k * p_phase) / Math::pow((double)k, p_falloff);
			}
			return (float)(acc * (2.0 / Math_PI));
		}

		case TrackerInstrument::WAVE_SQUARE: {
			double acc = 0.0;
			for (int k = 1; k <= p_harmonics; k += 2) {
				acc += Math::sin(tau * k * p_phase) / Math::pow((double)k, p_falloff);
			}
			return (float)(acc * (4.0 / Math_PI));
		}

		case TrackerInstrument::WAVE_TRIANGLE: {
			double acc = 0.0;
			double sign = 1.0;
			for (int k = 1; k <= p_harmonics; k += 2) {
				acc += sign * Math::sin(tau * k * p_phase) / ((double)k * (double)k);
				sign = -sign;
			}
			return (float)(acc * (8.0 / (Math_PI * Math_PI)));
		}

		case TrackerInstrument::WAVE_PULSE: {
			double acc = 0.0;
			for (int k = 1; k <= p_harmonics; k++) {
				acc += (Math::sin(k * Math_PI * p_pulse_width) / (double)k) * Math::cos(tau * k * p_phase);
			}
			return (float)(acc * (2.0 / Math_PI));
		}

		case TrackerInstrument::WAVE_NOISE:
			return lcg_bipolar(r_rng);

		default:
			return (float)Math::sin(tau * p_phase);
	}
}

void ProceduralKitGenerator::biquad_lowpass_circular(float *p_data, int p_n, double p_fc_norm, double p_q) {
	const double w0 = 2.0 * Math_PI * p_fc_norm;
	const double cw = Math::cos(w0);
	const double sw = Math::sin(w0);
	const double alpha = sw / (2.0 * p_q);

	double b0 = (1.0 - cw) * 0.5;
	double b1 = 1.0 - cw;
	double b2 = (1.0 - cw) * 0.5;
	double a0 = 1.0 + alpha;
	double a1 = -2.0 * cw;
	double a2 = 1.0 - alpha;
	b0 /= a0;
	b1 /= a0;
	b2 /= a0;
	a1 /= a0;
	a2 /= a0;

	double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
	// Warm-up passes so the periodic steady state is reached before capture.
	const int passes = 4;
	for (int pass = 0; pass < passes; pass++) {
		const bool capture = (pass == passes - 1);
		for (int i = 0; i < p_n; i++) {
			double x = p_data[i];
			double y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
			x2 = x1;
			x1 = x;
			y2 = y1;
			y1 = y;
			if (capture) {
				p_data[i] = (float)y;
			}
		}
	}
}

void ProceduralKitGenerator::normalize_peak(float *p_data, int p_n, float p_target) {
	float peak = 0.0f;
	for (int i = 0; i < p_n; i++) {
		float a = Math::abs(p_data[i]);
		if (a > peak) {
			peak = a;
		}
	}
	if (peak > 1e-6f) {
		const float g = p_target / peak;
		for (int i = 0; i < p_n; i++) {
			p_data[i] *= g;
		}
	}
}

void ProceduralKitGenerator::normalize_lufs(float *p_data, int p_n, double p_fs_eq, float p_target_lufs) {
	if (p_n <= 0 || p_fs_eq <= 0.0) {
		return;
	}

	// Stage 1: BS.1770 pre-filter (high shelf ~+4 dB @ 1.68 kHz).
	double b0_1, b1_1, b2_1, a1_1, a2_1;
	{
		const double f0 = 1681.974450955533, G = 3.999843853973347, Q = 0.7071752369554196;
		const double K = Math::tan(Math_PI * f0 / p_fs_eq);
		const double Vh = Math::pow(10.0, G / 20.0);
		const double Vb = Math::pow(Vh, 0.4996667741545416);
		const double a0 = 1.0 + K / Q + K * K;
		b0_1 = (Vh + Vb * K / Q + K * K) / a0;
		b1_1 = 2.0 * (K * K - Vh) / a0;
		b2_1 = (Vh - Vb * K / Q + K * K) / a0;
		a1_1 = 2.0 * (K * K - 1.0) / a0;
		a2_1 = (1.0 - K / Q + K * K) / a0;
	}
	// Stage 2: RLB high-pass (~38 Hz) — discounts sub energy (the reason RMS lied).
	double b0_2, b1_2, b2_2, a1_2, a2_2;
	{
		const double f0 = 38.13547087602444, Q = 0.5003270373238773;
		const double K = Math::tan(Math_PI * f0 / p_fs_eq);
		const double a0 = 1.0 + K / Q + K * K;
		b0_2 = 1.0 / a0;
		b1_2 = -2.0 / a0;
		b2_2 = 1.0 / a0;
		a1_2 = 2.0 * (K * K - 1.0) / a0;
		a2_2 = (1.0 - K / Q + K * K) / a0;
	}

	// Run both filters circularly to steady state, measure mean-square on the last pass.
	double x1 = 0, x2 = 0, y1 = 0, y2 = 0; // stage 1 state
	double u1 = 0, u2 = 0, z1 = 0, z2 = 0; // stage 2 state
	const int passes = 4;
	double ms = 0.0;
	for (int p = 0; p < passes; p++) {
		const bool cap = (p == passes - 1);
		double acc = 0.0;
		for (int i = 0; i < p_n; i++) {
			const double x = p_data[i];
			const double y = b0_1 * x + b1_1 * x1 + b2_1 * x2 - a1_1 * y1 - a2_1 * y2;
			x2 = x1;
			x1 = x;
			y2 = y1;
			y1 = y;
			const double z = b0_2 * y + b1_2 * u1 + b2_2 * u2 - a1_2 * z1 - a2_2 * z2;
			u2 = u1;
			u1 = y;
			z2 = z1;
			z1 = z;
			if (cap) {
				acc += z * z;
			}
		}
		if (cap) {
			ms = acc / (double)p_n;
		}
	}
	if (ms < 1e-12) {
		return;
	}

	const double loudness = -0.691 + 10.0 * (Math::log(ms) / 2.302585092994046); // LUFS
	const double gain = Math::pow(10.0, (p_target_lufs - loudness) / 20.0);
	for (int i = 0; i < p_n; i++) {
		p_data[i] = (float)(p_data[i] * gain);
	}

	// Peak-safe clamp (LUFS gain can push peaks past 1.0).
	float peak = 0.0f;
	for (int i = 0; i < p_n; i++) {
		const float a = Math::abs(p_data[i]);
		if (a > peak) {
			peak = a;
		}
	}
	if (peak > 0.97f) {
		const float g = 0.97f / peak;
		for (int i = 0; i < p_n; i++) {
			p_data[i] *= g;
		}
	}
}

bool ProceduralKitGenerator::bake(const TrackerInstrument *p_inst, Vector<Vector<float>> &p_out) {
	ERR_FAIL_NULL_V(p_inst, false);
	p_out.clear();

	const int N = p_inst->get_table_size();
	const int count = p_inst->get_variant_count();
	const int waveform = p_inst->get_waveform();
	const int harmonics = p_inst->get_harmonics();
	const double falloff = p_inst->get_harmonic_falloff();
	const double pw = p_inst->get_pulse_width();
	const double cmin = p_inst->get_filter_cutoff_min();
	const double cmax = p_inst->get_filter_cutoff_max();
	const double q = 0.7071 + p_inst->get_filter_resonance() * 7.0;
	const int fm_ratio = MAX(1, (int)(p_inst->get_fm_ratio() + 0.5)); // integer -> periodic
	const double fm_amount = p_inst->get_fm_amount();
	const double drive = p_inst->get_drive();

	ERR_FAIL_COND_V(N < 64, false);
	ERR_FAIL_COND_V(count < 1, false);

	for (int v = 0; v < count; v++) {
		const double t = (count > 1) ? (double)v / (double)(count - 1) : 0.0;
		const double cutoff01 = cmin + (cmax - cmin) * t;
		const double fc = 0.001 + cutoff01 * (0.5 - 0.001); // cycles/sample

		Vector<float> tab;
		tab.resize(N);
		float *w = tab.ptrw();

		uint32_t rng = 0x13572468u; // reseeded per variant -> identical noise base
		for (int i = 0; i < N; i++) {
			const double ph = (double)i / (double)N;
			w[i] = osc(waveform, ph, harmonics, falloff, pw, fm_ratio, fm_amount, rng);
		}
		biquad_lowpass_circular(w, N, fc, q);

		// Post-filter soft-clip drive (adds harmonics/warmth). tanh saturator.
		if (drive > 0.0) {
			const double k = 1.0 + drive * 6.0;
			const double norm = 1.0 / Math::tanh(k);
			for (int i = 0; i < N; i++) {
				w[i] = (float)(Math::tanh(k * w[i]) * norm);
			}
		}
		// K-weighted (BS.1770) loudness match so variants/instruments sit at the
		// same perceived level (RMS over-weighted the sub-heavy ones). fs_eq maps
		// harmonic k of this cycle to k*fundamental Hz.
		const double fundamental = MAX(20.0, (double)p_inst->get_base_pitch());
		normalize_lufs(w, N, (double)N * fundamental, -18.0f);

		p_out.push_back(tab);
	}

	// Hard invariant: every variant must be the same length + phase aligned, or
	// swapping tables mid-note clicks. (Same length + shared phase-0 start here.)
	for (int i = 0; i < p_out.size(); i++) {
		ERR_FAIL_COND_V_MSG(p_out[i].size() != N, false,
				"trackergen: variant tables must be equal length (phase-alignment invariant).");
	}
	return p_out.size() > 0;
}
