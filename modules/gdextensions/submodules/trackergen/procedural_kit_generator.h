/**************************************************************************/
/*  procedural_kit_generator.h                                           */
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

#ifndef PROCEDURAL_KIT_GENERATOR_H
#define PROCEDURAL_KIT_GENERATOR_H

#include "core/vector.h"

class TrackerInstrument;

// Bakes an instrument's phase-aligned wavetable variants: additive oscillator ->
// resonant lowpass (swept per variant) -> normalize. Runs off the audio thread.
class ProceduralKitGenerator {
public:
	// Fills p_out with `variant_count` tables, each `table_size` samples of one
	// waveform cycle, filtered at a per-variant cutoff. All tables share length
	// and phase (start at phase 0) so a mid-note swap does not click.
	// Returns false (and clears p_out) if the phase-alignment invariant fails.
	static bool bake(const TrackerInstrument *p_inst, Vector<Vector<float>> &p_out);

private:
	static float osc(int p_waveform, double p_phase, int p_harmonics, double p_falloff, double p_pulse_width, int p_fm_ratio, double p_fm_amount, uint32_t &r_rng);
	static void biquad_lowpass_circular(float *p_data, int p_n, double p_fc_norm, double p_q);
	static void normalize_peak(float *p_data, int p_n, float p_target);
	// K-weighted (ITU-R BS.1770) loudness normalization, peak-clamped. p_fs_eq is
	// the table's equivalent sample rate (cycle_len * fundamental_hz).
	static void normalize_lufs(float *p_data, int p_n, double p_fs_eq, float p_target_lufs);
};

#endif // PROCEDURAL_KIT_GENERATOR_H
