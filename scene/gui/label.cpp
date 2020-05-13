/*************************************************************************/
/*  label.cpp                                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

template <typename T> int sgn(T val) { return (T(0) < val) - (val < T(0)); }

#ifndef PI
# define PI 3.14159265358979323846f
#endif

// How to use:
// -----------
//  t = current time (in any unit measure, but same unit as duration)
//  b = starting value to interpolate
//  c = the total change in value of b that needs to occur
//  d = total time it should take to complete (duration)

// Linear Easing functions
static float ease_linear_none(float t, float b, float c, float d) { return (c*t/d + b); }
static float ease_linear_in(float t, float b, float c, float d) { return (c*t/d + b); }
static float ease_linear_out(float t, float b, float c, float d) { return (c*t/d + b); }
static float ease_linear_inout(float t,float b, float c, float d) { return (c*t/d + b); }
// Sine Easing functions
static float ease_sine_in(float t, float b, float c, float d) { return (-c*cosf(t/d*(PI/2.0f)) + c + b); }
static float ease_sine_out(float t, float b, float c, float d) { return (c*sinf(t/d*(PI/2.0f)) + b); }
static float ease_sine_inout(float t, float b, float c, float d) { return (-c/2.0f*(cosf(PI*t/d) - 1.0f) + b); }
// Circular Easing functions
static float ease_circ_in(float t, float b, float c, float d) { t /= d; return (-c*(sqrt(1.0f - t*t) - 1.0f) + b); }
static float ease_circ_out(float t, float b, float c, float d) { t = t/d - 1.0f; return (c*sqrt(1.0f - t*t) + b); }
static float ease_circ_inout(float t, float b, float c, float d) {
    if ((t/=d/2.0f) < 1.0f) return (-c/2.0f*(sqrt(1.0f - t*t) - 1.0f) + b);
    t -= 2.0f; return (c/2.0f*(sqrt(1.0f - t*t) + 1.0f) + b);
}
// Cubic Easing functions
static float ease_cubic_in(float t, float b, float c, float d) { t /= d; return (c*t*t*t + b); }
static float ease_cubic_out(float t, float b, float c, float d) { t = t/d - 1.0f; return (c*(t*t*t + 1.0f) + b); }
static float ease_cubic_inout(float t, float b, float c, float d) {
    if ((t/=d/2.0f) < 1.0f) return (c/2.0f*t*t*t + b);
        t -= 2.0f; return (c/2.0f*(t*t*t + 2.0f) + b);
}
// Quadratic Easing functions
static float ease_quad_in(float t, float b, float c, float d) { t /= d; return (c*t*t + b); }
static float ease_quad_out(float t, float b, float c, float d) { t /= d; return (-c*t*(t - 2.0f) + b); }
static float ease_quad_inout(float t, float b, float c, float d) {
    if ((t/=d/2) < 1) return (((c/2)*(t*t)) + b);
    return (-c/2.0f*(((t - 1.0f)*(t - 3.0f)) - 1.0f) + b);
}
// Exponential Easing functions
static float ease_expo_in(float t, float b, float c, float d) { return (t == 0.0f) ? b : (c*powf(2.0f, 10.0f*(t/d - 1.0f)) + b); }
static float ease_expo_out(float t, float b, float c, float d) { return (t == d) ? (b + c) : (c*(-powf(2.0f, -10.0f*t/d) + 1.0f) + b);    }
static float ease_expo_inout(float t, float b, float c, float d) {
    if (t == 0.0f) return b;
    if (t == d) return (b + c);
    if ((t/=d/2.0f) < 1.0f) return (c/2.0f*powf(2.0f, 10.0f*(t - 1.0f)) + b);
    return (c/2.0f*(-powf(2.0f, -10.0f*(t - 1.0f)) + 2.0f) + b);
}
// Back Easing functions
static float ease_back_in(float t, float b, float c, float d) {
    const float s = 1.70158f;
    const float postFix = t/=d;
    return (c*(postFix)*t*((s + 1.0f)*t - s) + b);
}
static float ease_back_out(float t, float b, float c, float d) {
    const float s = 1.70158f;
    t = t/d - 1.0f;
    return (c*(t*t*((s + 1.0f)*t + s) + 1.0f) + b);
}
static float ease_back_inout(float t, float b, float c, float d) {
    float s = 1.70158f;
    if ((t/=d/2.0f) < 1.0f) {
        s *= 1.525f;
        return (c/2.0f*(t*t*((s + 1.0f)*t - s)) + b);
    }
    const float postFix = t-=2.0f;
    s *= 1.525f;
    return (c/2.0f*((postFix)*t*((s + 1.0f)*t + s) + 2.0f) + b);
}
// Bounce Easing functions
static float ease_bounce_out(float t, float b, float c, float d) {
    if ((t/=d) < (1.0f/2.75f)) {
        return (c*(7.5625f*t*t) + b);
    } else if (t < (2.0f/2.75f)) {
        const float postFix = t-=(1.5f/2.75f);
        return (c*(7.5625f*(postFix)*t + 0.75f) + b);
    } else if (t < (2.5/2.75)) {
        const float postFix = t-=(2.25f/2.75f);
        return (c*(7.5625f*(postFix)*t + 0.9375f) + b);
    } else {
        const float postFix = t-=(2.625f/2.75f);
        return (c*(7.5625f*(postFix)*t + 0.984375f) + b);
    }
}
static float ease_bounce_in(float t, float b, float c, float d) { return (c - ease_bounce_out(d - t, 0.0f, c, d) + b); }
static float ease_bounce_inout(float t, float b, float c, float d) {
    if (t < d/2.0f) return (ease_bounce_in(t*2.0f, 0.0f, c, d)*0.5f + b);
    else return (ease_bounce_out(t*2.0f - d, 0.0f, c, d)*0.5f + c*0.5f + b);
}
// Elastic Easing functions
static float ease_elastic_in(float t, float b, float c, float d) {
    if (t == 0.0f) return b;
    if ((t/=d) == 1.0f) return (b + c);
    const float p = d*0.3f;
    const float a = c;
    const float s = p/4.0f;
    const float postFix = a*powf(2.0f, 10.0f*(t-=1.0f));
    return (-(postFix*sinf((t*d-s)*(2.0f*PI)/p )) + b);
}
static float ease_elastic_out(float t, float b, float c, float d) {
    if (t == 0.0f) return b;
    if ((t/=d) == 1.0f) return (b + c);
    const float p = d*0.3f;
    const float a = c;
    const float s = p/4.0f;
    return (a*powf(2.0f,-10.0f*t)*sinf((t*d-s)*(2.0f*PI)/p) + c + b);
}
static float ease_elastic_inout(float t, float b, float c, float d) {
    if (t == 0.0f) return b;
    if ((t/=d/2.0f) == 2.0f) return (b + c);
    const float p = d*(0.3f*1.5f);
    const float a = c;
    const float s = p/4.0f;
    if (t < 1.0f) {
        const float postFix = a*powf(2.0f, 10.0f*(t-=1.0f));
        return -0.5f*(postFix*sinf((t*d-s)*(2.0f*PI)/p)) + b;
    }
    const float postFix = a*powf(2.0f, -10.0f*(t-=1.0f));
    return (postFix*sinf((t*d-s)*(2.0f*PI)/p)*0.5f + c + b);
}
// End easing functions.

#define EASE_FUNC      \
    "EaseLinearNone,"  \
    "EaseLinearIn,"    \
    "EaseLinearOut,"   \
    "EaseLinearInOut," \
    "EaseSineIn,"      \
    "EaseSineOut,"     \
    "EaseSineInOut,"   \
    "EaseCircIn,"      \
    "EaseCircOut,"     \
    "EaseCircInOut,"   \
    "EaseCubicIn,"     \
    "EaseCubicOut,"    \
    "EaseCubicInOut,"  \
    "EaseQuadIn,"      \
    "EaseQuadOut,"     \
    "EaseQuadInOut,"   \
    "EaseExpoIn,"      \
    "EaseExpoOut,"     \
    "EaseExpoInOut,"   \
    "EaseBackIn,"      \
    "EaseBackOut,"     \
    "EaseBackInOut,"   \
    "EaseBounceIn,"    \
    "EaseBounceOut,"   \
    "EaseBounceInOut," \
    "EaseElasticIn,"   \
    "EaseElasticOut,"  \
    "EaseElasticInOut"

#define DefineEaseFunc(F) \
    ease_ ## F ## _in,    \
    ease_ ## F ## _out,   \
    ease_ ## F ## _inout  \

static ease_func_t ease_func_table[] = {
    ease_linear_none,
    DefineEaseFunc(linear),
    DefineEaseFunc(sine),
    DefineEaseFunc(circ),
    DefineEaseFunc(cubic),
    DefineEaseFunc(quad),
    DefineEaseFunc(expo),
    DefineEaseFunc(back),
    DefineEaseFunc(bounce),
    DefineEaseFunc(elastic)
};
#undef DefineEaseFunc

void AnimationTransform::_dump_xform() const {
    print_line("CharTransform:");
    print_line(vformat(" dest{scale:%f offset:%f}", xform.dest.scale, xform.dest.offset));
    print_line(vformat(" texture{clip:%f offset:%f}", xform.tex.clip, xform.tex.offset));
}

struct AnimationNone : public AnimationController {

    AnimationNone() { }

    virtual void init_xform(float duration, AnimationTransform &a) { }
    virtual AnimationController::AnimState update(float dt, ease_func_t ease, AnimationTransform &xform) { return ANIMCTRL_DONE; }
    virtual bool is_active() const { return false; }
    virtual AnimationController::AnimState state(const AnimationTransform &a) const { return ANIMCTRL_DONE; }
};

struct AnimationSlide : public AnimationController {

    enum AnimationSlideOrient {
        ANIMATION_SLIDE_UP,
        ANIMATION_SLIDE_DOWN
    };
    AnimationSlideOrient orientation = ANIMATION_SLIDE_UP;

    AnimationSlide(AnimationSlideOrient orientation) : orientation(orientation) { }

    virtual void init_xform(float duration, AnimationTransform &a) {
    }
    virtual  AnimationController::AnimState update(float dt, ease_func_t ease, AnimationTransform &xform) {
        return ANIMCTRL_DONE;
    }
    virtual AnimationController::AnimState state(const AnimationTransform &a) const { return a.active ? (a.current >= 0 ? ANIMCTRL_OUT : ANIMCTRL_IN) : ANIMCTRL_DONE; }
};

struct AnimationRotate : public AnimationController {

    enum AnimationRotateOrient {
        ANIMATION_ROTATE_H,
        ANIMATION_ROTATE_V
    };
    AnimationRotateOrient orientation = ANIMATION_ROTATE_H;

    AnimationRotate(AnimationRotateOrient orientation) : orientation(orientation) { }

    virtual void init_xform(float duration, AnimationTransform &a) {
        a = AnimationTransform();
        a.duration = duration;
        a.current = -duration; // transition from  -duration .. 0 .. duration
        a.active = duration > 0;
    }
    virtual AnimationController::AnimState update(float dt, ease_func_t ease, AnimationTransform &a) {
        if (!a.active) {
            return ANIMCTRL_DONE;
        }
        if (a.current > a.duration) {
            a.active = false;
            return ANIMCTRL_DONE;
        }

        if (orientation == ANIMATION_ROTATE_H) {
            a.xform.dest.scale.x = ease(a.current, 0, 1, a.current<0?-a.duration:a.duration); // 1..0..1
            a.xform.dest.offset.x = 0.5*(1-a.xform.dest.scale.x); // 0..0,5..0
        } else if (orientation == ANIMATION_ROTATE_V) {
            a.xform.dest.scale.y = ease(a.current, 0, 1, a.current<0?-a.duration:a.duration); // 1..0..1
            a.xform.dest.offset.y = 0.5*(1-a.xform.dest.scale.y); // 0..0,5..0
        } else {
			ERR_PRINT("Unknown orientation type - no transform peformed");
        }

        a.current += dt;
        if (a.current >= 0)
            return ANIMCTRL_OUT;
        else
            return ANIMCTRL_IN;
    }
    virtual AnimationController::AnimState state(const AnimationTransform &a) const { return a.active ? (a.current >= 0 ? ANIMCTRL_OUT : ANIMCTRL_IN) : ANIMCTRL_DONE; }
};

static AnimationController *transition_controllers_table[] = {
    new AnimationNone(),
    new AnimationSlide(AnimationSlide::ANIMATION_SLIDE_UP),
    new AnimationSlide(AnimationSlide::ANIMATION_SLIDE_DOWN),
    new AnimationRotate(AnimationRotate::ANIMATION_ROTATE_V),
    new AnimationRotate(AnimationRotate::ANIMATION_ROTATE_H)
};


#define _RemoveWordCache(wc)      \
	while (wc) {                  \
		WordCache *current = wc;  \
		wc = current->next;       \
		memdelete(current);       \
	}

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

Label::CharPair Label::_process_transition_char(CharTransform &xform, bool draw_state, int line, int line_pos, int char_pos, real_t &x_ofs) {

    String draw_text_state[] = { xl_text, transition_text.xl_text };
# define draw_text draw_text_state[draw_state]
# define other_draw_text draw_text_state[!draw_state]
    WordCache *draw_cache_state[] = { word_cache, transition_text.word_cache };
# define draw_cache draw_cache_state[draw_state]
# define other_draw_cache draw_cache_state[!draw_state]

    CharPair cn;
    int word = -1;
    cn.first = get_char_at(draw_cache, draw_text, line, line_pos, &word, &cn.second);

    if (uppercase) {
        cn.first = String::char_uppercase(cn.first);
        cn.second = String::char_uppercase(cn.second);
    }

    // reset glyph transformation if necessery
    if (transition_change_policy == TRANSITIONCHANGEPOLICY_NEW) {
        int other_word = -1;
        CharType c_ = get_char_at(other_draw_cache, other_draw_text, line, line_pos, &other_word);
        if (c_ != 0 && word == other_word) {
            if (uppercase) {
                c_ = String::char_uppercase(c_);
            }
            if (cn.first == c_) {
                if (draw_state) {
                    // correct position in OUT state for this char/pair
                    if (transition_text.same_chars_pos.has(char_pos)) {
                        x_ofs +=
                            (int(transition_text.same_chars_pos[char_pos])-x_ofs)
                            * (1-xform.dest.scale.x*xform.dest.scale.y);
                    }
                }
                xform = CharTransform(); // no transition
            }
        }
    }

    xform.scale_width = transition_opts.scale_width;
    xform.align_vrotation = transition_opts.align_vrotation;
    xform.align_vrotation_factor = transition_opts.align_vrotation_factor;

    return cn;
}

void Label::_notification(int p_what) {

	if (p_what == NOTIFICATION_TRANSLATION_CHANGED) {

        if (is_transition_enabled()) {
            String new_text = tr(transition_text.text);
            if (new_text == transition_text.xl_text)
                return; //nothing new
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

		int font_h = font->get_height() + line_spacing + vertical_spacing;
		int lines_visible = (size.y + line_spacing) / font_h;

		const real_t space_w = font->get_char_size(' ').width + horizontal_spacing;
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
            WARN_PRINT("Invalid word cache");
			return;
        }

		int line = 0, line_to = lines_skipped + (lines_visible > 0 ? lines_visible : 1);
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

			WordCache *from = wc; // first word
			WordCache *to = wc;   // last word

			int taken = 0, spaces = 0, line_chars = 0;
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
				int pos = from->char_pos;
				if (from->char_pos < 0) {

					ERR_PRINT("BUG");
					return;
				}

				const bool transition_draw_state = _current_transition_state();
                const bool transition_in_progress = is_transition_enabled() && transition_xform.is_active();

				if (from->space_count) {
					/* spacing */
                    if (!transition_in_progress || !transition_draw_state)
                        x_ofs += space_w * from->space_count;
					if (can_fill && align == ALIGN_FILL && spaces) {
						x_ofs += int((size.width - (taken + space_w * spaces)) / spaces);
					}
				}

				// calculate transition differences for different variants
				// and extra characters before and after the last/first word (line):
				// before: |  ABCD  | |ABCD   | |    ABCD|
				// after:  | ABCDEF | |AB     | |   ABCDE|
                // (this is necessery for OUT state to handle additional characters)
                int extra_chars_before = 0, extra_chars_after = 0;
                if (transition_in_progress && transition_draw_state) {

                    int extra_chars = 0;
                    if (from->next == to || wc == from) { // last/first word ?
                        extra_chars =
                            get_line_size(transition_text.word_cache, transition_text.xl_text, line)
                            - get_line_size(word_cache, xl_text, line);
                    }
                    if (extra_chars > 0) switch (align) {

                        case ALIGN_FILL:
                        case ALIGN_LEFT: {

                            extra_chars_after = extra_chars;
                        } break;
                        case ALIGN_CENTER: {

                            extra_chars_before = floor(extra_chars_before/2.);
                            extra_chars_after = ceil(extra_chars_before/2.);
                        } break;
                        case ALIGN_RIGHT: {

                            extra_chars_before = extra_chars;
                        } break;
                    }
                    extra_chars_before -= from->space_count; // we need to handle spaces too
                }

				if (font_color_shadow.a > 0) {

					int chars_total_shadow = chars_total; //save chars drawn
					real_t x_ofs_shadow = x_ofs;
					for (int i = 0; i < from->word_len; i++) {

						if (visible_chars < 0 || chars_total_shadow < visible_chars) {
							CharType c = xl_text[i + pos];
							CharType n = xl_text[i + pos + 1];
							if (uppercase) {
								c = String::char_uppercase(c);
								n = String::char_uppercase(n);
							}

							real_t move = drawer.draw_char(ci, Point2(x_ofs_shadow, y_ofs) + shadow_ofs, c, n, font_color_shadow);
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

						if (transition_in_progress) {

                            CharTransform xform = transition_xform.xform;
                            real_t x_ofs_move = x_ofs;

                            CharPair cn = _process_transition_char(xform, transition_draw_state, from->line, line_chars, from->char_pos + i, x_ofs_move);

                            if (cn.first) {
                                x_ofs += drawer.draw_char(ci, xform, Point2(x_ofs_move, y_ofs), cn.first, cn.second, font_color) + horizontal_spacing;
                            }
                        } else {
                            CharType c = xl_text[i + pos];
                            CharType n = xl_text[i + pos + 1];

                            if (uppercase) {
                                c = String::char_uppercase(c);
                                n = String::char_uppercase(n);
                            }
                            x_ofs += drawer.draw_char(ci, Point2(x_ofs, y_ofs), c, n, font_color) + horizontal_spacing;
                        }
						chars_total++;
					}
					line_chars++;
				}
                from = from->next;
			}

			line++;
            line_chars = 0;
			wc = to ? to->next : 0;
            printf("\n");
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

        if (is_transition_enabled() && transition_xform.is_active()) {
            float dt = get_process_delta_time();

            if (transition_controller->update(dt, ease_func_table[transition_ease], transition_xform) == AnimationController::ANIMCTRL_DONE) {

                transition_xform = AnimationTransform();
                if (text != transition_text.text || xl_text != transition_text.xl_text) {

                    text = transition_text.text;
                    transition_text.text = "";
                    xl_text = transition_text.xl_text;
                    transition_text.xl_text = "";

                    _RemoveWordCache(word_cache);
                    word_cache = transition_text.word_cache;

                    transition_text.word_cache = 0;
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
	real_t max_line_width = 0;
	real_t line_width = 0;

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

			real_t char_width = font->get_char_size(current, xl_text[i + 1]).width + horizontal_spacing;
			line_width += char_width;
		}
	}

	if (line_width > max_line_width)
		max_line_width = line_width;

	// ceiling to ensure autowrapping does not cut text
	return Math::ceil(max_line_width);
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

void Label::_dump_word_cache(const String &text, const Label::WordCache *wc) {
    print_line("WordCache:");
    while(wc) {
        print_line(vformat("  '"+text.substr(wc->char_pos, wc->word_len)+"' char_pos=%d,line=%d,line_pos=%d,len=%d,spc=%d"
            ,wc->char_pos
            ,wc->line
            ,wc->line_pos
            ,wc->word_len
            ,wc->space_count
        ));
        wc = wc->next;
    }
    print_line("----------");
}

// Notice: space_count is the number of spaces before the word
// Notice: spaces before end of the line/return are skipped
Label::WordCache *Label::calculate_word_cache(const Ref<Font> &font, const String &label_text, int &line_count, int &total_char_cache, int &width) const {

	if (autowrap) {
		Ref<StyleBox> style = get_stylebox("normal");
		width = MAX(get_size().width, get_custom_minimum_size().width) - style->get_minimum_size().width;
	} else {
		width = get_longest_line_width(label_text);
	}

	real_t current_word_size = 0;
	int word_pos = 0;
    int line_pos = 0;
	real_t line_width = 0;
	int space_count = 0;
	real_t space_width = font->get_char_size(' ').width + horizontal_spacing;
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
		real_t char_width = 0;

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
                wc->line = line_count - 1;
				wc->line_pos = line_pos - wc->word_len;
				wc->space_count = space_count;
				current_word_size = 0;
				space_count = 0;
			}

			if (current == '\n') {
				insert_newline = true;
			} else if (current != ' ') {
				total_char_cache++;
			}

			line_pos++;

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
			char_width = font->get_char_size(current, label_text[i + 1]).width + horizontal_spacing;
			current_word_size += char_width;
			line_width += char_width;
            line_pos++;
			total_char_cache++;

			// allow autowrap to cut words when they exceed line width
			if (autowrap && (current_word_size > width)) {
				separatable = true;
			}
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
                    wc->line = line_count - 1;
					wc->line_pos = line_pos - wc->word_len;
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
            wc->line = line_count - 1;

			line_width = current_word_size;
			line_count++;
            line_pos = 0;
			space_count = 0;
		}
	}

	return root;
}

