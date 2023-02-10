/**************************************************************************/
/*  gdscript_function.h                                                   */
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

#ifndef GDSCRIPT_FUNCTION_H
#define GDSCRIPT_FUNCTION_H

#include "core/os/thread.h"
#include "core/pair.h"
#include "core/reference.h"
#include "core/script_language.h"
#include "core/self_list.h"
#include "core/string_name.h"
#include "core/variant.h"

class GDScriptInstance;
class GDScript;

struct GDScriptDataType {
	bool has_type;
	enum {
		UNINITIALIZED,
		BUILTIN,
		NATIVE,
		SCRIPT,
		GDSCRIPT,
	} kind;
	Variant::Type builtin_type;
	StringName native_type;
	Script *script_type;
	Ref<Script> script_type_ref;

	bool is_type(const Variant &p_variant, bool p_allow_implicit_conversion = false) const {
		if (!has_type) {
			return true; // Can't type check
		}

		switch (kind) {
			case UNINITIALIZED:
				break;
			case BUILTIN: {
				Variant::Type var_type = p_variant.get_type();
				bool valid = builtin_type == var_type;
				if (!valid && p_allow_implicit_conversion) {
					valid = Variant::can_convert_strict(var_type, builtin_type);
				}
				return valid;
			} break;
			case NATIVE: {
				if (p_variant.get_type() == Variant::NIL) {
					return true;
				}
				if (p_variant.get_type() != Variant::OBJECT) {
					return false;
				}

				Object *obj = p_variant.operator Object *();
				if (!obj) {
					return false;
				}

				if (!ClassDB::is_parent_class(obj->get_class_name(), native_type)) {
					// Try with underscore prefix
					StringName underscore_native_type = "_" + native_type;
					if (!ClassDB::is_parent_class(obj->get_class_name(), underscore_native_type)) {
						return false;
					}
				}
				return true;
			} break;
			case SCRIPT:
			case GDSCRIPT: {
				if (p_variant.get_type() == Variant::NIL) {
					return true;
				}
				if (p_variant.get_type() != Variant::OBJECT) {
					return false;
				}

				Object *obj = p_variant.operator Object *();
				if (!obj) {
					return false;
				}

				Ref<Script> base = obj && obj->get_script_instance() ? obj->get_script_instance()->get_script() : nullptr;
				bool valid = false;
				while (base.is_valid()) {
					if (base == script_type) {
						valid = true;
						break;
					}
					base = base->get_base_script();
				}
				return valid;
			} break;
		}
		return false;
	}

	operator PropertyInfo() const {
		PropertyInfo info;
		if (has_type) {
			switch (kind) {
				case UNINITIALIZED:
					break;
				case BUILTIN: {
					info.type = builtin_type;
				} break;
				case NATIVE: {
					info.type = Variant::OBJECT;
					info.class_name = native_type;
				} break;
				case SCRIPT:
				case GDSCRIPT: {
					info.type = Variant::OBJECT;
					info.class_name = script_type->get_instance_base_type();
				} break;
			}
		} else {
			info.type = Variant::NIL;
			info.usage |= PROPERTY_USAGE_NIL_IS_VARIANT;
		}
		return info;
	}

	GDScriptDataType() :
			has_type(false),
			kind(UNINITIALIZED),
			builtin_type(Variant::NIL),
			script_type(nullptr) {}
};

class GDScriptFunction {
public:
	enum Opcode {
		/*  0 */ OPCODE_OPERATOR,
		/*  1 */ OPCODE_EXTENDS_TEST,
		/*  2 */ OPCODE_IS_BUILTIN,
		/*  3 */ OPCODE_SET,
		/*  4 */ OPCODE_GET,
		/*  5 */ OPCODE_SET_NAMED,
		/*  6 */ OPCODE_GET_NAMED,
		/*  7 */ OPCODE_SET_MEMBER,
		/*  8 */ OPCODE_GET_MEMBER,
		/*  9 */ OPCODE_ASSIGN,
		/* 10 */ OPCODE_ASSIGN_TRUE,
		/* 11 */ OPCODE_ASSIGN_FALSE,
		/* 12 */ OPCODE_ASSIGN_TYPED_BUILTIN,
		/* 13 */ OPCODE_ASSIGN_TYPED_NATIVE,
		/* 14 */ OPCODE_ASSIGN_TYPED_SCRIPT,
		/* 15 */ OPCODE_CAST_TO_BUILTIN,
		/* 16 */ OPCODE_CAST_TO_NATIVE,
		/* 17 */ OPCODE_CAST_TO_SCRIPT,
		/* 18 */ OPCODE_CONSTRUCT, //only for basic types!!
		/* 19 */ OPCODE_CONSTRUCT_ARRAY,
		/* 20 */ OPCODE_CONSTRUCT_DICTIONARY,
		/* 21 */ OPCODE_CALL,
		/* 22 */ OPCODE_CALL_RETURN,
		/* 23 */ OPCODE_CALL_BUILT_IN,
		/* 24 */ OPCODE_CALL_SELF,
		/* 25 */ OPCODE_CALL_SELF_BASE,
		/* 26 */ OPCODE_CALL_STACK,
		/* 27 */ OPCODE_CALL_STACK_RETURN,
		/* 28 */ OPCODE_YIELD,
		/* 29 */ OPCODE_YIELD_SIGNAL,
		/* 30 */ OPCODE_YIELD_RESUME,
		/* 31 */ OPCODE_JUMP,
		/* 32 */ OPCODE_JUMP_IF,
		/* 33 */ OPCODE_JUMP_IF_NOT,
		/* 34 */ OPCODE_JUMP_TO_DEF_ARGUMENT,
		/* 35 */ OPCODE_RETURN,
		/* 36 */ OPCODE_ITERATE_BEGIN,
		/* 37 */ OPCODE_ITERATE,
		/* 38 */ OPCODE_ASSERT,
		/* 39 */ OPCODE_BREAKPOINT,
		/* 40 */ OPCODE_LINE,
		/* 41 */ OPCODE_END
	};

