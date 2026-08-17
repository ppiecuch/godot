/**************************************************************************/
/*  trackergen_pattern.h                                                  */
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

#ifndef TRACKERGEN_PATTERN_H
#define TRACKERGEN_PATTERN_H

#include "core/array.h"
#include "core/dictionary.h"
#include "core/resource.h"
#include "core/vector.h"

// A compact, real-time-safe note event (baked from the authoring Array).
struct SeqEvent {
	int step;
	int lane;
	int note_type;
	int note_index;
	int octave;
	int instrument; // index into the song's kit
	float volume;
};

// A grid of steps x lanes. Notes are stored relative to the current chord (see
// ChordProgression) so patterns re-voice as the harmony moves. Authored as an
// Array of Dictionaries; baked into per-step event lists for playback.
class TrackerPattern : public Resource {
	GDCLASS(TrackerPattern, Resource);

public:
	enum NoteType {
		NOTE_CHORD_TONE, // index counts chord tones: 0=root,1=3rd,2=5th...
		NOTE_SCALE_DEGREE, // index counts scale degrees from the key root
		NOTE_ABSOLUTE, // index is a raw MIDI note
		NOTE_OFF, // release the voice on this lane
	};

private:
	int steps;
	// Each: Dictionary { step, lane, type, index, octave, instrument, volume }.
	Array events;

	Vector<Vector<SeqEvent>> _by_step; // baked
	bool _dirty;

protected:
	static void _bind_methods();

public:
	void set_steps(int p_steps);
	int get_steps() const { return steps; }

	void set_events(const Array &p_events);
	Array get_events() const { return events; }

	// Authoring helpers (mark dirty). add_event takes a full event dict; add_note
	// is a 5-arg convenience (CHORD_TONE, volume 1.0) — bind_method caps at 5 args.
	void add_event(const Dictionary &p_event);
	void add_note(int p_step, int p_lane, int p_note_index, int p_octave, int p_instrument);
	void clear();

	void mark_dirty() { _dirty = true; }
	void ensure_baked();

	// Real-time-safe once baked.
	const Vector<SeqEvent> &events_at(int p_step) const;

	TrackerPattern();
};

VARIANT_ENUM_CAST(TrackerPattern::NoteType);

#endif // TRACKERGEN_PATTERN_H
