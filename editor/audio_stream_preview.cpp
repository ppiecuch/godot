/**************************************************************************/
/*  audio_stream_preview.cpp                                              */
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

#include "audio_stream_preview.h"

#include "core/os/keyboard.h"
#include "scene/gui/box_container.h"
#include "editor/editor_settings.h"
#include "editor/editor_scale.h"
#include "editor/editor_node.h"

// BEGIN AudioStreamPreview

float AudioStreamPreview::get_length() const {
	return length;
}
float AudioStreamPreview::get_max(float p_time, float p_time_next) const {
	if (length == 0) {
		return 0;
	}

	int max = preview.size() / 2;
	int time_from = p_time / length * max;
	int time_to = p_time_next / length * max;
	time_from = CLAMP(time_from, 0, max - 1);
	time_to = CLAMP(time_to, 0, max - 1);

	if (time_to <= time_from) {
		time_to = time_from + 1;
	}

	uint8_t vmax = 0;

	for (int i = time_from; i < time_to; i++) {
		uint8_t v = preview[i * 2 + 1];
		if (i == 0 || v > vmax) {
			vmax = v;
		}
	}

	return (vmax / 255.0) * 2.0 - 1.0;
}
float AudioStreamPreview::get_min(float p_time, float p_time_next) const {
	if (length == 0) {
		return 0;
	}

	int max = preview.size() / 2;
	int time_from = p_time / length * max;
	int time_to = p_time_next / length * max;
	time_from = CLAMP(time_from, 0, max - 1);
	time_to = CLAMP(time_to, 0, max - 1);

	if (time_to <= time_from) {
		time_to = time_from + 1;
	}

	uint8_t vmin = 255;

	for (int i = time_from; i < time_to; i++) {
		uint8_t v = preview[i * 2];
		if (i == 0 || v < vmin) {
			vmin = v;
		}
	}

	return (vmin / 255.0) * 2.0 - 1.0;
}

void AudioStreamPreview::_bind_methods() {
	ClassDB::bind_method("get_length", &AudioStreamPreview::get_length);
	ClassDB::bind_method(D_METHOD("get_max", "p_time", "p_time_next"), &AudioStreamPreview::get_max);
	ClassDB::bind_method(D_METHOD("get_min", "p_time", "p_time_next"), &AudioStreamPreview::get_min);
}

AudioStreamPreview::AudioStreamPreview() {
	length = 0;
}

// END AudioStreamPreview


// BEGIN AudioStreamPreviewGenerator

void AudioStreamPreviewGenerator::_update_emit(ObjectID p_id) {
	emit_signal("preview_updated", p_id);
}

void AudioStreamPreviewGenerator::_preview_thread(void *p_preview) {
	Preview *preview = (Preview *)p_preview;

	float muxbuff_chunk_s = 0.25;

	int mixbuff_chunk_frames = AudioServer::get_singleton()->get_mix_rate() * muxbuff_chunk_s;

	Vector<AudioFrame> mix_chunk;
	mix_chunk.resize(mixbuff_chunk_frames);

	int frames_total = AudioServer::get_singleton()->get_mix_rate() * preview->preview->length;
	int frames_todo = frames_total;

	preview->playback->start();

	while (frames_todo) {
		int ofs_write = uint64_t(frames_total - frames_todo) * uint64_t(preview->preview->preview.size() / 2) / uint64_t(frames_total);
		int to_read = MIN(frames_todo, mixbuff_chunk_frames);
		int to_write = uint64_t(to_read) * uint64_t(preview->preview->preview.size() / 2) / uint64_t(frames_total);
		to_write = MIN(to_write, (preview->preview->preview.size() / 2) - ofs_write);

		preview->playback->mix(mix_chunk.ptrw(), 1.0, to_read);

		for (int i = 0; i < to_write; i++) {
			float max = -1000;
			float min = 1000;
			int from = uint64_t(i) * to_read / to_write;
			int to = (uint64_t(i) + 1) * to_read / to_write;
			to = MIN(to, to_read);
			from = MIN(from, to_read - 1);
			if (to == from) {
				to = from + 1;
			}

			for (int j = from; j < to; j++) {
				max = MAX(max, mix_chunk[j].l);
				max = MAX(max, mix_chunk[j].r);

				min = MIN(min, mix_chunk[j].l);
				min = MIN(min, mix_chunk[j].r);
			}

			uint8_t pfrom = CLAMP((min * 0.5 + 0.5) * 255, 0, 255);
			uint8_t pto = CLAMP((max * 0.5 + 0.5) * 255, 0, 255);

			preview->preview->preview.write[(ofs_write + i) * 2 + 0] = pfrom;
			preview->preview->preview.write[(ofs_write + i) * 2 + 1] = pto;
		}

		frames_todo -= to_read;
		singleton->call_deferred("_update_emit", preview->id);
	}

	preview->playback->stop();

	preview->generating.clear();
}