CharType Label::get_char_at(WordCache *cache, String &text, int line, int pos, int *word, CharType *next_char) const {
	int word_count = 0, line_pos = 0;
    if (word)
        *word = 0;
    WordCache *wc = cache;
    while (wc && wc->line != line)
        wc = wc->next;
    while (wc) {
        if (wc->char_pos < 0) // end of line
            return 0;
        line_pos += wc->word_len + wc->space_count;
        if (line_pos > pos) {
            const int index = wc->char_pos + wc->word_len - (line_pos - pos);
            if (next_char)
                *next_char = text[index + 1]; // character or NULL
            if (word)
                *word = word_count;
            return text[index];
        }

        wc = wc->next;
        if (!wc) // end of line
            break;
        word_count++;
    }
    return 0;
}

int Label::get_line_size(WordCache *cache, String &text, int line) const {
	int line_size = 0;
    WordCache *wc = cache;
    while (wc && wc->line != line)
        wc = wc->next;
    while (wc && wc->char_pos >= 0) {
        line_size += wc->word_len + wc->space_count; // including spaces
        wc = wc->next;
    }
    return line_size;
}

void Label::regenerate_word_cache() {

	_RemoveWordCache(word_cache);

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

	if (is_transition_enabled() && transition_xform.is_active()) {

        _RemoveWordCache(transition_text.word_cache);

        transition_text.word_cache = calculate_word_cache(font,
                                                          transition_text.xl_text,
                                                          transition_text.line_count,
                                                          transition_text.total_char_cache,
                                                          transition_text.width);
        // try to extmate position of the same chars in every words
        transition_text.same_chars_pos = Dictionary();
        const real_t space_w = font->get_char_size(' ').width + horizontal_spacing;
        int line_xpos = 0;
        WordCache *wc = word_cache, *tr_wc = transition_text.word_cache;
        while (wc) {
            if (wc->char_pos < 0) { // end of line / wrap line
                line_xpos = 0;
            } else {
                line_xpos += space_w*wc->space_count;
                // make sure we are on the same line
                if (wc && tr_wc && wc->line != tr_wc->line) {
                    while (tr_wc && wc->line != tr_wc->line)
                        tr_wc = tr_wc->next;
                }
                const String tr_word = tr_wc
                    ? transition_text.xl_text.substr(tr_wc->char_pos, tr_wc->word_len)
                    : "";
                for (int index=0; index<wc->word_len; ++index) {
                    CharType c = xl_text[wc->char_pos + index];
                    CharType c_ = index<tr_word.size() ? tr_word[index] : 0;
                    if (c_ != 0) {
                        if (uppercase) {
                            c = String::char_uppercase(c);
                            c_ = String::char_uppercase(c_);
                        }
                        if (c == c_) {
                            transition_text.same_chars_pos[wc->char_pos + index] = line_xpos;
                        }
                    }
                    line_xpos += font->get_char_size(xl_text[index], xl_text[index + 1]).width + horizontal_spacing;
                }
                line_xpos += space_w * wc->space_count;
            }

            if (tr_wc && tr_wc->char_pos >= 0) // next word in the line (donot switch new-line)
                tr_wc = tr_wc->next;
            wc = wc->next;
            if (!wc)
                break;
        }
    }

	word_cache_dirty = false;
}

