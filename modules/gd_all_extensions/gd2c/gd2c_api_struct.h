/*************************************************************************/
/*  gd2c_api_struct.h                                                    */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef __GD2C_API_STRUCT__
#define __GD2C_API_STRUCT__

#include "core/array.h"
#include "gdnative/gdnative.h"

typedef Map<StringName, StringName> gd2c_map_stringname_stringname;
typedef gd2c_map_stringname_stringname gd2c_class_hierarchy;

// This struct must be identical to the client
struct gd2c_api_1_0 {
	int major;
	int minor;

	void GDAPI (*variant_get_named)(const godot_variant *p_self, const godot_string_name *p_name, godot_variant *p_dest, godot_bool *r_error);
	void GDAPI (*variant_set_named)(godot_variant *p_self, const godot_string_name *p_name, const godot_variant *p_value, godot_bool *r_error);
	godot_error GDAPI (*variant_decode)(godot_variant *r_variant, const uint8_t *p_buffer, int p_len, int *r_len, godot_bool p_allow_objects);
	void GDAPI (*resource_load)(godot_variant *r_result, const godot_string *p_path);
	void GDAPI (*variant_convert)(godot_variant *r_result, godot_int variant_type, const godot_variant **p_args, int p_len, godot_variant_call_error *r_error);
	void GDAPI (*object_get_property)(godot_variant *r_result, godot_object *p_instance, godot_string_name *p_index);
	void GDAPI (*object_set_property)(godot_object *p_instance, godot_string_name *p_index, godot_variant *p_value);
	void GDAPI (*call_gdscript_builtin)(int p_func, const godot_variant **p_args, godot_int p_arg_count, godot_variant *r_result, godot_variant_call_error *r_error);
	godot_bool GDAPI (*variant_iter_init)(godot_variant *p_self, godot_variant *r_iter, godot_bool *r_valid);
	godot_bool GDAPI (*variant_iter_next)(godot_variant *p_self, godot_variant *r_iter, godot_bool *r_valid);
	void GDAPI (*variant_iter_get)(godot_variant *p_self, godot_variant *r_iter, godot_variant *r_result, godot_bool *r_valid);
	void GDAPI (*get_gdscript_nativeclass)(godot_variant *r_dest, godot_string_name *p_name);
	void GDAPI (*variant_call)(godot_variant *p_self, godot_string_name *p_method_name, godot_int p_argc, godot_variant **p_args, godot_variant *r_result, godot_variant_call_error *r_error);
	void GDAPI (*extends_test)(godot_variant *p_a, godot_variant *p_b, godot_variant *r_result);
	void GDAPI (*variant_construct)(godot_variant *r_result, godot_variant_type p_type, godot_int p_num_args, const godot_variant **p_args, godot_variant_call_error *r_error);
	void GDAPI (*variant_get)(const godot_variant *p_self, const godot_variant *p_index, godot_variant *p_dest, godot_bool *r_valid);
	void GDAPI (*variant_set)(const godot_variant *p_self, const godot_variant *p_index, const godot_variant *p_value, godot_bool *r_valid);
};

#endif