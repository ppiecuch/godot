/*************************************************************************/
/*  tinyexpr.cpp                                                         */
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

/*************************************************************************/
/*                                                                       */
/* TINYEXPR - Tiny recursive descent parser and evaluation engine in C   */
/* Copyright (c) 2015-2020 Lewis Van Winkle                              */
/*                                                                       */
/*************************************************************************/

/* COMPILE TIME OPTIONS */

/* Exponentiation associativity:
For a^b^c = (a^b)^c and -a^b = (-a)^b do nothing.
For a^b^c = a^(b^c) and -a^b = -(a^b) uncomment the next line.*/
/* #define TE_POW_FROM_RIGHT */

/* Logarithms
For log = base 10 log do nothing
For log = natural log uncomment the next line. */
/* #define TE_NAT_LOG */

#include "tinyexpr.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h> // strtod
#include <string.h>

#include "core/math/math_funcs.h"
#include "core/os/memory.h"

#ifndef NAN
#define NAN (0.0 / 0.0)
#endif

#ifndef INFINITY
#define INFINITY (1.0 / 0.0)
#endif

typedef real_t (*te_fun1)(real_t);
typedef real_t (*te_fun2)(real_t, real_t);

enum {
	TOK_NULL = TE_CLOSURE7 + 1,
	TOK_ERROR,
	TOK_END,
	TOK_SEP,
	TOK_OPEN,
	TOK_CLOSE,
	TOK_NUMBER,
	TOK_VARIABLE,
	TOK_INFIX
};

enum { TE_CONSTANT = 1 };

typedef struct state {
	const char *start;
	const char *next;
	int type;
	union {
		real_t value;
		const real_t *bound;
		const void *function;
	};
	te_expr *context;

	const te_variable *lookup;
	int lookup_len;
} state;

#define TYPE_MASK(TYPE) ((TYPE)&0x0000001F)

#define IS_PURE(TYPE) (((TYPE)&TE_FLAG_PURE) != 0)
#define IS_FUNCTION(TYPE) (((TYPE)&TE_FUNCTION0) != 0)
#define IS_CLOSURE(TYPE) (((TYPE)&TE_CLOSURE0) != 0)
#define ARITY(TYPE) (((TYPE) & (TE_FUNCTION0 | TE_CLOSURE0)) ? ((TYPE)&0x00000007) : 0)
#define NEW_EXPR(type, ...) new_expr((type), __VA_ARGS__)

static te_expr *new_expr(const int type, const te_expr *p1, const te_expr *p2 = 0, const te_expr *p3 = 0) {
	const te_expr *parameters[] = { p1, p2, p3 };
	const int arity = ARITY(type);
	const int psize = sizeof(void *) * arity;
	const int size = (sizeof(te_expr) - sizeof(void *)) + psize + (IS_CLOSURE(type) ? sizeof(void *) : 0);
	te_expr *ret = (te_expr *)memalloc(size);
	memset(ret, 0, size);
	if (arity) {
		memcpy(ret->parameters, parameters, psize);
	}
	ret->type = type;
	ret->bound = 0;
	return ret;
}

void te_free_parameters(te_expr *n) {
	if (!n)
		return;
	switch (TYPE_MASK(n->type)) {
		case TE_FUNCTION7:
		case TE_CLOSURE7:
			te_free(n->parameters[6]); /* Falls through. */
		case TE_FUNCTION6:
		case TE_CLOSURE6:
			te_free(n->parameters[5]); /* Falls through. */
		case TE_FUNCTION5:
		case TE_CLOSURE5:
			te_free(n->parameters[4]); /* Falls through. */
		case TE_FUNCTION4:
		case TE_CLOSURE4:
			te_free(n->parameters[3]); /* Falls through. */
		case TE_FUNCTION3:
		case TE_CLOSURE3:
			te_free(n->parameters[2]); /* Falls through. */
		case TE_FUNCTION2:
		case TE_CLOSURE2:
			te_free(n->parameters[1]); /* Falls through. */
		case TE_FUNCTION1:
		case TE_CLOSURE1:
			te_free(n->parameters[0]);
	}
}

void te_free(te_expr *n) {
	if (!n)
		return;
	te_free_parameters(n);
	memfree(n);
}