void Label::_clear_pending_animations() { // reset animation

    if (transition_xform.is_active()) {
        transition_xform = AnimationTransform();
        xl_text = transition_text.xl_text;
        text = transition_text.text;
        word_cache = transition_text.word_cache;
    }
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
    if (is_transition_enabled()) {
        transition_text.text = p_string;
        transition_text.xl_text = tr(p_string);
        transition_controller->init_xform(transition_duration, transition_xform);
    } else {
        text = p_string;
        xl_text = tr(p_string);
        if (percent_visible < 1)
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

    if (p_effect != transition_effect) {
        transition_effect = p_effect;
        transition_controller = transition_controllers_table[p_effect];
        if (p_effect == TRANSITIONEFFECT_NONE) {
            set_process_internal(false);
            _clear_pending_animations();
        } else
            set_process_internal(true);
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

void Label::set_transition_scale_width(bool p_scale_width) {

    if (p_scale_width != transition_opts.scale_width) {
        transition_opts.scale_width = p_scale_width;
        update();
    }
}

bool Label::is_transition_scale_width() const {

    return transition_opts.scale_width;
}

void Label::set_transition_align_vrotation(bool p_align_vrotation) {

    if (p_align_vrotation != transition_opts.align_vrotation) {
        transition_opts.align_vrotation = p_align_vrotation;
        update();
    }
}

bool Label::is_transition_align_vrotation() const {

    return transition_opts.align_vrotation;
}

void Label::set_transition_align_vrotation_factor(real_t p_align_vrotation_factor) {

    if (p_align_vrotation_factor != transition_opts.align_vrotation_factor) {
        transition_opts.align_vrotation_factor = p_align_vrotation_factor;
        update();
    }
}

real_t Label::get_transition_align_vrotation_factor() const {

    return transition_opts.align_vrotation_factor;
}

void Label::set_transition_change_policy(TransitionChangePolicy p_change_policy) {

    if (p_change_policy != transition_change_policy) {
        transition_change_policy = p_change_policy;
        update();
    }
}

Label::TransitionChangePolicy Label::get_transition_change_policy() const {

    return transition_change_policy;
}

bool Label::is_transition_active() const {

    return (is_transition_enabled() && transition_xform.is_active());
}


void Label::_bind_methods() {

	ClassDB::bind_method(D_METHOD("set_align", "align"), &Label::set_align);
	ClassDB::bind_method(D_METHOD("get_align"), &Label::get_align);
	ClassDB::bind_method(D_METHOD("set_valign", "valign"), &Label::set_valign);
	ClassDB::bind_method(D_METHOD("get_valign"), &Label::get_valign);
	ClassDB::bind_method(D_METHOD("set_text", "text"), &Label::set_text);
	ClassDB::bind_method(D_METHOD("get_text"), &Label::get_text);
	ClassDB::bind_method(D_METHOD("set_transition_scale_width"), &Label::set_transition_scale_width);
	ClassDB::bind_method(D_METHOD("is_transition_scale_width"), &Label::is_transition_scale_width);
	ClassDB::bind_method(D_METHOD("set_transition_align_vrotation"), &Label::set_transition_align_vrotation);
	ClassDB::bind_method(D_METHOD("is_transition_align_vrotation"), &Label::is_transition_align_vrotation);
	ClassDB::bind_method(D_METHOD("set_transition_align_vrotation_factor"), &Label::set_transition_align_vrotation_factor);
	ClassDB::bind_method(D_METHOD("get_transition_align_vrotation_factor"), &Label::get_transition_align_vrotation_factor);
	ClassDB::bind_method(D_METHOD("set_transition_duration"), &Label::set_transition_duration);
	ClassDB::bind_method(D_METHOD("get_transition_duration"), &Label::get_transition_duration);
	ClassDB::bind_method(D_METHOD("set_transition_ease"), &Label::set_transition_ease);
	ClassDB::bind_method(D_METHOD("get_transition_ease"), &Label::get_transition_ease);
	ClassDB::bind_method(D_METHOD("set_transition_effect"), &Label::set_transition_effect);
	ClassDB::bind_method(D_METHOD("get_transition_effect"), &Label::get_transition_effect);
	ClassDB::bind_method(D_METHOD("set_transition_change_policy"), &Label::set_transition_change_policy);
	ClassDB::bind_method(D_METHOD("get_transition_change_policy"), &Label::get_transition_change_policy);
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

    BIND_ENUM_CONSTANT(TRANSITIONEFFECT_NONE);
    BIND_ENUM_CONSTANT(TRANSITIONEFFECT_SLIDE_UP);
    BIND_ENUM_CONSTANT(TRANSITIONEFFECT_SLIDE_DOWN);
    BIND_ENUM_CONSTANT(TRANSITIONEFFECT_ROTATE_V);
    BIND_ENUM_CONSTANT(TRANSITIONEFFECT_ROTATE_H);

    BIND_ENUM_CONSTANT(TRANSITIONCHANGEPOLICY_ALL);
    BIND_ENUM_CONSTANT(TRANSITIONCHANGEPOLICY_NEW);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_MULTILINE_TEXT, "", PROPERTY_USAGE_DEFAULT_INTL), "set_text", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "align", PROPERTY_HINT_ENUM, "Left,Center,Right,Fill"), "set_align", "get_align");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "valign", PROPERTY_HINT_ENUM, "Top,Center,Bottom,Fill"), "set_valign", "get_valign");
	ADD_GROUP("Transition", "transition_");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "transition_duration"), "set_transition_duration", "get_transition_duration");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "transition_ease", PROPERTY_HINT_ENUM, EASE_FUNC), "set_transition_ease", "get_transition_ease");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "transition_effect", PROPERTY_HINT_ENUM, "None,SlideUp,SlideDown,RotateV,RotateH"), "set_transition_effect", "get_transition_effect");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "transition_change_policy", PROPERTY_HINT_ENUM, "All,New"), "set_transition_change_policy", "get_transition_change_policy");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "transition_scale_width"), "set_transition_scale_width", "is_transition_scale_width");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "transition_align_v_rotation"), "set_transition_align_vrotation", "is_transition_align_vrotation");
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "transition_align_v_rotation_factor",  PROPERTY_HINT_RANGE, "0,1,0.05"), "set_transition_align_vrotation_factor", "get_transition_align_vrotation_factor");
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
	word_cache = NULL;
	word_cache_dirty = true;
	autowrap = false;
    vertical_spacing = 0;
    horizontal_spacing = 0;
	line_count = 0;
	set_v_size_flags(0);
	clip = false;
	transition_text.xl_text = "";
    transition_duration = 1.0;
    transition_ease = TRANSITIONEASE_NONE;
    transition_opts.scale_width = false;
    transition_opts.align_vrotation = false;
    transition_opts.align_vrotation_factor = 0.5;
    transition_change_policy = TRANSITIONCHANGEPOLICY_NEW;
    transition_effect = TRANSITIONEFFECT_NONE;
    transition_controller = transition_controllers_table[TRANSITIONEFFECT_NONE];
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
