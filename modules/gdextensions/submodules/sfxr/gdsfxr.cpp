/**************************************************************************/
/*  gdsfxr.cpp                                                            */
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

#define GAUDIO_NO_ERROR 0
#define GAUDIO_CANNOT_OPEN_FILE -1
#define GAUDIO_UNRECOGNIZED_FORMAT -2
#define GAUDIO_ERROR_WHILE_READING -3
#define GAUDIO_UNSUPPORTED_FORMAT -4

#define g_id unsigned int
#define gaudio_Error int

#define LOG1(a) std::cout << a << "\n";
#define LOG2(a, b) std::cout << a << b << "\n";
#define LOG3(a, b, c) std::cout << a << b << c << "\n";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include <map>

#include "gdsfxr.h"
#include "retrosfxvoice.h"

#include "common/gd_core.h"
#include "core/os/file_access.h"
#include "scene/resources/resource_format_text.h"

// BEGIN AudioStreamSfxr

// UI description
static const char *_ui = R"(
[gd_scene load_steps=2 format=2]

[node name="window" type="AcceptDialog"]
margin_right = 300.0
margin_bottom = 240.0
rect_min_size = Vector2( 600, 480 )
focus_mode = 2
window_title = "Sfxr Editor"
resizable = true

[node name="body" type="HBoxContainer" parent="."]
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
columns = 4

[node name="waveform_label" type="Label" parent="body/controls"]
text = "Wave form"

[node name="waveform_value" type="OptionButton" parent="body/controls"]
items = [ "Square", null, false, 0, null, "Sawtooth", null, false, 1, null, "Sinwave", null, false, 2, null, "Noise", null, false, 3, null, "Triangle", null, false, 4, null, "PinkNoise", null, false, 5, null, "Tan", null, false, 6, null, "Whistle", null, false, 7, null, "Breaker", null, false, 8, null ]

[node name="overtones_label" type="Label" parent="body/controls"]
text = "Overtones"

[node name="overtone_value" type="HScrollBar" parent="body/controls"]
min_value = 0
max_value = 10
step = 1
value = 4

[node name="overtonefalloff_label" type="Label" parent="body/controls"]
text = "Overtone fall-off"

[node name="overtonefalloff_value" type="HScrollBar" parent="body/controls"]
min_value = 0
max_value = 1
step = 0.05
value = 1

[node name="loop_label" type="Label" parent="body/controls"]
text = "Loop"

[node name="loop_value" type="CheckBox" parent="body/controls"]
pressed = false

[node name="loopoffset_label" type="Label" parent="body/controls"]
text = "Loop offset"

[node name="loopoffset_value" type="LineEdit" parent="body/controls"]
pressed = false

[node name="repeatspeed_label" type="Label" parent="body/controls"]
text = "Repeat speed"

