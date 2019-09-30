/*************************************************************************/
/*  label.cpp                                                            */
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

#include "label.h"
#include "core/print_string.h"
#include "core/project_settings.h"
#include "core/translation.h"

static void dump_xform(const CharTransform &xform) {
    print_line("CharTransform:");
    print_line(vformat(" dest{scale:"+xform.dest.scale+" offset:"+xform.dest.offset+"}"));
    print_line(vformat(" texture{clip:"+xform.tex.clip+" offset:"+xform.tex.offset+"}"));
}

template <typename T> int sgn(T val) { return (T(0) < val) - (val < T(0)); }

struct AnimationSlide : public AnimationController {

    bool active;

    AnimationSlide() : active(false) { }

    virtual void init_xform(float duration, AnimationTransform &a) {
    }
    virtual int update(float dt, AnimationTransform &xform) {
        return ANIMCTRL_DONE;
    }
    virtual bool is_active() const { return active; }
};

struct AnimationRotate : public AnimationController {

    bool active;

    AnimationRotate() : active(false) { }

    virtual void init_xform(float duration, AnimationTransform &a) {
        a = AnimationTransform();
        a.duration = duration;
        a.current = -duration; // transition from -duration .. duration
        active = duration > 0;
    }
    virtual int update(float dt, AnimationTransform &a) {
        if (!active) {
            return ANIMCTRL_DONE;
        }
        if (a.current > a.duration) {
            active = false;
            return ANIMCTRL_DONE;
        }

        float ratio = 1.0/a.duration * dt;

        a.xform.dest.scale.x += ratio * sgn(a.current); // 1..0..1
        a.xform.dest.offset.x -= 0.5 *  ratio * sgn(a.current); // 0..0,5..0

        int ret = ANIMCTRL_OK;

        if (a.current < 0 && a.current+dt >= 0)
            ret |= ANIMCTRL_UPDATE_TEXT;

        a.current += dt;

        return ret;
    }
    virtual bool is_active() const { return active; }
};


void Label::set_autowrap(bool p_autowrap) {

	autowrap = p_autowrap;
	word_cache_dirty = true;
	update();
}
bool Label::has_autowrap() const {

	return autowrap;
}

void Label::set_uppercase(bool p_uppercase) {

	uppercase = p_uppercase;
	word_cache_dirty = true;
	update();
}
bool Label::is_uppercase() const {

	return uppercase;
}

int Label::get_line_height() const {

	return get_font("font")->get_height();
}

