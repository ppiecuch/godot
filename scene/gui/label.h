/*************************************************************************/
/*  label.h                                                              */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef LABEL_H
#define LABEL_H

#include "scene/gui/control.h"

struct AnimationTransform {
    CharTransform xform;
    float current;
    float duration;

    AnimationTransform() : current(0), duration(0) { }
};

struct AnimationController {
    enum AnimCtrlOpts {
        ANIMCTRL_OK  = 0,
        ANIMCTRL_UPDATE_TEXT = 1, // update text to transition text
        ANIMCTRL_DONE  = 2,       // transition complited
    };

    virtual void init_xform(float duration, AnimationTransform &xform) = 0;
    virtual int update(float dt, AnimationTransform &xform) = 0;
    virtual bool is_active() const = 0;
};

class Label : public Control {

	GDCLASS(Label, Control);

public:
	enum Align {

		ALIGN_LEFT,
		ALIGN_CENTER,
		ALIGN_RIGHT,
		ALIGN_FILL
	};

	enum VAlign {

		VALIGN_TOP,
		VALIGN_CENTER,
		VALIGN_BOTTOM,
		VALIGN_FILL
	};

    enum TransitionEffect {
        TRANSITIONEFFECT_SLIDE,
        TRANSITIONEFFECT_ROTATE
    };

    enum TransitionBehaviour {
        TRANSITIONBEHAVIOUR_ALL,
        TRANSITIONBEHAVIOUR_NEW
    };

private:
	Align align;
	VAlign valign;
	String text;
	String xl_text;
	bool autowrap;
	bool clip;
	Size2 minsize;
	int line_count;
	bool uppercase;
    float horizontal_spacing;
    float vertical_spacing;

	int get_longest_line_width(const String &s) const;

	struct WordCache {

		enum {
			CHAR_NEWLINE = -1,
			CHAR_WRAPLINE = -2
		};
		int char_pos; // if -1, then newline (CHAR_NEWLINE)
		int word_len;
		int pixel_width;
		int space_count;
		WordCache *next;
		WordCache() {
			char_pos = 0;
			word_len = 0;
			pixel_width = 0;
			next = 0;
			space_count = 0;
		}
	};

	bool word_cache_dirty;
    WordCache *calculate_word_cache(const Ref<Font> &font, const String &label_text, int &line_count, int &total_char_cache, int &width) const;
	void regenerate_word_cache();

	float percent_visible;

	WordCache *word_cache;
	int total_char_cache;
	int visible_chars;
	int lines_skipped;
	int max_lines_visible;

    bool animate;
    real_t transition_duration;
    struct {
        String text;
        String xl_text;
        WordCache *word_cache;
        int width;
        int line_count;
        int total_char_cache;
    } transition_text;
    TransitionEffect transition_effect;
    TransitionBehaviour transition_behaviour;
    AnimationTransform transition_xform;
    AnimationController *transition_controller;

    void clear_pending_animations();
    CharType get_pending_char_at(int line, int pos) const;
    int get_chars_from_cache_after(WordCache *cache, int x_offset) const;
    int get_chars_from_cache_before(WordCache *cache, int x_offset) const;

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

	void set_animate(bool p_animate);
	bool is_animate() const;

	void set_transition_duration(float p_duration);
	float get_transition_duration() const;

    void set_transition_effect(TransitionEffect p_effect);
    TransitionEffect get_transition_effect() const;

    void set_transition_behaviour(TransitionBehaviour p_behaviour);
    TransitionBehaviour get_transition_behaviour() const;

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
};

VARIANT_ENUM_CAST(Label::Align);
VARIANT_ENUM_CAST(Label::VAlign);
VARIANT_ENUM_CAST(Label::TransitionEffect);
VARIANT_ENUM_CAST(Label::TransitionBehaviour);

#endif
