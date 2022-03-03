/*************************************************************************/
/*  godot_error_handler.h                                                */
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

#ifndef GODOTERRORHANDLER_H
#define GODOTERRORHANDLER_H

#include <inttypes.h>

#include "core/dictionary.h"
#include "core/error_macros.h"
#include "core/object.h"
#include "core/reference.h"
#include "core/script_language.h"

class GodotErrorHandler : public Object {
public:
	static GodotErrorHandler *get_singleton();

	GodotErrorHandler();
	~GodotErrorHandler();

	void handle_error(Dictionary errorObject);

protected:
	static GodotErrorHandler *singleton;

	struct ErrorObject {
		String source_file;
		String source_func;
		int source_line;
		String error;
		String error_descr;
		bool warning;
		Array callstack;
	};

private:
	static void _err_handler(void *, const char *, const char *, int p_line, const char *, const char *, ErrorHandlerType p_type);

	ErrorHandlerList eh;

	GDCLASS(GodotErrorHandler, Object);
	OBJ_CATEGORY("GodotErrorHandler");
};
#endif // GODOTERRORHANDLER_H
