/**************************************************************************/
/*  explodomatica_plugin.cpp                                              */
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

#include "explodomatica_plugin.h"

#include "common/gd_core.h"
#include "core/math/math_funcs.h"
#include "core/print_string.h"
#include "core/script_language.h"
#include "editor/audio_stream_preview.h"
#include "scene/gui/progress_bar.h"
#include "scene/resources/audio_stream_sample.h"
#include "scene/resources/resource_format_text.h"

// UI description
static const char *_ui = R"(
[gd_scene load_steps=2 format=2]

[node name="window" type="AcceptDialog"]
margin_right = 300.0
margin_bottom = 240.0
rect_min_size = Vector2( 600, 480 )
focus_mode = 2
window_title = "Explodomatica Editor"
resizable = true

[node name="body" type="VBoxContainer" parent="."]
margin_left = 8.0
margin_top = 8.0
margin_right = 592.0
margin_bottom = 444.0
size_flags_horizontal = 3
size_flags_vertical = 3
custom_constants/separation = 16
__meta__ = {
"_edit_use_anchors_": false
}

[node name="controls" type="GridContainer" parent="body"]
size_flags_horizontal = 3
layout_mode = 2
columns = 3

[node name="layers_label" type="Label" parent="body/controls"]
text = "Num. of layers"

[node name="layers_value" type="HScrollBar" parent="body/controls"]
min_value = 1
max_value = 6
step = 1
value = 4
size_flags_horizontal = 3

[node name="layers_value_label" type="Label" parent="body/controls"]
rect_min_size = Vector2( 80, 1 )
text = "4"

[node name="duration_label" type="Label" parent="body/controls"]
text = "Duration"

[node name="duration_value" type="HScrollBar" parent="body/controls"]
min_value = 0.2
max_value = 60
step = 0.05
value = 4

[node name="duration_value_label" type="Label" parent="body/controls"]
text = "4"

[node name="preexplosions_label" type="Label" parent="body/controls"]
text = "Num. of pre-explosions"

[node name="preexplosions_value" type="HScrollBar" parent="body/controls"]
min_value = 0
max_value = 5
step = 1
value = 1

[node name="preexplosions_value_label" type="Label" parent="body/controls"]

[node name="preexplosiondelay_label" type="Label" parent="body/controls"]
text = "Num. of pre-explosions"

[node name="preexplosiondelay_value" type="HScrollBar" parent="body/controls"]
min_value = 0.1
max_value = 5
step = 0.05
value = 0.25

[node name="preexplosiondelay_value_label" type="Label" parent="body/controls"]
text = "0.25"

[node name="prelowpassfactor_label" type="Label" parent="body/controls"]
text = "Pre-explosion low pass factor"

[node name="prelowpassfactor_value" type="HScrollBar" parent="body/controls"]
min_value = 0.2
max_value = 0.9
step = 0.05
value = 0.8

[node name="prelowpassfactor_value_label" type="Label" parent="body/controls"]
text = "0.8"

[node name="prelowpassiters_label" type="Label" parent="body/controls"]
text = "Pre-explosion low pass iterations"

[node name="prelowpassiters_value" type="HScrollBar" parent="body/controls"]
min_value = 0
max_value = 10
step = 1
value = 1

[node name="prelowpassiters_value_label" type="Label" parent="body/controls"]
text = "1"

[node name="speedfactor_label" type="Label" parent="body/controls"]
text = "Final speed factor"

[node name="speedfactor_value" type="HScrollBar" parent="body/controls"]
min_value = 0.1
max_value = 10
step = 0.05
value = 0.45

[node name="speedfactor_value_label" type="Label" parent="body/controls"]
text = "0.45"

[node name="reverb_label" type="Label" parent="body/controls"]
text = "Add reverb"

[node name="reverb_value" type="CheckBox" parent="body/controls"]
pressed = true

[node name="fill1" type="Label" parent="body/controls"]

[node name="revearlyrefls_label" type="Label" parent="body/controls"]
text = "Final reverb early reflections"

[node name="revearlyrefls_value" type="HScrollBar" parent="body/controls"]
min_value = 1
max_value = 50
step = 1
value = 10

[node name="revearlyrefls_value_label" type="Label" parent="body/controls"]
text = "10"

[node name="revlaterefls_label" type="Label" parent="body/controls"]
text = "Final reverb late reflections"

[node name="revlaterefls_value" type="HScrollBar" parent="body/controls"]
min_value = 1
max_value = 1000
step = 1
value = 50

[node name="revlaterefls_value_label" type="Label" parent="body/controls"]
text = "50"

[node name="output_body" type="HBoxContainer" parent="body"]

[node name="outputfile_label" type="Label" parent="body/output_body"]
text = "Save as file"

[node name="outputfile_value" type="LineEdit" parent="body/output_body"]
size_flags_horizontal = 3
text = "explo.wav"

[node name="save" type="Button" parent="body/output_body"]
text = " Save "

[node name="silence_label" type="Label" parent="body/output_body"]
text = "Silence treshold"

[node name="silence_value" type="LineEdit" parent="body/output_body"]
rect_min_size = Vector2( 100, 1 )
text = "0.0001"

[node name="progress_body" type="HBoxContainer" parent="body"]

[node name="generate" type="Button" parent="body/progress_body"]
text = " Generate "

[node name="progress" type="ProgressBar" parent="body/progress_body"]
size_flags_horizontal = 3

[node name="copyparams" type="Button" parent="body/progress_body"]
text = " Copy params "

[node name="autogen_label" type="Label" parent="body/progress_body"]
text = "Auto-generate sound"

[node name="autogen_value" type="CheckBox" parent="body/progress_body"]
pressed = true