Ref<AudioStreamPreview> AudioStreamPreviewGenerator::generate_preview(const Ref<AudioStream> &p_stream) {
	ERR_FAIL_COND_V(p_stream.is_null(), Ref<AudioStreamPreview>());

	if (previews.has(p_stream->get_instance_id())) {
		return previews[p_stream->get_instance_id()].preview;
	}

	//no preview exists

	previews[p_stream->get_instance_id()] = Preview();

	Preview *preview = &previews[p_stream->get_instance_id()];
	preview->base_stream = p_stream;
	preview->playback = preview->base_stream->instance_playback();
	preview->generating.set();
	preview->id = p_stream->get_instance_id();

	float len_s = preview->base_stream->get_length();
	if (len_s == 0) {
		len_s = 60 * 5; //five minutes
	}

	int frames = AudioServer::get_singleton()->get_mix_rate() * len_s;

	Vector<uint8_t> maxmin;
	int pw = frames / 20;
	maxmin.resize(pw * 2);
	{
		uint8_t *ptr = maxmin.ptrw();
		for (int i = 0; i < pw * 2; i++) {
			ptr[i] = 127;
		}
	}

	preview->preview.instance();
	preview->preview->preview = maxmin;
	preview->preview->length = len_s;

	if (preview->playback.is_valid()) {
		preview->thread = memnew(Thread);
		preview->thread->start(_preview_thread, preview);
	}

	return preview->preview;
}

void AudioStreamPreviewGenerator::_bind_methods() {
	ClassDB::bind_method("_update_emit", &AudioStreamPreviewGenerator::_update_emit);
	ClassDB::bind_method(D_METHOD("generate_preview", "stream"), &AudioStreamPreviewGenerator::generate_preview);

	ADD_SIGNAL(MethodInfo("preview_updated", PropertyInfo(Variant::INT, "obj_id")));
}

AudioStreamPreviewGenerator *AudioStreamPreviewGenerator::singleton = nullptr;

void AudioStreamPreviewGenerator::_notification(int p_what) {
	if (p_what == NOTIFICATION_PROCESS) {
		List<ObjectID> to_erase;
		for (Map<ObjectID, Preview>::Element *E = previews.front(); E; E = E->next()) {
			if (!E->get().generating.is_set()) {
				if (E->get().thread) {
					E->get().thread->wait_to_finish();
					memdelete(E->get().thread);
					E->get().thread = nullptr;
				}
				if (!ObjectDB::get_instance(E->key())) { //no longer in use, get rid of preview
					to_erase.push_back(E->key());
				}
			}
		}

		while (to_erase.front()) {
			previews.erase(to_erase.front()->get());
			to_erase.pop_front();
		}
	}
}

AudioStreamPreviewGenerator::AudioStreamPreviewGenerator() {
	singleton = this;
	set_process(true);
}

// END AudioStreamPreviewGenerator


// BEGIN AudioStreamPlayerControl

void AudioStreamPlayerControl::_play() {
	if (_player->is_playing()) {
		// '_pausing' variable indicates that we want to pause the audio player, not stop it. See '_on_finished()'.
		_pausing = true;
		_player->stop();
		_play_button->set_icon(get_icon("MainPlay", "EditorIcons"));
		set_process(false);
	} else {
		_player->play(_current);
		_play_button->set_icon(get_icon("Pause", "EditorIcons"));
		set_process(true);
	}
}