void Label::_notification(int p_what) {

	if (p_what == NOTIFICATION_TRANSLATION_CHANGED) {

        if (animate) {
            String new_text = tr(text);
            if (new_text == xl_text)
                return; //nothing new
            xl_text = new_text;
        } else {
            String new_text = tr(transition_text.text);
            if (new_text == transition_text.xl_text)
                return; //nothing new
            transition_text.xl_text = new_text;
        }
		regenerate_word_cache();
		update();
	}

	else if (p_what == NOTIFICATION_DRAW) {

		if (clip) {
			VisualServer::get_singleton()->canvas_item_set_clip(get_canvas_item(), true);
		}

		if (word_cache_dirty)
			regenerate_word_cache();

		RID ci = get_canvas_item();

		Size2 string_size;
		Size2 size = get_size();
		Ref<StyleBox> style = get_stylebox("normal");
		Ref<Font> font = get_font("font");
		Color font_color = get_color("font_color");
		Color font_color_shadow = get_color("font_color_shadow");
		bool use_outline = get_constant("shadow_as_outline");
		Point2 shadow_ofs(get_constant("shadow_offset_x"), get_constant("shadow_offset_y"));
		int line_spacing = get_constant("line_spacing");
		Color font_outline_modulate = get_color("font_outline_modulate");

		style->draw(ci, Rect2(Point2(0, 0), get_size()));

		VisualServer::get_singleton()->canvas_item_set_distance_field_mode(get_canvas_item(), font.is_valid() && font->is_distance_field_hint());

		int font_h = font->get_height() + line_spacing + vertical_spacing;
;

		int lines_visible = (size.y + line_spacing) / font_h;

		// ceiling to ensure autowrapping does not cut text
		int space_w = Math::ceil(font->get_char_size(' ').width);
		int chars_total = 0;

		int vbegin = 0, vsep = 0;

		if (lines_visible > line_count) {
			lines_visible = line_count;
		}

		if (max_lines_visible >= 0 && lines_visible > max_lines_visible) {
			lines_visible = max_lines_visible;
		}

		if (lines_visible > 0) {

			switch (valign) {

				case VALIGN_TOP: {
					//nothing
				} break;
				case VALIGN_CENTER: {
					vbegin = (size.y - (lines_visible * font_h - line_spacing)) / 2;
					vsep = 0;

				} break;
				case VALIGN_BOTTOM: {
					vbegin = size.y - (lines_visible * font_h - line_spacing);
					vsep = 0;

				} break;
				case VALIGN_FILL: {
					vbegin = 0;
					if (lines_visible > 1) {
						vsep = (size.y - (lines_visible * font_h - line_spacing)) / (lines_visible - 1);
					} else {
						vsep = 0;
					}

				} break;
			}
		}

		WordCache *wc = word_cache;
		if (!wc) {
			return;
        }

		int line = 0;
		int line_to = lines_skipped + (lines_visible > 0 ? lines_visible : 1);
		FontDrawer drawer(font, font_outline_modulate);
		while (wc) {
			/* handle lines not meant to be drawn quickly */
			if (line >= line_to)
				break;
			if (line < lines_skipped) {

				while (wc && wc->char_pos >= 0)
					wc = wc->next;
				if (wc)
					wc = wc->next;
				line++;
				continue;
			}

			/* handle lines normally */

			if (wc->char_pos < 0) {
				//empty line
				wc = wc->next;
				line++;
				continue;
			}

			WordCache *from = wc;
			WordCache *to = wc;

			int taken = 0;
			int spaces = 0;
			while (to && to->char_pos >= 0) {

				taken += to->pixel_width;
				if (to != from && to->space_count) {
					spaces += to->space_count;
				}
				to = to->next;
			}

			bool can_fill = to && to->char_pos == WordCache::CHAR_WRAPLINE;

			float x_ofs = 0;

			switch (align) {

				case ALIGN_FILL:
				case ALIGN_LEFT: {

					x_ofs = style->get_offset().x;
				} break;
				case ALIGN_CENTER: {

					x_ofs = int(size.width - (taken + spaces * space_w)) / 2;
				} break;
				case ALIGN_RIGHT: {

					x_ofs = int(size.width - style->get_margin(MARGIN_RIGHT) - (taken + spaces * space_w));
				} break;
			}

			float y_ofs = style->get_offset().y;
			y_ofs += (line - lines_skipped) * font_h + font->get_ascent();
			y_ofs += vbegin + line * vsep;

			while (from != to) {

				// draw a word
                int line_pos = 0;
				int pos = from->char_pos;
				if (from->char_pos < 0) {

					ERR_PRINT("BUG");
					return;
				}
				if (from->space_count) {
					/* spacing */
					x_ofs += space_w * from->space_count;
					if (can_fill && align == ALIGN_FILL && spaces) {

						x_ofs += int((size.width - (taken + space_w * spaces)) / spaces);
					}
				}

                int extra_chars_before = 0, extra_chars_after = 0;
                if (animate && transition_controller->is_active()) {

                    if (from->next == to) { // last_word
                        extra_chars_after = get_chars_from_cache_after(transition_text.word_cache, x_ofs);
                    }
                    if (wc == from) { // first word
                        extra_chars_before = get_chars_from_cache_before(transition_text.word_cache, x_ofs);
                    }
                }

				if (font_color_shadow.a > 0) {

					int chars_total_shadow = chars_total; //save chars drawn
					float x_ofs_shadow = x_ofs;
					for (int i = 0; i < from->word_len; i++) {

						if (visible_chars < 0 || chars_total_shadow < visible_chars) {
							CharType c = xl_text[i + pos];
							CharType n = xl_text[i + pos + 1];
							if (uppercase) {
								c = String::char_uppercase(c);
								n = String::char_uppercase(n);
							}

							float move = drawer.draw_char(ci, Point2(x_ofs_shadow, y_ofs) + shadow_ofs, c, n, font_color_shadow);
							if (use_outline) {
								drawer.draw_char(ci, Point2(x_ofs_shadow, y_ofs) + Vector2(-shadow_ofs.x, shadow_ofs.y), c, n, font_color_shadow);
								drawer.draw_char(ci, Point2(x_ofs_shadow, y_ofs) + Vector2(shadow_ofs.x, -shadow_ofs.y), c, n, font_color_shadow);
								drawer.draw_char(ci, Point2(x_ofs_shadow, y_ofs) + Vector2(-shadow_ofs.x, -shadow_ofs.y), c, n, font_color_shadow);
							}
							x_ofs_shadow += move;
							chars_total_shadow++;
						}
					}
				}

				for (int i = -extra_chars_before; i < from->word_len + extra_chars_after; i++) {

					if (visible_chars < 0 || chars_total < visible_chars) {

						if (animate && transition_controller->is_active()) {

                            CharType c,n;
                            CharTransform xform = transition_xform.xform;

                            if (i < 0) {

                                c = get_pending_char_at(line, -i);
                                n = get_pending_char_at(line, -i+1);
                            } else if (i >= from->word_len) {

                                c = get_pending_char_at(line, from->word_len + i);
                                n = get_pending_char_at(line, from->word_len + i + 1);
                            } else {


                            }

                            if (transition_behaviour == TRANSITIONBEHAVIOUR_NEW) {
                                CharType c_ = get_pending_char_at(line, line_pos);
                                if (uppercase) {
                                    c_ = String::char_uppercase(c_);
                                }
                                if (c == c_) {
                                    xform = CharTransform(); // no transition
                                }
                            }

                            if (transition_xform.current >= 0 && !transition_text.xl_text.empty()) {
                                c = get_pending_char_at(line, line_pos);
                                n = get_pending_char_at(line, line_pos + 1);
                            }
                            if (uppercase) {
                                c = String::char_uppercase(c);
                                n = String::char_uppercase(n);
                            }
                            x_ofs += drawer.draw_char(ci, xform, Point2(x_ofs, y_ofs), c, n, font_color) + horizontal_spacing;
                        } else {
                            CharType c = xl_text[i + pos];
                            CharType n = xl_text[i + pos + 1];

                            if (uppercase) {
                                c = String::char_uppercase(c);
                                n = String::char_uppercase(n);
                            }
                            //print_line(vformat("draw char: %c (text '%s', line %d, char %d)", c, xl_text.utf8().get_data(), line, line_pos));
                            x_ofs += drawer.draw_char(ci, Point2(x_ofs, y_ofs), c, n, font_color) + horizontal_spacing;
                        }

						chars_total++; line_pos++;
					}
				}
				from = from->next;
			}

			wc = to ? to->next : 0;
			line++;
		}
	}

	else if (p_what == NOTIFICATION_THEME_CHANGED) {

		word_cache_dirty = true;
		update();
	}

	else if (p_what == NOTIFICATION_RESIZED) {

		word_cache_dirty = true;
	}

	else if (p_what == NOTIFICATION_INTERNAL_PROCESS) {

        if (animate && transition_controller->is_active()) {
            float dt = get_process_delta_time();

            int r = transition_controller->update(dt, transition_xform);
            if (r & AnimationController::ANIMCTRL_DONE) {

                transition_xform = AnimationTransform();
            }

            if (r & AnimationController::ANIMCTRL_UPDATE_TEXT) {

                if (text != transition_text.text) {
                    String old = text;
                    text = transition_text.text;
                    transition_text.text = old;
                    word_cache_dirty = true;
                }
                if (xl_text != transition_text.xl_text) {
                    String old = xl_text;
                    xl_text = transition_text.xl_text;
                    transition_text.xl_text = old;
                    word_cache_dirty = true;
                }
            }

            update();
        }
    }
}

