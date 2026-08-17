/**************************************************************************/
/*  trackergen_instrument.h                                               */
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

#ifndef TRACKERGEN_INSTRUMENT_H
#define TRACKERGEN_INSTRUMENT_H

#include "core/resource.h"

// A single procedural tracker instrument: an oscillator (additive or 2-op FM) run
// through a resonant lowpass + soft-clip drive, an ADSR envelope, and a set of
// phase-aligned wavetable "variants" that can be swapped mid-note (the wobble /
// deep-bass trick). Everything is synthesized — no sampled audio. See memo.md.

class TrackerInstrument : public Resource {
	GDCLASS(TrackerInstrument, Resource);

public:
	enum Waveform {
		WAVE_SINE,
		WAVE_SAW,
		WAVE_SQUARE,
		WAVE_TRIANGLE,
		WAVE_PULSE,
		WAVE_NOISE,
	};

	// How the active wavetable variant is chosen while a note plays.
	enum VariantMode {
		VARIANT_STATIC, // always index 0
		VARIANT_LFO, // an LFO sweeps across variants (dubstep wobble)
		VARIANT_TENSION, // gameplay "tension" picks the variant (deeper bass)
	};

private:
	StringName id;

	// Oscillator parameters.
	int waveform;
	int harmonics; // additive partials (1..64)
	real_t harmonic_falloff; // 1/k^falloff
	real_t pulse_width; // for WAVE_PULSE (0..1)

	// 2-op FM (phase modulation). When fm_amount > 0 the oscillator becomes a
	// simple carrier+modulator pair (bells, e-pianos, metallic basses). fm_ratio
	// is rounded to an integer at bake time so one cycle stays periodic.
	real_t fm_ratio; // modulator : carrier frequency ratio
	real_t fm_amount; // modulation index (0 = off -> additive waveform)

	// Post-filter soft-clip drive (0 = clean; adds harmonics/warmth).
	real_t drive;

	// Baked resonant lowpass, swept across variants.
	real_t filter_cutoff_min; // normalized 0..1
	real_t filter_cutoff_max; // normalized 0..1
	real_t filter_resonance; // 0..1

	// Variant set (must be equal length + phase aligned; enforced at bake).
	int table_size; // samples per variant (one cycle)
	int variant_count;
	int variant_mode; // VariantMode
	real_t lfo_hz; // for VARIANT_LFO

	// Envelope (seconds, sustain is a level 0..1).
	real_t attack, decay, sustain, release;

	// Mixing.
	real_t gain; // linear
	real_t pan; // -1..1
	real_t base_pitch; // Hz that one table cycle represents (usually equals note freq)
	int priority; // voice-stealing rank: higher survives (kick/bass high, fx low)
	real_t tension_gain; // P2: extra gain scaled by tension (0 = none; alarm voice > 0)

	int preset_index; // editor-only dropdown selection (0 = "(select)")

	// Runtime-baked tables (not serialized).
	Vector<Vector<float>> _variants;
	bool _dirty;

protected:
	static void _bind_methods();

public:
	// --- authoring accessors (mark dirty so tables rebake) ---
	void set_id(const StringName &p_id) { id = p_id; }
	StringName get_id() const { return id; }

	void set_waveform(int p_v) {
		waveform = p_v;
		_dirty = true;
	}
	int get_waveform() const { return waveform; }
	void set_harmonics(int p_v) {
		harmonics = CLAMP(p_v, 1, 64);
		_dirty = true;
	}
	int get_harmonics() const { return harmonics; }
	void set_harmonic_falloff(real_t p_v) {
		harmonic_falloff = p_v;
		_dirty = true;
	}
	real_t get_harmonic_falloff() const { return harmonic_falloff; }
	void set_pulse_width(real_t p_v) {
		pulse_width = p_v;
		_dirty = true;
	}
	real_t get_pulse_width() const { return pulse_width; }

	void set_fm_ratio(real_t p_v) {
		fm_ratio = p_v;
		_dirty = true;
	}
	real_t get_fm_ratio() const { return fm_ratio; }
	void set_fm_amount(real_t p_v) {
		fm_amount = p_v;
		_dirty = true;
	}
	real_t get_fm_amount() const { return fm_amount; }
	void set_drive(real_t p_v) {
		drive = p_v;
		_dirty = true;
	}
	real_t get_drive() const { return drive; }

	void set_filter_cutoff_min(real_t p_v) {
		filter_cutoff_min = p_v;
		_dirty = true;
	}
	real_t get_filter_cutoff_min() const { return filter_cutoff_min; }
	void set_filter_cutoff_max(real_t p_v) {
		filter_cutoff_max = p_v;
		_dirty = true;
	}
	real_t get_filter_cutoff_max() const { return filter_cutoff_max; }
	void set_filter_resonance(real_t p_v) {
		filter_resonance = p_v;
		_dirty = true;
	}
	real_t get_filter_resonance() const { return filter_resonance; }

	void set_table_size(int p_v);
	int get_table_size() const { return table_size; }
	void set_variant_count(int p_v) {
		variant_count = CLAMP(p_v, 1, 64);
		_dirty = true;
	}
	int get_variant_count() const { return variant_count; }
	void set_variant_mode(int p_v) { variant_mode = p_v; }
	int get_variant_mode() const { return variant_mode; }
	void set_lfo_hz(real_t p_v) { lfo_hz = p_v; }
	real_t get_lfo_hz() const { return lfo_hz; }

	void set_attack(real_t p_v) { attack = p_v; }
	real_t get_attack() const { return attack; }
	void set_decay(real_t p_v) { decay = p_v; }
	real_t get_decay() const { return decay; }
	void set_sustain(real_t p_v) { sustain = p_v; }
	real_t get_sustain() const { return sustain; }
	void set_release(real_t p_v) { release = p_v; }
	real_t get_release() const { return release; }

	void set_gain(real_t p_v) { gain = p_v; }
	real_t get_gain() const { return gain; }
	void set_pan(real_t p_v) { pan = CLAMP(p_v, (real_t)-1, (real_t)1); }
	real_t get_pan() const { return pan; }
	void set_base_pitch(real_t p_v) { base_pitch = p_v; }
	real_t get_base_pitch() const { return base_pitch; }
	void set_priority(int p_v) { priority = p_v; }
	int get_priority() const { return priority; }
	void set_tension_gain(real_t p_v) { tension_gain = p_v; }
	real_t get_tension_gain() const { return tension_gain; }

	// --- presets ---
	// Configure this instrument from a named preset (e.g. "deep_bass", "fm_bell").
	// See get_preset_names(). Unknown names leave the instrument unchanged.
	void load_preset(const String &p_name);
	PoolStringArray get_preset_names() const;

	// Editor dropdown: index 0 = "(select)", 1..N = the presets above. Setting it
	// applies the preset and refreshes the inspector.
	void set_preset(int p_index);
	int get_preset() const { return preset_index; }

	// --- baking / playback-side access ---
	// Rebuilds the variant tables if parameters changed. Call off the audio
	// thread (e.g. from AudioStreamPlaybackTracker::start()).
	void ensure_baked();
	void mark_dirty() { _dirty = true; }
	bool is_dirty() const { return _dirty; }

	// Read-only, real-time-safe once baked.
	_FORCE_INLINE_ int get_baked_variant_count() const { return _variants.size(); }
	const Vector<float> &get_variant(int p_index) const;

	TrackerInstrument();
};

VARIANT_ENUM_CAST(TrackerInstrument::Waveform);
VARIANT_ENUM_CAST(TrackerInstrument::VariantMode);

#endif // TRACKERGEN_INSTRUMENT_H