[node name="preview" type="AudioStreamPlayerControl" parent="body"]
size_flags_horizontal = 3
size_flags_vertical = 3

[connection signal="about_to_show" from="." to="." method="_on_window_about_to_show"]
[connection signal="popup_hide" from="." to="." method="_on_window_popup_hide"]
[connection signal="resized" from="." to="." method="_on_window_resized"]
[connection signal="visibility_changed" from="." to="." method="_on_window_visibility_changed"]
[connection signal="value_changed" from="body/controls/layers_value" to="." method="_on_value_changed" binds= [ "layers" ]]
[connection signal="value_changed" from="body/controls/duration_value" to="." method="_on_value_changed" binds= [ "duration" ]]
[connection signal="value_changed" from="body/controls/preexplosions_value" to="." method="_on_value_changed" binds= [ "preexplosions" ]]
[connection signal="value_changed" from="body/controls/preexplosiondelay_value" to="." method="_on_value_changed" binds= [ "preexplosiondelay" ]]
[connection signal="value_changed" from="body/controls/prelowpassfactor_value" to="." method="_on_value_changed" binds= [ "prelowpassfactor" ]]
[connection signal="value_changed" from="body/controls/prelowpassiters_value" to="." method="_on_value_changed" binds= [ "prelowpassiters" ]]
[connection signal="value_changed" from="body/controls/speedfactor_value" to="." method="_on_value_changed" binds= [ "speedfactor" ]]
[connection signal="value_changed" from="body/controls/revearlyrefls_value" to="." method="_on_value_changed" binds= [ "revearlyrefls" ]]
[connection signal="value_changed" from="body/controls/revlaterefls_value" to="." method="_on_value_changed" binds= [ "revlaterefls" ]]
[connection signal="pressed" from="body/progress_body/generate" to="." method="_on_generate_pressed"]
[connection signal="pressed" from="body/progress_body/copyparams" to="." method="_on_copyparams_pressed"]
[connection signal="pressed" from="body/progress_body/save" to="." method="_on_save_pressed"]
)";

static struct explosion_def explodomatica_defaults = {
	"",
	Samples(),
	4, /* duration in seconds (roughly) */
	4, /* nlayers */
	1, /* preexplosions */
	0.25, /* preexplosion delay, 250ms */
	0.8, /* preexplosion low pass factor */
	1, /* preexplosion low pass iters */
	0.45, /* final speed factor */
	10, /* final reverb early reflections */
	50, /* final reverb late reflections */
	true, /* reverb needed? */
	0.0001, /* silence threshold */
	0, /* progress */
};

#define SAMPLE_RATE 44100

static _FORCE_INLINE_ double drand(void) { return Math::randd(); }
static _FORCE_INLINE_ int irand(int n) { return (n * (Math::rand() & 0x0ffff)) / 0x0ffff; }

#pragma pack(push, 1)
struct WavFileHeader {
	union {
		char RIFF[4];
		struct {
			char R, I, F1, F2;
		};
	};
	uint32_t file_size;
	union {
		struct {
			char W, A, V, E;
		};
		char WAVE[4];
	};
	union {
		char fmt[4];
		struct {
			char f, m, t, space;
		};
	};
	uint32_t header_size;

	uint16_t format;
	uint16_t channels;
	uint32_t sample_rate;
	uint32_t bytes_per_second;
	uint16_t bytes_per_sample;
	uint16_t bits_per_sample;

	union {
		char data_header[4];
		struct {
			char D, A1, T, A2;
		};
	};
	uint32_t data_size;
};
static_assert(sizeof(WavFileHeader) == 44, "We expect the wav header to be 44 bytes long");
#pragma pack(pop)

Samples load_wav_file(const String filename, WavFileHeader *header) {
	FILE *wav = fopen(filename.utf8().c_str(), "r");

	if (!wav) {
		ERR_PRINT("Unable to open " + filename);
		return Samples();
	}

	if (!fseek(wav, 0, SEEK_END)) {
		return Samples();
	}

	if (ftell(wav) < sizeof(WavFileHeader)) {
		ERR_PRINT(filename + " was less than " + itos(sizeof(WavFileHeader)) + " bytes and is most likely not a wav file");
		return Samples();
	}

	if (fseek(wav, 0, SEEK_SET)) {
		ERR_PRINT(filename + " seek failed");
		return Samples();
	}

	WavFileHeader loaded_wav = {};
	const size_t header_size = fread(reinterpret_cast<char *>(&loaded_wav), 1, sizeof(WavFileHeader), wav);
	if (header_size != sizeof(WavFileHeader)) {
		ERR_PRINT(filename + " failed to load wav header");
		return Samples();
	}

	// Check every byte of the header to make sure it is valid
	if (loaded_wav.R != 'R' || loaded_wav.I != 'I' || loaded_wav.F1 != 'F' || loaded_wav.F2 != 'F' || loaded_wav.W != 'W' || loaded_wav.A != 'A' || loaded_wav.V != 'V' || loaded_wav.E != 'E' || loaded_wav.f != 'f' || loaded_wav.m != 'm' || loaded_wav.t != 't' || loaded_wav.header_size != 16 || loaded_wav.D != 'd' || loaded_wav.A1 != 'a' || loaded_wav.T != 't' || loaded_wav.A2 != 'a') {
		ERR_PRINT(filename + " was not a valid wav file");
		return Samples();
	}

	if (loaded_wav.format != 1 || (loaded_wav.bits_per_sample != 8 && loaded_wav.bits_per_sample != 16)) {
		ERR_PRINT(filename + " format not supported");
		return Samples();
	}

	LocalVector<uint8_t> data(loaded_wav.data_size);
	const size_t load_size = fread(data.ptr(), 1, loaded_wav.data_size, wav);

	fclose(wav);

	if (load_size != loaded_wav.data_size) {
		ERR_PRINT(filename + " was loaded successfully");
		return Samples();
	}

	const uint16_t channels = loaded_wav.channels;
	const uint16_t bits_per_sample = loaded_wav.bits_per_sample;

	uint8_t *ucharp = (uint8_t *)data.ptr();
	int16_t *shortp = (int16_t *)data.ptr();
	const size_t num_samples = load_size / (channels * bits_per_sample / 8);

	Samples samples(num_samples);
	double *buf = samples.ptr();

	const double norm_factor = 1.0 / ((bits_per_sample == 8) ? 0x80 : 0x8000);

	if (channels == 1 && bits_per_sample == 8) {
		for (size_t i = 0; i < num_samples; i++) {
			*(buf++) = norm_factor * (int(ucharp[i]) - 0x80);
		}
	} else if (channels == 1 && bits_per_sample == 16) {
		for (size_t i = 0; i < num_samples; i++) {
			*(buf++) = shortp[i] * norm_factor;
		}
	} else if (channels == 2 && bits_per_sample == 8) {
		for (size_t i = 0; i < num_samples; i += 2) {
			*(buf++) = norm_factor * (int(ucharp[i] + ucharp[i + 1]) / 2 - 0x80);
		}
	} else if (channels == 2 && bits_per_sample == 16) {
		for (size_t i = 0; i < num_samples; i += 2) {
			*(buf++) = norm_factor * (shortp[i] + shortp[i + 1]) / 2;
		}
	}
	if (header) {
		*header = loaded_wav;
	}
	return samples;
}

