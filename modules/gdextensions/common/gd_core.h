/**************************************************************************/
/*  gd_core.h                                                             */
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

#ifndef GD_CORE_H
#define GD_CORE_H

#include "common/gd_core_defs.h"
#include "core/array.h"
#include "core/bind/core_bind.h"
#include "core/class_db.h"
#include "core/engine.h"
#include "core/error_macros.h"
#include "core/list.h"
#include "core/math/vector2.h"
#include "core/os/os.h"
#include "core/print_string.h"
#include "core/ustring.h"
#include "scene/resources/font.h"

#include <algorithm>
#include <deque>
#include <functional>
#include <list>
#include <ostream>
#include <vector>

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

#define safe_delete(pPtr) (memdelete(pPtr), pPtr = nullptr)
#define newref(pClass, ...) Ref<pClass>(memnew(pClass(__VA_ARGS__)))
#define nullref(pClass) Ref<pClass>()
#define selfref(pClass) Ref<pClass>(this)

String string_ellipsis(const Ref<Font> &p_font, const String &p_text, real_t p_max_width);
String string_format(const char *p_format, ...);
String string_format(const char *p_format, va_list p_list);
String array_concat(const Array &p_args);
#define vconcat(...) array_concat(array(__VA_ARGS__))
#define printf_line(format, ...) print_line(string_format(format, ##__VA_ARGS__))
#define printf_verbose(format, ...) print_verbose(string_format(format, ##__VA_ARGS__))
#define printv_line(...) print_line(array_concat(array(__VA_ARGS__)))
#define printv_verbose(...) print_verbose(array_concat(array(__VA_ARGS__)))

#ifdef DEBUG_ENABLED
#define DEBUG_PRINT(pText) print_line(pText)
#define DEBUG_VAR(pVar) print_line(vformat("%s: %s", #pVar, pVar))
#else
#define DEBUG_PRINT(pText)
#define DEBUG_VAR(pVar)
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

static _FORCE_INLINE_ Error store_var(const Variant &var, const String &file_name, bool full_objects = true) {
	Ref<_File> file = memnew(_File);
	Error open = file->open(file_name, _File::WRITE);
	ERR_FAIL_COND_V(open != OK, open);
	file->store_var(var, full_objects);
	return OK;
}

static _FORCE_INLINE_ Variant load_var(const String &file_name, bool allow_objects = true) {
	Ref<_File> file = memnew(_File);
	Error open = file->open(file_name, _File::READ);
	ERR_FAIL_COND_V(open != OK, open);
	return file->get_var(allow_objects);
}

static _FORCE_INLINE_ uint64_t gd_file_size(const String &file_name) {
	Ref<_File> file = memnew(_File);
	Error open = file->open(file_name, _File::READ);
	ERR_FAIL_COND_V(open != OK, 0);
	return file->get_len();
}

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

/// Color support
_FORCE_INLINE_ static uint8_t g_red(uint32_t rgb) { return ((rgb >> 16) & 0xff); }
_FORCE_INLINE_ static uint8_t g_green(uint32_t rgb) { return ((rgb >> 8) & 0xff); }
_FORCE_INLINE_ static uint8_t g_blue(uint32_t rgb) { return (rgb & 0xff); }
_FORCE_INLINE_ static uint8_t g_alpha(uint32_t rgb) { return rgb >> 24; }
_FORCE_INLINE_ static uint32_t g_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { return ((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff); }
_FORCE_INLINE_ static uint8_t g_gray(uint8_t r, uint8_t g, uint8_t b) { return (r * 11 + g * 16 + b * 5) / 32; } // convert R,G,B to gray 0..255

// Passthrough Script for dynamic scripting

class PassthroughScriptInstance : public PlaceHolderScriptInstance {
public:
	virtual Variant call(const StringName &p_method, const Variant **p_args, int p_argcount, Variant::CallError &r_error) {
		return get_owner()->call(p_method, p_args, p_argcount, r_error);
	}
};

template <typename InstanceBaseClass, typename Receiver>
class PassthroughScript : public Script {
	Receiver *recv;

#ifdef TOOLS_ENABLED
	Set<PlaceHolderScriptInstance *> placeholders;
	virtual void _placeholder_erased(PlaceHolderScriptInstance *p_placeholder) {
		placeholders.erase(p_placeholder);
	}
#endif

public:
	void remove_instance(Object *p_object) {}
	virtual bool can_instance() const { return false; }

	virtual StringName get_instance_base_type() const { return InstanceBaseClass::get_class_static(); }
	virtual ScriptInstance *instance_create(Object *p_this) { return nullptr; }
	virtual bool instance_has(const Object *p_this) const { return false; }

	virtual PlaceHolderScriptInstance *placeholder_instance_create(Object *p_this) {
#ifdef TOOLS_ENABLED
		PlaceHolderScriptInstance *sins = memnew(PlaceHolderScriptInstance(nullptr, Ref<Script>(this), recv));
		placeholders.insert(sins);
		return sins;
#else
		return nullptr;
#endif
	}

	virtual bool has_source_code() const { return false; }
	virtual String get_source_code() const { return ""; }
	virtual void set_source_code(const String &p_code) {}
	virtual Error reload(bool p_keep_state = false) { return OK; }

	virtual bool is_tool() const { return false; }
	virtual bool is_valid() const { return true; }

	virtual bool inherits_script(const Ref<Script> &p_script) const { return false; }

	virtual String get_node_type() const { return ""; }
	virtual ScriptLanguage *get_language() const { return nullptr; }

	// Be carfull with possible recursion

	virtual Ref<Script> get_base_script() const { return Ref<Script>(); }
	virtual bool has_method(const StringName &p_method) const { return recv->has_method(p_method); }
	virtual MethodInfo get_method_info(const StringName &p_method) const {
		List<MethodInfo> methods;
		recv->get_method_list(&methods);
		for (List<MethodInfo>::Element *E = methods.front(); E; E = E->next()) {
			if (E->get().name == p_method) {
				return E->get();
			}
		}
		return MethodInfo();
	}
	virtual bool has_script_signal(const StringName &p_signal) const { return recv->has_signal(p_signal); }
	virtual void get_script_signal_list(List<MethodInfo> *r_signals) const { recv->get_signal_list(r_signals); }
	virtual bool get_property_default_value(const StringName &p_property, Variant &r_value) const { return false; }
	virtual void get_script_method_list(List<MethodInfo> *p_list) const { recv->get_method_list(p_list); }
	virtual void get_script_property_list(List<PropertyInfo> *p_list) const { recv->get_property_list(p_list); }
	virtual void update_exports() {}

	void set_receiver(Receiver *p_recv) { recv = p_recv; }
	Receiver *get_receiver() const { return recv; }
};

#endif // GD_CORE_H
