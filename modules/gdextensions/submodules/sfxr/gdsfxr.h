/**************************************************************************/
/*  gdsfxr.h                                                              */
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

/* Custom importer for .sfx resources */

#ifndef RESOURCE_IMPORTER_SFXR
#define RESOURCE_IMPORTER_SFXR

#include "core/io/resource_importer.h"
#include "core/io/resource_saver.h"
#include "core/math/math_funcs.h"
#include "editor/editor_plugin.h"
#include "servers/audio/audio_stream.h"

#include "retrosfxvoice.h"

class AudioStreamSfxr : public AudioStream {
	GDCLASS(AudioStreamSfxr, AudioStream);

	friend class AudioStreamPlaybackSfxr;

public:
	enum WaveFormType {
		SQUAREWAVE,
		SAWTOOTH,
		SINEWAVE,
		NOISEWAVE,
		TRIANGLEWAVE,
		PINKNOISEWAVE,
		TANWAVE,
		WHISTLEWAVE,
		BREAKERWAVE,
	};

private:
	bool loop;
	real_t loop_offset;
	RetroSFXVoice sfx_voice;

	PoolRealArray _cache;
	bool _dirty;

	void _update_voice();

	AcceptDialog *dlg;

	_FORCE_INLINE_ static double rnd(double limit) { return Math::random(0.0, limit); }
	_FORCE_INLINE_ static float rnd(float limit) { return Math::random(0.0f, limit); }
	_FORCE_INLINE_ static int rnd(int limit) { return Math::random(0, limit); }

protected:
	void _get_property_list(List<PropertyInfo> *p_list) const {
		if (p_list) {
			for (List<PropertyInfo>::Element *E = p_list->front(); E; E = E->next()) {
				PropertyInfo &prop = E->get();
				if (prop.name.to_lower() == "square_duty" || prop.name.to_lower() == "duty_sweep") {
					if (sfx_voice.voice.waveform_shape == 0) {
						prop.usage |= PROPERTY_USAGE_EDITOR;
					} else {
						prop.usage &= ~PROPERTY_USAGE_EDITOR;
					}
				}
			}
		}
	}