[node name="repeatspeed_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="morphrate_label" type="Label" parent="body/controls"]
text = "Morph rate"

[node name="morphrate_value" type="LineEdit" parent="body/controls"]
min_value = 0
max_value = 1
step = 0.05
value = 0.25

[node name="volume_label" type="Label" parent="body/controls"]
text = "Volume"

[node name="volume_value" type="HScrollBar" parent="body/controls"]
min_value = 0
max_value = 1
step = 0.05
value = 0.25

[node name="square_group_label" type="Label" parent="body/controls"]
text = "SQUARE"
align = 2

[node name="sep1" type="HSeparator" parent="body/controls"]
rect_min_size = Vector2( 200, 1 )
[node name="sep2" type="HSeparator" parent="body/controls"]
[node name="sep3" type="HSeparator" parent="body/controls"]
rect_min_size = Vector2( 200, 1 )

[node name="squareduty_label" type="Label" parent="body/controls"]
text = "Square duty"

[node name="squareduty_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.8

[node name="dutysweep_label" type="Label" parent="body/controls"]
text = "Duty sweep"

[node name="dutysweep_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 1

[node name="envelope_group_label" type="Label" parent="body/controls"]
text = "ENVELOPE"
align = 2

[node name="sep4" type="HSeparator" parent="body/controls"]
rect_min_size = Vector2( 200, 1 )
[node name="sep5" type="HSeparator" parent="body/controls"]
[node name="sep6" type="HSeparator" parent="body/controls"]
rect_min_size = Vector2( 200, 1 )

[node name="attacktime_label" type="Label" parent="body/controls"]
text = "Attack time"

[node name="attacktime_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="sustaintime_label" type="Label" parent="body/controls"]
text = "Sustain time"

[node name="sustaintime_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="sustainpunch_label" type="Label" parent="body/controls"]
text = "Sustain time"

[node name="sustainpunch_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="decaytime_label" type="Label" parent="body/controls"]
text = "Sustain time"

[node name="decaytime_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="frequency_group_label" type="Label" parent="body/controls"]
text = "FREQUENCY"
align = 2

[node name="sep7" type="HSeparator" parent="body/controls"]
rect_min_size = Vector2( 200, 1 )
[node name="sep8" type="HSeparator" parent="body/controls"]
[node name="sep9" type="HSeparator" parent="body/controls"]
rect_min_size = Vector2( 200, 1 )

[node name="startfrequency_label" type="Label" parent="body/controls"]
text = "Start frequency"

[node name="startfrequency_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="minfrequency_label" type="Label" parent="body/controls"]
text = "Start frequency"

[node name="minfrequency_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="slide_label" type="Label" parent="body/controls"]
text = "Slide"

[node name="slide_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="deltaslide_label" type="Label" parent="body/controls"]
text = "Delta slide"

[node name="deltaslide_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="vibratodepth_label" type="Label" parent="body/controls"]
text = "Vibrato depth"

[node name="vibratodepth_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="vibratospeed_label" type="Label" parent="body/controls"]
text = "Vibrato speed"

[node name="vibratospeed_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="change_group_label" type="Label" parent="body/controls"]
text = "CHANGE"
align = 2

[node name="sep10" type="HSeparator" parent="body/controls"]
rect_min_size = Vector2( 200, 1 )
[node name="sep11" type="HSeparator" parent="body/controls"]
[node name="sep12" type="HSeparator" parent="body/controls"]
rect_min_size = Vector2( 200, 1 )

[node name="changerepeat_label" type="Label" parent="body/controls"]
text = "Change repeat"

[node name="changerepeat_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.05
value = 0.45

[node name="changeamount_label" type="Label" parent="body/controls"]
text = "Change amount"

[node name="changeamount_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="changespeed_label" type="Label" parent="body/controls"]
text = "Change speed"

[node name="changespeed_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="changeamount2_label" type="Label" parent="body/controls"]
text = "Change amount2"

[node name="changeamount2_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="changespeed2_label" type="Label" parent="body/controls"]
text = "Change speed2"

[node name="changespeed2_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="fill15" type="Label" parent="body/controls"]
[node name="fill16" type="Label" parent="body/controls"]

[node name="flanger_group_label" type="Label" parent="body/controls"]
text = "FLANGER"
align = 2

[node name="sep13" type="HSeparator" parent="body/controls"]
rect_min_size = Vector2( 200, 1 )
[node name="sep14" type="HSeparator" parent="body/controls"]
[node name="sep15" type="HSeparator" parent="body/controls"]
rect_min_size = Vector2( 200, 1 )

[node name="flangeroffset_label" type="Label" parent="body/controls"]
text = "Flanger offset"

[node name="flangeroffset_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="flangersweep_label" type="Label" parent="body/controls"]
text = "Flanger sweep"

[node name="flangersweep_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="filters_group_label" type="Label" parent="body/controls"]
text = "FILTERS"
align = 2

[node name="sep16" type="HSeparator" parent="body/controls"]
rect_min_size = Vector2( 200, 1 )
[node name="sep17" type="HSeparator" parent="body/controls"]
[node name="sep18" type="HSeparator" parent="body/controls"]
rect_min_size = Vector2( 200, 1 )

[node name="lpfiltercutoff_label" type="Label" parent="body/controls"]
text = "Lp filter cut-off"

[node name="lpfiltercutoff_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="lpfiltercutoffsweep_label" type="Label" parent="body/controls"]
text = "Lp filter cut-off sweep"

[node name="lpfiltercutoffsweep_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="lpfilterreson_label" type="Label" parent="body/controls"]
text = "Lp filter resonance"

[node name="lpfilterreson_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="hpfiltercutoff_label" type="Label" parent="body/controls"]
text = "Hp filter cut-off"

[node name="hpfiltercutoff_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="hpfiltercutoffsweep_label" type="Label" parent="body/controls"]
text = "Hp filter cut-off sweep"

[node name="hpfiltercutoffsweep_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="bitcrush_label" type="Label" parent="body/controls"]
text = "Bit crush"

[node name="bitcrush_value" type="HScrollBar" parent="body/controls"]
min_value = 0
max_value = 1
step = 0.05
value = 0.45

[node name="bitcrushsweep_label" type="Label" parent="body/controls"]
text = "Bit crush sweep"

[node name="bitcrushsweep_value" type="HScrollBar" parent="body/controls"]
min_value = -1
max_value = 1
step = 0.1
value = 0.45

[node name="compressionamount_label" type="Label" parent="body/controls"]
text = "Compression amount"

[node name="compressionamount_value" type="HScrollBar" parent="body/controls"]
min_value = 0
max_value = 1
step = 0.05
value = 0.45

[node name="vdiv1" type="VSeparator" parent="body"]

[node name="preview_body" type="VBoxContainer" parent="body"]

[node name="save_options_body" type="HBoxContainer" parent="body/preview_body"]

[node name="outputfile_label" type="Label" parent="body/preview_body/save_options_body"]
text = "Save as file"

[node name="outputfile_value" type="LineEdit" parent="body/preview_body/save_options_body"]
rect_min_size = Vector2( 250, 1 )
size_flags_horizontal = 3

[node name="save" type="Button" parent="body/preview_body/save_options_body"]
text = " Save "

[node name="presets_body" type="VBoxContainer" parent="body/preview_body"]

[node name="presets_label" type="Label" parent="body/preview_body/presets_body"]
text = "Load presets:"

[node name="preset_coin" type="Button" parent="body/preview_body/presets_body"]
size_flags_horizontal = 3
text = "Pickup / Coin"

[node name="preset_laser" type="Button" parent="body/preview_body/presets_body"]
size_flags_horizontal = 3
text = "Laser / Shoot"

[node name="preset_explosion" type="Button" parent="body/preview_body/presets_body"]
size_flags_horizontal = 3
text = "Explosion"

[node name="preset_powerup" type="Button" parent="body/preview_body/presets_body"]
size_flags_horizontal = 3
text = "Power Up"

[node name="preset_hit" type="Button" parent="body/preview_body/presets_body"]
size_flags_horizontal = 3
text = "Hit / Hurt"

[node name="preset_jump" type="Button" parent="body/preview_body/presets_body"]
size_flags_horizontal = 3
text = "Jump"

[node name="preset_blip" type="Button" parent="body/preview_body/presets_body"]
size_flags_horizontal = 3
text = "Blip / Select"

[node name="preset_roboton" type="Button" parent="body/preview_body/presets_body"]
size_flags_horizontal = 3
text = "Robotron"

[node name="hdiv1" type="HSeparator" parent="body/preview_body"]

[node name="actions_body" type="HBoxContainer" parent="body/preview_body"]

[node name="generate" type="Button" parent="body/preview_body/actions_body"]
size_flags_horizontal = 3
text = " Mutate "

[node name="generate" type="Button" parent="body/preview_body/actions_body"]
size_flags_horizontal = 3
text = " Randomize "

[node name="generate" type="Button" parent="body/preview_body/actions_body"]
size_flags_horizontal = 3
text = " Reset "

[node name="hdiv2" type="HSeparator" parent="body/preview_body"]

[node name="preview" type="AudioStreamPlayerControl" parent="body/preview_body"]
size_flags_horizontal = 3

[node name="hdiv3" type="HSeparator" parent="body/preview_body"]

[node name="loadfile_label" type="Label" parent="body/preview_body"]
text = "Load from file:"

[node name="files_list" type="ItemList" parent="body/preview_body"]
size_flags_horizontal = 3
size_flags_vertical = 3

[node name="hdiv4" type="HSeparator" parent="body/preview_body"]

[node name="autogen_body" type="HBoxContainer" parent="body/preview_body"]

[node name="autogen_label" type="Label" parent="body/preview_body/autogen_body"]
text = "Auto-generate sound"

[node name="autogen_value" type="CheckBox" parent="body/preview_body/autogen_body"]
pressed = true

[node name="progress_body" type="HBoxContainer" parent="body/preview_body"]

[node name="generate" type="Button" parent="body/preview_body/progress_body"]
text = " Generate "

[node name="progress" type="ProgressBar" parent="body/preview_body/progress_body"]
size_flags_horizontal = 3

[connection signal="about_to_show" from="." to="." method="_on_window_about_to_show"]
[connection signal="popup_hide" from="." to="." method="_on_window_popup_hide"]
[connection signal="resized" from="." to="." method="_on_window_resized"]
[connection signal="visibility_changed" from="." to="." method="_on_window_visibility_changed"]
[connection signal="preview_updated" from="body/preview" to="." method="_on_preview_updated"]
[connection signal="value_changed" from="body/controls/layers_value" to="." method="_on_value_changed"]
[connection signal="pressed" from="body/controls/generate" to="." method="_on_generate_pressed"]
)";

void AudioStreamSfxr::_update_voice() {
	struct SampleBuffer : public BufferCallback {
		PoolRealArray data;

		virtual void append_sample(float sample) { data.push_back(sample); }
		size_t samples() const { return data.size(); }
	};

	SampleBuffer buffer;
	sfx_voice.Play();
	while (sfx_voice.IsActive()) {
		if (!sfx_voice.Render(256, &buffer)) {
			WARN_PRINT("No samples generated");
		}
	}

	_cache = buffer.data;
	_dirty = false;
}

void AudioStreamSfxr::from_file(const String p_file) {
	if (!sfx_voice.LoadSettings(p_file.utf8().c_str())) {
		ERR_PRINT("Failed to load sfx settings from " + p_file);
	}
}

void AudioStreamSfxr::from_dict(const Dictionary &p_dict) {
}

Error AudioStreamSfxr::save_to_wav(const String &p_path, int quality, int sample_size) {
	ERR_FAIL_COND_V_MSG(sample_size != 8 && sample_size != 16, ERR_PARAMETER_RANGE_ERROR, "Invalid wave bits size (8/16)");
	ERR_FAIL_COND_V_MSG(quality != 11025 && quality != 22050 && quality != 44100, ERR_PARAMETER_RANGE_ERROR, "Invalid wave freq. size (11025/22050/44100)");

	if (!sfx_voice.ExportWav(p_path.utf8().get_data(), sample_size, quality)) {
		WARN_PRINT("Export to wav file failed: " + p_path);
		return ERR_FILE_CANT_WRITE;
	}

	return OK;
}

int AudioStreamSfxr::fill(AudioFrame *p_buffer, int p_frames, int p_from) {
	if (_dirty) {
		const_cast<AudioStreamSfxr *>(this)->_update_voice();
	}

	ERR_FAIL_COND_V(p_from >= _cache.size(), 0);

	for (int p = p_from; p < p_from + p_frames; p++) {
		if (p == _cache.size()) {
			return p - p_from;
		}
		const float sample = _cache[p];
		*p_buffer++ = AudioFrame(sample, sample);
	}

	return p_frames;
}

float AudioStreamSfxr::get_length() const {
	if (_dirty) {
		const_cast<AudioStreamSfxr *>(this)->_update_voice();
	}
	return sfx_voice.GetVoiceLengthInSamples() / 44100.0;
}

Ref<AudioStreamPlayback> AudioStreamSfxr::instance_playback() {
	Ref<AudioStreamPlaybackSfxr> sfx;

	sfx.instance();
	sfx->sfx_stream = Ref<AudioStreamSfxr>(this);

	return sfx;
}

String AudioStreamSfxr::get_stream_name() const {
	return "SFXR Stream";
}

#ifdef TOOLS_ENABLED
AcceptDialog *AudioStreamSfxr::load_ui() {
	if (!dlg) {
		ResourceFormatLoaderText rl;
		Ref<PackedScene> ui = rl.load_from_data(_ui, "gdsfxr_ui.tscn");
		if (ui) {
			dlg = cast_to<AcceptDialog>(ui->instance());
		}
	}
	return dlg;
}

void AudioStreamSfxr::open_ui() {
	dlg->popup_centered_ratio(0.25);
}
#endif // TOOLS_ENABLED

AudioStreamSfxr::AudioStreamSfxr() {
	_dirty = true;
	loop = false;
	loop_offset = 0;
	sfx_voice.ResetParams();
}

/// AudioStreamPlaybackSfxr

void AudioStreamPlaybackSfxr::mix(AudioFrame *p_buffer, float p_rate_scale, int p_frames) {
	ERR_FAIL_COND(!active);

	int filled = sfx_stream->fill(p_buffer, p_frames, sample_position);
	int todo = p_frames - filled;

	sample_position += filled;

	if (todo) {
		//end of file!
		if (sfx_stream->loop) {
			do {
				seek(sfx_stream->loop_offset);
				filled = sfx_stream->fill(p_buffer, p_frames, sample_position);
				todo = p_frames - filled;
				sample_position += filled;
			} while (todo > 0);
			loops++;
		} else {
			for (int i = filled; i < p_frames; i++) {
				p_buffer[i] = AudioFrame(0, 0);
			}
			active = false;
		}
	}
}

void AudioStreamPlaybackSfxr::start(float p_from_pos) {
	active = true;
	loops = 0;
	seek(p_from_pos);
}
void AudioStreamPlaybackSfxr::stop() {
	active = false;
}

bool AudioStreamPlaybackSfxr::is_playing() const {
	return active;
}

int AudioStreamPlaybackSfxr::get_loop_count() const {
	return loops;
}

float AudioStreamPlaybackSfxr::get_playback_position() const {
	return sample_position / sfx_stream->get_sample_rate();
}

void AudioStreamPlaybackSfxr::seek(float p_time) {
	if (!active) {
		return;
	}
	if (p_time >= sfx_stream->get_length()) {
		p_time = 0;
	}
	sample_position = sfx_stream->get_sample_rate() * p_time;
}

float AudioStreamPlaybackSfxr::get_length() const {
	return sfx_stream->get_length();
}

AudioStreamPlaybackSfxr::AudioStreamPlaybackSfxr() {
	active = false;
	loops = 0;
	sample_position = 0;
}

/// Sfx (.rsfx) resource importer

String ResourceImporterSfxr::get_preset_name(int p_idx) const {
	return String();
}

void ResourceImporterSfxr::get_import_options(List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "edit/loop"), false));
	r_options->push_back(ImportOption(PropertyInfo(Variant::REAL, "edit/loop_offset"), 0));
}

bool ResourceImporterSfxr::get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const {
	return true;
}

String ResourceImporterSfxr::get_importer_name() const {
	return "SFXR";
}

String ResourceImporterSfxr::get_visible_name() const {
	return "RetroSoundFX";
}

void ResourceImporterSfxr::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("rsfx");
}

String ResourceImporterSfxr::get_save_extension() const {
	return "res";
}

String ResourceImporterSfxr::get_resource_type() const {
	return "AudioStreamSfxr";
}

int ResourceImporterSfxr::get_preset_count() const {
	return 0;
}

Error ResourceImporterSfxr::import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	const bool loop = p_options["edit/loop"];
	const float loop_offset = p_options["edit/loop_offset"];

	Ref<AudioStreamSfxr> sfx_stream;
	sfx_stream.instance();

	sfx_stream->from_file(p_source_file);
	sfx_stream->set_loop(loop);
	sfx_stream->set_loop_offset(loop_offset);

	return ResourceSaver::save(p_save_path + ".res", sfx_stream);
}

// END

// BEGIN Godot editor plugin

#ifdef TOOLS_ENABLED
void SfxrEditorPlugin::add_icons_menu_item(const String &p_name, const String &p_callback) {
	if (int(Engine::get_singleton()->get_version_info()["hex"]) >= 0x030100) {
		add_tool_menu_item(p_name, this, p_callback);
	}
}

void SfxrEditorPlugin::remove_icons_menu_item(const String &p_name) {
	if (int(Engine::get_singleton()->get_version_info()["hex"]) >= 0x030100) {
		remove_tool_menu_item(p_name);
	}
}

void SfxrEditorPlugin::_on_show_sfxr_editor_pressed(Variant p_null) {
	gen->open_ui();
}

void SfxrEditorPlugin::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			if (AcceptDialog *dlg = gen->load_ui()) {
				get_editor_interface()->get_base_control()->add_child(dlg);
			}
		} break;
		case NOTIFICATION_ENTER_TREE: {
			add_icons_menu_item("SFXR Sound Editor", "_on_show_sfxr_editor_pressed");
		} break;
		case NOTIFICATION_EXIT_TREE: {
			remove_icons_menu_item("SFXR Sound Editor");
		} break;
	}
}

void SfxrEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_show_sfxr_editor_pressed"), &SfxrEditorPlugin::_on_show_sfxr_editor_pressed);
}

SfxrEditorPlugin::SfxrEditorPlugin(EditorNode *p_node) {
	editor = p_node;
	gen = newref(AudioStreamSfxr);
}
#endif // TOOLS_ENABLED

// END
