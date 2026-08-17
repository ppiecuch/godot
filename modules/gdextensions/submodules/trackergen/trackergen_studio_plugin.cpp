/**************************************************************************/
/*  trackergen_studio_plugin.cpp                                         */
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

#include "trackergen_studio_plugin.h"

#ifdef TOOLS_ENABLED

#include "audio_stream_tracker.h"

#include "core/dictionary.h"
#include "core/print_string.h"

static Dictionary _chord(int p_degree, int p_a, int p_b, int p_c, int p_bars) {
	Dictionary d;
	d["degree"] = p_degree;
	PoolIntArray q;
	q.push_back(p_a);
	q.push_back(p_b);
	q.push_back(p_c);
	d["quality"] = q;
	d["bars"] = p_bars;
	return d;
}

Ref<TrackerSong> TrackerStudioEditorPlugin::_build_demo_song() {
	Ref<TrackerInstrument> bass;
	bass.instance();
	bass->set_waveform(TrackerInstrument::WAVE_SAW);
	bass->set_harmonics(16);
	bass->set_filter_cutoff_min(0.45);
	bass->set_filter_cutoff_max(0.06);
	bass->set_variant_count(5);
	bass->set_variant_mode(TrackerInstrument::VARIANT_TENSION);
	bass->set_priority(8);
	bass->set_gain(0.9);

	Ref<TrackerInstrument> lead;
	lead.instance();
	lead->set_waveform(TrackerInstrument::WAVE_SQUARE);
	lead->set_harmonics(20);
	lead->set_variant_count(1);
	lead->set_gain(0.5);
	lead->set_priority(1);

	Ref<TrackerKit> kit;
	kit.instance();
	Array insts;
	insts.push_back(bass);
	insts.push_back(lead);
	kit->set_instruments(insts);

	Ref<ChordProgression> prog;
	prog.instance();
	prog->set_key_root(9); // A
	prog->set_base_octave(3);
	Array chords;
	chords.push_back(_chord(0, 0, 3, 7, 1)); // i  (Am)
	chords.push_back(_chord(5, 0, 4, 7, 1)); // VI (F)
	chords.push_back(_chord(2, 0, 4, 7, 1)); // III(C)
	chords.push_back(_chord(6, 0, 4, 7, 1)); // VII(G)
	prog->set_chords(chords);

	Ref<TrackerPattern> pat;
	pat.instance();
	pat->set_steps(16);
	pat->add_note(0, 0, 0, 0, 0);
	pat->add_note(4, 0, 0, 0, 0);
	pat->add_note(8, 0, 0, 0, 0);
	pat->add_note(12, 0, 0, 0, 0);
	const int tones[4] = { 0, 1, 2, 1 };
	for (int i = 0; i < 8; i++) {
		pat->add_note(i * 2, 1, tones[i % 4], 1, 1);
	}

	Ref<TrackerSong> song;
	song.instance();
	song->set_kit(kit);
	song->set_chords(prog);
	Array pats;
	pats.push_back(pat);
	song->set_patterns(pats);
	PoolIntArray order;
	for (int i = 0; i < 4; i++) {
		order.push_back(0);
	}
	song->set_order(order);
	song->set_tempo_bpm(128.0);
	song->set_steps_per_beat(4);
	song->set_beats_per_bar(4);
	return song;
}

void TrackerStudioEditorPlugin::_render_demo(Variant p_ud) {
	Ref<AudioStreamTracker> stream;
	stream.instance();
	stream->set_song(_build_demo_song());

	const String path = "user://trackergen_studio.wav";
	const Error e = stream->save_to_wav(path, 8.0f, 44100);
	if (e == OK) {
		print_line("trackergen: Tracker Studio rendered demo to " + path);
	} else {
		ERR_PRINT("trackergen: Tracker Studio render failed.");
	}
}

void TrackerStudioEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_render_demo", "ud"), &TrackerStudioEditorPlugin::_render_demo);
}

TrackerStudioEditorPlugin::TrackerStudioEditorPlugin(EditorNode *p_node) {
	editor = p_node;
	add_tool_menu_item("Tracker Studio: Render Demo WAV", this, "_render_demo");
}

TrackerStudioEditorPlugin::~TrackerStudioEditorPlugin() {
	remove_tool_menu_item("Tracker Studio: Render Demo WAV");
}

#endif // TOOLS_ENABLED