void AudioStreamPlayerControl::_stop() {
	_player->stop();
	_play_button->set_icon(get_icon("MainPlay", "EditorIcons"));
	_current = 0;
	_indicator->update();
	set_process(false);
}

void AudioStreamPlayerControl::_on_finished() {
	_play_button->set_icon(get_icon("MainPlay", "EditorIcons"));
	if (!_pausing) {
		_current = 0;
		_indicator->update();
	} else {
		_pausing = false;
	}
	set_process(false);
}

void AudioStreamPlayerControl::_draw_indicator() {
	if (!stream.is_valid()) {
		return;
	}

	Rect2 rect = _preview->get_rect();
	float len = stream->get_length();
	float ofs_x = _current / len * rect.size.width;
	const Color color = get_color("accent_color", "Editor");
	_indicator->draw_line(Point2(ofs_x, 0), Point2(ofs_x, rect.size.height), color, Math::round(2 * EDSCALE));
	_indicator->draw_texture(
			get_icon("TimelineIndicator", "EditorIcons"),
			Point2(ofs_x - get_icon("TimelineIndicator", "EditorIcons")->get_width() * 0.5, 0),
			color);

	_current_label->set_text(String::num(_current, 2).pad_decimals(2) + " /");
}

void AudioStreamPlayerControl::_draw_preview() {
	if (!stream.is_valid()) {
		return;
	}

	Rect2 rect = _preview->get_rect();
	Size2 size = get_size();

	Ref<AudioStreamPreview> preview = AudioStreamPreviewGenerator::get_singleton()->generate_preview(stream);
	const float preview_len = preview->get_length();

	Vector<Vector2> lines;
	lines.resize(size.width * 2);

	for (int i = 0; i < size.width; i++) {
		const float ofs = i * preview_len / size.width;
		const float ofs_n = (i + 1) * preview_len / size.width;
		const float max = preview->get_max(ofs, ofs_n) * 0.5 + 0.5;
		const float min = preview->get_min(ofs, ofs_n) * 0.5 + 0.5;

		const int idx = i;
		lines.write[idx * 2 + 0] = Vector2(i + 1, rect.position.y + min * rect.size.y);
		lines.write[idx * 2 + 1] = Vector2(i + 1, rect.position.y + max * rect.size.y);
	}

	Vector<Color> color;
	color.push_back(get_color("contrast_color_2", "Editor"));

	VS::get_singleton()->canvas_item_add_multiline(_preview->get_canvas_item(), lines, color);
}

void AudioStreamPlayerControl::_preview_changed(ObjectID p_which) {
	if (stream.is_valid() && stream->get_instance_id() == p_which) {
		_preview->update();
	}
}

void AudioStreamPlayerControl::_changed_callback(Object *p_changed, const char *p_prop) {
	if (!is_visible()) {
		return;
	}
	update();
}

void AudioStreamPlayerControl::_on_input_indicator(Ref<InputEvent> p_event) {
	const Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->get_button_index() == BUTTON_LEFT) {
		if (mb->is_pressed()) {
			_seek_to(mb->get_position().x);
		}
		_dragging = mb->is_pressed();
	}

	const Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid()) {
		if (_dragging) {
			_seek_to(mm->get_position().x);
		}
	}
}


void AudioStreamPlayerControl::_seek_to(real_t p_x) {
	_current = p_x / _preview->get_rect().size.x * stream->get_length();
	_current = CLAMP(_current, 0, stream->get_length());
	_player->seek(_current);
	_indicator->update();
}

void AudioStreamPlayerControl::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_play"), &AudioStreamPlayerControl::_play);
	ClassDB::bind_method(D_METHOD("_stop"), &AudioStreamPlayerControl::_stop);
	ClassDB::bind_method(D_METHOD("_draw_preview"), &AudioStreamPlayerControl::_draw_preview);
	ClassDB::bind_method(D_METHOD("_draw_indicator"), &AudioStreamPlayerControl::_draw_indicator);
	ClassDB::bind_method(D_METHOD("_preview_changed", "which"), &AudioStreamPlayerControl::_preview_changed);
	ClassDB::bind_method(D_METHOD("_on_finished"), &AudioStreamPlayerControl::_on_finished);
	ClassDB::bind_method(D_METHOD("_on_input_indicator", "event"), &AudioStreamPlayerControl::_on_input_indicator);
}

