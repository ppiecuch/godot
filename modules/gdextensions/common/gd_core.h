/*************************************************************************/
/*  gd_core.h                                                            */
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

#ifndef GD_CORE_H
#define GD_CORE_H

#include "core/array.h"
#include "core/class_db.h"
#include "core/engine.h"
#include "core/error_macros.h"
#include "core/list.h"
#include "core/math/vector2.h"
#include "core/os/os.h"
#include "core/ustring.h"

#include <algorithm>
#include <deque>
#include <functional>
#include <list>
#include <ostream>
#include <vector>

// Architecture
#define GD_ARCH_32BIT 0
#define GD_ARCH_64BIT 0

#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(__64BIT__) || defined(__mips64) || defined(__powerpc64__) || defined(__ppc64__) || defined(__LP64__)
#undef GD_ARCH_64BIT
#define GD_ARCH_64BIT 64
#else
#undef GD_ARCH_32BIT
#define GD_ARCH_32BIT 32
#endif //

// C++ variants
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#define CPP17
#endif
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201402L) || __cplusplus >= 201402L)
#define CPP14
#endif
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201103L) || __cplusplus >= 201103L)
#define CPP11
#endif

#ifndef _HAS_EXCEPTIONS
#if defined(__has_feature)
#if __has_feature(cxx_exceptions)
#define _HAS_EXCEPTIONS
#endif
#endif
#ifndef _HAS_EXCEPTIONS
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || (defined(_MSC_VER) && defined(_CPPUNWIND))
#define _HAS_EXCEPTIONS
#endif
#endif
#endif // _HAS_EXCEPTIONS

#ifdef _HAS_EXCEPTIONS
#define ERR_THROW(_E) throw _E
#define ERR_THROW_V(_E, _V) throw _E
#else
#define ERR_THROW(_E)                                          \
	{                                                          \
		ERR_PRINT(vformat("Exception thrown: %s", _E.what())); \
		return;                                                \
	}
#define ERR_THROW_V(_E, _V)                                    \
	do {                                                       \
		ERR_PRINT(vformat("Exception thrown: %s", _E.what())); \
		return _V;                                             \
	} while (0)
#endif

#if TOOLS_ENABLED
#define IN_EDITOR (Engine::get_singleton()->is_editor_hint() || OS::get_singleton()->is_no_window_mode_enabled())
#else
#define IN_EDITOR (false)
#endif

#define safe_delete(pPtr) (memdelete(pPtr), pPtr = nullptr)
#define newref(pClass, ...) Ref<pClass>(memnew(pClass(__VA_ARGS__)))
#define nullref(pClass) Ref<pClass>()

String string_format(const char *p_format, ...);
String array_concat(const Array &p_args);
#define vconcat(...) array_concat(array(__VA_ARGS__))

#ifdef DEBUG_ENABLED
#define DEBUG_PRINT(pText) print_line(pText)
#define DEBUG_VAR(pVar) print_line(vformat("%s: %s", #pVar, pVar))
#else
#define DEBUG_PRINT(pText)
#define DEBUG_VAR(pVar)
#endif

#ifndef _DEPRECATED
#if (__GNUC__ >= 4) /* technically, this arrived in gcc 3.1, but oh well. */
#define _DEPRECATED __attribute__((deprecated))
#else
#define _DEPRECATED
#endif
#endif

#ifndef _UNUSED
#ifdef __GNUC__
#define _UNUSED __attribute__((unused))
#else
#define _UNUSED
#endif
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
			sc->add_exit_callback([&]() {                 \
				pRef.unref();                             \
			});                                           \
		} else {                                          \
			WARN_PRINT("Cannot register exit callback."); \
		}                                                 \
	}

#define for_all(range, func) \
	std::for_each(std::begin(range), std::end(range), [this](decltype(range)::value_type &e) { func(e); })

#define call_method(range, func) \
	std::for_each(std::begin(range), std::end(range), [&](decltype(range)::value_type &e) { e.func; })
#define call_method_r(range, func) \
	std::for_each(std::begin(range), std::end(range), [&](decltype(range)::value_type &e) { e->func; })

namespace gd {
template <bool B>
struct isEnabled {
	static constexpr bool value = B; // Template for avoiding MSVC: C4127: conditional expression is constant
};

inline constexpr bool ignoreC4127(bool _x) {
	return _x;
}
} //namespace gd

#if _MSC_VER
#define _GD_IGNORE_C4127(_x) gd::ignoreC4127(!!(_x))
#else
#define _GD_IGNORE_C4127(_x) (!!(_x))
#endif

#define GD_ENABLED(_x) _GD_IGNORE_C4127(gd::isEnabled<!!(_x)>::value)

template <typename T>
List<T> array_to_list(const Array &p_array) {
	List<T> ret;
	for (int i = 0; i < p_array.size(); i++) {
		ret.push_back(p_array[i]);
	}
	return ret;
}

template <typename T>
Array list_to_array(const List<T> &p_list) {
	Array ret;
	for (const typename List<T>::Element *E = p_list.front(); E; E = E->next()) {
		ret.append(E->get());
	}
	return ret;
}

static const Vector2 ONE = Vector2(1, 1);
static const Vector2 ZERO = Vector2(0, 0);

/// operator<< overload to display the contents of a vector.
template <typename T, typename A>
std::ostream &operator<<(std::ostream &os, const std::vector<T, A> &vec) {
	os << "[";
	std::string comma;
	for (auto &v : vec) {
		os << comma << v;
		comma = ", ";
	}
	os << "]";
	return os;
}

/// operator<< overload to display the contents of a list.
template <typename T, typename A>
std::ostream &operator<<(std::ostream &os, const std::list<T, A> &list) {
	os << "[";
	std::string comma;
	for (auto &v : list) {
		os << comma << v;
		comma = ", ";
	}
	os << "]";
	return os;
}

/// operator<< overload to display the contents of a deque.
template <typename T, typename A>
std::ostream &operator<<(std::ostream &os, const std::deque<T, A> &deq) {
	os << "[";
	std::string comma;
	for (auto &v : deq) {
		os << comma << v;
		comma = ", ";
	}
	os << "]";
	return os;
}

#ifndef CPP14

#include <memory>
#include <type_traits>

namespace std {
template <class T>
struct _Unique_if {
	typedef std::unique_ptr<T> _Single_object;
};

template <class T>
struct _Unique_if<T[]> {
	typedef std::unique_ptr<T[]> _Unknown_bound;
};

template <class T, size_t N>
struct _Unique_if<T[N]> {
	typedef void _Known_bound;
};

template <class T, class... Args>
typename _Unique_if<T>::_Single_object
make_unique(Args &&...args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
};

template <class T>
typename _Unique_if<T>::_Unknown_bound
make_unique(size_t n) {
	typedef typename std::remove_extent<T>::type U;
	return std::unique_ptr<T>(new U[n]());
};

template <class T, class... Args>
typename _Unique_if<T>::_Known_bound
make_unique(Args &&...) = delete;

template <class Iterator>
reverse_iterator<Iterator> make_reverse_iterator(Iterator i) {
	return reverse_iterator<Iterator>(i);
}
} // namespace std
#endif // CPP14

/// make_gd_unique_ptr(ptr)
/// ptr will be deleted using 'memdelete' function
namespace std {
template <typename T>
using gd_unique_ptr = std::unique_ptr<T, std::function<void(T *)>>;
template <typename T>
gd_unique_ptr<T> make_gd_unique_ptr(T *p) {
	return gd_unique_ptr<T>(p, [](T *ptr) { memdelete(ptr); });
}
} //namespace std

#endif // GD_CORE_H