	static void _bind_methods() {
		BIND_ENUM_CONSTANT(SQUAREWAVE);
		BIND_ENUM_CONSTANT(SAWTOOTH);
		BIND_ENUM_CONSTANT(SINEWAVE);
		BIND_ENUM_CONSTANT(NOISEWAVE);
		BIND_ENUM_CONSTANT(TRIANGLEWAVE);
		BIND_ENUM_CONSTANT(PINKNOISEWAVE);
		BIND_ENUM_CONSTANT(TANWAVE);
		BIND_ENUM_CONSTANT(WHISTLEWAVE);
		BIND_ENUM_CONSTANT(BREAKERWAVE);

		ClassDB::bind_method(D_METHOD("from_file", "file"), &AudioStreamSfxr::from_file);
		ClassDB::bind_method(D_METHOD("from_dict", "dict"), &AudioStreamSfxr::from_dict);
		ClassDB::bind_method(D_METHOD("save_to_wav", "path", "quality", "sample_size"), &AudioStreamSfxr::save_to_wav);

		ClassDB::bind_method(D_METHOD("set_loop", "value"), &AudioStreamSfxr::set_loop);
		ClassDB::bind_method(D_METHOD("get_loop"), &AudioStreamSfxr::get_loop);
		ClassDB::bind_method(D_METHOD("set_loop_offset", "value"), &AudioStreamSfxr::set_loop_offset);
		ClassDB::bind_method(D_METHOD("get_loop_offset"), &AudioStreamSfxr::get_loop_offset);

		ClassDB::bind_method(D_METHOD("set_volume", "value"), &AudioStreamSfxr::set_volume);
		ClassDB::bind_method(D_METHOD("get_volume"), &AudioStreamSfxr::get_volume);

		ClassDB::bind_method(D_METHOD("set_wave_form", "value"), &AudioStreamSfxr::set_wave_form);
		ClassDB::bind_method(D_METHOD("get_wave_form"), &AudioStreamSfxr::get_wave_form);
		ClassDB::bind_method(D_METHOD("set_overtones", "value"), &AudioStreamSfxr::set_overtones);
		ClassDB::bind_method(D_METHOD("get_overtones"), &AudioStreamSfxr::get_overtones);
		ClassDB::bind_method(D_METHOD("set_overtone_falloff", "value"), &AudioStreamSfxr::set_overtone_falloff);
		ClassDB::bind_method(D_METHOD("get_overtone_falloff"), &AudioStreamSfxr::get_overtone_falloff);
		ClassDB::bind_method(D_METHOD("set_attack_time", "value"), &AudioStreamSfxr::set_attack_time);
		ClassDB::bind_method(D_METHOD("get_attack_time"), &AudioStreamSfxr::get_attack_time);
		ClassDB::bind_method(D_METHOD("set_sustain_time", "value"), &AudioStreamSfxr::set_sustain_time);
		ClassDB::bind_method(D_METHOD("get_sustain_time"), &AudioStreamSfxr::get_sustain_time);
		ClassDB::bind_method(D_METHOD("set_decay_time", "value"), &AudioStreamSfxr::set_decay_time);
		ClassDB::bind_method(D_METHOD("get_decay_time"), &AudioStreamSfxr::get_decay_time);
		ClassDB::bind_method(D_METHOD("set_start_frequency", "value"), &AudioStreamSfxr::set_start_frequency);
		ClassDB::bind_method(D_METHOD("get_start_frequency"), &AudioStreamSfxr::get_start_frequency);
		ClassDB::bind_method(D_METHOD("set_min_frequency", "value"), &AudioStreamSfxr::set_min_frequency);
		ClassDB::bind_method(D_METHOD("get_min_frequency"), &AudioStreamSfxr::get_min_frequency);
		ClassDB::bind_method(D_METHOD("set_slide", "value"), &AudioStreamSfxr::set_slide);
		ClassDB::bind_method(D_METHOD("get_slide"), &AudioStreamSfxr::get_slide);
		ClassDB::bind_method(D_METHOD("set_delta_slide", "value"), &AudioStreamSfxr::set_delta_slide);
		ClassDB::bind_method(D_METHOD("get_delta_slide"), &AudioStreamSfxr::get_delta_slide);
		ClassDB::bind_method(D_METHOD("set_vibrato_depth", "value"), &AudioStreamSfxr::set_vibrato_depth);
		ClassDB::bind_method(D_METHOD("get_vibrato_depth"), &AudioStreamSfxr::get_vibrato_depth);
		ClassDB::bind_method(D_METHOD("set_vibrato_speed", "value"), &AudioStreamSfxr::set_vibrato_speed);
		ClassDB::bind_method(D_METHOD("get_vibrato_speed"), &AudioStreamSfxr::get_vibrato_speed);
		ClassDB::bind_method(D_METHOD("set_change_repeat", "value"), &AudioStreamSfxr::set_change_repeat);
		ClassDB::bind_method(D_METHOD("get_change_repeat"), &AudioStreamSfxr::get_change_repeat);
		ClassDB::bind_method(D_METHOD("set_change_amount", "value"), &AudioStreamSfxr::set_change_amount);
		ClassDB::bind_method(D_METHOD("get_change_amount"), &AudioStreamSfxr::get_change_amount);
		ClassDB::bind_method(D_METHOD("set_change_speed", "value"), &AudioStreamSfxr::set_change_speed);
		ClassDB::bind_method(D_METHOD("get_change_speed"), &AudioStreamSfxr::get_change_speed);
		ClassDB::bind_method(D_METHOD("set_change_amount2", "value"), &AudioStreamSfxr::set_change_amount2);
		ClassDB::bind_method(D_METHOD("get_change_amount2"), &AudioStreamSfxr::get_change_amount2);
		ClassDB::bind_method(D_METHOD("set_change_speed2", "value"), &AudioStreamSfxr::set_change_speed2);
		ClassDB::bind_method(D_METHOD("get_change_speed2"), &AudioStreamSfxr::get_change_speed2);
		ClassDB::bind_method(D_METHOD("set_square_duty", "value"), &AudioStreamSfxr::set_square_duty);
		ClassDB::bind_method(D_METHOD("get_square_duty"), &AudioStreamSfxr::get_square_duty);
		ClassDB::bind_method(D_METHOD("set_duty_sweep", "value"), &AudioStreamSfxr::set_duty_sweep);
		ClassDB::bind_method(D_METHOD("get_duty_sweep"), &AudioStreamSfxr::get_duty_sweep);
		ClassDB::bind_method(D_METHOD("set_repeat_speed", "value"), &AudioStreamSfxr::set_repeat_speed);
		ClassDB::bind_method(D_METHOD("get_repeat_speed"), &AudioStreamSfxr::get_repeat_speed);
		ClassDB::bind_method(D_METHOD("set_flanger_offset", "value"), &AudioStreamSfxr::set_flanger_offset);
		ClassDB::bind_method(D_METHOD("get_flanger_offset"), &AudioStreamSfxr::get_flanger_offset);
		ClassDB::bind_method(D_METHOD("set_flanger_sweep", "value"), &AudioStreamSfxr::set_flanger_sweep);
		ClassDB::bind_method(D_METHOD("get_flanger_sweep"), &AudioStreamSfxr::get_flanger_sweep);
		ClassDB::bind_method(D_METHOD("set_lp_filter_cutoff", "value"), &AudioStreamSfxr::set_lp_filter_cutoff);
		ClassDB::bind_method(D_METHOD("get_lp_filter_cutoff"), &AudioStreamSfxr::get_lp_filter_cutoff);
		ClassDB::bind_method(D_METHOD("set_lp_filter_cutoff_sweep", "value"), &AudioStreamSfxr::set_lp_filter_cutoff_sweep);
		ClassDB::bind_method(D_METHOD("get_lp_filter_cutoff_sweep"), &AudioStreamSfxr::get_lp_filter_cutoff_sweep);
		ClassDB::bind_method(D_METHOD("set_lp_filter_resonance", "value"), &AudioStreamSfxr::set_lp_filter_resonance);
		ClassDB::bind_method(D_METHOD("get_lp_filter_resonance"), &AudioStreamSfxr::get_lp_filter_resonance);
		ClassDB::bind_method(D_METHOD("set_hp_filter_cutoff", "value"), &AudioStreamSfxr::set_hp_filter_cutoff);
		ClassDB::bind_method(D_METHOD("get_hp_filter_cutoff"), &AudioStreamSfxr::get_hp_filter_cutoff);
		ClassDB::bind_method(D_METHOD("set_hp_filter_cutoff_sweep", "value"), &AudioStreamSfxr::set_hp_filter_cutoff_sweep);
		ClassDB::bind_method(D_METHOD("get_hp_filter_cutoff_sweep"), &AudioStreamSfxr::get_hp_filter_cutoff_sweep);
		ClassDB::bind_method(D_METHOD("get_bit_crush"), &AudioStreamSfxr::get_bit_crush);
		ClassDB::bind_method(D_METHOD("set_bit_crush", "value"), &AudioStreamSfxr::set_bit_crush);
		ClassDB::bind_method(D_METHOD("get_bit_crush_sweep"), &AudioStreamSfxr::get_bit_crush_sweep);
		ClassDB::bind_method(D_METHOD("set_bit_crush_sweep", "value"), &AudioStreamSfxr::set_bit_crush_sweep);
		ClassDB::bind_method(D_METHOD("get_compression_amount"), &AudioStreamSfxr::get_compression_amount);
		ClassDB::bind_method(D_METHOD("set_compression_amount", "value"), &AudioStreamSfxr::set_compression_amount);

		ClassDB::bind_method(D_METHOD("set_morph_rate", "value"), &AudioStreamSfxr::set_morph_rate);
		ClassDB::bind_method(D_METHOD("get_morph_rate"), &AudioStreamSfxr::get_morph_rate);

		ClassDB::bind_method(D_METHOD("load_presets", "value"), &AudioStreamSfxr::load_presets);
		ClassDB::bind_method(D_METHOD("call_action", "action"), &AudioStreamSfxr::call_action);

		ADD_PROPERTY(PropertyInfo(Variant::INT, "load_presets", PROPERTY_HINT_ENUM, "Select one:,Pickup/Coin,Laser/Shoot,Explosion,Power Up,Hit/Hurt,Jump,Blip/Select,Robotron"), "load_presets", "");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "call_action", PROPERTY_HINT_ENUM, "Select one:,Mutate,Randomize,Reset"), "call_action", "");

		// Sound properties
		ADD_PROPERTY(PropertyInfo(Variant::INT, "wave_form", PROPERTY_HINT_ENUM, "Square,Sawtooth,Sinwave,Noise,Triangle,PinkNoise,Tan,Whistle,Breaker"), "set_wave_form", "get_wave_form");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "overtones", PROPERTY_HINT_RANGE, "0,10"), "set_overtones", "get_overtones");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "overtone_falloff", PROPERTY_HINT_RANGE, "0,1,0.05"), "set_overtone_falloff", "get_overtone_falloff");
		ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loop"), "set_loop", "get_loop");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "loop_offset"), "set_loop_offset", "get_loop_offset");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "repeat_speed", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_repeat_speed", "get_repeat_speed");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "morph_rate", PROPERTY_HINT_RANGE, "0,1,0.05"), "set_morph_rate", "get_morph_rate");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "volume", PROPERTY_HINT_RANGE, "0,1,0.05"), "set_volume", "get_volume");

		ADD_GROUP("Square", "");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "square_duty", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_square_duty", "get_square_duty");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "duty_sweep", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_duty_sweep", "get_duty_sweep");

		ADD_GROUP("Envelope", "");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "attack_time", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_attack_time", "get_attack_time");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "sustain_time", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_sustain_time", "get_sustain_time");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "sustain_punch", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_sustain_time", "get_sustain_time");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "decay_time", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_decay_time", "get_decay_time");

		ADD_GROUP("Frequency", "");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "start_frequency", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_start_frequency", "get_start_frequency");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "min_frequency", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_min_frequency", "get_min_frequency");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "slide", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_slide", "get_slide");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "delta_slide", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_delta_slide", "get_delta_slide");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "vibrato_depth", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_vibrato_depth", "get_vibrato_depth");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "vibrato_speed", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_vibrato_speed", "get_vibrato_speed");

		ADD_GROUP("Change", "");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "change_repeat", PROPERTY_HINT_RANGE, "0,1,0.05"), "set_change_repeat", "get_change_repeat");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "change_amount", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_change_amount", "get_change_amount");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "change_speed", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_change_speed", "get_change_speed");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "change_amount2", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_change_amount2", "get_change_amount2");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "change_speed2", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_change_speed2", "get_change_speed2");

		ADD_GROUP("Flanger", "flanger_");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "flanger_offset", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_flanger_offset", "get_flanger_offset");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "flanger_sweep", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_flanger_sweep", "get_flanger_sweep");

		ADD_GROUP("Filters", "");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "lp_filter_cutoff", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_lp_filter_cutoff", "get_lp_filter_cutoff");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "lp_filter_cutoff_sweep", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_lp_filter_cutoff_sweep", "get_lp_filter_cutoff_sweep");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "lp_filter_resonance", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_lp_filter_resonance", "get_lp_filter_resonance");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "hp_filter_cutoff", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_hp_filter_cutoff", "get_hp_filter_cutoff");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "hp_filter_cutoff_sweep", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_hp_filter_cutoff_sweep", "get_hp_filter_cutoff_sweep");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "bit_crush", PROPERTY_HINT_RANGE, "0,1,0.05"), "set_bit_crush", "get_bit_crush");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "bit_crush_sweep", PROPERTY_HINT_RANGE, "-1,1,0.1"), "set_bit_crush_sweep", "get_bit_crush_sweep");
		ADD_PROPERTY(PropertyInfo(Variant::REAL, "compression_amount", PROPERTY_HINT_RANGE, "0,1,0.05"), "set_compression_amount", "get_compression_amount");
	}