	enum Address {
		ADDR_BITS = 24,
		ADDR_MASK = ((1 << ADDR_BITS) - 1),
		ADDR_TYPE_MASK = ~ADDR_MASK,
		ADDR_TYPE_SELF = 0,
		ADDR_TYPE_CLASS = 1,
		ADDR_TYPE_MEMBER = 2,
		ADDR_TYPE_CLASS_CONSTANT = 3,
		ADDR_TYPE_LOCAL_CONSTANT = 4,
		ADDR_TYPE_STACK = 5,
		ADDR_TYPE_STACK_VARIABLE = 6,
		ADDR_TYPE_GLOBAL = 7,
		ADDR_TYPE_NAMED_GLOBAL = 8,
		ADDR_TYPE_FUNCTION = 9,
		ADDR_TYPE_LAMBDA_FUNCTION = 10,
		ADDR_TYPE_NIL = 11
	};

	struct StackDebug {
		int line;
		int pos;
		bool added;
		StringName identifier;
	};

private:
	friend class GDScriptCompiler;
	friend class GDScriptInstance;
	friend class GDScriptLambdaFunctionObject;

	StringName source;

	mutable Variant nil;
	mutable Variant *_constants_ptr;
	int _constant_count;
	const StringName *_global_names_ptr;
	int _global_names_count;
#ifdef TOOLS_ENABLED
	const StringName *_named_globals_ptr;
	int _named_globals_count;
#endif
	const int *_default_arg_ptr;
	int _default_arg_count;
	const int *_code_ptr;
	int _code_size;
	int _argument_count;
	int _stack_size;
	int _call_size;
	int _initial_line;
	bool _static;
	bool _lambda;
	MultiplayerAPI::RPCMode rpc_mode;

	GDScript *_script;

	StringName name;
	Vector<Variant> constants;
	Vector<StringName> global_names;
#ifdef TOOLS_ENABLED
	Vector<StringName> named_globals;
#endif
	Vector<int> default_arguments;
	Vector<int> code;
	Vector<GDScriptDataType> argument_types;
	Vector<int> lambda_variants;
	GDScriptDataType return_type;

#ifdef TOOLS_ENABLED
	Vector<StringName> arg_names;
#endif

	mutable Vector<Variant> cache;
	List<StackDebug> stack_debug;

	_FORCE_INLINE_ Variant *_get_variant(int p_address, GDScriptInstance *p_instance, GDScript *p_script, Variant &self, Variant &static_ref, Variant *p_stack, String &r_error) const;
	_FORCE_INLINE_ String _get_call_error(const Variant::CallError &p_err, const String &p_where, const Variant **argptrs) const;

	friend class GDScriptLanguage;

	SelfList<GDScriptFunction> function_list;
#ifdef DEBUG_ENABLED
	CharString func_cname;
	const char *_func_cname;

	struct Profile {
		StringName signature;
		uint64_t call_count;
		uint64_t self_time;
		uint64_t total_time;
		uint64_t frame_call_count;
		uint64_t frame_self_time;
		uint64_t frame_total_time;
		uint64_t last_frame_call_count;
		uint64_t last_frame_self_time;
		uint64_t last_frame_total_time;
	} profile;

#endif

public:
	struct CallState {
		GDScript *script;
		GDScriptInstance *instance;
#ifdef DEBUG_ENABLED
		StringName function_name;
		String script_path;
#endif
		Vector<uint8_t> stack;
		int stack_size;
		Variant self;
		uint32_t alloca_size;
		int ip;
		int line;
		int defarg;
		Variant result;
	};

