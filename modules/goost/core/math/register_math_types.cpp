/**************************************************************************/
/*  register_math_types.cpp                                               */
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

#include "register_math_types.h"

#include "geometry/register_geometry_types.h"
#include "goost_math.h"
#include "random.h"

#include "classes_enabled.gen.h"

namespace goost {

static Ref<Random> _random;
static GoostMath *_math = nullptr;

void register_math_types() {
#ifdef GOOST_GoostMath
	_math = memnew(GoostMath);
	ClassDB::register_class<GoostMath>();
	Engine::get_singleton()->add_singleton(Engine::Singleton("GoostMath", GoostMath::get_singleton()));
#endif
#ifdef GOOST_Random
	_random.instance();
	ClassDB::register_class<Random>();
	Object *random = Object::cast_to<Object>(Random::get_singleton());
	Engine::get_singleton()->add_singleton(Engine::Singleton("Random", random));
#endif

#ifdef GOOST_GEOMETRY_ENABLED
	goost::register_geometry_types();
#endif
}

void unregister_math_types() {
#ifdef GOOST_GoostMath
	if (_math) {
		memdelete(_math);
	}
#endif
#ifdef GOOST_Random
	_random.unref();
#endif
#ifdef GOOST_GEOMETRY_ENABLED
	goost::unregister_geometry_types();
#endif
}

} // namespace goost
