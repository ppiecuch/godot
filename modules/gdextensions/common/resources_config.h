/*************************************************************************/
/*  resources_config.h                                                   */
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

#ifndef RESOURCES_H
#define RESOURCES_H

#include "core/reference.h"
#include "core/resource.h"
#include "core/variant.h"

#include <map>

struct ObjectNode {
	typedef std::map<String, ObjectNode> Attribs;

	String name;
	String value;
	Attribs attribs;

	ObjectNode(const String &p_name = "", const String &p_value = "", const Attribs &p_attribs = Attribs()) :
			name(p_name), value(p_value), attribs(p_attribs) {}
	const ObjectNode &get(const String &p_res_name);
};

struct ObjectConfig {
	static ObjectNode load_config_file(const String &p_file, String &error_string);
	static Variant load_json_file(const String &p_file);
	static void _dump(const ObjectNode &p_node, int level = 0);
};

class Resources : public Object {
	GDCLASS(Resources, Object);

	ObjectNode config_root;
	bool loaded;

	std::map<String, std::pair<String, String>> _resources_loaded;

	void load_config();

protected:
	static Resources *instance;

public:
	static Resources *get_singleton();
	RES get_resource(const String &p_res_name);

	Resources();
	~Resources();
};

#endif // RESOURCES_H