Size2 Label::get_minimum_size() const {

	Size2 min_style = get_stylebox("normal")->get_minimum_size();

	// don't want to mutable everything
	if (word_cache_dirty)
		const_cast<Label *>(this)->regenerate_word_cache();

	if (autowrap)
		return Size2(1, clip ? 1 : minsize.height) + min_style;
	else {
		Size2 ms = minsize;
		if (clip)
			ms.width = 1;
		return ms + min_style;
	}
}

int Label::get_longest_line_width(const String &s) const {

	Ref<Font> font = get_font("font");
	int max_line_width = 0;
	int line_width = 0;

	for (int i = 0; i < xl_text.size(); i++) {

		CharType current = xl_text[i];
		if (uppercase)
			current = String::char_uppercase(current);

		if (current < 32) {

			if (current == '\n') {

				if (line_width > max_line_width)
					max_line_width = line_width;
				line_width = 0;
			}
		} else {

			// ceiling to ensure autowrapping does not cut text
			int char_width = Math::ceil(font->get_char_size(current, xl_text[i + 1]).width);
			line_width += char_width;
		}
	}

	if (line_width > max_line_width)
		max_line_width = line_width;

	return max_line_width;
}

int Label::get_line_count() const {

	if (!is_inside_tree())
		return 1;
	if (word_cache_dirty)
		const_cast<Label *>(this)->regenerate_word_cache();

	return line_count;
}

