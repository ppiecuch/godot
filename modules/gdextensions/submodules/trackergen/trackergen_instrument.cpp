/**************************************************************************/
/*  trackergen_instrument.cpp                                             */
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

#include "trackergen_instrument.h"

#include "procedural_kit_generator.h"

void TrackerInstrument::set_table_size(int p_v) {
	// Keep it a sane power-of-two-ish size for cheap wavetable playback.
	table_size = CLAMP(p_v, 64, 8192);
	_dirty = true;
}

void TrackerInstrument::ensure_baked() {
	if (!_dirty && _variants.size() > 0) {
		return;
	}

	Vector<Vector<float>> baked;
	if (ProceduralKitGenerator::bake(this, baked)) {
		_variants = baked;
	} else if (_variants.empty()) {
		// Guarantee at least one (silent) table so playback never dereferences null.
		Vector<float> t;
		t.resize(table_size);
		for (int i = 0; i < table_size; i++) {
			t.write[i] = 0.0f;
		}
		_variants.clear();
		_variants.push_back(t);
	}
	_dirty = false;
}

const Vector<float> &TrackerInstrument::get_variant(int p_index) const {
	static const Vector<float> empty;
	if (_variants.empty()) {
		return empty;
	}
	return _variants[CLAMP(p_index, 0, _variants.size() - 1)];
}

static const char *TG_PRESETS[] = {
	"deep_bass", "reese_bass", "wobble_bass", "sub_sine",
	"saw_lead", "square_lead", "pluck", "pad", "organ",
	"fm_bell", "fm_epiano", "fm_bass", "alarm", "hat", "blip"
};
static const int TG_PRESET_COUNT = (int)(sizeof(TG_PRESETS) / sizeof(TG_PRESETS[0]));

static String _tg_preset_enum_hint() {
	String h = "(select)";
	for (int i = 0; i < TG_PRESET_COUNT; i++) {
		h += ",";
		h += TG_PRESETS[i];
	}
	return h;
}

PoolStringArray TrackerInstrument::get_preset_names() const {
	PoolStringArray a;
	for (int i = 0; i < TG_PRESET_COUNT; i++) {
		a.push_back(String(TG_PRESETS[i]));
	}
	return a;
}

void TrackerInstrument::set_preset(int p_index) {
	preset_index = p_index;
	if (p_index >= 1 && p_index <= TG_PRESET_COUNT) {
		load_preset(String(TG_PRESETS[p_index - 1]));
#ifdef TOOLS_ENABLED
		property_list_changed_notify(); // refresh the inspector with the applied values
#endif
	}
}