static int seconds_to_frames(double seconds) { return seconds * SAMPLE_RATE; }

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

static void check(const Samples &s, const String &hdr = "") {
	String ind = "";
	if (!hdr.empty()) {
		print_verbose(hdr);
		ind = "  ";
	}
	print_verbose(ind + "Size: " + itos(s.size()));
	int sounds = 0;
	for (int i = 0; i < s.size(); i++) {
		if (Math::abs(s[i]) > 0.00001) {
			sounds++;
		}
	}
	print_verbose(ind + "Sounds: " + itos(sounds));
}

static Samples make_sinewave(int nsamples, real_t frequency) {
	Samples s(nsamples);
	const real_t delta = frequency * 2 * Math_PI / SAMPLE_RATE;
	real_t theta = 0;
	for (int i = 0; i < nsamples; i++) {
		s[i] = Math::sin(theta) * 0.5;
		theta += delta;
	}
	return s;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

static Samples add_sound(const Samples &s1, const Samples &s2) {
	Samples s(MAX(s1.size(), s2.size()));
	for (int i = 0; i < s.size(); i++) {
		s[i] = 0;
		if (i < s1.size()) {
			s[i] += s1[i];
		}
		if (i < s2.size()) {
			s[i] += s2[i];
		}
	}
	return s;
}

static _FORCE_INLINE_ void accumulate_sound(Samples &acc, const Samples &inc) {
	acc = add_sound(acc, inc);
}

static _FORCE_INLINE_ void amplify_in_place(Samples &s, double gain) {
	for (int i = 0; i < s.size(); i++) {
		s[i] *= gain;
		s[i] = CLAMP(s[i], -1, 1);
	}
}

static Samples make_noise(explosion_def *e, size_t nsamples) {
	ERR_FAIL_COND_V(nsamples == 0, Samples());

	Samples s(nsamples);

	// If there is input data, use that rather than generating noise
	if (!e->input_data.empty()) {
		s.copy_from(e->input_data);
		return s;
	}

	// generate noise
	for (int i = 0; i < nsamples; i++) {
		s[i] = 2.0 * drand() - 1.0;
	}
	amplify_in_place(s, 0.70);
	return s;
}

static _FORCE_INLINE_ void fadeout(Samples &s) {
	for (int i = 0; i < s.size(); i++) {
		const double factor = 1.0 - (double(i) / double(s.size()));
		s[i] *= factor;
	}
}

// algorithm for low pass filter gleaned from wikipedia
// and adapted for stereo samples
static Samples sliding_low_pass(const Samples &s, double alpha1, double alpha2) {
	ERR_FAIL_COND_V(s.size() == 0, Samples());
	Samples o(s.size());
	o[0] = s[0];
	for (int i = 1; i < s.size();) {
		double alpha = (double(i) / double(s.size())) * (alpha2 - alpha1) + alpha1;
		alpha = alpha * alpha;
		o[i] = o[i - 1] + alpha * (s[i] - o[i - 1]);
		i++;
	}
	return o;
}

static void sliding_low_pass_inplace(Samples &s, double alpha1, double alpha2) {
	ERR_FAIL_COND(s.size() == 0);
	for (int i = 1; i < s.size();) {
		double alpha = (double(i) / double(s.size())) * (alpha2 - alpha1) + alpha1;
		alpha = alpha * alpha;
		s[i] = s[i - 1] + alpha * (s[i] - s[i - 1]);
		i++;
	}
}

static double interpolate(double x, double x1, double y1, double x2, double y2) {
	// return corresponding y on line (x1,y1), (x2,y2) for value x by similar triangles:
	//   (y2 -y1)/(x2 - x1) = (y - y1) / (x - x1)
	//   (x -x1) * (y2 -y1)/(x2 -x1) = y - y1
	//   y = (x - x1) * (y2 - y1) / (x2 -x1) + y1
	if (Math::abs(x2 - x1) < (0.01 * 1.0 / double(SAMPLE_RATE))) {
		return (y1 + y2) / 2.0;
	}
	return (x - x1) * (y2 - y1) / (x2 - x1) + y1;
}

static Samples change_speed(const Samples &s, double factor) {
	ERR_FAIL_COND_V(factor <= 0, Samples());
	ERR_FAIL_COND_V(s.size() == 0, Samples());

	const int nsamples = s.size() / factor;
	Samples o(nsamples);

	o[0] = s[0];

	for (int i = 0; i < nsamples - 2; i++) {
		const double sample_point = double(i) / double(nsamples) * double(s.size());
		const int sp1 = sample_point;
		const int sp2 = sp1 + 1;
		o[i] = interpolate(sample_point, sp1, s[sp1], sp2, s[sp2]);
	}
	return o;
}

static void change_speed_inplace(Samples &s, double factor) {
	const Samples &o = change_speed(s, factor);
	ERR_FAIL_COND(o.size() == 0);
	s = o;
}

static void renormalize(Samples &s) {
	double max = 0;
	for (int i = 0; i < s.size(); i++) {
		if (Math::abs(s[i]) > max) {
			max = Math::abs(s[i]);
		}
	}
	for (int i = 0; i < s.size(); i++) {
		s[i] = s[i] / (1.05 * max);
	}
}

static void delay_effect_in_place(Samples &s, int delay_samples) {
	for (int i = s.size() - 1; i >= 0; i--) {
		const int source = i - delay_samples;
		if (s.size() > source) {
			if (source > 0) {
				s[i] = s[source];
			} else {
				s[i] = 0;
			}
		}
	}
}

static void dot(const char chr = '.') {
	printf("%c", chr);
	fflush(stdout);
}

static Samples make_explosion(explosion_def *e, double seconds, int nlayers) {
	Samples s[10];

	for (int i = 0; i < nlayers; i++) {
		Samples t = make_noise(e, seconds_to_frames(seconds));

		if (i > 0) {
			change_speed_inplace(t, i * 2);
		}

		int iters = i + 1;
		if (iters > 3) {
			iters = 3;
		}
		for (int j = 0; j < iters; j++) {
			fadeout(t);
		}
		const double a1 = double(i + 1) / double(nlayers);
		const double a2 = double(i) / double(nlayers);

		iters = 3 - i;
		if (iters < 0) {
			iters = 1;
		}
		for (int j = 0; j < iters; j++) {
			sliding_low_pass_inplace(t, a1, a2);
			renormalize(t);
		}
		s[i] = t;
	}

	for (int i = 1; i < nlayers; i++) {
		accumulate_sound(s[0], s[i]);
	}
	renormalize(s[0]);
	return s[0];
}

static void trim_trailing_silence(Samples &s, real_t threshold) {
	int silent = 0;
	for (int i = s.size() - 1; i >= 0; i--) {
		if (Math::abs(s[i]) < threshold) {
			silent++;
		}
	}
	if (silent) {
		s.resize(s.size() - silent);
	}
}

static Samples make_preexplosions(explosion_def *e) {
	if (!e->preexplosions) {
		return Samples();
	}
	Samples pe(seconds_to_frames(e->duration));
	for (int i = 0; i < e->preexplosions; i++) {
		Samples exp = make_explosion(e, e->duration / 2, e->nlayers);
		const int offset = irand(seconds_to_frames(e->preexplosion_delay));
		delay_effect_in_place(exp, offset);
		accumulate_sound(pe, exp);
		renormalize(pe);
	}
	for (int i = 0; i < e->preexplosion_lp_iters; i++) {
		sliding_low_pass_inplace(pe, e->preexplosion_low_pass_factor, e->preexplosion_low_pass_factor);
	}
	renormalize(pe);
	return pe;
}

static Samples read_input_file(String filename) {
	WavFileHeader hdr;
	Samples data = load_wav_file(filename, &hdr);
	if (!data.empty()) {
		print_verbose("Input file:    " + filename);
		print_verbose("  sample rate: " + itos(hdr.sample_rate));
		print_verbose("  channels:    " + itos(hdr.channels));
		print_verbose("  format:      " + itos(hdr.format));
		print_verbose("  data size:   " + itos(hdr.data_size));
	}
	return data;
}

// BEGIN Explodomatica generator

typedef PassthroughScript<AcceptDialog, ExplodomaticaGenerator> ExplodomaticaUIScriptInstanceBase;

class ExplodomaticaUIScript : public ExplodomaticaUIScriptInstanceBase {
	GDCLASS(ExplodomaticaUIScript, ExplodomaticaUIScriptInstanceBase)

	_THREAD_SAFE_CLASS_

public:
	ExplodomaticaUIScript(ExplodomaticaGenerator *p_recv) {
		set_receiver(p_recv);
	}
};

void ExplodomaticaGenerator::_set_progress(explosion_def *e, float progress_val) {
	e->explodomatica_progress = progress_val;
	if (e->explodomatica_progress > 1.05) {
		e->explodomatica_progress = 0;
	}
	emit_signal("sound_process_progress", e->explodomatica_progress);
}

void ExplodomaticaGenerator::_update_progress(explosion_def *e, float progress_inc) {
	e->explodomatica_progress += progress_inc;
	if (e->explodomatica_progress > 1.05) {
		e->explodomatica_progress = 0;
	}
	emit_signal("sound_process_progress", e->explodomatica_progress);
}

Samples ExplodomaticaGenerator::_simple_reverb_effect(explosion_def *e, const Samples &s) {
	ERR_FAIL_NULL_V(e, Samples());
	ERR_FAIL_COND_V(s.size() == 0, Samples());

	const int early_refls = e->reverb_early_refls;
	const int late_refls = e->reverb_late_refls;
	const float progress_inc = 1.0 / (early_refls + late_refls);

	print_verbose("Calculating basic reverb effect");

	dot();

	Samples withverb = s;
	Samples echo = withverb;

	dot('E');
	for (int i = 0; i < early_refls; i++) {
		dot();
		Samples echo2 = sliding_low_pass(echo, 0.5, 0.5);
		const double gain = drand() * 0.03 + 0.03;
		amplify_in_place(echo, gain);
		// 300 ms range
		const int delay = (0.3 * SAMPLE_RATE * (rand() & 0x0ffff)) / 0x0ffff;
		delay_effect_in_place(echo2, delay);
		accumulate_sound(withverb, echo2);
		_update_progress(e, progress_inc);
	}

	dot('L');
	for (int i = 0; i < late_refls; i++) {
		dot();
		Samples echo2 = sliding_low_pass(echo, 0.5, 0.2);
		const double gain = drand() * 0.01 + 0.03;
		amplify_in_place(echo, gain);
		// 2000 ms range
		const int delay = (2 * SAMPLE_RATE * (rand() & 0x0ffff)) / 0x0ffff;
		delay_effect_in_place(echo2, delay);
		accumulate_sound(withverb, echo2);
		_update_progress(e, progress_inc);
	}
	dot('\n');
	return withverb;
}

PoolVector<uint8_t> ExplodomaticaGenerator::_get_data(const Samples &s, AudioStreamSample::Format format) const {
	PoolVector<uint8_t> data;
	const bool is8 = format == AudioStreamSample::FORMAT_8_BITS;
	if (data.resize(s.size() * (is8 ? 1 : 2)) == OK) {
		if (format == AudioStreamSample::FORMAT_8_BITS) {
			int8_t *buf = (int8_t *)data.write().ptr();
			for (size_t i = 0; i < s.size(); i++) {
				*(buf++) = s[i] * 0x80;
			}
		} else if (format == AudioStreamSample::FORMAT_16_BITS) {
			int16_t *buf = (int16_t *)data.write().ptr();
			for (size_t i = 0; i < s.size(); i++) {
				*(buf++) = s[i] * 0x8000;
			}
		} else {
			WARN_PRINT("Unknown format");
		}
	}
	return data;
}

Ref<AudioStreamSample> ExplodomaticaGenerator::_get_samples(const Samples &s, AudioStreamSample::Format p_format) const {
	Ref<AudioStreamSample> sample;
	sample.instance();
	sample->set_data(_get_data(s, p_format));
	sample->set_format(p_format);
	sample->set_mix_rate(SAMPLE_RATE);
	sample->set_loop_mode(AudioStreamSample::LOOP_DISABLED);
	sample->set_loop_begin(0);
	sample->set_loop_end(0);
	sample->set_stereo(false);

	return sample;
}

Samples ExplodomaticaGenerator::_explodomatica(struct explosion_def *e) {
	Samples s2;

	_set_progress(e, 0);

	if (!e->input_file.empty()) {
		e->input_data = read_input_file(e->input_file);
	}
	Samples pe = make_preexplosions(e);
	check(pe, "Preexplosions:");

	if (!e->reverb) {
		_set_progress(e, 0.33);
	}
	Samples s = make_explosion(e, e->duration, e->nlayers);
	check(s, "Explosion:");
	if (!e->reverb) {
		_set_progress(e, 0.5);
	}
	if (!pe.empty()) {
		accumulate_sound(s, pe);
		renormalize(s);
	}
	if (!e->reverb) {
		_set_progress(e, 0.8);
	}
	change_speed_inplace(s, e->final_speed_factor);
	check(s, "Speed:");
	trim_trailing_silence(s, e->silence_threshold);
	if (e->reverb) {
		s2 = _simple_reverb_effect(e, s);
		check(s2, "Reverb:");
		trim_trailing_silence(s2, e->silence_threshold);
	} else {
		s2 = s;
		if (!e->reverb) {
			_set_progress(e, 0.9);
		}
	}
	_set_progress(e, 1);

	return s2;
}

void ExplodomaticaGenerator::set_num_sound_layers(int p_nlayers) {
	ERR_FAIL_COND(p_nlayers <= 0);
	defs.nlayers = p_nlayers;
}

int ExplodomaticaGenerator::get_num_sound_layers() const {
	return defs.nlayers;
}

void ExplodomaticaGenerator::set_sound_quality(int p_quality) {
	ERR_FAIL_INDEX(p_quality, 2);
	sample_format = p_quality;
}

int ExplodomaticaGenerator::get_sound_quality() const {
	return sample_format;
}

void ExplodomaticaGenerator::set_sound_duration(real_t p_duration) {
}

real_t ExplodomaticaGenerator::get_sound_duration() const {
	return defs.duration;
}

void ExplodomaticaGenerator::set_num_preexplosions(int p_num) {
}

int ExplodomaticaGenerator::get_num_preexplosions() const {
	return defs.preexplosions;
}

void ExplodomaticaGenerator::set_preexplosion_delay(real_t p_delay) {
}

real_t ExplodomaticaGenerator::get_preexplosion_delay() const {
	return defs.preexplosion_delay;
}

void ExplodomaticaGenerator::set_preexplosion_low_pass_factor(real_t p_factor) {
}

real_t ExplodomaticaGenerator::get_preexplosion_low_pass_factor() const {
	return defs.preexplosion_low_pass_factor;
}

void ExplodomaticaGenerator::set_preexplosion_lp_iters(int p_iters) {
}

int ExplodomaticaGenerator::get_preexplosion_lp_iters() const {
	return defs.preexplosion_lp_iters;
}

void ExplodomaticaGenerator::set_final_speed_factor(real_t p_factor) {
}

real_t ExplodomaticaGenerator::get_final_speed_factor() const {
	return defs.final_speed_factor;
}

void ExplodomaticaGenerator::set_reverb_early_refls(int p_reverb_early) {
}

int ExplodomaticaGenerator::get_reverb_early_refls() const {
	return defs.reverb_early_refls;
}

void ExplodomaticaGenerator::set_reverb_late_refls(int p_reverb_late) {
}

int ExplodomaticaGenerator::get_reverb_late_refls() const {
	return defs.reverb_late_refls;
}

void ExplodomaticaGenerator::set_reverb(bool p_state) {
	if (defs.reverb != p_state) {
		defs.reverb = p_state;
		if (auto_generate) {
			call_deferred("generate()");
		}
	}
}

bool ExplodomaticaGenerator::is_reverb() const {
	return defs.reverb;
}

void ExplodomaticaGenerator::set_silence_threshold(real_t p_threshold) {
	ERR_FAIL_COND(p_threshold < 0 || p_threshold > 1);
	if (defs.silence_threshold != p_threshold) {
		defs.silence_threshold = p_threshold;
		if (auto_generate) {
			call_deferred("generate()");
		}
	}
}

real_t ExplodomaticaGenerator::get_silence_threshold() const {
	return defs.silence_threshold;
}

void ExplodomaticaGenerator::set_output_file(const String &p_path) {
}

String ExplodomaticaGenerator::get_output_file() const {
	return output_file;
}

void ExplodomaticaGenerator::set_auto_generate(bool p_state) {
}

bool ExplodomaticaGenerator::is_auto_generate() const {
	return auto_generate;
}

void ExplodomaticaGenerator::generate() {
	sound = _explodomatica(&defs);
	emit_signal("sound_process_ready", get_samples());
}

Ref<AudioStreamSample> ExplodomaticaGenerator::get_samples() const {
	static const AudioStreamSample::Format _fmt[] = { AudioStreamSample::FORMAT_8_BITS, AudioStreamSample::FORMAT_16_BITS };
	Ref<AudioStreamSample> sample;
	sample.instance();
	sample->set_data(_get_data(sound, _fmt[sample_format]));
	sample->set_format(_fmt[sample_format]);
	sample->set_mix_rate(SAMPLE_RATE);
	sample->set_loop_mode(AudioStreamSample::LOOP_DISABLED);
	sample->set_loop_begin(0);
	sample->set_loop_end(0);
	sample->set_stereo(false);

	return sample;
}

#ifdef TOOLS_ENABLED
AcceptDialog *ExplodomaticaGenerator::load_ui() {
	if (!dlg) {
		ResourceFormatLoaderText rl;
		Ref<PackedScene> ui = rl.load_from_data(_ui, "explodomatica_ui.tscn");
		if (ui) {
			dlg = cast_to<AcceptDialog>(ui->instance());
			dlg_script = newref(ExplodomaticaUIScript, this);
			dlg->set_script(dlg_script.get_ref_ptr());
			if ((cleanup = memnew(Timer))) {
				cleanup->set_one_shot(false);
				cleanup->set_wait_time(1);
				cleanup->set_timer_process_mode(Timer::TIMER_PROCESS_IDLE);
				cleanup->connect("timeout", this, "_cleanup_ui");
				dlg->add_child(cleanup);
			}
		}
	}
	return dlg;
}

void ExplodomaticaGenerator::open_ui() {
	dlg->popup_centered_ratio(0.25);
}

void ExplodomaticaGenerator::_cleanup_ui() {
	ERR_FAIL_NULL(dlg);
	if (ProgressBar *bar = cast_to<ProgressBar>(dlg->get_node_or_null(String("body/controls/progress")))) {
		bar->set_value(0);
	}
}

void ExplodomaticaGenerator::_on_sound_progress(real_t p_progress) {
	ERR_FAIL_NULL(dlg);
	if (ProgressBar *bar = cast_to<ProgressBar>(dlg->get_node_or_null(String("body/controls/progress")))) {
		bar->set_value(p_progress * 100);
	}
}

void ExplodomaticaGenerator::_on_sound_ready(Ref<AudioStreamSample> sound) {
	cleanup->start();
	if (AudioStreamPlayerControl *player = cast_to<AudioStreamPlayerControl>(dlg->get_node_or_null(String("body/preview")))) {
		player->set_stream(sound);
	}
}

void ExplodomaticaGenerator::_on_generate_pressed() {
	// update silence treshold
	if (LineEdit *edt = cast_to<LineEdit>(dlg->get_node_or_null(String("body/output_body/silence_value")))) {
		defs.silence_threshold = CLAMP(MAX(0, edt->get_text().to_double()), 0, 1);
	}
	generate();
}

void ExplodomaticaGenerator::_on_copyparams_pressed() {
	String params;
	params += "layers: " + itos(defs.nlayers) + "\n";
	params += "preexplosion: " + itos(defs.preexplosions) + "\n";
	params += "preexplosion_delay: " + String::num(defs.preexplosion_delay, 2) + "\n";
	params += "preexplosion_low_pass_factor: " + String::num(defs.preexplosion_low_pass_factor, 2) + "\n";
	params += "preexplosion_lp_iters: " + itos(defs.preexplosion_lp_iters) + "\n";
	params += "final_speed_factor: " + String::num(defs.preexplosion_low_pass_factor, 2) + "\n";
	params += "reverb_early_refls: " + itos(defs.reverb_early_refls) + "\n";
	params += "reverb_late_refls: " + itos(defs.reverb_late_refls) + "\n";
	params += "reverb: " + itos(defs.reverb) + "\n";
	params += "silence_threshold: " + rtos(defs.silence_threshold) + "\n";
	OS::get_singleton()->set_clipboard(params);
}

void ExplodomaticaGenerator::_on_save_pressed() {
	if (LineEdit *edt = cast_to<LineEdit>(dlg->get_node_or_null(String("body/controls/outputfile_value")))) {
		if (!edt->get_text().empty()) {
			get_samples()->save_to_wav(edt->get_text());
		}
	}
}

void ExplodomaticaGenerator::_on_value_changed(float value, String node) {
	const String value_node = "body/controls/" + node + "_value";
	const String label_node = "body/controls/" + node + "_value_label";
	if (Label *lbl = cast_to<Label>(dlg->get_node_or_null(label_node))) {
		if (HScrollBar *scr = cast_to<HScrollBar>(dlg->get_node_or_null(value_node))) {
			const float value = scr->get_value();
			if (scr->get_step() == 1) { // int
				lbl->set_text(itos(value));
			} else {
				lbl->set_text(String::num(value, 2));
			}
			if (node == "layers") {
				defs.nlayers = value;
			} else if (node == "duration") {
				defs.duration = value;
			} else if (node == "preexplosions") {
				defs.preexplosions = value;
			} else if (node == "preexplosiondelay") {
				defs.preexplosion_delay = value;
			} else if (node == "prelowpassfactor") {
				defs.preexplosion_low_pass_factor = value;
			} else if (node == "prelpiters") {
				defs.preexplosion_lp_iters = value;
			} else if (node == "speedfactor") {
				defs.final_speed_factor = value;
			} else if (node == "revearlyrefls") {
				defs.reverb_early_refls = value;
			} else if (node == "revlaterefls") {
				defs.reverb_late_refls = value;
			}
			_change_notify();
		}
	}
}

void ExplodomaticaGenerator::_on_window_about_to_show() {
}

void ExplodomaticaGenerator::_on_window_popup_hide() {
}

void ExplodomaticaGenerator::_on_window_resized() {
}

void ExplodomaticaGenerator::_on_window_visibility_changed() {
}
#endif

void ExplodomaticaGenerator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_num_sound_layers", "nlayers"), &ExplodomaticaGenerator::set_num_sound_layers);
	ClassDB::bind_method(D_METHOD("get_num_sound_layers"), &ExplodomaticaGenerator::get_num_sound_layers);
	ClassDB::bind_method(D_METHOD("set_sound_quality", "quality"), &ExplodomaticaGenerator::set_sound_quality);
	ClassDB::bind_method(D_METHOD("get_sound_quality"), &ExplodomaticaGenerator::get_sound_quality);
	ClassDB::bind_method(D_METHOD("set_sound_duration", "duration"), &ExplodomaticaGenerator::set_sound_duration);
	ClassDB::bind_method(D_METHOD("get_sound_duration"), &ExplodomaticaGenerator::get_sound_duration);
	ClassDB::bind_method(D_METHOD("set_num_preexplosions", "num"), &ExplodomaticaGenerator::set_num_preexplosions);
	ClassDB::bind_method(D_METHOD("get_num_preexplosions"), &ExplodomaticaGenerator::get_num_preexplosions);
	ClassDB::bind_method(D_METHOD("set_preexplosion_delay", "delay"), &ExplodomaticaGenerator::set_preexplosion_delay);
	ClassDB::bind_method(D_METHOD("get_preexplosion_delay"), &ExplodomaticaGenerator::get_preexplosion_delay);
	ClassDB::bind_method(D_METHOD("set_preexplosion_low_pass_factor", "factor"), &ExplodomaticaGenerator::set_preexplosion_low_pass_factor);
	ClassDB::bind_method(D_METHOD("get_preexplosion_low_pass_factor"), &ExplodomaticaGenerator::get_preexplosion_low_pass_factor);
	ClassDB::bind_method(D_METHOD("set_preexplosion_lp_iters", "iters"), &ExplodomaticaGenerator::set_preexplosion_lp_iters);
	ClassDB::bind_method(D_METHOD("get_preexplosion_lp_iters"), &ExplodomaticaGenerator::get_preexplosion_lp_iters);
	ClassDB::bind_method(D_METHOD("set_final_speed_factor", "factor"), &ExplodomaticaGenerator::set_final_speed_factor);
	ClassDB::bind_method(D_METHOD("get_final_speed_factor"), &ExplodomaticaGenerator::get_final_speed_factor);
	ClassDB::bind_method(D_METHOD("set_reverb_early_refls", "rev_early"), &ExplodomaticaGenerator::set_reverb_early_refls);
	ClassDB::bind_method(D_METHOD("get_reverb_early_refls"), &ExplodomaticaGenerator::get_reverb_early_refls);
	ClassDB::bind_method(D_METHOD("set_reverb_late_refls", "rev_late"), &ExplodomaticaGenerator::set_reverb_late_refls);
	ClassDB::bind_method(D_METHOD("get_reverb_late_refls"), &ExplodomaticaGenerator::get_reverb_late_refls);
	ClassDB::bind_method(D_METHOD("set_reverb", "enable"), &ExplodomaticaGenerator::set_reverb);
	ClassDB::bind_method(D_METHOD("is_reverb"), &ExplodomaticaGenerator::is_reverb);
	ClassDB::bind_method(D_METHOD("set_silence_threshold", "level"), &ExplodomaticaGenerator::set_silence_threshold);
	ClassDB::bind_method(D_METHOD("get_silence_threshold"), &ExplodomaticaGenerator::get_silence_threshold);
	ClassDB::bind_method(D_METHOD("set_output_file", "filename"), &ExplodomaticaGenerator::set_output_file);
	ClassDB::bind_method(D_METHOD("get_output_file"), &ExplodomaticaGenerator::get_output_file);
	ClassDB::bind_method(D_METHOD("set_auto_generate", "enable"), &ExplodomaticaGenerator::set_auto_generate);
	ClassDB::bind_method(D_METHOD("is_auto_generate"), &ExplodomaticaGenerator::is_auto_generate);

#ifdef TOOLS_ENABLED
	ClassDB::bind_method(D_METHOD("_cleanup_ui"), &ExplodomaticaGenerator::_cleanup_ui);
	ClassDB::bind_method(D_METHOD("_on_generate_pressed"), &ExplodomaticaGenerator::_on_generate_pressed);
	ClassDB::bind_method(D_METHOD("_on_copyparams_pressed"), &ExplodomaticaGenerator::_on_copyparams_pressed);
	ClassDB::bind_method(D_METHOD("_on_save_pressed"), &ExplodomaticaGenerator::_on_save_pressed);
	ClassDB::bind_method(D_METHOD("_on_sound_progress"), &ExplodomaticaGenerator::_on_sound_progress);
	ClassDB::bind_method(D_METHOD("_on_sound_ready", "sound"), &ExplodomaticaGenerator::_on_sound_ready);
	ClassDB::bind_method(D_METHOD("_on_value_changed", "value"), &ExplodomaticaGenerator::_on_value_changed);
	ClassDB::bind_method(D_METHOD("_on_window_about_to_show"), &ExplodomaticaGenerator::_on_window_about_to_show);
	ClassDB::bind_method(D_METHOD("_on_window_popup_hide"), &ExplodomaticaGenerator::_on_window_popup_hide);
	ClassDB::bind_method(D_METHOD("_on_window_resized"), &ExplodomaticaGenerator::_on_window_resized);
	ClassDB::bind_method(D_METHOD("_on_window_visibility_changed"), &ExplodomaticaGenerator::_on_window_visibility_changed);
#endif

	ADD_PROPERTY(PropertyInfo(Variant::INT, "num_sound_layers", PROPERTY_HINT_RANGE, "1,6,1"), "set_num_sound_layers", "get_num_sound_layers");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "set_sound_quality", PROPERTY_HINT_ENUM, "8 bit,16 bit"), "set_sound_quality", "get_sound_quality");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "duration", PROPERTY_HINT_RANGE, "0.2,60,0.05"), "set_sound_duration", "get_sound_duration");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "preexplosions", PROPERTY_HINT_RANGE, "0,5,1"), "set_num_preexplosions", "get_num_preexplosions");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "preexplosion_delay", PROPERTY_HINT_RANGE, "0.1,3,0.05"), "set_preexplosion_delay", "get_preexplosion_delay");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "preexplosion_low_pass_factor", PROPERTY_HINT_RANGE, "0.2,0.9,0.05"), "set_preexplosion_low_pass_factor", "get_preexplosion_low_pass_factor");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "preexplosion_lp_iters", PROPERTY_HINT_RANGE, "0,10,1"), "set_preexplosion_lp_iters", "get_preexplosion_lp_iters");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "final_speed_factor", PROPERTY_HINT_RANGE, "0.1,10,0.05"), "set_final_speed_factor", "get_final_speed_factor");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "reverb_early_reflections", PROPERTY_HINT_RANGE, "1,50,1"), "set_reverb_early_refls", "get_reverb_early_refls");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "reverb_late_reflections", PROPERTY_HINT_RANGE, "1,1000,1"), "set_reverb_late_refls", "get_reverb_late_refls");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "reverb"), "set_reverb", "is_reverb");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "silence_threshold", PROPERTY_HINT_RANGE, "0,1"), "set_silence_threshold", "get_silence_threshold");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "output_file"), "set_output_file", "get_output_file");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_generate"), "set_auto_generate", "is_auto_generate");

	ADD_SIGNAL(MethodInfo("sound_process_progress", PropertyInfo(Variant::REAL, "progress")));
	ADD_SIGNAL(MethodInfo("sound_process_ready", PropertyInfo(Variant::OBJECT, "sound")));
}