static real_t pi(void) {
	return 3.14159265358979323846;
}
static real_t e(void) {
	return 2.71828182845904523536;
}
const double UIntMax = UINT_MAX;
const double ULongMax = ULONG_MAX;
static real_t fac(real_t a) { /* simplest version of fac */
	if (a < 0)
		return NAN;
	if (a > UIntMax)
		return INFINITY;
	unsigned int ua = (unsigned int)(a);
	unsigned long int result = 1, i;
	for (i = 1; i <= ua; i++) {
		if (i > ULongMax / result)
			return INFINITY;
		result *= i;
	}
	return (real_t)result;
}
static real_t ncr(real_t n, real_t r) {
	if (n < 0 || r < 0 || n < r)
		return NAN;
	if (n > UIntMax || r > UIntMax)
		return INFINITY;
	unsigned long int un = (unsigned int)(n), ur = (unsigned int)(r), i;
	unsigned long int result = 1;
	if (ur > un / 2)
		ur = un - ur;
	for (i = 1; i <= ur; i++) {
		if (result > ULongMax / (un - ur + i))
			return INFINITY;
		result *= un - ur + i;
		result /= i;
	}
	return result;
}
static real_t npr(real_t n, real_t r) {
	return ncr(n, r) * fac(r);
}

static real_t _fabs(real_t arg) {
	return Math::abs(arg);
}
static real_t _fmod(real_t arg1, real_t arg2) {
	return Math::fmod(arg1, arg2);
}
static real_t _acos(real_t arg) {
	return Math::acos(arg);
}
static real_t _asin(real_t arg) {
	return Math::asin(arg);
}
static real_t _atan(real_t arg) {
	return Math::atan(arg);
}
static real_t _atan2(real_t arg1, real_t arg2) {
	return Math::atan2(arg1, arg2);
}
static real_t _ceil(real_t arg) {
	return Math::ceil(arg);
}
static real_t _cos(real_t arg) {
	return Math::cos(arg);
}
static real_t _cosh(real_t arg) {
	return Math::cosh(arg);
}
static real_t _exp(real_t arg) {
	return Math::exp(arg);
}
static real_t _floor(real_t arg) {
	return Math::floor(arg);
}
static real_t _log(real_t arg) {
	return Math::log(arg);
}
static real_t _log10(real_t arg) {
	return Math::log10(arg);
}
static real_t _pow(real_t arg1, real_t arg2) {
	return Math::pow(arg1, arg2);
}
static real_t _sin(real_t arg) {
	return Math::sin(arg);
}
static real_t _sinh(real_t arg) {
	return Math::sinh(arg);
}
static real_t _sqrt(real_t arg) {
	return Math::sqrt(arg);
}
static real_t _tan(real_t arg) {
	return Math::tan(arg);
}
static real_t _tanh(real_t arg) {
	return Math::tanh(arg);
}

#define FVOID(F) (const void *)(F)
#define FREAL(F) (const real_t *)(F)
#define FFUN1(F) (te_fun1)(F)
#define FFUN2(F) (te_fun2)(F)