int Label::get_visible_line_count() const {

	int line_spacing = get_constant("line_spacing");
	int font_h = get_font("font")->get_height() + line_spacing;
	int lines_visible = (get_size().height - get_stylebox("normal")->get_minimum_size().height + line_spacing) / font_h;

	if (lines_visible > line_count)
		lines_visible = line_count;

	if (max_lines_visible >= 0 && lines_visible > max_lines_visible)
		lines_visible = max_lines_visible;

	return lines_visible;
}

Label::WordCache *Label::calculate_word_cache(const Ref<Font> &font, const String &label_text, int &line_count, int &total_char_cache, int &width) const {

    Ref<StyleBox> style = get_stylebox("normal");
	width = autowrap ? (get_size().width - style->get_minimum_size().width) : get_longest_line_width(label_text);

	int current_word_size = 0;
	int word_pos = 0;
	int line_width = 0;
	int space_count = 0;
	// ceiling to ensure autowrapping does not cut text
	int space_width = Math::ceil(font->get_char_size(' ').width);

    line_count = 1;
	total_char_cache = 0;

	WordCache *root = NULL, *last = NULL;

	for (int i = 0; i <= label_text.length(); i++) {

		CharType current = i < label_text.length() ? label_text[i] : L' '; //always a space at the end, so the algo works

		if (uppercase)
			current = String::char_uppercase(current);

		// ranges taken from http://www.unicodemap.org/
		// if your language is not well supported, consider helping improve
		// the unicode support in Godot.
		bool separatable = (current >= 0x2E08 && current <= 0xFAFF) || (current >= 0xFE30 && current <= 0xFE4F);
		//current>=33 && (current < 65||current >90) && (current<97||current>122) && (current<48||current>57);
		bool insert_newline = false;
		int char_width = 0;

		if (current < 33) {

			if (current_word_size > 0) {
				WordCache *wc = memnew(WordCache);
				if (root) {
					last->next = wc;
				} else {
					root = wc;
				}
				last = wc;

				wc->pixel_width = current_word_size;
				wc->char_pos = word_pos;
				wc->word_len = i - word_pos;
				wc->space_count = space_count;
				current_word_size = 0;
				space_count = 0;
			}

			if (current == '\n') {
				insert_newline = true;
			} else if (current != ' ') {
				total_char_cache++;
			}

			if (i < label_text.length() && label_text[i] == ' ') {
				if (line_width > 0 || last == NULL || last->char_pos != WordCache::CHAR_WRAPLINE) {
					space_count++;
					line_width += space_width;
				} else {
					space_count = 0;
				}
			}

		} else {
			// latin characters
			if (current_word_size == 0) {
				word_pos = i;
			}
			// ceiling to ensure autowrapping does not cut text
			char_width = Math::ceil(font->get_char_size(current, label_text[i + 1]).width);
			current_word_size += char_width;
			line_width += char_width;
			total_char_cache++;
		}

		if ((autowrap && (line_width >= width) && ((last && last->char_pos >= 0) || separatable)) || insert_newline) {
			if (separatable) {
				if (current_word_size > 0) {
					WordCache *wc = memnew(WordCache);
					if (root) {
						last->next = wc;
					} else {
						root = wc;
					}
					last = wc;

					wc->pixel_width = current_word_size - char_width;
					wc->char_pos = word_pos;
					wc->word_len = i - word_pos;
					wc->space_count = space_count;
					current_word_size = char_width;
					word_pos = i;
				}
			}

			WordCache *wc = memnew(WordCache);
			if (root) {
				last->next = wc;
			} else {
				root = wc;
			}
			last = wc;

			wc->pixel_width = 0;
			wc->char_pos = insert_newline ? WordCache::CHAR_NEWLINE : WordCache::CHAR_WRAPLINE;

			line_width = current_word_size;
			line_count++;
			space_count = 0;
		}
	}

	return root;
}