ExplodomaticaGenerator::ExplodomaticaGenerator() {
	sample_format = 1;
	auto_generate = false;
	defs = explodomatica_defaults;
	_dirty = false;

#ifdef TOOLS_ENABLED
	connect("sound_process_progress", this, "_on_sound_progress");
	connect("sound_process_ready", this, "_on_sound_ready");
#endif
}

// END Explodomatica generator

// BEGIN Godot editor plugin

void ExplodomaticaEditorPlugin::add_icons_menu_item(const String &p_name, const String &p_callback) {
	if (int(Engine::get_singleton()->get_version_info()["hex"]) >= 0x030100) {
		add_tool_menu_item(p_name, this, p_callback);
	}
}

void ExplodomaticaEditorPlugin::remove_icons_menu_item(const String &p_name) {
	if (int(Engine::get_singleton()->get_version_info()["hex"]) >= 0x030100) {
		remove_tool_menu_item(p_name);
	}
}

void ExplodomaticaEditorPlugin::_on_show_explodomatica_editor_pressed(Variant p_null) {
	gen->open_ui();
}

void ExplodomaticaEditorPlugin::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			if (AcceptDialog *dlg = gen->load_ui()) {
				get_editor_interface()->get_base_control()->add_child(dlg);
			}
		} break;
		case NOTIFICATION_ENTER_TREE: {
			add_icons_menu_item("Explodomatica Editor", "_on_show_explodomatica_editor_pressed");
		} break;
		case NOTIFICATION_EXIT_TREE: {
			remove_icons_menu_item("Explodomatica Editor");
		} break;
	}
}

void ExplodomaticaEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_show_explodomatica_editor_pressed"), &ExplodomaticaEditorPlugin::_on_show_explodomatica_editor_pressed);
}

ExplodomaticaEditorPlugin::ExplodomaticaEditorPlugin(EditorNode *p_node) {
	editor = p_node;
	gen = newref(ExplodomaticaGenerator);
}

// END
