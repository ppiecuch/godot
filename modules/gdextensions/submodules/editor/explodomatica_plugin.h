#ifndef EXPLODOMATICA_PLUGIN_H
#define EXPLODOMATICA_PLUGIN_H

/// Generate explosion sound effects
/// (C) Copyright 2011, Stephen M. Cameron.

// duration n    Specifies the approximate duration in seconds the explosion should last. Fractional seconds are permitted.
//               Default value is: explodomatica_defaults.duration
// nlayers n     Specifies the number of sound layers which should be used to create each sub-explosion within the explosion.
//               Default is: explodomatica_defaults.nlayers
// preexplosions Specifies the number of "preexplosions" to generate. You can think of the "preexplosions" as the "ka" part
//               of "ka-BOOM!"
//               Default is: explodomatica_defaults.preexplosions
// pre-delay     Specifies the approximate duration in seconds of the "ka" part of "ka-BOOM!". The value is somewhat randomized
//               and this value is more like an upper bound with zero being the lower bound.
//               Default is: explodomatica_defaults.preexplosion_delay
// pre-lp-factor Specifies the factor to use with the low pass filter on the pre-explosion. Values closer to zero do more
//               (suppress the pre-explosion more) while values closer to 1 do less. Value should be between 0.2 and 0.9.
//               Default is: explodomatica_defaults.preexplosion_low_pass_factor
// pre-lp-count  Specifies how many times to apply the low pass filter to the pre-explosion.
//               Default is: explodomatica_defaults.preexplosion_lp_iters
// speedfactor   Specifies the factor by which to speed up or slow down the final explosion sound.  Values greater than 1.0
//               speed the sound up, values less than 1.0 slow the sound down.
//               Default is: explodomatica_defaults.final_speed_factor
// noreverb      Suppress the 'reverb' effect.
// early-refls   Number of early reflections in reverb.
// late-refls    Number of late reflections in reverb.
// input file    Use the given (44100Hz mono) wav file as input instead of generating white noise for input.
//
// Examples:
// ---------
// ./explodomatica --nlayers 10 --speedfactor 2 --duration 4.5 --preexplosions 0 demo1.wav
// ./explodomatica --nlayers 10 --speedfactor 3 --duration 2.5 --preexplosions 0 demo2.wav
// ./explodomatica --nlayers 1 --speedfactor 0.3 --duration 5 --preexplosions 2 --pre-delay 0.4 demo3.wav

#include "editor/editor_plugin.h"
#include "scene/gui/dialogs.h"
#include "scene/resources/audio_stream_sample.h"

struct Samples : public LocalVector<double> {
	void _FORCE_INLINE_ zero() { zero(size()); }

	void _FORCE_INLINE_ zero(size_t nsamples) {
		nsamples = MIN(size(), nsamples);
		memset(ptr(), 0, sizeof(double) * nsamples);
	}

	void _FORCE_INLINE_ copy_to(Samples &dest, size_t nsamples) {
		const size_t n = MIN(size(), dest.size());
		memcpy(dest.ptr(), ptr(), sizeof(double) * n);
	}

	void _FORCE_INLINE_ copy_from(const Samples &src) {
		const size_t n = MIN(size(), src.size());
		memcpy(ptr(), src.ptr(), sizeof(double) * n);
	}

	Samples() :
			LocalVector<double>() {}
	Samples(size_t nsamples) :
			LocalVector<double>(nsamples) {
		zero();
	}
};

struct explosion_def {
	String input_file;
	Samples input_data;
	real_t duration;
	int nlayers;
	int preexplosions;
	real_t preexplosion_delay;
	real_t preexplosion_low_pass_factor;
	int preexplosion_lp_iters;
	real_t final_speed_factor;
	int reverb_early_refls;
	int reverb_late_refls;
	bool reverb;
	real_t silence_threshold;
	float explodomatica_progress;
};

class ExplodomaticaGenerator : public Reference {
	GDCLASS(ExplodomaticaGenerator, Reference)

	Samples sound;
	int sample_format;
	String output_file;
	bool auto_generate;

	explosion_def defs;
	void _set_progress(explosion_def *e, real_t progress_val);
	void _update_progress(explosion_def *e, real_t progress_inc = 0);
	Samples _simple_reverb_effect(explosion_def *e, const Samples &s);
	Samples _explodomatica(explosion_def *e);

	bool _dirty;
	PoolVector<uint8_t> _get_data(const Samples &s, AudioStreamSample::Format p_format) const;
	Ref<AudioStreamSample> _get_samples(const Samples &s, AudioStreamSample::Format p_format) const;

	Ref<Script> dlg_script;
	AcceptDialog *dlg;
	Timer *cleanup;

#ifdef TOOLS_ENABLED
	void _cleanup_ui();
	void _on_generate_pressed();
	void _on_copyparams_pressed();
	void _on_save_pressed();
	void _on_sound_progress(float p_progress);
	void _on_sound_ready(Ref<AudioStreamSample> sound);
	void _on_value_changed(float value, String node);
	void _on_window_about_to_show();
	void _on_window_popup_hide();
	void _on_window_resized();
	void _on_window_visibility_changed();
#endif

protected:
	static void _bind_methods();

public:
	void set_num_sound_layers(int p_nlayers);
	int get_num_sound_layers() const;
	void set_sound_quality(int p_quality);
	int get_sound_quality() const;
	void set_sound_duration(real_t p_duration);
	real_t get_sound_duration() const;
	void set_num_preexplosions(int p_num);
	int get_num_preexplosions() const;
	void set_preexplosion_delay(real_t p_delay);
	real_t get_preexplosion_delay() const;
	void set_preexplosion_low_pass_factor(real_t p_factor);
	real_t get_preexplosion_low_pass_factor() const;
	void set_preexplosion_lp_iters(int p_iters);
	int get_preexplosion_lp_iters() const;
	void set_final_speed_factor(real_t p_factor);
	real_t get_final_speed_factor() const;
	void set_reverb_early_refls(int p_reverb_early);
	int get_reverb_early_refls() const;
	void set_reverb_late_refls(int p_reverb_late);
	int get_reverb_late_refls() const;
	void set_reverb(bool p_state);
	bool is_reverb() const;
	void set_silence_threshold(real_t p_threshold);
	real_t get_silence_threshold() const;
	void set_output_file(const String &p_path);
	String get_output_file() const;
	void set_auto_generate(bool p_state);
	bool is_auto_generate() const;

	Ref<AudioStreamSample> get_samples() const;
#ifdef TOOLS_ENABLED
	AcceptDialog *load_ui();
	void open_ui();
#endif
	void generate();

	ExplodomaticaGenerator();
};

/// Godot editor plugin

class ExplodomaticaEditorPlugin : public EditorPlugin {
	GDCLASS(ExplodomaticaEditorPlugin, EditorPlugin)

	EditorNode *editor;

	Ref<ExplodomaticaGenerator> gen;

	void add_icons_menu_item(const String &p_name, const String &p_callback);
	void remove_icons_menu_item(const String &p_name);

	void _on_show_explodomatica_editor_pressed(Variant p_null);
	void _on_sound_ready();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void generate();

	ExplodomaticaEditorPlugin(EditorNode *p_node);
};

#endif // EXPLODOMATICA_PLUGIN_H
