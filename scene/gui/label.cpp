/*************************************************************************/
/*  label.cpp                                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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
#include "label_transitions.h"

#define _RemoveCacheList(wc)    \
	while (wc) {                \
		WordList *current = wc; \
		wc = current->next;     \
		memdelete(current);     \
	};                          \
	wc = nullptr

void Label::set_autowrap(bool p_autowrap) {
	if (autowrap == p_autowrap) {
		return;
	}

	autowrap = p_autowrap;
	word_cache_dirty = true;
	update();

	if (clip) {
		minimum_size_changed();
	}
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
		if (is_transition_enabled()) {
			String new_text = tr(transition_text.text);
			if (new_text == transition_text.xl_text) {
				return; //nothing new
			}
			transition_text.xl_text = new_text;
		} else {
			String new_text = tr(text);
			if (new_text == xl_text)
				return; //nothing new
			xl_text = new_text;
		}
		regenerate_word_cache();
		update();
	}

	else if (p_what == NOTIFICATION_DRAW) {
		if (clip) {
			VisualServer::get_singleton()->canvas_item_set_clip(get_canvas_item(), true);
		}

		if (word_cache_dirty) {
			regenerate_word_cache();
		}
		if (_transition_dirty || _cache_changed) {
			if (is_transition_enabled())
				_transition_controller->init_transition(this, transition_duration, ease_func_table[transition_ease], &transition_text.word_cache, &word_cache);
			_cache_changed = _transition_dirty = false;
		}

		std::vector<WordCache *> draw_set{ &word_cache };

		if (is_transition_active()) {
			draw_set = _transition_controller->get_draw_cache();
		}

		if (draw_set.empty()) {
			// nothing to draw
			return;
		}

		RID ci = get_canvas_item();

		Size2 string_size;
		Size2 size = get_size();
		Ref<StyleBox> style = get_stylebox("normal");
		Ref<Font> font = get_font("font");
		Color font_color = get_color("font_color");
		Color font_color_shadow = get_color("font_color_shadow");
		bool use_outline = get_constant("shadow_as_outline");
		Point2 shadow_ofs(get_constant("shadow_offset_x"), get_constant("shadow_offset_y"));
		const int line_spacing = get_constant("line_spacing");
		Color font_outline_modulate = get_color("font_outline_modulate");

		style->draw(ci, Rect2(Point2(0, 0), get_size()));

		VisualServer::get_singleton()->canvas_item_set_distance_field_mode(get_canvas_item(), font.is_valid() && font->is_distance_field_hint());

		const int font_h = font->get_height() + line_spacing + vertical_spacing;
		const int lines_visible_rc = (size.y + line_spacing) / font_h;

		const real_t space_w = font->get_char_size(' ').width + horizontal_spacing;
		int chars_total = 0;

		int vbegin = 0, vsep = 0;

		FontDrawer drawer(font, font_outline_modulate);

		size_t draw_index = 0;
	draw_loop:
		const WordCache *cc = draw_set[draw_index];
		const WordList *wc = cc->words;

		int lines_visible = MAX(lines_visible_rc, cc->line_count);

		if (max_lines_visible >= 0 && lines_visible > max_lines_visible) {
			lines_visible = max_lines_visible;
		}

		int line = 0, line_to = lines_skipped + (lines_visible > 0 ? lines_visible : 1);
		float total_h = 0;

		if (!wc) {
			// nothing to draw
			goto draw_next;
		}

		total_h += lines_visible * font_h;
		total_h += style->get_margin(MARGIN_TOP) + style->get_margin(MARGIN_BOTTOM);

		if (lines_visible > 0) {
			switch (valign) {
				case VALIGN_TOP: {
					//nothing
				} break;
				case VALIGN_CENTER: {
					vbegin = (size.y - (total_h - line_spacing)) / 2;
					vsep = 0;

				} break;
				case VALIGN_BOTTOM: {
					vbegin = size.y - (total_h - line_spacing);
					vsep = 0;

				} break;
				case VALIGN_FILL: {
					vbegin = 0;
					if (lines_visible > 1) {
						vsep = (size.y - (total_h - line_spacing)) / (lines_visible - 1);
					} else {
						vsep = 0;
					}

				} break;
				default: {
					WARN_PRINT("Invalid valign enumeration.");
				}
			}
		}

		while (wc) {
			/* quickly handle lines not meant to be drawn */
			if (line >= line_to) {
				break;
			}
			if (line < lines_skipped) {
				while (wc && wc->char_pos >= 0) {
					wc = wc->next;
				}
				if (wc) {
					wc = wc->next;
				}
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

			const WordList *from = wc; // first word
			const WordList *to = wc; // last word

			int taken = 0;
			int spaces = 0;
			while (to && to->char_pos >= 0) {
				taken += to->pixel_width;
				if (to->space_count) {
					spaces += to->space_count;
				}
				to = to->next;
			}

			bool can_fill = to && (to->char_pos == WordCache::CHAR_WRAPLINE || to->char_pos == WordCache::CHAR_NEWLINE);

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
				default: {
					WARN_PRINT("Invalid align enumeration.");
				}
			}

			float y_ofs = style->get_offset().y;
			y_ofs += (line - lines_skipped) * font_h + font->get_ascent();
			y_ofs += vbegin + line * vsep;

			while (from != to) {
				// draw a word
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

				if (font_color_shadow.a > 0) {
					int chars_total_shadow = chars_total; //save chars drawn
					real_t x_ofs_shadow = x_ofs;
					for (int i = 0; i < from->word_len; i++) {
						if (visible_chars < 0 || chars_total_shadow < visible_chars) {
							CharType c = cc->cache_text[i + pos];
							CharType n = cc->cache_text[i + pos + 1];
							if (uppercase) {
								c = String::char_uppercase(c);
								n = String::char_uppercase(n);
							}

							if (const CharTransform *xform = _transition_controller->get_char_xform(cc, i + pos)) {
								const real_t move = drawer.draw_char(ci, *xform, Point2(x_ofs_shadow, y_ofs) + shadow_ofs, c, n, font_color_shadow);
								if (use_outline) {
									drawer.draw_char(ci, *xform, Point2(x_ofs_shadow, y_ofs) + Vector2(-shadow_ofs.x, shadow_ofs.y), c, n, font_color_shadow);
									drawer.draw_char(ci, *xform, Point2(x_ofs_shadow, y_ofs) + Vector2(shadow_ofs.x, -shadow_ofs.y), c, n, font_color_shadow);
									drawer.draw_char(ci, *xform, Point2(x_ofs_shadow, y_ofs) + Vector2(-shadow_ofs.x, -shadow_ofs.y), c, n, font_color_shadow);
								}
								x_ofs_shadow += move;
							} else {
								const real_t move = drawer.draw_char(ci, Point2(x_ofs_shadow, y_ofs) + shadow_ofs, c, n, font_color_shadow);
								if (use_outline) {
									drawer.draw_char(ci, Point2(x_ofs_shadow, y_ofs) + Vector2(-shadow_ofs.x, shadow_ofs.y), c, n, font_color_shadow);
									drawer.draw_char(ci, Point2(x_ofs_shadow, y_ofs) + Vector2(shadow_ofs.x, -shadow_ofs.y), c, n, font_color_shadow);
									drawer.draw_char(ci, Point2(x_ofs_shadow, y_ofs) + Vector2(-shadow_ofs.x, -shadow_ofs.y), c, n, font_color_shadow);
								}
								x_ofs_shadow += move;
							}

							chars_total_shadow++;
						}
					}
				}

				const int last_char = from->word_len - 1;
				for (int i = 0; i < from->word_len; i++) {
					if (visible_chars < 0 || chars_total < visible_chars) {
						CharType c = cc->cache_text[i + pos];
						CharType n = cc->cache_text[i + pos + 1];
						if (uppercase) {
							c = String::char_uppercase(c);
							n = String::char_uppercase(n);
						}

						if (const CharTransform *xform = _transition_controller->get_char_xform(cc, i + pos)) {
							x_ofs += drawer.draw_char(ci, *xform, Point2(x_ofs, y_ofs), c, n, font_color);
						} else {
							x_ofs += drawer.draw_char(ci, Point2(x_ofs, y_ofs), c, n, font_color);
						}
						if (i < last_char) {
							x_ofs += horizontal_spacing;
						}
						chars_total++;
					}
				}
				from = from->next;
			}

			wc = to ? to->next : nullptr;
			line++;
		}
	draw_next:
		if (++draw_index == draw_set.size())
			return;
		else
			goto draw_loop;
	}

	else if (p_what == NOTIFICATION_THEME_CHANGED) {
		word_cache_dirty = true;
		update();
	}

	else if (p_what == NOTIFICATION_RESIZED) {
		word_cache_dirty = true;
	}

	else if (p_what == NOTIFICATION_INTERNAL_PROCESS) {
		if (is_transition_active()) {
			const real_t dt = get_process_delta_time();

			if (_transition_controller->update(dt, ease_func_table[transition_ease])) {
				if (text != transition_text.text || xl_text != transition_text.xl_text) {
					_clear_pending_animations();

					if (!autowrap || !clip) {
						//helps speed up some labels that may change a lot, as no resizing is requested. Do not change.
						minimum_size_changed();
					}

					_change_notify();
				}
			}

			update();
		}
	}
}