public:
	void from_file(const String p_file);
	void from_dict(const Dictionary &p_dict);
	Error save_to_wav(const String &p_path, int quality, int sample_size);

	int fill(AudioFrame *p_buffer, int p_frames, int p_from = 0);

	void load_presets(int p_value) {
		switch (p_value) {
			case 1: {
				sfx_voice.ResetParams();
				sfx_voice.voice.fx_base_params.BaseFreq = 0.4 + rnd(0.5);
				sfx_voice.voice.fx_base_params.EnvAttack = 0;
				sfx_voice.voice.fx_base_params.EnvSustain = rnd(0.1);
				sfx_voice.voice.fx_base_params.EnvDecay = 0.1 + rnd(0.4);
				sfx_voice.voice.fx_base_params.EnvPunch = 0.3 + rnd(0.3);
				if (rnd(1)) {
					sfx_voice.voice.fx_base_params.ArmSpeed = 0.5 + rnd(0.2);
					sfx_voice.voice.fx_base_params.ArmMod = 0.2 + rnd(0.4);
				}
				_dirty = true;
				property_list_changed_notify();
			} break;
			case 2: {
				sfx_voice.ResetParams();
				sfx_voice.voice.waveform_shape = rnd(2);
				if (sfx_voice.voice.waveform_shape == 2 && rnd(1)) {
					sfx_voice.voice.waveform_shape = rnd(1);
				}
				sfx_voice.voice.fx_base_params.BaseFreq = 0.5 + rnd(0.5);
				sfx_voice.voice.fx_base_params.FreqLimit = sfx_voice.voice.fx_base_params.BaseFreq - 0.2 - rnd(0.6);
				if (sfx_voice.voice.fx_base_params.FreqLimit < 0.2) {
					sfx_voice.voice.fx_base_params.FreqLimit = 0.2;
				}
				sfx_voice.voice.fx_base_params.FreqRamp = -0.15 - rnd(0.2);
				if (rnd(2) == 0) {
					sfx_voice.voice.fx_base_params.BaseFreq = 0.3 + rnd(0.6);
					sfx_voice.voice.fx_base_params.FreqLimit = rnd(0.1);
					sfx_voice.voice.fx_base_params.FreqRamp = -0.35 - rnd(0.3);
				}
				if (rnd(1)) {
					sfx_voice.voice.fx_base_params.Duty = rnd(0.5);
					sfx_voice.voice.fx_base_params.DutyRamp = rnd(0.2);
				} else {
					sfx_voice.voice.fx_base_params.Duty = 0.4 + rnd(0.5);
					sfx_voice.voice.fx_base_params.DutyRamp = -rnd(0.7);
				}
				sfx_voice.voice.fx_base_params.EnvAttack = 0;
				sfx_voice.voice.fx_base_params.EnvSustain = 0.1 + rnd(0.2);
				sfx_voice.voice.fx_base_params.EnvDecay = rnd(0.4);
				if (rnd(1)) {
					sfx_voice.voice.fx_base_params.EnvPunch = rnd(0.3);
				}
				if (rnd(2) == 0) {
					sfx_voice.voice.fx_base_params.FlangerOffset = rnd(0.2);
					sfx_voice.voice.fx_base_params.FlangerRamp = -rnd(0.2);
				}
				if (rnd(1)) {
					sfx_voice.voice.fx_base_params.HPFFreq = rnd(0.3);
				}
				_dirty = true;
				property_list_changed_notify();
			} break;
			case 3: {
				sfx_voice.ResetParams();
				sfx_voice.voice.waveform_shape = 3;
				if (rnd(1)) {
					sfx_voice.voice.fx_base_params.BaseFreq = 0.1 + rnd(0.4);
					sfx_voice.voice.fx_base_params.FreqRamp = -0.1 + rnd(0.4);
				} else {
					sfx_voice.voice.fx_base_params.BaseFreq = 0.2 + rnd(0.7);
					sfx_voice.voice.fx_base_params.FreqRamp = -0.2 - rnd(0.2);
				}
				sfx_voice.voice.fx_base_params.BaseFreq *= sfx_voice.voice.fx_base_params.BaseFreq;
				if (rnd(4) == 0) {
					sfx_voice.voice.fx_base_params.FreqRamp = 0;
				}
				if (rnd(2) == 0) {
					sfx_voice.voice.fx_base_params.RepeatSpeed = 0.3 + rnd(0.5);
				}
				sfx_voice.voice.fx_base_params.EnvAttack = 0;
				sfx_voice.voice.fx_base_params.EnvSustain = 0.1 + rnd(0.3);
				sfx_voice.voice.fx_base_params.EnvDecay = rnd(0.5);
				if (rnd(1) == 0) {
					sfx_voice.voice.fx_base_params.FlangerOffset = -0.3 + rnd(0.9);
					sfx_voice.voice.fx_base_params.FlangerRamp = -rnd(0.3);
				}
				sfx_voice.voice.fx_base_params.EnvPunch = 0.2 + rnd(0.6);
				if (rnd(1)) {
					sfx_voice.voice.fx_base_params.VibStrength = rnd(0.7);
					sfx_voice.voice.fx_base_params.VibSpeed = rnd(0.6);
				}
				if (rnd(2) == 0) {
					sfx_voice.voice.fx_base_params.ArmSpeed = 0.6 + rnd(0.3);
					sfx_voice.voice.fx_base_params.ArmMod = 0.8 - rnd(1.6);
				}
				_dirty = true;
				property_list_changed_notify();
			} break;
			case 4: {
				sfx_voice.ResetParams();
				if (rnd(1)) {
					sfx_voice.voice.waveform_shape = 1;
				} else {
					sfx_voice.voice.fx_base_params.Duty = rnd(0.6);
				}
				if (rnd(1)) {
					sfx_voice.voice.fx_base_params.BaseFreq = 0.2 + rnd(0.3);
					sfx_voice.voice.fx_base_params.FreqRamp = 0.1 + rnd(0.4);
					sfx_voice.voice.fx_base_params.RepeatSpeed = 0.4 + rnd(0.4);
				} else {
					sfx_voice.voice.fx_base_params.BaseFreq = 0.2 + rnd(0.3);
					sfx_voice.voice.fx_base_params.FreqRamp = 0.05 + rnd(0.2);
					if (rnd(1)) {
						sfx_voice.voice.fx_base_params.VibStrength = rnd(0.7);
						sfx_voice.voice.fx_base_params.VibSpeed = rnd(0.6);
					}
				}
				sfx_voice.voice.fx_base_params.EnvAttack = 0;
				sfx_voice.voice.fx_base_params.EnvSustain = rnd(0.4);
				sfx_voice.voice.fx_base_params.EnvDecay = 0.1 + rnd(0.4);
				_dirty = true;
				property_list_changed_notify();
			} break;
			case 5: {
				sfx_voice.ResetParams();
				sfx_voice.voice.waveform_shape = rnd(2);
				if (sfx_voice.voice.waveform_shape == 2) {
					sfx_voice.voice.waveform_shape = 3;
				}
				if (sfx_voice.voice.waveform_shape == 0) {
					sfx_voice.voice.fx_base_params.Duty = rnd(0.6);
				}
				sfx_voice.voice.fx_base_params.BaseFreq = 0.2 + rnd(0.6);
				sfx_voice.voice.fx_base_params.FreqRamp = -0.3 - rnd(0.4);
				sfx_voice.voice.fx_base_params.EnvAttack = 0;
				sfx_voice.voice.fx_base_params.EnvSustain = rnd(0.1);
				sfx_voice.voice.fx_base_params.EnvDecay = 0.1 + rnd(0.2);
				if (rnd(1)) {
					sfx_voice.voice.fx_base_params.HPFFreq = rnd(0.3);
				}
				_dirty = true;
				property_list_changed_notify();
			} break;
			case 6: {
				sfx_voice.ResetParams();
				sfx_voice.voice.waveform_shape = 0;
				sfx_voice.voice.fx_base_params.Duty = rnd(0.6);
				sfx_voice.voice.fx_base_params.BaseFreq = 0.3 + rnd(0.3);
				sfx_voice.voice.fx_base_params.FreqRamp = 0.1 + rnd(0.2);
				sfx_voice.voice.fx_base_params.EnvAttack = 0;
				sfx_voice.voice.fx_base_params.EnvSustain = 0.1 + rnd(0.3);
				sfx_voice.voice.fx_base_params.EnvDecay = 0.1 + rnd(0.2);
				if (rnd(1)) {
					sfx_voice.voice.fx_base_params.HPFFreq = rnd(0.3);
				}
				if (rnd(1)) {
					sfx_voice.voice.fx_base_params.LPFFreq = 1 - rnd(0.6);
				}
				_dirty = true;
				property_list_changed_notify();
			} break;
			case 7: {
				sfx_voice.ResetParams();
				sfx_voice.voice.waveform_shape = rnd(1);
				if (sfx_voice.voice.waveform_shape == 0) {
					sfx_voice.voice.fx_base_params.Duty = rnd(0.6);
				}
				sfx_voice.voice.fx_base_params.BaseFreq = 0.2 + rnd(0.4);
				sfx_voice.voice.fx_base_params.EnvAttack = 0;
				sfx_voice.voice.fx_base_params.EnvSustain = 0.1 + rnd(0.1);
				sfx_voice.voice.fx_base_params.EnvDecay = rnd(0.2);
				sfx_voice.voice.fx_base_params.HPFFreq = 0.1;
				_dirty = true;
				property_list_changed_notify();
			} break;
			case 8: {
				sfx_voice.Randomize();
				if (sfx_voice.voice.morph_rate < 0.25) {
					sfx_voice.voice.morph_rate = 0.25;
				}
				if (sfx_voice.voice.fx_base_params.EnvSustain < 0.5) {
					sfx_voice.voice.fx_base_params.EnvSustain = 0.5;
				}
				if (sfx_voice.voice.fx_base_params.EnvDecay < 0.5) {
					sfx_voice.voice.fx_base_params.EnvDecay = 0.5;
				}
				if (sfx_voice.voice.fx_base_params.RepeatSpeed <= 0.5) {
					sfx_voice.voice.fx_base_params.RepeatSpeed = 0.5;
				}
				_dirty = true;
				property_list_changed_notify();
			} break;
		}
	}

	void call_action(int p_action) {
		switch (p_action) {
			case 1:
				sfx_voice.Mutate();
				_dirty = true;
				break;
			case 2:
				sfx_voice.Randomize();
				_dirty = true;
				break;
			case 3:
				sfx_voice.ResetParams();
				_dirty = true;
				break;
		}
	}

	void set_wave_form(int p_value) {
		sfx_voice.voice.waveform_shape = p_value;
		_dirty = true;
	}
	int get_wave_form() const { return int(sfx_voice.voice.waveform_shape); }
	void set_overtones(int p_value) {
		sfx_voice.voice.fx_base_params.Overtones = p_value;
		_dirty = true;
	}
	int get_overtones() const { return sfx_voice.voice.fx_base_params.Overtones; }
	void set_overtone_falloff(real_t p_value) {
		sfx_voice.voice.fx_base_params.OvertoneRamp = p_value;
		_dirty = true;
	}
	real_t get_overtone_falloff() const { return sfx_voice.voice.fx_base_params.OvertoneRamp; }
	void set_attack_time(real_t p_value) {
		sfx_voice.voice.fx_base_params.EnvAttack = p_value;
		_dirty = true;
	}
	real_t get_attack_time() const { return sfx_voice.voice.fx_base_params.EnvAttack; }
	void set_sustain_time(real_t p_value) {
		sfx_voice.voice.fx_base_params.EnvSustain = p_value;
		_dirty = true;
	}
	real_t get_sustain_time() const { return sfx_voice.voice.fx_base_params.EnvSustain; }
	void set_sustain_punch(real_t p_value) {
		sfx_voice.voice.fx_base_params.EnvPunch = p_value;
		_dirty = true;
	}
	real_t get_sustain_punch() const { return sfx_voice.voice.fx_base_params.EnvPunch; }
	void set_decay_time(real_t p_value) {
		sfx_voice.voice.fx_base_params.EnvDecay = p_value;
		_dirty = true;
	}
	real_t get_decay_time() const { return sfx_voice.voice.fx_base_params.EnvDecay; }
	void set_start_frequency(real_t p_value) {
		sfx_voice.voice.fx_base_params.BaseFreq = p_value;
		_dirty = true;
	}
	real_t get_start_frequency() const { return sfx_voice.voice.fx_base_params.BaseFreq; }
	void set_min_frequency(real_t p_value) {
		sfx_voice.voice.fx_base_params.FreqLimit = p_value;
		_dirty = true;
	}
	real_t get_min_frequency() const { return sfx_voice.voice.fx_base_params.FreqLimit; }
	void set_slide(real_t p_value) {
		sfx_voice.voice.fx_base_params.FreqRamp = p_value;
		_dirty = true;
	}
	real_t get_slide() const { return sfx_voice.voice.fx_base_params.FreqRamp; }
	void set_delta_slide(real_t p_value) {
		sfx_voice.voice.fx_base_params.FreqDRamp = p_value;
		_dirty = true;
	}
	real_t get_delta_slide() const { return sfx_voice.voice.fx_base_params.FreqDRamp; }
	void set_vibrato_depth(real_t p_value) {
		sfx_voice.voice.fx_base_params.VibStrength = p_value;
		_dirty = true;
	}
	real_t get_vibrato_depth() const { return sfx_voice.voice.fx_base_params.VibStrength; }
	void set_vibrato_speed(real_t p_value) {
		sfx_voice.voice.fx_base_params.VibSpeed = p_value;
		_dirty = true;
	}
	real_t get_vibrato_speed() const { return sfx_voice.voice.fx_base_params.VibSpeed; }
	void set_change_repeat(real_t p_value) {
		sfx_voice.voice.fx_base_params.ArmRepeat = p_value;
		_dirty = true;
	}
	real_t get_change_repeat() const { return sfx_voice.voice.fx_base_params.ArmRepeat; }
	void set_change_amount(real_t p_value) {
		sfx_voice.voice.fx_base_params.ArmMod = p_value;
		_dirty = true;
	}
	real_t get_change_amount() const { return sfx_voice.voice.fx_base_params.ArmMod; }
	void set_change_speed(real_t p_value) {
		sfx_voice.voice.fx_base_params.ArmSpeed = p_value;
		_dirty = true;
	}
	real_t get_change_speed() const { return sfx_voice.voice.fx_base_params.ArmSpeed; }
	void set_change_amount2(real_t p_value) {
		sfx_voice.voice.fx_base_params.ArmMod2 = p_value;
		_dirty = true;
	}
	real_t get_change_amount2() const { return sfx_voice.voice.fx_base_params.ArmMod2; }
	void set_change_speed2(real_t p_value) {
		sfx_voice.voice.fx_base_params.ArmSpeed2 = p_value;
		_dirty = true;
	}
	real_t get_change_speed2() const { return sfx_voice.voice.fx_base_params.ArmSpeed2; }
	void set_square_duty(real_t p_value) {
		sfx_voice.voice.fx_base_params.Duty = p_value;
		_dirty = true;
	}
	real_t get_square_duty() const { return sfx_voice.voice.fx_base_params.Duty; }
	void set_duty_sweep(real_t p_value) {
		sfx_voice.voice.fx_base_params.DutyRamp = p_value;
		_dirty = true;
	}
	real_t get_duty_sweep() const { return sfx_voice.voice.fx_base_params.DutyRamp; }
	void set_repeat_speed(real_t p_value) {
		sfx_voice.voice.fx_base_params.RepeatSpeed = p_value;
		_dirty = true;
	}
	real_t get_repeat_speed() const { return sfx_voice.voice.fx_base_params.RepeatSpeed; }
	void set_flanger_offset(real_t p_value) {
		sfx_voice.voice.fx_base_params.FlangerOffset = p_value;
		_dirty = true;
	}
	real_t get_flanger_offset() const { return sfx_voice.voice.fx_base_params.FlangerOffset; }
	void set_flanger_sweep(real_t p_value) {
		sfx_voice.voice.fx_base_params.FlangerRamp = p_value;
		_dirty = true;
	}
	real_t get_flanger_sweep() const { return sfx_voice.voice.fx_base_params.FlangerRamp; }
	void set_lp_filter_cutoff(real_t p_value) {
		sfx_voice.voice.fx_base_params.LPFFreq = p_value;
		_dirty = true;
	}
	real_t get_lp_filter_cutoff() const { return sfx_voice.voice.fx_base_params.LPFFreq; }
	void set_lp_filter_cutoff_sweep(real_t p_value) {
		sfx_voice.voice.fx_base_params.LPFRamp = p_value;
		_dirty = true;
	}
	real_t get_lp_filter_cutoff_sweep() const { return sfx_voice.voice.fx_base_params.LPFRamp; }
	void set_lp_filter_resonance(real_t p_value) {
		sfx_voice.voice.fx_base_params.LPFResonance = p_value;
		_dirty = true;
	}
	real_t get_lp_filter_resonance() const { return sfx_voice.voice.fx_base_params.LPFResonance; }
	void set_hp_filter_cutoff(real_t p_value) {
		sfx_voice.voice.fx_base_params.HPFFreq = p_value;
		_dirty = true;
	}
	real_t get_hp_filter_cutoff() const { return sfx_voice.voice.fx_base_params.HPFFreq; }
	void set_hp_filter_cutoff_sweep(real_t p_value) {
		sfx_voice.voice.fx_base_params.HPFRamp = p_value;
		_dirty = true;
	}
	real_t get_hp_filter_cutoff_sweep() const { return sfx_voice.voice.fx_base_params.HPFRamp; }
	void set_bit_crush(real_t p_value) {
		sfx_voice.voice.fx_base_params.BitCrush = p_value;
		_dirty = true;
	}
	real_t get_bit_crush() const { return sfx_voice.voice.fx_base_params.BitCrush; }
	void set_bit_crush_sweep(real_t p_value) {
		sfx_voice.voice.fx_base_params.BitCrushSweep = p_value;
		_dirty = true;
	}
	real_t get_bit_crush_sweep() const { return sfx_voice.voice.fx_base_params.BitCrushSweep; }
	void set_compression_amount(real_t p_value) {
		sfx_voice.voice.fx_base_params.CompressionAmount = p_value;
		_dirty = true;
	}
	real_t get_compression_amount() const { return sfx_voice.voice.fx_base_params.CompressionAmount; }

	void set_volume(real_t p_value) { sfx_voice.voice.sound_vol = p_value; }
	real_t get_volume() const { return sfx_voice.voice.sound_vol; }
	void set_morph_rate(real_t p_value) { sfx_voice.voice.morph_rate = p_value; }
	real_t get_morph_rate() const { return sfx_voice.voice.morph_rate; }
	void set_loop(bool p_value) { loop = p_value; }
	bool get_loop() const { return loop; }
	void set_loop_offset(real_t p_value) { loop_offset = p_value; }
	real_t get_loop_offset() const { return loop_offset; }

	virtual Ref<AudioStreamPlayback> instance_playback();
	virtual String get_stream_name() const;
	virtual float get_length() const;

	_FORCE_INLINE_ float get_sample_rate() const { return 44100; }