void Label::regenerate_word_cache() {

	while (word_cache) {

		WordCache *current = word_cache;
		word_cache = current->next;
		memdelete(current);
	}

	Ref<Font> font = get_font("font");

	int width;
	word_cache = calculate_word_cache(font, xl_text, line_count, total_char_cache, width);

	if (!autowrap)
		minsize.width = width;

    int line_spacing = get_constant("line_spacing");

	if (max_lines_visible > 0 && line_count > max_lines_visible) {
		minsize.height = (font->get_height() * max_lines_visible) + (line_spacing * (max_lines_visible - 1));
	} else {
		minsize.height = (font->get_height() * line_count) + (line_spacing * (line_count - 1));
	}

	if (!autowrap || !clip) {
		//helps speed up some labels that may change a lot, as no resizing is requested. Do not change.
		minimum_size_changed();
	}

	if (animate && transition_controller->is_active()) {
        transition_text.word_cache = calculate_word_cache(font,
                                                           transition_text.xl_text,
                                                           transition_text.line_count,
                                                           transition_text.total_char_cache,
                                                           transition_text.width);
    }

	word_cache_dirty = false;
}

void Label::clear_pending_animations() { // reset animation
    transition_xform = AnimationTransform();
    xl_text = transition_text.xl_text;
    text = transition_text.text;
}

CharType Label::get_pending_char_at(int line, int pos) const {
	Ref<Font> font = get_font("font");
	int max_line_width = 0;
	int line_width = 0, line_count = 0, char_pos = 0;

	for (int i = 0; i < transition_text.xl_text.size(); i++) {

		CharType current = transition_text.xl_text[i];

		if (current < 32) {

			if (current == '\n') {

				if (line_width > max_line_width)
					max_line_width = line_width;
				line_width = 0; line_count++;
			}
		} else {

            if (line_count == line && char_pos == pos)
                return  transition_text.xl_text[i];

			// ceiling to ensure autowrapping does not cut text
			int char_width = Math::ceil(font->get_char_size(current, transition_text.xl_text[i + 1]).width);
			line_width += char_width; char_pos++;
		}
	}
    return ' ';
}
int Label::get_chars_from_cache_after(WordCache *cache, int x_offset) const {

    return 0;
}
int Label::get_chars_from_cache_before(WordCache *cache, int x_offset) const {

    return 0;
}

void Label::set_align(Align p_align) {

	ERR_FAIL_INDEX((int)p_align, 4);
	align = p_align;
	update();
}

Label::Align Label::get_align() const {

	return align;
}

void Label::set_valign(VAlign p_align) {

	ERR_FAIL_INDEX((int)p_align, 4);
	valign = p_align;
	update();
}

Label::VAlign Label::get_valign() const {

	return valign;
}

void Label::set_text(const String &p_string) {

	if (text == p_string)
		return;
    if (animate) {
        transition_text.text = p_string;
        transition_text.xl_text = tr(p_string);
        transition_controller->init_xform(transition_duration, transition_xform);
    } else {
        text = p_string;
        xl_text = tr(p_string);
        word_cache_dirty = true;
        if (percent_visible < 1)
            visible_chars = get_total_character_count() * percent_visible;
    }
	update();
}

void Label::set_clip_text(bool p_clip) {

	clip = p_clip;
	update();
	minimum_size_changed();
}

bool Label::is_clipping_text() const {

	return clip;
}

String Label::get_text() const {

	return text;
}

void Label::set_visible_characters(int p_amount) {

	visible_chars = p_amount;
	if (get_total_character_count() > 0) {
		percent_visible = (float)p_amount / (float)total_char_cache;
	}
	_change_notify("percent_visible");
	update();
}

int Label::get_visible_characters() const {

	return visible_chars;
}

void Label::set_percent_visible(float p_percent) {

	if (p_percent < 0 || p_percent >= 1) {

		visible_chars = -1;
		percent_visible = 1;

	} else {

		visible_chars = get_total_character_count() * p_percent;
		percent_visible = p_percent;
	}
	_change_notify("visible_chars");
	update();
}

float Label::get_percent_visible() const {

	return percent_visible;
}

void Label::set_lines_skipped(int p_lines) {

	lines_skipped = p_lines;
	update();
}

int Label::get_lines_skipped() const {

	return lines_skipped;
}

void Label::set_max_lines_visible(int p_lines) {

	max_lines_visible = p_lines;
	update();
}

