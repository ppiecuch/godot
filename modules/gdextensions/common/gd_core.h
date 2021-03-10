/*************************************************************************/
/*  gd_core.h                                                            */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef GD_CORE_H
#define GD_CORE_H

#include "core/class_db.h"
#include "core/ustring.h"
#include "core/os/os.h"
#include "core/math/vector2.h"
#include "scene/main/scene_tree.h"

#define safe_delete(pPtr) (memdelete(pPtr), pPtr = nullptr)
#define newref(pClass) Ref<pClass>(memnew(pClass))

#ifdef DEBUG_ENABLED
# define DEBUG_PRINT(pText) print_line(pText)
#else
# define DEBUG_PRINT(pText)
#endif

#ifdef DEBUG_ENABLED

// Bind constant with custom name
#define BIND_ENUM_CONSTANT_CUSTOM(pConstant, pName) \
	ClassDB::bind_integer_constant(get_class_static(), __constant_get_enum_name(pConstant, pName), pName, ((int)(pConstant)));

#else

// Bind constant with custom name
#define BIND_ENUM_CONSTANT_CUSTOM(pConstant, pName) \
	ClassDB::bind_integer_constant(get_class_static(), StringName(), pName, ((int)(pConstant)));

#endif // DEBUG_ENABLED

static inline void _trace(int line, const char *file, const String &text) {
	OS::get_singleton()->print("%s", text.utf8().get_data());
}
#define TRACE(text, ...) _trace(__LINE__, __FILE__, vformat(text, __VA_ARGS__))

#define _register_global_ref(pRef)                        \
	{                                                     \
		if (SceneTree *sc = SceneTree::get_singleton()) { \
			sc->add_exit_callback([=]() {                 \
				pRef.unref();                             \
			});                                           \
		}                                                 \
	}

static const Vector2 ONE = Vector2(1, 1);
static const Vector2 ZERO = Vector2(0, 0);

#endif // GD_CORE_H