	_FORCE_INLINE_ bool is_static() const { return _static; }

	const int *get_code() const; //used for debug
	int get_code_size() const;
	Variant get_constant(int p_idx) const;
	int get_constant_count() const;
	StringName get_global_name(int p_idx) const;
	int get_global_name_count() const;
	StringName get_name() const;
	int get_max_stack_size() const;
	int get_default_argument_count() const;
	int get_default_argument_addr(int p_idx) const;
	GDScriptDataType get_return_type() const;
	GDScriptDataType get_argument_type(int p_idx) const;
	GDScript *get_script() const { return _script; }
	StringName get_source() const { return source; }

	void debug_get_stack_member_state(int p_line, List<Pair<StringName, int>> *r_stackvars) const;

	_FORCE_INLINE_ bool is_empty() const { return _code_size == 0; }

	int get_argument_count() const { return _argument_count; }
	StringName get_argument_name(int p_idx) const {
#ifdef TOOLS_ENABLED
		ERR_FAIL_INDEX_V(p_idx, arg_names.size(), StringName());
		return arg_names[p_idx];
#else
		return StringName();
#endif
	}
	Variant get_default_argument(int p_idx) const {
		ERR_FAIL_INDEX_V(p_idx, default_arguments.size(), Variant());
		return default_arguments[p_idx];
	}

	Variant call(GDScriptInstance *p_instance, const Variant **p_args, int p_argcount, Variant::CallError &r_err, CallState *p_state = nullptr, const Variant **p_requires_args = nullptr);

	_FORCE_INLINE_ MultiplayerAPI::RPCMode get_rpc_mode() const { return rpc_mode; }
	GDScriptFunction();
	~GDScriptFunction();
};

class GDScriptFunctionState : public Reference {
	GDCLASS(GDScriptFunctionState, Reference);
	friend class GDScriptFunction;
	GDScriptFunction *function;
	GDScriptFunction::CallState state;
	Variant _signal_callback(const Variant **p_args, int p_argcount, Variant::CallError &r_error);
	Ref<GDScriptFunctionState> first_state;

	SelfList<GDScriptFunctionState> scripts_list;
	SelfList<GDScriptFunctionState> instances_list;

protected:
	static void _bind_methods();

public:
	bool is_valid(bool p_extended_check = false) const;
	Variant resume(const Variant &p_arg = Variant());

	void _clear_stack();

	GDScriptFunctionState();
	~GDScriptFunctionState();
};

class GDScriptFunctionObject : public Reference {
	GDCLASS(GDScriptFunctionObject, Reference);

	friend class GDScriptInstance;
	friend class GDScriptFunction;
	friend class GDScriptSignalObject;

protected:
	static void _bind_methods();

	GDScriptFunction *function;
	GDScriptInstance *instance;

public:
	_FORCE_INLINE_ virtual bool is_valid() const { return instance && function; }
	virtual Object *get_owner() const;

	_FORCE_INLINE_ virtual StringName get_name() const { return function->get_name(); }
	virtual Variant applyv(const Array p_args);
	Variant _apply(const Variant **p_args, int p_argcount, Variant::CallError &r_error);
	virtual Variant apply(const Variant **p_args, int p_argcount, Variant::CallError &r_error);
	Variant apply(VARIANT_ARG_LIST);
	virtual Variant apply_with(Object *p_target, const Array p_args);

	GDScriptFunctionObject() { instance = nullptr, function = nullptr; }
};

class GDScriptNativeFunctionObject : public GDScriptFunctionObject {
	GDCLASS(GDScriptNativeFunctionObject, GDScriptFunctionObject);

	friend class GDScriptFunction;
	friend class GDScriptInstance;

	ObjectID target_id;
	StringName method_name;

public:
	virtual Object *get_owner() const { return (target_id == 0 ? nullptr : ObjectDB::get_instance(target_id)); }
	_FORCE_INLINE_ virtual bool is_valid() const { return target_id != 0 && ObjectDB::get_instance(target_id); }

	_FORCE_INLINE_ virtual StringName get_name() const { return method_name; }
	virtual Variant apply(const Variant **p_args, int p_argcount, Variant::CallError &r_error);
	virtual Variant apply_with(Object *p_target, const Array p_args);

	GDScriptNativeFunctionObject() { target_id = 0; }
};

class GDScriptLambdaFunctionObject : public GDScriptFunctionObject {
	GDCLASS(GDScriptLambdaFunctionObject, GDScriptFunctionObject);

	friend class GDScriptInstance;

	Vector<Variant> variants;

public:
	virtual Variant apply(const Variant **p_args, int p_argcount, Variant::CallError &r_error);
	virtual Variant apply_with(Object *p_target, const Array p_args);

	~GDScriptLambdaFunctionObject();
};

#endif // GDSCRIPT_FUNCTION_H