int Label::get_max_lines_visible() const {

	return max_lines_visible;
}

int Label::get_total_character_count() const {

	if (word_cache_dirty)
		const_cast<Label *>(this)->regenerate_word_cache();

	return total_char_cache;
}

void Label::set_horizontal_spacing(float p_offset) {

    if (horizontal_spacing != p_offset) {

        horizontal_spacing = p_offset;
        update();
    }
}
float Label::get_horizontal_spacing() const {

    return horizontal_spacing;
}
void Label::set_vertical_spacing(float p_offset) {

    if (vertical_spacing != p_offset) {

        vertical_spacing = p_offset;
        update();
    }
}
float Label::get_vertical_spacing() const {

    return vertical_spacing;
}

bool Label::is_animate() const {

    return animate;
}

void Label::set_animate(bool p_animate) {
	animate = p_animate;
    if (!p_animate) {
        clear_pending_animations();
    }
    set_process_internal(p_animate);
	update();
}

void Label::set_transition_duration(float p_duration) {
    if (p_duration != transition_duration) {
        transition_duration = p_duration;
        update();
    }
}

float Label::get_transition_duration() const {

    return transition_duration;
}

void Label::set_transition_effect(TransitionEffect p_effect) {

    if (p_effect != transition_effect) {
        transition_effect = p_effect;
        update();
    }
}

Label::TransitionEffect Label::get_transition_effect() const {
    return transition_effect;
}

void Label::set_transition_behaviour(TransitionBehaviour p_behaviour) {

    if (p_behaviour != transition_behaviour) {
        transition_behaviour = p_behaviour;
        update();
    }
}

Label::TransitionBehaviour Label::get_transition_behaviour() const {
    return transition_behaviour;
}

