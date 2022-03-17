/*
 * Copyright (C) Michael Larson on 1/6/2022
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * gl_error.h
 * MGL
 *
 */

#include "gl_error.h"

#include "core/variant.h"
#include "core/error_macros.h"

GLenum  mglGetError(GLMContext ctx) {
	GLenum err = ctx->state.error;
	ctx->state.error = GL_NO_ERROR;
	return err;
}

void error_func(GLMContext ctx, const char *func, GLenum error) {
	WARN_PRINT(vformat("GL Error func: %s type: %d\n", func, error));
	if (ctx->state.error) {
		return;
	}
	ctx->state.error = error;
	if (ctx->assert_on_error) {
		DEV_ASSERT(false);
	}
}