Size2 Label::get_minimum_size() const {
	Size2 min_style = get_stylebox("normal")->get_minimum_size();

	// don't want to mutable everything
	if (word_cache_dirty) {
		const_cast<Label *>(this)->regenerate_word_cache();
	}

	if (autowrap) {
		return Size2(1, clip ? 1 : word_cache.minsize.height) + min_style;
	} else {
		Size2 ms = word_cache.minsize;
		if (clip) {
			ms.width = 1;
		}
		return ms + min_style;
	}
}

int Label::get_longest_line_width(const String &s) const {
	Ref<Font> font = get_font("font");
	real_t max_line_width = 0;
	real_t line_width = 0;

	for (int i = 0; i < s.size(); i++) {
		CharType current = s[i];
		if (uppercase) {
			current = String::char_uppercase(current);
		}

		if (current < 32) {
			if (line_width) {
				line_width -= horizontal_spacing; // remove last spacing
			}
			if (current == '\n') {
				if (line_width > max_line_width) {
					max_line_width = line_width;
				}
				line_width = 0;
			}
		} else {
			real_t char_width = font->get_char_size(current, s[i + 1]).width + horizontal_spacing;
			line_width += char_width;
		}
	}

	if (line_width > max_line_width) {
		max_line_width = line_width;
	}

	// ceiling to ensure autowrapping does not cut text
	return Math::ceil(max_line_width);
}