void Label::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_align", "align"), &Label::set_align);
	ClassDB::bind_method(D_METHOD("get_align"), &Label::get_align);
	ClassDB::bind_method(D_METHOD("set_valign", "valign"), &Label::set_valign);
	ClassDB::bind_method(D_METHOD("get_valign"), &Label::get_valign);
	ClassDB::bind_method(D_METHOD("set_text", "text"), &Label::set_text);
	ClassDB::bind_method(D_METHOD("get_text"), &Label::get_text);
	ClassDB::bind_method(D_METHOD("set_animate"), &Label::set_animate);
	ClassDB::bind_method(D_METHOD("is_animate"), &Label::is_animate);
	ClassDB::bind_method(D_METHOD("set_transition_duration"), &Label::set_transition_duration);
	ClassDB::bind_method(D_METHOD("get_transition_duration"), &Label::get_transition_duration);
	ClassDB::bind_method(D_METHOD("set_transition_effect"), &Label::set_transition_effect);
	ClassDB::bind_method(D_METHOD("get_transition_effect"), &Label::get_transition_effect);
	ClassDB::bind_method(D_METHOD("set_transition_behaviour"), &Label::set_transition_behaviour);
	ClassDB::bind_method(D_METHOD("get_transition_behaviour"), &Label::get_transition_behaviour);
	ClassDB::bind_method(D_METHOD("set_autowrap", "enable"), &Label::set_autowrap);
	ClassDB::bind_method(D_METHOD("has_autowrap"), &Label::has_autowrap);
	ClassDB::bind_method(D_METHOD("set_clip_text", "enable"), &Label::set_clip_text);
	ClassDB::bind_method(D_METHOD("is_clipping_text"), &Label::is_clipping_text);
	ClassDB::bind_method(D_METHOD("set_uppercase", "enable"), &Label::set_uppercase);
	ClassDB::bind_method(D_METHOD("is_uppercase"), &Label::is_uppercase);
    ClassDB::bind_method(D_METHOD("set_horizontal_spacing", "px"), &Label::set_horizontal_spacing);
	ClassDB::bind_method(D_METHOD("get_horizontal_spacing"), &Label::get_horizontal_spacing);
    ClassDB::bind_method(D_METHOD("set_vertical_spacing", "px"), &Label::set_vertical_spacing);
	ClassDB::bind_method(D_METHOD("get_vertical_spacing"), &Label::get_vertical_spacing);
	ClassDB::bind_method(D_METHOD("get_line_height"), &Label::get_line_height);
	ClassDB::bind_method(D_METHOD("get_line_count"), &Label::get_line_count);
	ClassDB::bind_method(D_METHOD("get_visible_line_count"), &Label::get_visible_line_count);
	ClassDB::bind_method(D_METHOD("get_total_character_count"), &Label::get_total_character_count);
	ClassDB::bind_method(D_METHOD("set_visible_characters", "amount"), &Label::set_visible_characters);
	ClassDB::bind_method(D_METHOD("get_visible_characters"), &Label::get_visible_characters);
	ClassDB::bind_method(D_METHOD("set_percent_visible", "percent_visible"), &Label::set_percent_visible);
	ClassDB::bind_method(D_METHOD("get_percent_visible"), &Label::get_percent_visible);
	ClassDB::bind_method(D_METHOD("set_lines_skipped", "lines_skipped"), &Label::set_lines_skipped);
	ClassDB::bind_method(D_METHOD("get_lines_skipped"), &Label::get_lines_skipped);
	ClassDB::bind_method(D_METHOD("set_max_lines_visible", "lines_visible"), &Label::set_max_lines_visible);
	ClassDB::bind_method(D_METHOD("get_max_lines_visible"), &Label::get_max_lines_visible);

	BIND_ENUM_CONSTANT(ALIGN_LEFT);
	BIND_ENUM_CONSTANT(ALIGN_CENTER);
	BIND_ENUM_CONSTANT(ALIGN_RIGHT);
	BIND_ENUM_CONSTANT(ALIGN_FILL);

	BIND_ENUM_CONSTANT(VALIGN_TOP);
	BIND_ENUM_CONSTANT(VALIGN_CENTER);
	BIND_ENUM_CONSTANT(VALIGN_BOTTOM);
	BIND_ENUM_CONSTANT(VALIGN_FILL);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_MULTILINE_TEXT, "", PROPERTY_USAGE_DEFAULT_INTL), "set_text", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "animate"), "set_animate", "is_animate");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "transition_duration"), "set_transition_duration", "get_transition_duration");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "transition_effect", PROPERTY_HINT_ENUM, "Slide,Rotate"), "set_transition_effect", "get_transition_effect");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "transition_behaviour", PROPERTY_HINT_ENUM, "All,New"), "set_transition_behaviour", "get_transition_behaviour");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "align", PROPERTY_HINT_ENUM, "Left,Center,Right,Fill"), "set_align", "get_align");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "valign", PROPERTY_HINT_ENUM, "Top,Center,Bottom,Fill"), "set_valign", "get_valign");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "offset horizontal spacing", PROPERTY_HINT_RANGE, "-100,100,0.5"), "set_horizontal_spacing", "get_horizontal_spacing");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "offset vertical spacing", PROPERTY_HINT_RANGE, "-100,100,0.5"), "set_vertical_spacing", "get_vertical_spacing");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autowrap"), "set_autowrap", "has_autowrap");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "clip_text"), "set_clip_text", "is_clipping_text");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "uppercase"), "set_uppercase", "is_uppercase");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "visible_characters", PROPERTY_HINT_RANGE, "-1,128000,1", PROPERTY_USAGE_EDITOR), "set_visible_characters", "get_visible_characters");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "percent_visible", PROPERTY_HINT_RANGE, "0,1,0.001"), "set_percent_visible", "get_percent_visible");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "lines_skipped", PROPERTY_HINT_RANGE, "0,999,1"), "set_lines_skipped", "get_lines_skipped");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_lines_visible", PROPERTY_HINT_RANGE, "-1,999,1"), "set_max_lines_visible", "get_max_lines_visible");
}

Label::Label(const String &p_text) {

	align = ALIGN_LEFT;
	valign = VALIGN_TOP;
	xl_text = "";
	word_cache = NULL;
	word_cache_dirty = true;
	autowrap = false;
    vertical_spacing = 0;
    horizontal_spacing = 0;
	line_count = 0;
	set_v_size_flags(0);
	clip = false;
    animate = false;
	transition_text.xl_text = "";
    transition_duration = 1;
    transition_controller = new AnimationRotate;
	set_mouse_filter(MOUSE_FILTER_IGNORE);
	total_char_cache = 0;
	visible_chars = -1;
	percent_visible = 1;
	lines_skipped = 0;
	max_lines_visible = -1;
	set_text(p_text);
	uppercase = false;
	set_v_size_flags(SIZE_SHRINK_CENTER);
}

Label::~Label() {

	while (word_cache) {

		WordCache *current = word_cache;
		word_cache = current->next;
		memdelete(current);
	}
}