void AudioStreamPlayerControl::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		AudioStreamPreviewGenerator::get_singleton()->connect("preview_updated", this, "_preview_changed");
	}

	if (p_what == NOTIFICATION_THEME_CHANGED || p_what == NOTIFICATION_ENTER_TREE) {
		_play_button->set_icon(get_icon("MainPlay", "EditorIcons"));
		_stop_button->set_icon(get_icon("Stop", "EditorIcons"));
		_current_label->add_font_override("font", EditorNode::get_singleton()->get_gui_base()->get_font("status_source", "EditorFonts"));
		_duration_label->add_font_override("font", EditorNode::get_singleton()->get_gui_base()->get_font("status_source", "EditorFonts"));
		_preview->set_frame_color(get_color("dark_color_2", "Editor"));

		_indicator->update();
		_preview->update();
	}

	if (p_what == NOTIFICATION_PROCESS) {
		_current = _player->get_playback_position();
		_indicator->update();
	}

	if (p_what == NOTIFICATION_VISIBILITY_CHANGED) {
		if (!is_visible_in_tree()) {
			_stop();
		}
	}
}

void AudioStreamPlayerControl::set_stream(Ref<AudioStream> p_stream) {
	if (!stream.is_null()) {
		stream->remove_change_receptor(this);
	}

	stream = p_stream;
	_player->set_stream(stream);
	_current = 0;
	String text = String::num(stream->get_length(), 2).pad_decimals(2) + "s";
	_duration_label->set_text(text);

	if (!stream.is_null()) {
		stream->add_change_receptor(this);
		update();
	} else {
		hide();
	}
}

AudioStreamPlayerControl::AudioStreamPlayerControl() {
	set_custom_minimum_size(Size2(1, 100) * EDSCALE);

	_player = memnew(AudioStreamPlayer);
	_player->connect("finished", this, "_on_finished");
	add_child(_player);

	VBoxContainer *vbox = memnew(VBoxContainer);
	vbox->set_anchors_and_margins_preset(PRESET_WIDE, PRESET_MODE_MINSIZE, 0);
	add_child(vbox);

	_preview = memnew(ColorRect);
	_preview->set_v_size_flags(SIZE_EXPAND_FILL);
	_preview->connect("draw", this, "_draw_preview");
	vbox->add_child(_preview);

	_indicator = memnew(Control);
	_indicator->set_anchors_and_margins_preset(PRESET_WIDE);
	_indicator->connect("draw", this, "_draw_indicator");
	_indicator->connect("gui_input", this, "_on_input_indicator");
	_preview->add_child(_indicator);

	HBoxContainer *hbox = memnew(HBoxContainer);
	hbox->add_constant_override("separation", 0);
	vbox->add_child(hbox);

	_play_button = memnew(ToolButton);
	hbox->add_child(_play_button);
	_play_button->set_focus_mode(Control::FOCUS_NONE);
	_play_button->connect("pressed", this, "_play");
	_play_button->set_shortcut(ED_SHORTCUT("inspector/audio_preview_play_pause", TTR("Audio Preview Play/Pause"), KEY_SPACE));

	_stop_button = memnew(ToolButton);
	hbox->add_child(_stop_button);
	_stop_button->set_focus_mode(Control::FOCUS_NONE);
	_stop_button->connect("pressed", this, "_stop");

	_current_label = memnew(Label);
	_current_label->set_align(Label::ALIGN_RIGHT);
	_current_label->set_h_size_flags(SIZE_EXPAND_FILL);
	_current_label->set_modulate(Color(1, 1, 1, 0.5));
	hbox->add_child(_current_label);

	_duration_label = memnew(Label);
	hbox->add_child(_duration_label);

	_current = 0;
	_pausing = false;
	_dragging = false;
}

// END AudioStreamPlayerControl
