/*************************************************************************/
/*  tinyexpr.h                                                           */
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

/*
 * TINYEXPR - Tiny recursive descent parser and evaluation engine in C
 * Copyright (c) 2015-2020 Lewis Van Winkle
 *
 */

#ifndef TINYEXPR_H
#define TINYEXPR_H

#include "core/math/math_defs.h"

typedef struct te_expr {
	int type;
	union {
		real_t value;
		const real_t *bound;
		const void *function;
	};
	te_expr *parameters[1];
} te_expr;

enum {
	TE_VARIABLE = 0,

	TE_FUNCTION0 = 8,
	TE_FUNCTION1,
	TE_FUNCTION2,
	TE_FUNCTION3,
	TE_FUNCTION4,
	TE_FUNCTION5,
	TE_FUNCTION6,
	TE_FUNCTION7,

	TE_CLOSURE0 = 16,
	TE_CLOSURE1,
	TE_CLOSURE2,
	TE_CLOSURE3,
	TE_CLOSURE4,
	TE_CLOSURE5,
	TE_CLOSURE6,
	TE_CLOSURE7,

	TE_FLAG_PURE = 32
};

typedef struct te_variable {
	const char *name;
	const void *address;
	int type;
	te_expr *context;
} te_variable;

/* Parses the input expression, evaluates it, and frees it. */
/* Returns NaN on error. */
real_t te_interp(const char *expression, int *error);

/* Parses the input expression and binds variables. */
/* Returns NULL on error. */
te_expr *te_compile(const char *expression, const te_variable *variables, int var_count, int *error);

/* Evaluates the expression. */
real_t te_eval(const te_expr *n);

/* Prints debugging information on the syntax tree. */
void te_print(const te_expr *n);

/* Frees the expression. */
/* This is safe to call on NULL pointers. */
void te_free(te_expr *n);

#endif /* TINYEXPR_H */