void TrackerInstrument::load_preset(const String &p_name) {
	// Common baseline; each preset overrides what it cares about.
	waveform = WAVE_SAW;
	harmonics = 12;
	harmonic_falloff = 1.0;
	pulse_width = 0.5;
	fm_ratio = 1.0;
	fm_amount = 0.0;
	drive = 0.0;
	filter_cutoff_min = 0.3;
	filter_cutoff_max = 0.5;
	filter_resonance = 0.2;
	table_size = 2048;
	variant_count = 1;
	variant_mode = VARIANT_STATIC;
	lfo_hz = 4.0;
	attack = 0.005;
	decay = 0.1;
	sustain = 0.7;
	release = 0.15;
	gain = 0.6;
	pan = 0.0;
	base_pitch = 440.0;
	priority = 4;
	tension_gain = 0.0;

	const String n = p_name.to_lower();
	if (n == "deep_bass") {
		harmonics = 20;
		filter_cutoff_min = 0.45;
		filter_cutoff_max = 0.06;
		variant_count = 5;
		variant_mode = VARIANT_TENSION;
		drive = 0.2;
		sustain = 0.9;
		release = 0.12;
		gain = 0.9;
		priority = 8;
	} else if (n == "reese_bass") {
		harmonics = 24;
		filter_cutoff_min = 0.15;
		filter_cutoff_max = 0.4;
		filter_resonance = 0.35;
		drive = 0.35;
		sustain = 0.9;
		gain = 0.85;
		priority = 8;
	} else if (n == "wobble_bass") {
		waveform = WAVE_SQUARE;
		harmonics = 24;
		filter_cutoff_min = 0.05;
		filter_cutoff_max = 0.5;
		filter_resonance = 0.5;
		variant_count = 8;
		variant_mode = VARIANT_LFO;
		lfo_hz = 5.0;
		drive = 0.3;
		sustain = 1.0;
		gain = 0.85;
		priority = 7;
	} else if (n == "sub_sine") {
		waveform = WAVE_SINE;
		harmonics = 1;
		filter_cutoff_max = 0.3;
		sustain = 0.95;
		gain = 0.9;
		priority = 8;
	} else if (n == "saw_lead") {
		harmonics = 24;
		filter_cutoff_min = 0.5;
		decay = 0.12;
		sustain = 0.6;
		gain = 0.5;
		priority = 2;
	} else if (n == "square_lead") {
		waveform = WAVE_SQUARE;
		harmonics = 20;
		sustain = 0.6;
		gain = 0.5;
		priority = 2;
	} else if (n == "pluck") {
		harmonics = 20;
		attack = 0.002;
		decay = 0.18;
		sustain = 0.0;
		release = 0.08;
		gain = 0.6;
		priority = 3;
	} else if (n == "pad") {
		waveform = WAVE_TRIANGLE;
		harmonics = 16;
		attack = 0.35;
		decay = 0.3;
		sustain = 0.8;
		release = 0.6;
		filter_cutoff_max = 0.35;
		gain = 0.4;
		priority = 3;
	} else if (n == "organ") {
		waveform = WAVE_SQUARE;
		harmonics = 12;
		harmonic_falloff = 0.6;
		sustain = 0.9;
		gain = 0.45;
		priority = 3;
	} else if (n == "fm_bell") {
		waveform = WAVE_SINE;
		fm_ratio = 3.5;
		fm_amount = 4.0;
		attack = 0.002;
		decay = 0.6;
		sustain = 0.0;
		release = 0.4;
		gain = 0.5;
		priority = 3;
	} else if (n == "fm_epiano") {
		waveform = WAVE_SINE;
		fm_ratio = 1.0;
		fm_amount = 2.2;
		attack = 0.003;
		decay = 0.25;
		sustain = 0.4;
		release = 0.2;
		drive = 0.1;
		gain = 0.55;
		priority = 3;
	} else if (n == "fm_bass") {
		waveform = WAVE_SINE;
		fm_ratio = 2.0;
		fm_amount = 1.8;
		filter_cutoff_max = 0.35;
		sustain = 0.85;
		drive = 0.2;
		gain = 0.85;
		priority = 8;
	} else if (n == "alarm") {
		waveform = WAVE_SQUARE;
		harmonics = 16;
		sustain = 0.8;
		gain = 0.35;
		tension_gain = 3.0;
		priority = 2;
	} else if (n == "hat") {
		waveform = WAVE_NOISE;
		attack = 0.001;
		decay = 0.05;
		sustain = 0.0;
		release = 0.02;
		filter_cutoff_min = 0.6;
		filter_cutoff_max = 0.6;
		gain = 0.4;
		priority = 5;
	} else if (n == "blip") {
		waveform = WAVE_PULSE;
		pulse_width = 0.25;
		attack = 0.001;
		decay = 0.08;
		sustain = 0.0;
		release = 0.02;
		gain = 0.6;
		priority = 2;
	} else {
		WARN_PRINT("trackergen: unknown instrument preset '" + p_name + "'.");
	}
	_dirty = true;
}