#ifdef TOOLS_ENABLED
	AcceptDialog *load_ui();
	void open_ui();
#endif

	AudioStreamSfxr();
};
VARIANT_ENUM_CAST(AudioStreamSfxr::WaveFormType);

class AudioStreamPlaybackSfxr : public AudioStreamPlayback {
	GDCLASS(AudioStreamPlaybackSfxr, AudioStreamPlayback);

	friend class AudioStreamSfxr;

private:
	bool active;
	Ref<AudioStreamSfxr> sfx_stream;

	int loops;
	int sample_position;

protected:
	virtual void mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames);

public:
	virtual void start(float p_from_pos = 0);
	virtual void stop();
	virtual bool is_playing() const;
	virtual int get_loop_count() const; //times it looped
	virtual float get_playback_position() const;
	virtual void seek(float p_time);
	virtual float get_length() const;

	AudioStreamPlaybackSfxr();
};

class ResourceImporterSfxr : public ResourceImporter {
	GDCLASS(ResourceImporterSfxr, ResourceImporter);

public:
	virtual String get_importer_name() const;
	virtual String get_visible_name() const;
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual String get_save_extension() const;
	virtual String get_resource_type() const;

	virtual int get_preset_count() const;
	virtual String get_preset_name(int p_idx) const;

	virtual void get_import_options(List<ImportOption> *r_options, int p_preset = 0) const;
	virtual bool
	get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const;
	virtual Error import(const String &p_source_file, const String &p_save_path,
			const Map<StringName, Variant> &p_options,
			List<String> *r_platform_variants,
			List<String> *r_gen_files = nullptr,
			Variant *r_metadata = nullptr);

	ResourceImporterSfxr() {}
	~ResourceImporterSfxr() {}
};

/// Godot editor plugin

#ifdef TOOLS_ENABLED
class SfxrEditorPlugin : public EditorPlugin {
	GDCLASS(SfxrEditorPlugin, EditorPlugin)

	EditorNode *editor;

	Ref<AudioStreamSfxr> gen;

	void add_icons_menu_item(const String &p_name, const String &p_callback);
	void remove_icons_menu_item(const String &p_name);

	void _on_show_sfxr_editor_pressed(Variant p_null);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void generate();

	SfxrEditorPlugin(EditorNode *p_node);
};
#endif //TOOLS_ENABLED

#endif // RESOURCE_IMPORTER_SFXR