int Label::get_line_count() const {
	if (!is_inside_tree()) {
		return 1;
	}
	if (word_cache_dirty) {
		const_cast<Label *>(this)->regenerate_word_cache();
	}

	return word_cache.line_count;
}

int Label::get_visible_line_count() const {
	int line_spacing = get_constant("line_spacing");
	int font_h = get_font("font")->get_height() + line_spacing;
	int lines_visible = (get_size().height - get_stylebox("normal")->get_minimum_size().height + line_spacing) / font_h;

	if (lines_visible > word_cache.line_count) {
		lines_visible = word_cache.line_count;
	}

	if (max_lines_visible >= 0 && lines_visible > max_lines_visible) {
		lines_visible = max_lines_visible;
	}

	return lines_visible;
}

void Label::_dump_word_cache(const WordCache &cache) const {
	print_line("WordCache dump:");
	print_line("cached text: " + cache.cache_text);
	WordList *wc = cache.words;
	while (wc) {
		print_line(vformat("  '" + cache.cache_text.substr(wc->char_pos, wc->word_len) + "' char_pos=%d,line=%d,line_pos=%d,len=%d,spc=%d", wc->char_pos, wc->line, wc->line_pos, wc->word_len, wc->space_count));
		wc = wc->next;
	}
	print_line(vformat("total chars in cache: %d", cache.total_char_cache));
	print_line(vformat("lines count: %d", cache.line_count));
	print_line("---------------------");
}

