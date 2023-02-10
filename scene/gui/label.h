/**************************************************************************/
/*  label.h                                                               */
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

#ifndef LABEL_H
#define LABEL_H

#include "core/pair.h"
#include "scene/gui/control.h"
#include "scene/resources/font.h"

#include <memory>

// allow for testing private Label's methods
namespace TestFont {
class TestMainLoop;
}

class Label : public Control {
	GDCLASS(Label, Control);

public:
	typedef Pair<CharType, CharType> CharPair;

	struct AnimationController;
	struct AnimationNone;
	struct GenericSingleTransformController;
	struct GenericDualTransformController;
	struct GenericMulti1TransformController;
	struct GenericMulti2TransformController;

	enum Align {

		ALIGN_LEFT,
		ALIGN_CENTER,
		ALIGN_RIGHT,
		ALIGN_FILL,
		AlignCount
	};

	enum VAlign {

		VALIGN_TOP,
		VALIGN_CENTER,
		VALIGN_BOTTOM,
		VALIGN_FILL,
		VAlignCount
	};

	enum TransitionEffect {
		TRANSITIONEFFECT_NONE,
		TRANSITIONEFFECT_SLIDE_UP,
		TRANSITIONEFFECT_SLIDE_DOWN,
		TRANSITIONEFFECT_SLIDE_UP_NEW,
		TRANSITIONEFFECT_SLIDE_DOWN_NEW,
		TRANSITIONEFFECT_WHEEL_UP,
		TRANSITIONEFFECT_WHEEL_DOWN,
		TRANSITIONEFFECT_WHEEL_UP_NEW,
		TRANSITIONEFFECT_WHEEL_DOWN_NEW,
		TRANSITIONEFFECT_REVEAL_UP,
		TRANSITIONEFFECT_REVEAL_DOWN,
		TRANSITIONEFFECT_REVEAL_UP_NEW,
		TRANSITIONEFFECT_REVEAL_DOWN_NEW,
		TRANSITIONEFFECT_ROTATE_V,
		TRANSITIONEFFECT_ROTATE_H,
		TRANSITIONEFFECT_ROTATE_V_SEQ,
		TRANSITIONEFFECT_ROTATE_H_SEQ,
		TRANSITIONEFFECT_SLIDE_UP_SEQ,
		TRANSITIONEFFECT_SLIDE_DOWN_SEQ,
		TRANSITIONEFFECT_COUNT
	};

#define DEFINE_TRANSITIONEASE(M) TRANSITIONEASE_##M##_IN, TRANSITIONEASE_##M##_OUT, TRANSITIONEASE_##M##_INOUT

	enum TransitionEase {
		TRANSITIONEASE_NONE,
		DEFINE_TRANSITIONEASE(LINEAR),
		DEFINE_TRANSITIONEASE(SINE),
		DEFINE_TRANSITIONEASE(CIRC),
		DEFINE_TRANSITIONEASE(CUBIC),
		DEFINE_TRANSITIONEASE(QUAD),
		DEFINE_TRANSITIONEASE(EXPO),
		DEFINE_TRANSITIONEASE(BACK),
		DEFINE_TRANSITIONEASE(BOUNCE),
		DEFINE_TRANSITIONEASE(ELASTIC)
	};
#undef DEFINE_TRANSITIONEASE

private:
	Align align;
	VAlign valign;
	String text;
	String xl_text;
	bool autowrap;
	bool clip;
	bool uppercase;
	float horizontal_spacing;
	float vertical_spacing;

	int get_longest_line_width(const String &s) const;

	struct WordList {
		enum {
			CHAR_NEWLINE = -1,
			CHAR_WRAPLINE = -2
		};
		int char_pos; // if -1, then newline '\n' (CHAR_NEWLINE), -2 if wrapline (soft-break)
		int line, line_pos;
		int word_len;
		int pixel_width;
		int space_count; // spaces before the word
		WordList *next;
		WordList() {
			char_pos = 0;
			line = line_pos = 0;
			word_len = 0;
			pixel_width = 0;
			next = nullptr;
			space_count = 0;
		}
	};

	struct WordCache {
		WordList *words;
		String cache_text;
		int total_char_cache;
		int line_count;
		int width;
		Size2 minsize;
		WordCache() {
			words = nullptr;
			total_char_cache = 0;
			line_count = 0;
			width = 0;
			minsize = Size2(0, 0);
		}
	};

	void _dump_word_cache(const WordCache &cache) const;

	bool word_cache_dirty;
	void regenerate_word_cache();
	WordCache calculate_word_cache(const Ref<Font> &font, const String &label_text) const;
	int get_line_size(const WordCache &cache, int line) const;

	WordCache word_cache;

	real_t percent_visible;
	int visible_chars;
	int lines_skipped;
	int max_lines_visible;

	bool animate;
	real_t transition_duration;
	struct TransitionText {
		String text, xl_text;
		WordCache word_cache;
	} transition_text;
	TransitionEase transition_ease;
	TransitionEffect transition_effect;

	bool _transition_dirty;
	bool _cache_changed;
	std::unique_ptr<AnimationController> _transition_controller;

	void _clear_pending_animations();

protected:
	void _notification(int p_what);

	static void _bind_methods();

	// bind helpers
public:
	virtual Size2 get_minimum_size() const;

	void set_align(Align p_align);
	Align get_align() const;

	void set_valign(VAlign p_align);
	VAlign get_valign() const;

	void set_text(const String &p_string);
	String get_text() const;

	void set_autowrap(bool p_autowrap);
	bool has_autowrap() const;

	void set_uppercase(bool p_uppercase);
	bool is_uppercase() const;

	void set_visible_characters(int p_amount);
	int get_visible_characters() const;
	int get_total_character_count() const;

	void set_transition_duration(real_t p_duration);
	real_t get_transition_duration() const;

	void set_transition_ease(TransitionEase p_ease);
	TransitionEase get_transition_ease() const;

	void set_transition_effect(TransitionEffect p_effect);
	TransitionEffect get_transition_effect() const;

	bool is_transition_active() const;
	bool is_transition_enabled() const;

	void set_clip_text(bool p_clip);
	bool is_clipping_text() const;

	void set_percent_visible(float p_percent);
	float get_percent_visible() const;

	void set_lines_skipped(int p_lines);
	int get_lines_skipped() const;

	void set_max_lines_visible(int p_lines);
	int get_max_lines_visible() const;

	int get_line_height() const;
	int get_line_count() const;
	int get_visible_line_count() const;

	void set_horizontal_spacing(float p_offset);
	float get_horizontal_spacing() const;
	void set_vertical_spacing(float p_offset);
	float get_vertical_spacing() const;

	Label(const String &p_text = String());
	~Label();

	friend class TestFont::TestMainLoop;
};

VARIANT_ENUM_CAST(Label::Align);
VARIANT_ENUM_CAST(Label::VAlign);
VARIANT_ENUM_CAST(Label::TransitionEase);
VARIANT_ENUM_CAST(Label::TransitionEffect);

#endif // LABEL_H
