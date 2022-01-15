/*************************************************************************/
/*  shader_code_parser.h                                                 */
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

// See the original implementation: https://code.google.com/p/nya-engine/

#pragma once

#include <map>
#include <string>
#include <vector>

class shader_code_parser {
public:
	const char *get_code() const { return m_code.c_str(); }
	const char *get_error() const { return m_error.c_str(); }

public:
	bool convert_to_hlsl();
	bool convert_to_glsl(); //resolves extensions
	bool convert_to_glsl_es2(const char *precision = "mediump");
	bool convert_to_glsl3();

public:
	enum variable_type {
		type_invalid,
		type_float,
		type_bool,
		type_int,
		type_vec2,
		type_vec3,
		type_vec4,
		type_ivec2,
		type_ivec3,
		type_ivec4,
		type_mat2,
		type_mat3,
		type_mat4,
		type_sampler2d,
		type_sampler_cube
	};

	struct variable {
		variable_type type;
		std::string name;
		union {
			unsigned int array_size, idx;
		};

		variable() :
				type(type_invalid), array_size(0) {}
		variable(variable_type type, const char *name, unsigned int array_size) :
				type(type), name(name ? name : ""), array_size(array_size) {}
		bool operator<(const variable &v) const { return name < v.name; }
	};

	int get_uniforms_count();
	variable get_uniform(int idx) const;

	int get_attributes_count();
	variable get_attribute(int idx) const;

public:
	bool fix_per_component_functions();

public:
	shader_code_parser(const char *text, const char *replace_prefix_str = "_gl_", const char *flip_y_uniform = nullptr) :
			m_code(text ? text : ""), m_replace_str(replace_prefix_str ? replace_prefix_str : ""), m_flip_y_uniform(flip_y_uniform ? flip_y_uniform : "") { remove_comments(); }

private:
	void remove_comments();

	bool parse_uniforms(bool remove);
	bool parse_predefined_uniforms(const char *replace_prefix_str, bool replace);
	bool parse_attributes(const char *info_replace_str, const char *code_replace_str);
	bool parse_varying(bool remove);

	bool replace_main_function_header(const char *replace_str);
	bool replace_vec_from_float(const char *func_name);
	bool replace_hlsl_types();
	bool replace_hlsl_mul(const char *func_name);
	bool replace_variable(const char *from, const char *to, size_t start_pos = 0);
	bool replace_precision();
	bool find_variable(const char *str, size_t start_pos = 0);

private:
	std::string m_code, m_replace_str, m_flip_y_uniform, m_error;
	std::vector<variable> m_uniforms, m_attributes, m_varying;
};
