/**************************************************************************/
/*  label_transitions.h                                                   */
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

#ifndef LABEL_TRANSITIONS_H
#define LABEL_TRANSITIONS_H

#include "scene/resources/font.h"

#include <memory>
#include <vector>

// How to use:
// -----------
//  t = current time (in any unit measure, but same unit as duration)
//  b = starting value to interpolate
//  c = the total change in value of b that needs to occur
//  d = total time it should take to complete (duration)

// Linear Easing functions
static real_t ease_linear_none(real_t t, real_t b, real_t c, real_t d) {
	return (c * t / d + b);
}
static real_t ease_linear_in(real_t t, real_t b, real_t c, real_t d) {
	return (c * t / d + b);
}
static real_t ease_linear_out(real_t t, real_t b, real_t c, real_t d) {
	return (c * t / d + b);
}
static real_t ease_linear_inout(real_t t, real_t b, real_t c, real_t d) {
	return (c * t / d + b);
}
// Sine Easing functions
static real_t ease_sine_in(real_t t, real_t b, real_t c, real_t d) {
	return (-c * Math::cos(t / d * (Math_PI / 2.0f)) + c + b);
}
static real_t ease_sine_out(real_t t, real_t b, real_t c, real_t d) {
	return (c * Math::sin(t / d * (Math_PI / 2.0f)) + b);
}
static real_t ease_sine_inout(real_t t, real_t b, real_t c, real_t d) {
	return (-c / 2.0f * (Math::cos(Math_PI * t / d) - 1.0f) + b);
}
// Circular Easing functions
static real_t ease_circ_in(real_t t, real_t b, real_t c, real_t d) {
	t /= d;
	return (-c * (Math::sqrt(1.0f - t * t) - 1.0f) + b);
}
static real_t ease_circ_out(real_t t, real_t b, real_t c, real_t d) {
	t = t / d - 1.0f;
	return (c * Math::sqrt(1.0f - t * t) + b);
}
static real_t ease_circ_inout(real_t t, real_t b, real_t c, real_t d) {
	t /= d / 2.0f;
	if (t < 1.0f)
		return (-c / 2.0f * (Math::sqrt(1.0f - t * t) - 1.0f) + b);
	t -= 2.0f;
	return (c / 2.0f * (Math::sqrt(1.0f - t * t) + 1.0f) + b);
}
// Cubic Easing functions
static real_t ease_cubic_in(real_t t, real_t b, real_t c, real_t d) {
	t /= d;
	return (c * t * t * t + b);
}
static real_t ease_cubic_out(real_t t, real_t b, real_t c, real_t d) {
	t = t / d - 1.0f;
	return (c * (t * t * t + 1.0f) + b);
}
static real_t ease_cubic_inout(real_t t, real_t b, real_t c, real_t d) {
	t /= d / 2.0f;
	if (t < 1.0f)
		return (c / 2.0f * t * t * t + b);
	t -= 2.0f;
	return (c / 2.0f * (t * t * t + 2.0f) + b);
}
// Quadratic Easing functions
static real_t ease_quad_in(real_t t, real_t b, real_t c, real_t d) {
	t /= d;
	return (c * t * t + b);
}
static real_t ease_quad_out(real_t t, real_t b, real_t c, real_t d) {
	t /= d;
	return (-c * t * (t - 2.0f) + b);
}
static real_t ease_quad_inout(real_t t, real_t b, real_t c, real_t d) {
	if ((t /= d / 2) < 1)
		return (((c / 2) * (t * t)) + b);
	return (-c / 2.0f * (((t - 1.0f) * (t - 3.0f)) - 1.0f) + b);
}
// Exponential Easing functions
static real_t ease_expo_in(real_t t, real_t b, real_t c, real_t d) {
	return (t == 0.0f) ? b : (c * Math::pow(2.0f, 10.0f * (t / d - 1.0f)) + b);
}
static real_t ease_expo_out(real_t t, real_t b, real_t c, real_t d) {
	return (t == d) ? (b + c) : (c * (-Math::pow(2.0f, -10.0f * t / d) + 1.0f) + b);
}
static real_t ease_expo_inout(real_t t, real_t b, real_t c, real_t d) {
	if (t == 0.0f)
		return b;
	if (t == d)
		return (b + c);
	if ((t /= d / 2.0f) < 1.0f)
		return (c / 2.0f * Math::pow(2.0f, 10.0f * (t - 1.0f)) + b);
	return (c / 2.0f * (-Math::pow(2.0f, -10.0f * (t - 1.0f)) + 2.0f) + b);
}
// Back Easing functions
static real_t ease_back_in(real_t t, real_t b, real_t c, real_t d) {
	const real_t s = 1.70158f;
	const real_t postFix = t /= d;
	return (c * (postFix)*t * ((s + 1.0f) * t - s) + b);
}
static real_t ease_back_out(real_t t, real_t b, real_t c, real_t d) {
	const real_t s = 1.70158f;
	t = t / d - 1.0f;
	return (c * (t * t * ((s + 1.0f) * t + s) + 1.0f) + b);
}
static real_t ease_back_inout(real_t t, real_t b, real_t c, real_t d) {
	const real_t s = 1.70158f * 1.525f;
	if ((t /= d / 2.0f) < 1.0f)
		return (c / 2.0f * (t * t * ((s + 1.0f) * t - s)) + b);
	const real_t postFix = t -= 2.0f;
	return (c / 2.0f * ((postFix)*t * ((s + 1.0f) * t + s) + 2.0f) + b);
}
// Bounce Easing functions
static real_t ease_bounce_out(real_t t, real_t b, real_t c, real_t d) {
	if ((t /= d) < (1.0f / 2.75f)) {
		return (c * (7.5625f * t * t) + b);
	} else if (t < (2.0f / 2.75f)) {
		const real_t postFix = t -= (1.5f / 2.75f);
		return (c * (7.5625f * (postFix)*t + 0.75f) + b);
	} else if (t < (2.5 / 2.75)) {
		const real_t postFix = t -= (2.25f / 2.75f);
		return (c * (7.5625f * (postFix)*t + 0.9375f) + b);
	} else {
		const real_t postFix = t -= (2.625f / 2.75f);
		return (c * (7.5625f * (postFix)*t + 0.984375f) + b);
	}
}
static real_t ease_bounce_in(real_t t, real_t b, real_t c, real_t d) {
	return (c - ease_bounce_out(d - t, 0.0f, c, d) + b);
}
static real_t ease_bounce_inout(real_t t, real_t b, real_t c, real_t d) {
	if (t < d / 2.0f)
		return (ease_bounce_in(t * 2.0f, 0.0f, c, d) * 0.5f + b);
	else
		return (ease_bounce_out(t * 2.0f - d, 0.0f, c, d) * 0.5f + c * 0.5f + b);
}
// Elastic Easing functions
static real_t ease_elastic_in(real_t t, real_t b, real_t c, real_t d) {
	if (t == 0.0f)
		return b;
	if ((t /= d) == 1.0f)
		return (b + c);
	const real_t p = d * 0.3f;
	const real_t a = c;
	const real_t s = p / 4.0f;
	const real_t postFix = a * Math::pow(2.0f, 10.0f * (t -= 1.0f));
	return (-(postFix * Math::sin((t * d - s) * (2.0f * Math_PI) / p)) + b);
}
static real_t ease_elastic_out(real_t t, real_t b, real_t c, real_t d) {
	if (t == 0.0f)
		return b;
	if ((t /= d) == 1.0f)
		return (b + c);
	const real_t p = d * 0.3f;
	const real_t a = c;
	const real_t s = p / 4.0f;
	return (a * Math::pow(2.0f, -10.0f * t) * Math::sin((t * d - s) * (2.0f * Math_PI) / p) + c + b);
}
static real_t ease_elastic_inout(real_t t, real_t b, real_t c, real_t d) {
	if (t == 0.0f)
		return b;
	if ((t /= d / 2.0f) == 2.0f)
		return (b + c);
	const real_t p = d * (0.3f * 1.5f);
	const real_t a = c;
	const real_t s = p / 4.0f;
	if (t < 1.0f) {
		const real_t postFix = a * Math::pow(2.0f, 10.0f * (t -= 1.0f));
		return -0.5f * (postFix * Math::sin((t * d - s) * (2.0f * Math_PI) / p)) + b;
	}
	const real_t postFix = a * Math::pow(2.0f, -10.0f * (t -= 1.0f));
	return (postFix * Math::sin((t * d - s) * (2.0f * Math_PI) / p) * 0.5f + c + b);
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

#define DefineEaseFunc(F)   \
	ease_##F##_in,          \
			ease_##F##_out, \
			ease_##F##_inout

typedef real_t (*ease_func_t)(real_t t, real_t b, real_t c, real_t d);

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

static inline void _dump_xform(const CharTransform &xform) {
	print_line("CharTransform:");
	print_line(" dest_rect: " + xform.dest_rect);
	print_line(" tex_clip: " + xform.tex_clip);
}

struct Label::AnimationController {
	virtual bool init_transition(Label *p_owner, real_t p_duration, ease_func_t p_ease, WordCache *p_cache_in, WordCache *p_cache_out) = 0;
	virtual bool update(real_t p_dt, ease_func_t p_ease) = 0; // -> true: transition done
	virtual const std::vector<WordCache *> get_draw_cache() = 0;
	virtual const CharTransform *get_char_xform(const WordCache *p_cache, int p_pos) = 0;
	virtual bool is_done() const = 0;
	virtual bool is_valid() const = 0;
	virtual ~AnimationController() {}

	bool _same_char(WordCache &cache1, WordCache &cache2, int position) {
		if (position >= cache1.cache_text.length() || position >= cache2.cache_text.length())
			return false;
		return cache1.cache_text[position] == cache2.cache_text[position];
	}
};

struct Label::AnimationNone : public Label::AnimationController {
	virtual bool init_transition(Label *p_owner, real_t p_duration, ease_func_t p_ease, WordCache *p_cache_in, WordCache *p_cache_out) { return false; }
	virtual bool update(real_t p_dt, ease_func_t p_ease) { return true; }
	virtual const std::vector<WordCache *> get_draw_cache() { return std::vector<WordCache *>(); }
	virtual const CharTransform *get_char_xform(const WordCache *p_cache, int p_pos) { return nullptr; }
	virtual bool is_done() const { return true; }
	virtual bool is_valid() const { return false; }
};

struct Label::GenericDualTransformController : public Label::AnimationController {
	enum AnimationOrient {
		ANIMATION_WHEEL_UP,
		ANIMATION_WHEEL_DOWN,
		ANIMATION_REVEAL_UP,
		ANIMATION_REVEAL_DOWN,
		ANIMATION_SLIDE_UP,
		ANIMATION_SLIDE_DOWN,
	};

	CharTransform xform_out;
	CharTransform xform_in;
	real_t current = 0;
	real_t duration = 0;
	bool active = false;
	bool change_new_chars_only = true;

	Label *owner = nullptr;
	AnimationOrient orientation = ANIMATION_WHEEL_UP;

	WordCache cache_in;
	WordCache cache_out;

	void _update_xform(ease_func_t ease) {
		xform_in.progress = xform_out.progress = 1.0 - Math::abs(current / duration);
		const real_t t = ease(current, 0, 1, duration);
		switch (orientation) {
			case ANIMATION_WHEEL_UP: {
				xform_out.dest_rect.size.y = 1.0 - t; // 1 .. 0
				xform_in.dest_rect.position.y = 1.0 - t; // 1 .. 0
			} break;
			case ANIMATION_WHEEL_DOWN: {
				xform_out.dest_rect.position.y = t; // 0 .. 1
				xform_in.dest_rect.size.y = t; // 0 .. 1
			} break;
			case ANIMATION_REVEAL_UP: {
				xform_out.dest_rect.size.y = 1.0 - t; // 1 .. 0
				xform_out.tex_clip.position.y = t; // 0 .. 1
				xform_in.dest_rect.position.y = 1.0 - t; // 1 .. 0
				xform_in.tex_clip.size.y = t; // 0 .. 1
			} break;
			case ANIMATION_REVEAL_DOWN: {
				xform_out.dest_rect.position.y = t; // 0 .. 1
				xform_out.tex_clip.position.y = t; // 0 .. 1
				xform_in.dest_rect.size.y = t; // 0 .. 1
				xform_in.tex_clip.position.y = t; // 1 .. 0
			} break;
			case ANIMATION_SLIDE_UP: {
				xform_out.dest_rect.size.y = 1.0 - t; // 1 .. 0
				xform_out.tex_clip.position.y = t; // 0 .. 1
				xform_in.dest_rect.position.y = 1.0 - t; // 1 .. 0
				xform_in.tex_clip.size.y = t; // 0 .. 1
			} break;
			case ANIMATION_SLIDE_DOWN: {
				xform_out.dest_rect.position.y = t; // 0 .. 1
				xform_out.tex_clip.size.y = 1.0 - t; // 1 .. 0
				xform_in.dest_rect.size.y = t; // 0 .. 1
				xform_in.tex_clip.position.y = 1.0 - t; // 1 .. 0
			} break;
			default:
				ERR_PRINT("Unknown orientation type - no transform performed");
		}
	}

	virtual bool init_transition(Label *p_owner, real_t p_duration, ease_func_t p_ease, WordCache *p_cache_in, WordCache *p_cache_out) {
		ERR_FAIL_COND_V(p_cache_in == 0, false);
		ERR_FAIL_COND_V(p_cache_out == 0, false);
		ERR_FAIL_COND_V(p_duration <= 0, false);

		xform_in = CharTransform();
		xform_out = CharTransform();

		cache_in = *p_cache_in;
		cache_out = *p_cache_out;

		duration = p_duration;
		current = 0; // transition from 0 .. duration
		active = cache_in.words || cache_out.words;

		_update_xform(p_ease); // initial xform value

		return true;
	}
	virtual bool update(real_t p_dt, ease_func_t p_ease) {
		if (!active) {
			return true;
		}
		if (current > duration) {
			active = false;
			return true;
		}
		_update_xform(p_ease);
		current += p_dt;

		return false;
	}
	virtual const std::vector<WordCache *> get_draw_cache() { return std::vector<WordCache *>{ &cache_in, &cache_out }; }
	virtual const CharTransform *get_char_xform(const WordCache *p_cache, int p_pos) {
		if (!change_new_chars_only || !_same_char(cache_in, cache_out, p_pos)) {
			if (p_cache == &cache_in)
				return &xform_in;
			if (p_cache == &cache_out)
				return &xform_out;
		}
		return nullptr;
	}
	virtual bool is_done() const { return !active; }
	virtual bool is_valid() const {
		if (!owner)
			return false;
		else
			return (cache_in.line_count != cache_out.line_count) && (owner->get_valign() == VALIGN_CENTER || owner->get_align() == ALIGN_CENTER);
	}

	GenericDualTransformController(AnimationOrient p_orientation, bool p_change_new_chars_only) {
		orientation = p_orientation;
		change_new_chars_only = p_change_new_chars_only;
	}
};

struct Label::GenericSingleTransformController : public Label::AnimationController {
	enum AnimationOrient {

		ANIMATION_ROTATE_H,
		ANIMATION_ROTATE_V,
	};

	CharTransform xform;
	real_t current = 0;
	real_t duration = 0;
	bool active = false;

	Label *owner = nullptr;
	AnimationOrient orientation = ANIMATION_ROTATE_H;

	WordCache cache_in;
	WordCache cache_out;

	void _update_xform(ease_func_t ease) {
		xform.progress = 1.0 - Math::abs(current / duration);
		switch (orientation) {
			case ANIMATION_ROTATE_H: {
				xform.dest_rect.position.x = 0.5 - ease(current, 0, 0.5, current < 0 ? -duration : duration); // 0 .. 0,5 .. 0
				xform.dest_rect.size.x = 1 - xform.dest_rect.position.x / (1 - xform.dest_rect.position.x); // 1 .. 0 .. 1
			} break;
			case ANIMATION_ROTATE_V: {
				xform.dest_rect.position.y = 0.5 - ease(current, 0, 0.5, current < 0 ? -duration : duration); // 0 .. 0,5 .. 0
				xform.dest_rect.size.y = 1 - xform.dest_rect.position.y / (1 - xform.dest_rect.position.y); // 1 .. 0 .. 1
			} break;
			default:
				ERR_PRINT("Unknown orientation type - no transform performed");
		}
	}

	virtual bool init_transition(Label *p_owner, real_t p_duration, ease_func_t p_ease, WordCache *p_cache_in, WordCache *p_cache_out) {
		ERR_FAIL_COND_V(p_cache_out == 0, false);
		ERR_FAIL_COND_V(p_cache_in == 0, false);
		ERR_FAIL_COND_V(p_duration <= 0, false);

		const real_t duration_by_2 = p_duration / 2;

		xform = CharTransform();

		xform.vertical_align = orientation == ANIMATION_ROTATE_V;

		cache_in = *p_cache_in;
		cache_out = *p_cache_out;

		duration = duration_by_2;

		// transition from range:  -duration/2 .. 0 .. duration/2
		if (!cache_out.words) // nothing to hide - jump to showing new text
			current = 0;
		else
			current = -duration_by_2;
		active = cache_in.words || cache_out.words;

		_update_xform(p_ease); // initial xform value

		return true;
	}
	virtual bool update(real_t p_dt, ease_func_t p_ease) {
		if (!active) {
			return true;
		}
		if (current > duration || !get_draw_cache().front()->words) {
			active = false;
			return true;
		}
		_update_xform(p_ease);
#if 0
		_dump_xform(xform);
		print_line(vformat(" current:%f", current));
		print_line(" text:"+get_draw_cache()[0]->cache_text);
#endif
		current += p_dt;

		return false;
	}
	virtual const std::vector<WordCache *> get_draw_cache() {
		if (current >= 0) {
			return std::vector<WordCache *>{ &cache_in }; // new text
		} else {
			return std::vector<WordCache *>{ &cache_out }; // old text
		}
	}
	virtual const CharTransform *get_char_xform(const WordCache *p_cache, int p_pos) { return &xform; }
	virtual bool is_done() const { return !active; }
	virtual bool is_valid() const { return !!owner; }

	GenericSingleTransformController(AnimationOrient p_orientation) {
		orientation = p_orientation;
	}
};

struct Label::GenericMulti1TransformController : public Label::AnimationController {
	enum AnimationOrient {

		ANIMATION_ROTATE_H,
		ANIMATION_ROTATE_V
	};

	struct _xform {
		CharTransform xform;
		real_t current = 0;
		real_t delay = 0;
	};
	Vector<_xform> trans_info;
	real_t duration = 0;
	bool active = false;

	Label *owner = nullptr;
	AnimationOrient orientation = ANIMATION_ROTATE_H;

	WordCache cache_in;
	WordCache cache_out;

	void _update_xform(_xform &info, real_t dt, ease_func_t ease) {
		info.xform.progress = 1.0 - Math::abs(info.current / duration);
		const real_t t = ease(info.current, 0, 0.5, info.current < 0 ? -duration : duration);
		switch (orientation) {
			case ANIMATION_ROTATE_H: {
				info.xform.dest_rect.position.x = 0.5 - t; // 0 .. 0,5 .. 0
				info.xform.dest_rect.size.x = 1 - info.xform.dest_rect.position.x / (1 - info.xform.dest_rect.position.x); // 1 .. 0 .. 1
			} break;
			case ANIMATION_ROTATE_V: {
				info.xform.dest_rect.position.y = 0.5 - t; // 0 .. 0,5 .. 0
				info.xform.dest_rect.size.y = 1 - info.xform.dest_rect.position.y / (1 - info.xform.dest_rect.position.y); // 1 .. 0 .. 1
			} break;
			default:
				ERR_PRINT("Unknown orientation type - no transform performed");
		}
		info.current += dt;
	}
	bool _update_xform(real_t dt, ease_func_t ease) {
		bool status = false;
		for (int f = 0; f < trans_info.size(); ++f) {
			_xform &info = trans_info.write[f];
			if (info.delay <= 0) {
				if (info.current < duration) {
					_update_xform(info, dt, ease);
					status = true;
				}
			} else {
				info.delay -= dt;
				status = true;
			}
		}
		return status;
	}

	virtual bool init_transition(Label *p_owner, real_t p_duration, ease_func_t p_ease, WordCache *p_cache_in, WordCache *p_cache_out) {
		ERR_FAIL_COND_V(p_cache_out == 0, false);
		ERR_FAIL_COND_V(p_cache_in == 0, false);
		ERR_FAIL_COND_V(p_duration <= 0, false);

		const real_t duration_by_2 = p_duration / 2;
		const int xforms_size = MAX(p_cache_in->cache_text.length(), p_cache_out->cache_text.length());

		trans_info.clear();
		trans_info.resize(xforms_size);

		cache_in = *p_cache_in;
		cache_out = *p_cache_out;

		duration = duration_by_2;

		for (int f = 0; f < xforms_size; ++f) {
			_xform &info = trans_info.write[f];
			info.xform.vertical_align = orientation == ANIMATION_ROTATE_V;
			info.delay = f * duration * 0.2;
			// transition from range:  -duration/2 .. 0 .. duration/2
			if (!cache_out.words) // nothing to hide - jump to showing new text
				info.current = 0;
			else
				info.current = -duration_by_2;
			_update_xform(info, 0, p_ease); // initial xform value
		}

		active = cache_in.words || cache_out.words;

		return true;
	}
	virtual bool update(real_t p_dt, ease_func_t p_ease) {
		if (!active) {
			return true;
		}
		if ((active = _update_xform(p_dt, p_ease))) {
			return false;
		}
		cache_in = WordCache();
		cache_out = WordCache();
		return true;
	}
	virtual const std::vector<WordCache *> get_draw_cache() {
		return std::vector<WordCache *>{ &cache_out, &cache_in };
	}
	virtual const CharTransform *get_char_xform(const WordCache *p_cache, int p_pos) {
		ERR_FAIL_INDEX_V(p_pos, trans_info.size(), nullptr);

		static CharTransform _hidden(true);
		const _xform &info = trans_info[p_pos];
		if (info.current > 0) {
			if (p_cache == &cache_out)
				return &_hidden;
			if (p_cache == &cache_in)
				return &info.xform;
		} else {
			if (p_cache == &cache_out)
				return &info.xform;
			if (p_cache == &cache_in)
				return &_hidden;
		}
		return nullptr;
	}
	virtual bool is_done() const { return !active; }
	virtual bool is_valid() const { return !!owner; }

	GenericMulti1TransformController(AnimationOrient p_orientation) {
		orientation = p_orientation;
	}
};

struct Label::GenericMulti2TransformController : public Label::AnimationController {
	enum AnimationOrient {

		ANIMATION_SLIDE_UP,
		ANIMATION_SLIDE_DOWN,
	};

	struct _xform {
		CharTransform xform_in, xform_out;
		real_t current = 0;
		real_t delay = 0;
	};
	Vector<_xform> trans_info;
	real_t duration = 0;
	bool active = false;

	Label *owner = nullptr;
	AnimationOrient orientation = ANIMATION_SLIDE_UP;

	WordCache cache_in;
	WordCache cache_out;

	void _update_xform(_xform &info, real_t dt, ease_func_t ease) {
		info.xform_out.progress = info.xform_in.progress = 1.0 - Math::abs(info.current / duration);
		const real_t t = ease(info.current, 0, 1, duration);
		switch (orientation) {
			case ANIMATION_SLIDE_UP: {
				info.xform_out.dest_rect.size.y = 1.0 - t; // 1 .. 0
				info.xform_out.tex_clip.position.y = t; // 0 .. 1
				info.xform_in.dest_rect.position.y = 1.0 - t; // 1 .. 0
				info.xform_in.tex_clip.size.y = t; // 0 .. 1
			} break;
			case ANIMATION_SLIDE_DOWN: {
				info.xform_out.dest_rect.position.y = t; // 0 .. 1
				info.xform_out.tex_clip.size.y = 1.0 - t; // 1 .. 0
				info.xform_in.dest_rect.size.y = t; // 0 .. 1
				info.xform_in.tex_clip.position.y = 1.0 - t; // 1 .. 0
			} break;
			default:
				ERR_PRINT("Unknown orientation type - no transform performed");
		}
		info.current += dt;
	}
	bool _update_xform(real_t dt, ease_func_t ease) {
		bool status = false;
		for (int f = 0; f < trans_info.size(); ++f) {
			_xform &info = trans_info.write[f];
			if (info.delay <= 0) {
				if (info.current < duration) {
					_update_xform(info, dt, ease);
					status = true;
				}
			} else {
				info.delay -= dt;
				status = true;
			}
		}
		return status;
	}

	virtual bool init_transition(Label *p_owner, real_t p_duration, ease_func_t p_ease, WordCache *p_cache_in, WordCache *p_cache_out) {
		ERR_FAIL_COND_V(p_cache_out == 0, false);
		ERR_FAIL_COND_V(p_cache_in == 0, false);
		ERR_FAIL_COND_V(p_duration <= 0, false);

		const int xforms_size = MAX(p_cache_in->cache_text.length(), p_cache_out->cache_text.length());

		trans_info.clear();
		trans_info.resize(xforms_size);

		cache_in = *p_cache_in;
		cache_out = *p_cache_out;

		duration = p_duration;

		for (int f = 0; f < xforms_size; ++f) {
			_xform &info = trans_info.write[f];
			info.delay = f * p_duration * 0.2;
			info.current = 0;
			_update_xform(info, 0, p_ease); // initial xform value
		}

		active = cache_in.words || cache_out.words;

		return true;
	}
	virtual bool update(real_t p_dt, ease_func_t p_ease) {
		if (!active) {
			return true;
		}
		if ((active = _update_xform(p_dt, p_ease))) {
			return false;
		}
		cache_in = WordCache();
		cache_out = WordCache();
		return true;
	}
	virtual const std::vector<WordCache *> get_draw_cache() {
		return std::vector<WordCache *>{ &cache_out, &cache_in };
	}
	virtual const CharTransform *get_char_xform(const WordCache *p_cache, int p_pos) {
		ERR_FAIL_INDEX_V(p_pos, trans_info.size(), nullptr);

		const _xform &info = trans_info[p_pos];
		if (p_cache == &cache_in)
			return &info.xform_in;
		if (p_cache == &cache_out)
			return &info.xform_out;
		return nullptr;
	}
	virtual bool is_done() const { return !active; }
	virtual bool is_valid() const { return !!owner; }

	GenericMulti2TransformController(AnimationOrient p_orientation) {
		orientation = p_orientation;
	}
};

std::unique_ptr<Label::AnimationController> AnimationControllerFactory(Label::TransitionEffect p_effect) {
	switch (p_effect) {
		case Label::TRANSITIONEFFECT_NONE:
			return std::unique_ptr<Label::AnimationController>(new Label::AnimationNone());
		case Label::TRANSITIONEFFECT_SLIDE_UP:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericDualTransformController(Label::GenericDualTransformController::ANIMATION_SLIDE_UP, false));
		case Label::TRANSITIONEFFECT_SLIDE_DOWN:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericDualTransformController(Label::GenericDualTransformController::ANIMATION_SLIDE_DOWN, false));
		case Label::TRANSITIONEFFECT_SLIDE_UP_NEW:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericDualTransformController(Label::GenericDualTransformController::ANIMATION_SLIDE_UP, true));
		case Label::TRANSITIONEFFECT_SLIDE_DOWN_NEW:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericDualTransformController(Label::GenericDualTransformController::ANIMATION_SLIDE_DOWN, true));
		case Label::TRANSITIONEFFECT_WHEEL_UP:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericDualTransformController(Label::GenericDualTransformController::ANIMATION_WHEEL_UP, false));
		case Label::TRANSITIONEFFECT_WHEEL_DOWN:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericDualTransformController(Label::GenericDualTransformController::ANIMATION_WHEEL_DOWN, false));
		case Label::TRANSITIONEFFECT_WHEEL_UP_NEW:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericDualTransformController(Label::GenericDualTransformController::ANIMATION_WHEEL_UP, true));
		case Label::TRANSITIONEFFECT_WHEEL_DOWN_NEW:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericDualTransformController(Label::GenericDualTransformController::ANIMATION_WHEEL_DOWN, true));
		case Label::TRANSITIONEFFECT_REVEAL_UP:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericDualTransformController(Label::GenericDualTransformController::ANIMATION_REVEAL_UP, false));
		case Label::TRANSITIONEFFECT_REVEAL_DOWN:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericDualTransformController(Label::GenericDualTransformController::ANIMATION_REVEAL_DOWN, false));
		case Label::TRANSITIONEFFECT_REVEAL_UP_NEW:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericDualTransformController(Label::GenericDualTransformController::ANIMATION_REVEAL_UP, true));
		case Label::TRANSITIONEFFECT_REVEAL_DOWN_NEW:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericDualTransformController(Label::GenericDualTransformController::ANIMATION_REVEAL_DOWN, true));
		case Label::TRANSITIONEFFECT_ROTATE_V:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericSingleTransformController(Label::GenericSingleTransformController::ANIMATION_ROTATE_V));
		case Label::TRANSITIONEFFECT_ROTATE_H:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericSingleTransformController(Label::GenericSingleTransformController::ANIMATION_ROTATE_H));
		case Label::TRANSITIONEFFECT_ROTATE_V_SEQ:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericMulti1TransformController(Label::GenericMulti1TransformController::ANIMATION_ROTATE_V));
		case Label::TRANSITIONEFFECT_ROTATE_H_SEQ:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericMulti1TransformController(Label::GenericMulti1TransformController::ANIMATION_ROTATE_H));
		case Label::TRANSITIONEFFECT_SLIDE_UP_SEQ:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericMulti2TransformController(Label::GenericMulti2TransformController::ANIMATION_SLIDE_UP));
		case Label::TRANSITIONEFFECT_SLIDE_DOWN_SEQ:
			return std::unique_ptr<Label::AnimationController>(new Label::GenericMulti2TransformController(Label::GenericMulti2TransformController::ANIMATION_SLIDE_DOWN));
		default:
			ERR_PRINT("Unknown transition - defaults to None");
	}
	return std::unique_ptr<Label::AnimationController>(new Label::AnimationNone());
};

#endif /* LABEL_TRANSITIONS_H */