static const te_variable functions[] = {
	/* must be in alphabetical order */
	{ "abs", FVOID(_fabs), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
	{ "acos", FVOID(_acos), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
	{ "asin", FVOID(_asin), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
	{ "atan", FVOID(_atan), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
	{ "atan2", FVOID(_atan2), TE_FUNCTION2 | TE_FLAG_PURE, 0 },
	{ "ceil", FVOID(_ceil), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
	{ "cos", FVOID(_cos), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
	{ "cosh", FVOID(_cosh), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
	{ "exp", FVOID(_exp), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
	{ "floor", FVOID(_floor), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
	{ "ln", FVOID(_log), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
#ifdef TE_NAT_LOG
	{ "log", FVOID(_log), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
#else
	{ "log", FVOID(_log10), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
#endif
	{ "log10", FVOID(_log10), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
	{ "pow", FVOID(_pow), TE_FUNCTION2 | TE_FLAG_PURE, 0 },
	{ "sin", FVOID(_sin), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
	{ "sinh", FVOID(_sinh), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
	{ "sqrt", FVOID(_sqrt), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
	{ "tan", FVOID(_tan), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
	{ "tanh", FVOID(_tanh), TE_FUNCTION1 | TE_FLAG_PURE, 0 },

	{ "e", FVOID(e), TE_FUNCTION0 | TE_FLAG_PURE, 0 },
	{ "fac", FVOID(fac), TE_FUNCTION1 | TE_FLAG_PURE, 0 },
	{ "ncr", FVOID(ncr), TE_FUNCTION2 | TE_FLAG_PURE, 0 },
	{ "npr", FVOID(npr), TE_FUNCTION2 | TE_FLAG_PURE, 0 },
	{ "pi", FVOID(pi), TE_FUNCTION0 | TE_FLAG_PURE, 0 },

	{ 0, 0, 0, 0 }
};

static const te_variable *find_builtin(const char *name, int len) {
	int imin = 0;
	int imax = sizeof(functions) / sizeof(te_variable) - 2;

	/*Binary search.*/
	while (imax >= imin) {
		const int i = (imin + ((imax - imin) / 2));
		int c = strncmp(name, functions[i].name, len);
		if (!c)
			c = '\0' - functions[i].name[len];
		if (c == 0) {
			return functions + i;
		} else if (c > 0) {
			imin = i + 1;
		} else {
			imax = i - 1;
		}
	}

	return 0;
}

static const te_variable *find_lookup(const state *s, const char *name, int len) {
	int iters;
	const te_variable *var;
	if (!s->lookup)
		return 0;

	for (var = s->lookup, iters = s->lookup_len; iters; ++var, --iters) {
		if (strncmp(name, var->name, len) == 0 && var->name[len] == '\0') {
			return var;
		}
	}
	return 0;
}

static real_t add(real_t a, real_t b) {
	return a + b;
}
static real_t sub(real_t a, real_t b) {
	return a - b;
}
static real_t mul(real_t a, real_t b) {
	return a * b;
}
static real_t divide(real_t a, real_t b) {
	return a / b;
}
static real_t negate(real_t a) {
	return -a;
}
static real_t comma(real_t a, real_t b) {
	(void)a;
	return b;
}

void next_token(state *s) {
	s->type = TOK_NULL;

	do {
		if (!*s->next) {
			s->type = TOK_END;
			return;
		}

		/* Try reading a number. */
		if ((s->next[0] >= '0' && s->next[0] <= '9') || s->next[0] == '.') {
			s->value = strtod(s->next, (char **)&s->next);
			s->type = TOK_NUMBER;
		} else {
			/* Look for a variable or builtin function call. */
			if (s->next[0] >= 'a' && s->next[0] <= 'z') {
				const char *start;
				start = s->next;
				while ((s->next[0] >= 'a' && s->next[0] <= 'z') || (s->next[0] >= '0' && s->next[0] <= '9') || (s->next[0] == '_'))
					s->next++;

				const te_variable *var = find_lookup(s, start, s->next - start);
				if (!var)
					var = find_builtin(start, s->next - start);

				if (!var) {
					s->type = TOK_ERROR;
				} else {
					switch (TYPE_MASK(var->type)) {
						case TE_VARIABLE:
							s->type = TOK_VARIABLE;
							s->bound = FREAL(var->address);
							break;

						case TE_CLOSURE0:
						case TE_CLOSURE1:
						case TE_CLOSURE2:
						case TE_CLOSURE3: /* Falls through. */
						case TE_CLOSURE4:
						case TE_CLOSURE5:
						case TE_CLOSURE6:
						case TE_CLOSURE7: /* Falls through. */
							s->context = var->context; /* Falls through. */

						case TE_FUNCTION0:
						case TE_FUNCTION1:
						case TE_FUNCTION2:
						case TE_FUNCTION3: /* Falls through. */
						case TE_FUNCTION4:
						case TE_FUNCTION5:
						case TE_FUNCTION6:
						case TE_FUNCTION7: /* Falls through. */
							s->type = var->type;
							s->function = var->address;
							break;
					}
				}

			} else {
				/* Look for an operator or special character. */
				switch (s->next++[0]) {
					case '+':
						s->type = TOK_INFIX;
						s->function = FVOID(add);
						break;
					case '-':
						s->type = TOK_INFIX;
						s->function = FVOID(sub);
						break;
					case '*':
						s->type = TOK_INFIX;
						s->function = FVOID(mul);
						break;
					case '/':
						s->type = TOK_INFIX;
						s->function = FVOID(divide);
						break;
					case '^':
						s->type = TOK_INFIX;
						s->function = FVOID(_pow);
						break;
					case '%':
						s->type = TOK_INFIX;
						s->function = FVOID(_fmod);
						break;
					case '(':
						s->type = TOK_OPEN;
						break;
					case ')':
						s->type = TOK_CLOSE;
						break;
					case ',':
						s->type = TOK_SEP;
						break;
					case ' ':
					case '\t':
					case '\n':
					case '\r':
						break;
					default:
						s->type = TOK_ERROR;
						break;
				}
			}
		}
	} while (s->type == TOK_NULL);
}

static te_expr *list(state *s);
static te_expr *expr(state *s);
static te_expr *power(state *s);

static te_expr *base(state *s) {
	/* <base>      =    <constant> | <variable> | <function-0> {"(" ")"} | <function-1> <power> | <function-X> "(" <expr> {"," <expr>} ")" | "(" <list> ")" */
	te_expr *ret;
	int arity;

	switch (TYPE_MASK(s->type)) {
		case TOK_NUMBER:
			ret = new_expr(TE_CONSTANT, 0);
			ret->value = s->value;
			next_token(s);
			break;

		case TOK_VARIABLE:
			ret = new_expr(TE_VARIABLE, 0);
			ret->bound = s->bound;
			next_token(s);
			break;

		case TE_FUNCTION0:
		case TE_CLOSURE0:
			ret = new_expr(s->type, 0);
			ret->function = s->function;
			if (IS_CLOSURE(s->type))
				ret->parameters[0] = s->context;
			next_token(s);
			if (s->type == TOK_OPEN) {
				next_token(s);
				if (s->type != TOK_CLOSE) {
					s->type = TOK_ERROR;
				} else {
					next_token(s);
				}
			}
			break;

		case TE_FUNCTION1:
		case TE_CLOSURE1:
			ret = new_expr(s->type, 0);
			ret->function = s->function;
			if (IS_CLOSURE(s->type))
				ret->parameters[1] = s->context;
			next_token(s);
			ret->parameters[0] = power(s);
			break;

		case TE_FUNCTION2:
		case TE_FUNCTION3:
		case TE_FUNCTION4:
		case TE_FUNCTION5:
		case TE_FUNCTION6:
		case TE_FUNCTION7:
		case TE_CLOSURE2:
		case TE_CLOSURE3:
		case TE_CLOSURE4:
		case TE_CLOSURE5:
		case TE_CLOSURE6:
		case TE_CLOSURE7:
			arity = ARITY(s->type);

			ret = new_expr(s->type, 0);
			ret->function = s->function;
			if (IS_CLOSURE(s->type))
				ret->parameters[arity] = s->context;
			next_token(s);

			if (s->type != TOK_OPEN) {
				s->type = TOK_ERROR;
			} else {
				int i;
				for (i = 0; i < arity; i++) {
					next_token(s);
					ret->parameters[i] = expr(s);
					if (s->type != TOK_SEP) {
						break;
					}
				}
				if (s->type != TOK_CLOSE || i != arity - 1) {
					s->type = TOK_ERROR;
				} else {
					next_token(s);
				}
			}

			break;

		case TOK_OPEN:
			next_token(s);
			ret = list(s);
			if (s->type != TOK_CLOSE) {
				s->type = TOK_ERROR;
			} else {
				next_token(s);
			}
			break;

		default:
			ret = new_expr(0, 0);
			s->type = TOK_ERROR;
			ret->value = NAN;
			break;
	}

	return ret;
}

static te_expr *power(state *s) {
	/* <power>     =    {("-" | "+")} <base> */
	int sign = 1;
	while (s->type == TOK_INFIX && (s->function == add || s->function == sub)) {
		if (s->function == sub)
			sign = -sign;
		next_token(s);
	}

	te_expr *ret;

	if (sign == 1) {
		ret = base(s);
	} else {
		ret = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, base(s));
		ret->function = FVOID(negate);
	}

	return ret;
}

#ifdef TE_POW_FROM_RIGHT
static te_expr *factor(state *s) {
	/* <factor>    =    <power> {"^" <power>} */
	te_expr *ret = power(s);

	int neg = 0;

	if (ret->type == (TE_FUNCTION1 | TE_FLAG_PURE) && ret->function == negate) {
		te_expr *se = ret->parameters[0];
		free(ret);
		ret = se;
		neg = 1;
	}

	te_expr *insertion = 0;

	while (s->type == TOK_INFIX && (s->function == pow)) {
		te_fun2 t = s->function;
		next_token(s);

		if (insertion) {
			/* Make exponentiation go right-to-left. */
			te_expr *insert = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, insertion->parameters[1], power(s));
			insert->function = t;
			insertion->parameters[1] = insert;
			insertion = insert;
		} else {
			ret = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, power(s));
			ret->function = t;
			insertion = ret;
		}
	}

	if (neg) {
		ret = NEW_EXPR(TE_FUNCTION1 | TE_FLAG_PURE, ret);
		ret->function = negate;
	}

	return ret;
}
#else
static te_expr *factor(state *s) {
	/* <factor>    =    <power> {"^" <power>} */
	te_expr *ret = power(s);

	while (s->type == TOK_INFIX && (s->function == _pow)) {
		const void *t = s->function;
		next_token(s);
		ret = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, power(s));
		ret->function = t;
	}

	return ret;
}
#endif

static te_expr *term(state *s) {
	/* <term>      =    <factor> {("*" | "/" | "%") <factor>} */
	te_expr *ret = factor(s);

	while (s->type == TOK_INFIX && (s->function == mul || s->function == divide || s->function == _fmod)) {
		const void *t = s->function;
		next_token(s);
		ret = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, factor(s));
		ret->function = t;
	}

	return ret;
}

static te_expr *expr(state *s) {
	/* <expr>      =    <term> {("+" | "-") <term>} */
	te_expr *ret = term(s);

	while (s->type == TOK_INFIX && (s->function == add || s->function == sub)) {
		const void *t = s->function;
		next_token(s);
		ret = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, term(s));
		ret->function = t;
	}

	return ret;
}

static te_expr *list(state *s) {
	/* <list>      =    <expr> {"," <expr>} */
	te_expr *ret = expr(s);

	while (s->type == TOK_SEP) {
		next_token(s);
		ret = NEW_EXPR(TE_FUNCTION2 | TE_FLAG_PURE, ret, expr(s));
		ret->function = FVOID(comma);
	}

	return ret;
}

#define TE_FUN(...) ((real_t(*)(__VA_ARGS__))n->function)
#define M(e) te_eval(n->parameters[e])

real_t te_eval(const te_expr *n) {
	if (!n)
		return NAN;

	switch (TYPE_MASK(n->type)) {
		case TE_CONSTANT:
			return n->value;
		case TE_VARIABLE:
			return *n->bound;

		case TE_FUNCTION0:
		case TE_FUNCTION1:
		case TE_FUNCTION2:
		case TE_FUNCTION3:
		case TE_FUNCTION4:
		case TE_FUNCTION5:
		case TE_FUNCTION6:
		case TE_FUNCTION7:
			switch (ARITY(n->type)) {
				case 0:
					return TE_FUN(void)();
				case 1:
					return TE_FUN(real_t)(M(0));
				case 2:
					return TE_FUN(real_t, real_t)(M(0), M(1));
				case 3:
					return TE_FUN(real_t, real_t, real_t)(M(0), M(1), M(2));
				case 4:
					return TE_FUN(real_t, real_t, real_t, real_t)(M(0), M(1), M(2), M(3));
				case 5:
					return TE_FUN(real_t, real_t, real_t, real_t, real_t)(M(0), M(1), M(2), M(3), M(4));
				case 6:
					return TE_FUN(real_t, real_t, real_t, real_t, real_t, real_t)(M(0), M(1), M(2), M(3), M(4), M(5));
				case 7:
					return TE_FUN(real_t, real_t, real_t, real_t, real_t, real_t, real_t)(M(0), M(1), M(2), M(3), M(4), M(5), M(6));
				default:
					return NAN;
			}

		case TE_CLOSURE0:
		case TE_CLOSURE1:
		case TE_CLOSURE2:
		case TE_CLOSURE3:
		case TE_CLOSURE4:
		case TE_CLOSURE5:
		case TE_CLOSURE6:
		case TE_CLOSURE7:
			switch (ARITY(n->type)) {
				case 0:
					return TE_FUN(void *)(n->parameters[0]);
				case 1:
					return TE_FUN(void *, real_t)(n->parameters[1], M(0));
				case 2:
					return TE_FUN(void *, real_t, real_t)(n->parameters[2], M(0), M(1));
				case 3:
					return TE_FUN(void *, real_t, real_t, real_t)(n->parameters[3], M(0), M(1), M(2));
				case 4:
					return TE_FUN(void *, real_t, real_t, real_t, real_t)(n->parameters[4], M(0), M(1), M(2), M(3));
				case 5:
					return TE_FUN(void *, real_t, real_t, real_t, real_t, real_t)(n->parameters[5], M(0), M(1), M(2), M(3), M(4));
				case 6:
					return TE_FUN(void *, real_t, real_t, real_t, real_t, real_t, real_t)(n->parameters[6], M(0), M(1), M(2), M(3), M(4), M(5));
				case 7:
					return TE_FUN(void *, real_t, real_t, real_t, real_t, real_t, real_t, real_t)(n->parameters[7], M(0), M(1), M(2), M(3), M(4), M(5), M(6));
				default:
					return NAN;
			}

		default:
			return NAN;
	}
}

#undef TE_FUN
#undef M

static void optimize(te_expr *n) {
	/* Evaluates as much as possible. */
	if (n->type == TE_CONSTANT)
		return;
	if (n->type == TE_VARIABLE)
		return;

	/* Only optimize out functions flagged as pure. */
	if (IS_PURE(n->type)) {
		const int arity = ARITY(n->type);
		int known = 1;
		int i;
		for (i = 0; i < arity; ++i) {
			optimize(n->parameters[i]);
			if (((te_expr *)(n->parameters[i]))->type != TE_CONSTANT) {
				known = 0;
			}
		}
		if (known) {
			const real_t value = te_eval(n);
			te_free_parameters(n);
			n->type = TE_CONSTANT;
			n->value = value;
		}
	}
}

te_expr *te_compile(const char *expression, const te_variable *variables, int var_count, int *error) {
	state s;
	s.start = s.next = expression;
	s.lookup = variables;
	s.lookup_len = var_count;

	next_token(&s);
	te_expr *root = list(&s);

	if (s.type != TOK_END) {
		te_free(root);
		if (error) {
			*error = (s.next - s.start);
			if (*error == 0)
				*error = 1;
		}
		return 0;
	} else {
		optimize(root);
		if (error)
			*error = 0;
		return root;
	}
}

real_t te_interp(const char *expression, int *error) {
	te_expr *n = te_compile(expression, 0, 0, error);
	real_t ret;
	if (n) {
		ret = te_eval(n);
		te_free(n);
	} else {
		ret = NAN;
	}
	return ret;
}

static void pn(const te_expr *n, int depth) {
	int i, arity;
	printf("%*s", depth, "");

	switch (TYPE_MASK(n->type)) {
		case TE_CONSTANT:
			printf("%f\n", n->value);
			break;
		case TE_VARIABLE:
			printf("bound %p\n", n->bound);
			break;

		case TE_FUNCTION0:
		case TE_FUNCTION1:
		case TE_FUNCTION2:
		case TE_FUNCTION3:
		case TE_FUNCTION4:
		case TE_FUNCTION5:
		case TE_FUNCTION6:
		case TE_FUNCTION7:
		case TE_CLOSURE0:
		case TE_CLOSURE1:
		case TE_CLOSURE2:
		case TE_CLOSURE3:
		case TE_CLOSURE4:
		case TE_CLOSURE5:
		case TE_CLOSURE6:
		case TE_CLOSURE7:
			arity = ARITY(n->type);
			printf("f%d", arity);
			for (i = 0; i < arity; i++) {
				printf(" %p", n->parameters[i]);
			}
			printf("\n");
			for (i = 0; i < arity; i++) {
				pn(n->parameters[i], depth + 1);
			}
			break;
	}
}

void te_print(const te_expr *n) {
	pn(n, 0);
}