void TrackerInstrument::_bind_methods() {
	BIND_ENUM_CONSTANT(WAVE_SINE);
	BIND_ENUM_CONSTANT(WAVE_SAW);
	BIND_ENUM_CONSTANT(WAVE_SQUARE);
	BIND_ENUM_CONSTANT(WAVE_TRIANGLE);
	BIND_ENUM_CONSTANT(WAVE_PULSE);
	BIND_ENUM_CONSTANT(WAVE_NOISE);

	BIND_ENUM_CONSTANT(VARIANT_STATIC);
	BIND_ENUM_CONSTANT(VARIANT_LFO);
	BIND_ENUM_CONSTANT(VARIANT_TENSION);

	ClassDB::bind_method(D_METHOD("set_id", "id"), &TrackerInstrument::set_id);
	ClassDB::bind_method(D_METHOD("get_id"), &TrackerInstrument::get_id);
	ClassDB::bind_method(D_METHOD("set_waveform", "waveform"), &TrackerInstrument::set_waveform);
	ClassDB::bind_method(D_METHOD("get_waveform"), &TrackerInstrument::get_waveform);
	ClassDB::bind_method(D_METHOD("set_harmonics", "n"), &TrackerInstrument::set_harmonics);
	ClassDB::bind_method(D_METHOD("get_harmonics"), &TrackerInstrument::get_harmonics);
	ClassDB::bind_method(D_METHOD("set_harmonic_falloff", "v"), &TrackerInstrument::set_harmonic_falloff);
	ClassDB::bind_method(D_METHOD("get_harmonic_falloff"), &TrackerInstrument::get_harmonic_falloff);
	ClassDB::bind_method(D_METHOD("set_pulse_width", "v"), &TrackerInstrument::set_pulse_width);
	ClassDB::bind_method(D_METHOD("get_pulse_width"), &TrackerInstrument::get_pulse_width);
	ClassDB::bind_method(D_METHOD("set_fm_ratio", "v"), &TrackerInstrument::set_fm_ratio);
	ClassDB::bind_method(D_METHOD("get_fm_ratio"), &TrackerInstrument::get_fm_ratio);
	ClassDB::bind_method(D_METHOD("set_fm_amount", "v"), &TrackerInstrument::set_fm_amount);
	ClassDB::bind_method(D_METHOD("get_fm_amount"), &TrackerInstrument::get_fm_amount);
	ClassDB::bind_method(D_METHOD("set_drive", "v"), &TrackerInstrument::set_drive);
	ClassDB::bind_method(D_METHOD("get_drive"), &TrackerInstrument::get_drive);
	ClassDB::bind_method(D_METHOD("load_preset", "name"), &TrackerInstrument::load_preset);
	ClassDB::bind_method(D_METHOD("get_preset_names"), &TrackerInstrument::get_preset_names);
	ClassDB::bind_method(D_METHOD("set_preset", "index"), &TrackerInstrument::set_preset);
	ClassDB::bind_method(D_METHOD("get_preset"), &TrackerInstrument::get_preset);

	ClassDB::bind_method(D_METHOD("set_filter_cutoff_min", "v"), &TrackerInstrument::set_filter_cutoff_min);
	ClassDB::bind_method(D_METHOD("get_filter_cutoff_min"), &TrackerInstrument::get_filter_cutoff_min);
	ClassDB::bind_method(D_METHOD("set_filter_cutoff_max", "v"), &TrackerInstrument::set_filter_cutoff_max);
	ClassDB::bind_method(D_METHOD("get_filter_cutoff_max"), &TrackerInstrument::get_filter_cutoff_max);
	ClassDB::bind_method(D_METHOD("set_filter_resonance", "v"), &TrackerInstrument::set_filter_resonance);
	ClassDB::bind_method(D_METHOD("get_filter_resonance"), &TrackerInstrument::get_filter_resonance);

	ClassDB::bind_method(D_METHOD("set_table_size", "n"), &TrackerInstrument::set_table_size);
	ClassDB::bind_method(D_METHOD("get_table_size"), &TrackerInstrument::get_table_size);
	ClassDB::bind_method(D_METHOD("set_variant_count", "n"), &TrackerInstrument::set_variant_count);
	ClassDB::bind_method(D_METHOD("get_variant_count"), &TrackerInstrument::get_variant_count);
	ClassDB::bind_method(D_METHOD("set_variant_mode", "mode"), &TrackerInstrument::set_variant_mode);
	ClassDB::bind_method(D_METHOD("get_variant_mode"), &TrackerInstrument::get_variant_mode);
	ClassDB::bind_method(D_METHOD("set_lfo_hz", "hz"), &TrackerInstrument::set_lfo_hz);
	ClassDB::bind_method(D_METHOD("get_lfo_hz"), &TrackerInstrument::get_lfo_hz);

	ClassDB::bind_method(D_METHOD("set_attack", "s"), &TrackerInstrument::set_attack);
	ClassDB::bind_method(D_METHOD("get_attack"), &TrackerInstrument::get_attack);
	ClassDB::bind_method(D_METHOD("set_decay", "s"), &TrackerInstrument::set_decay);
	ClassDB::bind_method(D_METHOD("get_decay"), &TrackerInstrument::get_decay);
	ClassDB::bind_method(D_METHOD("set_sustain", "level"), &TrackerInstrument::set_sustain);
	ClassDB::bind_method(D_METHOD("get_sustain"), &TrackerInstrument::get_sustain);
	ClassDB::bind_method(D_METHOD("set_release", "s"), &TrackerInstrument::set_release);
	ClassDB::bind_method(D_METHOD("get_release"), &TrackerInstrument::get_release);

	ClassDB::bind_method(D_METHOD("set_gain", "v"), &TrackerInstrument::set_gain);
	ClassDB::bind_method(D_METHOD("get_gain"), &TrackerInstrument::get_gain);
	ClassDB::bind_method(D_METHOD("set_pan", "v"), &TrackerInstrument::set_pan);
	ClassDB::bind_method(D_METHOD("get_pan"), &TrackerInstrument::get_pan);
	ClassDB::bind_method(D_METHOD("set_base_pitch", "hz"), &TrackerInstrument::set_base_pitch);
	ClassDB::bind_method(D_METHOD("get_base_pitch"), &TrackerInstrument::get_base_pitch);
	ClassDB::bind_method(D_METHOD("set_priority", "p"), &TrackerInstrument::set_priority);
	ClassDB::bind_method(D_METHOD("get_priority"), &TrackerInstrument::get_priority);
	ClassDB::bind_method(D_METHOD("set_tension_gain", "v"), &TrackerInstrument::set_tension_gain);
	ClassDB::bind_method(D_METHOD("get_tension_gain"), &TrackerInstrument::get_tension_gain);

	ClassDB::bind_method(D_METHOD("ensure_baked"), &TrackerInstrument::ensure_baked);
	ClassDB::bind_method(D_METHOD("get_baked_variant_count"), &TrackerInstrument::get_baked_variant_count);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "id"), "set_id", "get_id");
	// Editor convenience: pick a preset to fill in all the params below.
	ADD_PROPERTY(PropertyInfo(Variant::INT, "preset", PROPERTY_HINT_ENUM, _tg_preset_enum_hint(),
						 PROPERTY_USAGE_EDITOR),
			"set_preset", "get_preset");

	ADD_GROUP("Oscillator", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "waveform", PROPERTY_HINT_ENUM, "Sine,Saw,Square,Triangle,Pulse,Noise"), "set_waveform", "get_waveform");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "harmonics", PROPERTY_HINT_RANGE, "1,64,1"), "set_harmonics", "get_harmonics");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "harmonic_falloff", PROPERTY_HINT_RANGE, "0.1,4,0.05"), "set_harmonic_falloff", "get_harmonic_falloff");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "pulse_width", PROPERTY_HINT_RANGE, "0.05,0.95,0.01"), "set_pulse_width", "get_pulse_width");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "fm_ratio", PROPERTY_HINT_RANGE, "0,16,0.5"), "set_fm_ratio", "get_fm_ratio");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "fm_amount", PROPERTY_HINT_RANGE, "0,12,0.05"), "set_fm_amount", "get_fm_amount");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "drive", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_drive", "get_drive");

	ADD_GROUP("Filter / Variants", "");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "filter_cutoff_min", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_filter_cutoff_min", "get_filter_cutoff_min");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "filter_cutoff_max", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_filter_cutoff_max", "get_filter_cutoff_max");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "filter_resonance", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_filter_resonance", "get_filter_resonance");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "table_size", PROPERTY_HINT_RANGE, "64,8192,1"), "set_table_size", "get_table_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "variant_count", PROPERTY_HINT_RANGE, "1,64,1"), "set_variant_count", "get_variant_count");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "variant_mode", PROPERTY_HINT_ENUM, "Static,LFO,Tension"), "set_variant_mode", "get_variant_mode");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "lfo_hz", PROPERTY_HINT_RANGE, "0.1,20,0.1"), "set_lfo_hz", "get_lfo_hz");

	ADD_GROUP("Envelope", "");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "attack", PROPERTY_HINT_RANGE, "0,4,0.001"), "set_attack", "get_attack");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "decay", PROPERTY_HINT_RANGE, "0,4,0.001"), "set_decay", "get_decay");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "sustain", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_sustain", "get_sustain");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "release", PROPERTY_HINT_RANGE, "0,4,0.001"), "set_release", "get_release");

	ADD_GROUP("Mix", "");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "gain", PROPERTY_HINT_RANGE, "0,2,0.01"), "set_gain", "get_gain");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "pan", PROPERTY_HINT_RANGE, "-1,1,0.01"), "set_pan", "get_pan");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "base_pitch", PROPERTY_HINT_RANGE, "1,1000,0.1"), "set_base_pitch", "get_base_pitch");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "priority", PROPERTY_HINT_RANGE, "0,16,1"), "set_priority", "get_priority");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "tension_gain", PROPERTY_HINT_RANGE, "0,8,0.05"), "set_tension_gain", "get_tension_gain");
}

TrackerInstrument::TrackerInstrument() {
	id = StringName();
	waveform = WAVE_SAW;
	harmonics = 8;
	harmonic_falloff = 1.0;
	pulse_width = 0.5;
	fm_ratio = 1.0;
	fm_amount = 0.0;
	drive = 0.0;
	filter_cutoff_min = 0.08;
	filter_cutoff_max = 0.45;
	filter_resonance = 0.2;
	table_size = 2048;
	variant_count = 1;
	variant_mode = VARIANT_STATIC;
	lfo_hz = 4.0;
	attack = 0.005;
	decay = 0.05;
	sustain = 0.8;
	release = 0.2;
	gain = 0.8;
	pan = 0.0;
	base_pitch = 440.0;
	priority = 0;
	tension_gain = 0.0;
	preset_index = 0;
	_dirty = true;
}