// Notice: space_count is the number of spaces before the word
// Notice: spaces before end of the line/return are skipped
Label::WordCache Label::calculate_word_cache(const Ref<Font> &font, const String &label_text) const {
	WordCache cache;
	cache.line_count = 1;
	cache.total_char_cache = 0;
	cache.cache_text = label_text;

	if (autowrap) {
		Ref<StyleBox> style = get_stylebox("normal");
		cache.width = MAX(get_size().width, get_custom_minimum_size().width) - style->get_minimum_size().width;
	} else {
		cache.width = get_longest_line_width(label_text);
	}

	real_t current_word_size = 0;
	int word_pos = 0;
	int line_pos = 0;
	real_t line_width = 0;
	int space_count = 0;
	real_t space_width = font->get_char_size(' ').width + horizontal_spacing;
	bool was_separatable = false;

	WordList *root = nullptr, *last = nullptr;

	for (int i = 0; i <= label_text.length(); i++) {
		CharType current = i < label_text.length() ? label_text[i] : L' '; //always a space at the end, so the algo. works

		if (uppercase) {
			current = String::char_uppercase(current);
		}

		// ranges taken from https://en.wikipedia.org/wiki/Plane_(Unicode)
		// if your language is not well supported, consider helping improve
		// the unicode support in Godot.
		bool separatable = (current >= 0x2E08 && current <= 0x9FFF) || // CJK scripts and symbols.
				(current >= 0xAC00 && current <= 0xD7FF) || // Hangul Syllables and Hangul Jamo Extended-B.
				(current >= 0xF900 && current <= 0xFAFF) || // CJK Compatibility Ideographs.
				(current >= 0xFE30 && current <= 0xFE4F) || // CJK Compatibility Forms.
				(current >= 0xFF65 && current <= 0xFF9F) || // Halfwidth forms of katakana
				(current >= 0xFFA0 && current <= 0xFFDC) || // Halfwidth forms of compatibility jamo characters for Hangul
				(current >= 0x20000 && current <= 0x2FA1F) || // CJK Unified Ideographs Extension B ~ F and CJK Compatibility Ideographs Supplement.
				(current >= 0x30000 && current <= 0x3134F); // CJK Unified Ideographs Extension G.
		bool insert_newline = false;
		real_t char_width = 0;

		bool separation_changed = i > 0 && was_separatable != separatable;
		was_separatable = separatable;

		if (current < 33) { // Control characters and space.
			if (current_word_size > 0) { // These characters always create a word-break.
				WordList *wc = memnew(WordList);
				if (root) {
					last->next = wc;
				} else {
					root = wc;
				}
				last = wc;

				wc->pixel_width = current_word_size - horizontal_spacing; // remove last horizontal_spacing
				wc->char_pos = word_pos;
				wc->word_len = i - word_pos;
				wc->line = cache.line_count - 1;
				wc->line_pos = line_pos - wc->word_len;
				wc->space_count = space_count;
				current_word_size = 0;
				space_count = 0;
			} else if ((i == xl_text.length() || current == '\n') && last != nullptr && space_count != 0) {
				// In case there are trailing white spaces we add a placeholder word cache with just the spaces.
				WordList *wc = memnew(WordList);
				if (root) {
					last->next = wc;
				} else {
					root = wc;
				}
				last = wc;

				wc->pixel_width = 0;
				wc->char_pos = 0;
				wc->word_len = 0;
				wc->space_count = space_count;
				current_word_size = 0;
				space_count = 0;
			}

			if (current == '\n') {
				insert_newline = true;
			} else if (current != ' ') {
				cache.total_char_cache++;
			}

			if (i < xl_text.length() && xl_text[i] == ' ') {
				if (line_width == 0) {
					if (current_word_size == 0) {
						word_pos = i;
					}
					current_word_size += space_width;
					line_width += space_width;
				} else if (line_width > 0 || last == nullptr || last->char_pos != WordList::CHAR_WRAPLINE) {
					space_count++;
					line_width += space_width;
				} else {
					space_count = 0;
				}
			}

		} else { // Characters with graphical representation.
			// Word-break on CJK & non-CJK edge.
			if (separation_changed && current_word_size > 0) {
				WordList *wc = memnew(WordList);
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
			if (current_word_size == 0) {
				word_pos = i;
			}
			char_width = font->get_char_size(current, label_text[i + 1]).width + horizontal_spacing;
			current_word_size += char_width;
			line_width += char_width;
			line_pos++;
			cache.total_char_cache++;

			// allow autowrap to cut words when they exceed line width
			if (autowrap && (current_word_size > word_cache.width)) {
				separatable = true;
			}
		}

		if ((autowrap && (line_width - horizontal_spacing >= word_cache.width) && ((last && last->char_pos >= 0) || separatable)) || insert_newline) {
			if (separatable) {
				if (current_word_size > 0) {
					WordList *wc = memnew(WordList);
					if (root) {
						last->next = wc;
					} else {
						root = wc;
					}
					last = wc;

					wc->pixel_width = current_word_size - char_width - horizontal_spacing;
					wc->char_pos = word_pos;
					wc->word_len = i - word_pos;
					wc->line = cache.line_count - 1;
					wc->line_pos = line_pos - wc->word_len;
					wc->space_count = space_count;
					current_word_size = char_width;
					word_pos = i;
				}
			}

			WordList *wc = memnew(WordList);
			if (root) {
				last->next = wc;
			} else {
				root = wc;
			}
			last = wc;

			wc->pixel_width = 0;
			wc->char_pos = insert_newline ? WordList::CHAR_NEWLINE : WordList::CHAR_WRAPLINE;
			wc->line = cache.line_count - 1;

			cache.line_count++;

			line_width = current_word_size;
			line_pos = 0;
			space_count = 0;
		}
	}
	cache.words = root;

	return cache;
}

int Label::get_line_size(const WordCache &cache, int line) const {
	int line_size = 0;
	WordList *wc = cache.words;
	while (wc && wc->line != line)
		wc = wc->next;
	while (wc && wc->char_pos >= 0) {
		line_size += wc->space_count + wc->word_len; // including spaces
		wc = wc->next;
	}
	return line_size;
}

void Label::regenerate_word_cache() {
	Ref<Font> font = get_font("font");

	_RemoveCacheList(word_cache.words);
	word_cache = calculate_word_cache(font, xl_text);

	if (!autowrap) {
		word_cache.minsize.width = word_cache.width;
	}
	const int line_spacing = get_constant("line_spacing");

	if (max_lines_visible > 0 && word_cache.line_count > max_lines_visible) {
		word_cache.minsize.height = (font->get_height() * max_lines_visible) + (line_spacing * (max_lines_visible - 1));
	} else {
		word_cache.minsize.height = (font->get_height() * word_cache.line_count) + (line_spacing * (word_cache.line_count - 1));
	}

	_RemoveCacheList(transition_text.word_cache.words);
	transition_text.word_cache = calculate_word_cache(font, transition_text.xl_text);

	if (!autowrap || !clip) {
		//helps speed up some labels that may change a lot, as no resizing is requested. Do not change.
		minimum_size_changed();
	}

	word_cache_dirty = false;
	_cache_changed = true;
}

void Label::_clear_pending_animations() { // reset animation
	text = transition_text.text;
	transition_text.text = "";
	xl_text = transition_text.xl_text;
	transition_text.xl_text = "";

	// swap caches:
	_RemoveCacheList(word_cache.words);
	word_cache = transition_text.word_cache;
	transition_text.word_cache = WordCache();
}

void Label::set_align(Align p_align) {
	ERR_FAIL_INDEX(p_align, AlignCount);
	align = p_align;
	update();
}

Label::Align Label::get_align() const {
	return align;
}

void Label::set_valign(VAlign p_align) {
	ERR_FAIL_INDEX(p_align, VAlignCount);
	valign = p_align;
	update();
}

Label::VAlign Label::get_valign() const {
	return valign;
}

void Label::set_text(const String &p_string) {
	if (text == p_string) {
		return;
	}
	if (is_transition_enabled()) {
		if (is_transition_active())
			_clear_pending_animations(); // finish now current animation
		transition_text.text = p_string;
		transition_text.xl_text = tr(p_string);
	} else {
		text = p_string;
		xl_text = tr(p_string);
	}
	if (percent_visible < 1) {
		visible_chars = get_total_character_count() * percent_visible;
	}
	word_cache_dirty = true;
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
		percent_visible = (float)p_amount / (float)word_cache.total_char_cache;
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
	ERR_FAIL_COND(p_lines < 0);
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
	if (word_cache_dirty) {
		const_cast<Label *>(this)->regenerate_word_cache();
	}

	return word_cache.total_char_cache;
}

void Label::set_horizontal_spacing(float p_offset) {
	if (horizontal_spacing != p_offset) {
		horizontal_spacing = p_offset;
		word_cache_dirty = true;
		update();
	}
}
float Label::get_horizontal_spacing() const {
	return horizontal_spacing;
}
void Label::set_vertical_spacing(float p_offset) {
	if (vertical_spacing != p_offset) {
		vertical_spacing = p_offset;
		word_cache_dirty = true;
		update();
	}
}
float Label::get_vertical_spacing() const {
	return vertical_spacing;
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
	ERR_FAIL_INDEX(p_effect, TRANSITIONEFFECT_COUNT);

	if (p_effect != transition_effect) {
		transition_effect = p_effect;
		_transition_controller = AnimationControllerFactory(p_effect);
		if (p_effect == TRANSITIONEFFECT_NONE) {
			set_process_internal(false);
			_clear_pending_animations();
		} else {
			set_process_internal(true);
			_transition_dirty = true;
		}
		update();
	}
}

Label::TransitionEffect Label::get_transition_effect() const {
	return transition_effect;
}

void Label::set_transition_ease(TransitionEase p_ease) {
	if (p_ease != transition_ease) {
		transition_ease = p_ease;
		update();
	}
}

Label::TransitionEase Label::get_transition_ease() const {
	return transition_ease;
}

bool Label::is_transition_active() const {
	return !_transition_controller->is_done();
}

bool Label::is_transition_enabled() const {
	return transition_effect != TRANSITIONEFFECT_NONE;
}

void Label::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_align", "align"), &Label::set_align);
	ClassDB::bind_method(D_METHOD("get_align"), &Label::get_align);
	ClassDB::bind_method(D_METHOD("set_valign", "valign"), &Label::set_valign);
	ClassDB::bind_method(D_METHOD("get_valign"), &Label::get_valign);
	ClassDB::bind_method(D_METHOD("set_text", "text"), &Label::set_text);
	ClassDB::bind_method(D_METHOD("get_text"), &Label::get_text);
	ClassDB::bind_method(D_METHOD("set_transition_duration"), &Label::set_transition_duration);
	ClassDB::bind_method(D_METHOD("get_transition_duration"), &Label::get_transition_duration);
	ClassDB::bind_method(D_METHOD("set_transition_ease"), &Label::set_transition_ease);
	ClassDB::bind_method(D_METHOD("get_transition_ease"), &Label::get_transition_ease);
	ClassDB::bind_method(D_METHOD("set_transition_effect"), &Label::set_transition_effect);
	ClassDB::bind_method(D_METHOD("get_transition_effect"), &Label::get_transition_effect);
	ClassDB::bind_method(D_METHOD("is_transition_active"), &Label::is_transition_active);
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

	BIND_ENUM_CONSTANT(TRANSITIONEASE_NONE);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_LINEAR_IN);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_LINEAR_OUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_LINEAR_INOUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_SINE_IN);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_SINE_OUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_SINE_INOUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_CIRC_IN);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_CIRC_OUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_CIRC_INOUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_CUBIC_IN);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_CUBIC_OUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_CUBIC_INOUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_QUAD_IN);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_QUAD_OUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_QUAD_INOUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_EXPO_IN);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_EXPO_OUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_EXPO_INOUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_BACK_IN);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_BACK_OUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_BACK_INOUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_BOUNCE_IN);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_BOUNCE_OUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_BOUNCE_INOUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_ELASTIC_IN);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_ELASTIC_OUT);
	BIND_ENUM_CONSTANT(TRANSITIONEASE_ELASTIC_INOUT);

	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_NONE);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_SLIDE_UP);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_SLIDE_DOWN);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_SLIDE_UP_NEW);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_SLIDE_DOWN_NEW);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_WHEEL_UP);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_WHEEL_DOWN);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_WHEEL_UP_NEW);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_WHEEL_DOWN_NEW);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_REVEAL_UP);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_REVEAL_DOWN);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_REVEAL_UP_NEW);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_REVEAL_DOWN_NEW);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_ROTATE_V);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_ROTATE_H);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_ROTATE_V_SEQ);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_ROTATE_H_SEQ);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_SLIDE_UP_SEQ);
	BIND_ENUM_CONSTANT(TRANSITIONEFFECT_SLIDE_DOWN_SEQ);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_MULTILINE_TEXT, "", PROPERTY_USAGE_DEFAULT_INTL), "set_text", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "align", PROPERTY_HINT_ENUM, "Left,Center,Right,Fill"), "set_align", "get_align");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "valign", PROPERTY_HINT_ENUM, "Top,Center,Bottom,Fill"), "set_valign", "get_valign");
	ADD_GROUP("Transition", "transition_");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "transition_duration"), "set_transition_duration", "get_transition_duration");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "transition_ease", PROPERTY_HINT_ENUM, EASE_FUNC), "set_transition_ease", "get_transition_ease");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "transition_effect", PROPERTY_HINT_ENUM, "None,SlideUp,SlideDown,SlideUpNew,SlideDownNew,WheelUp,WheelDown,WheelUpNew,WheelDownNew,RevelUp,RevelDown,RevelUpNew,RevelDownNew,RotateV,RotateH,RotateVSeq,RotateHSeq,SlideUpSeq,SlideDownSeq"), "set_transition_effect", "get_transition_effect");
	ADD_GROUP("", "");
	ADD_GROUP("Extra Spacing", "extra_spacing_");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "extra_spacing_horizontal", PROPERTY_HINT_RANGE, "-10,10,0.5"), "set_horizontal_spacing", "get_horizontal_spacing");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "extra_spacing_vertical", PROPERTY_HINT_RANGE, "-10,10,0.5"), "set_vertical_spacing", "get_vertical_spacing");
	ADD_GROUP("", "");
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
	word_cache_dirty = true;
	autowrap = false;
	vertical_spacing = 0;
	horizontal_spacing = 0;
	set_v_size_flags(0);
	clip = false;
	transition_text.xl_text = "";
	transition_duration = 1.0;
	transition_ease = TRANSITIONEASE_NONE;
	transition_effect = TRANSITIONEFFECT_NONE;
	_transition_controller = AnimationControllerFactory(TRANSITIONEFFECT_NONE);
	_transition_dirty = false;
	_cache_changed = false;
	set_mouse_filter(MOUSE_FILTER_IGNORE);
	visible_chars = -1;
	percent_visible = 1;
	lines_skipped = 0;
	max_lines_visible = -1;
	set_text(p_text);
	uppercase = false;
	set_v_size_flags(SIZE_SHRINK_CENTER);
}

Label::~Label() {
	_RemoveCacheList(word_cache.words);
	_RemoveCacheList(transition_text.word_cache.words);
}
