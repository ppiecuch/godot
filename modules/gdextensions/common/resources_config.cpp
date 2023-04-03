/*************************************************************************/
/*  resources_config.cpp                                                 */
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

// game = Section {
//   axe_sprite = SpriteAnimation {
//     center_x = 16
//     center_y = 33
//     div_x = 9
//     div_y = 1
//     frame_height = 42
//     frame_width = 39
//     surface = game.axe_surf
//   }
//   axe_surf = Sprite {
//     file = data/gfx/pickaxe.png
//   }
//   axe_surf = Mesh {
//     file = data/gfx/pickaxe.png
//   }
// }

#include "resources_config.h"

#include "core/io/json.h"
#include "core/io/resource_loader.h"
#include "core/os/file_access.h"

#include <vector>

static const std::vector<String> _default_locations = {
	"user://resources.cfg",
	"user://data/resources.cfg",
	"res://resources.cfg",
	"res://data/resources.cfg",
};

static String _strip_comments(String line) {
	const int comment_pos = line.find("#");
	if (comment_pos >= 0) {
		line = line.substr(0, comment_pos);
	}
	return line;
}

Resources *Resources::instance = nullptr;

const ObjectNode &ObjectNode::get(const String &p_res_name) {
	Vector<String> names = p_res_name.split(".");
	ObjectNode *node = this;
	for (int n = 0; n < names.size(); n++) {
		node = &node->attribs[names[n]];
	}
	return *node;
}

ObjectNode ObjectConfig::load_config_file(const String &p_file, String &error_string) {
	ObjectNode root("root", "root"); // Returns the root node

	FileAccessRef file = FileAccess::open(p_file, FileAccess::READ);
	if (!file) {
		error_string = "Can not open config file for reading.";
	} else {
		std::vector<ObjectNode *> nodes;
		nodes.push_back(&root);

		while (!file->eof_reached()) {
			String line = _strip_comments(file->get_line()).trim();

			int assign = line.find("=");
			if (assign >= 0) {
				if (line.ends_with("{")) {
					ObjectNode node = ObjectNode(line.substr(0, assign).trim(), line.substr(assign + 1, line.length() - assign - 2).trim());
					nodes.back()->attribs[node.name] = node;
					nodes.push_back(&nodes.back()->attribs[node.name]);
				} else {
					ObjectNode node = ObjectNode(line.substr(0, assign).trim(), line.substr(assign + 1).trim());
					nodes.back()->attribs[node.name] = node;
				}
			} else if (line == "}") {
				if (nodes.size() > 1) {
					nodes.pop_back();
				} else {
					error_string = "too many }'s";
					break;
				}
			}
		}

		if (error_string.empty()) {
			if (nodes.size() != 1) {
				error_string = "too few {'s";
			}
		}
	}

	if (!error_string.empty()) {
		ERR_PRINT(error_string);
		return ObjectNode();
	}

	return root;
}

Variant ObjectConfig::load_json_file(const String &p_file) {
	FileAccessRef file = FileAccess::create_for_path(p_file);
	ERR_FAIL_COND_V_MSG(!file, "Failed to open file: " + p_file, Variant());

	Error err;
	String json_string = file->get_file_as_string(p_file, &err);
	ERR_FAIL_COND_V_MSG(err != OK, FAILED, "Can not read json file.");

	String error_string;
	int error_line;
	Variant data;
	err = JSON::parse(json_string, data, error_string, error_line);
	ERR_FAIL_COND_V_MSG(err != OK, FAILED, String("Can not parse JSON ") + error_string + " on line " + rtos(error_line) + ".");

	return data;
}

void ObjectConfig::_dump(const ObjectNode &p_node, int level) {
	String pad = String(level ? "|" : "+").rpad(level * 2);
	String brn = String("|").rpad(level * 2, "-");
	print_line(vformat((p_node.value == "Section" ? brn : pad) + "%s = %s", p_node.name, p_node.value));
	for (const auto &n : p_node.attribs) {
		_dump(n.second, level + 1);
	}
}

// Resources

void Resources::load_config() {
	for (const auto &p : _default_locations) {
		if (FileAccess::exists(p)) {
#ifdef DEBUG_ENABLED
			print_line("Trying to load resource file: " + p);
#endif
			String error_line;
			const ObjectNode &root = ObjectConfig::load_config_file(p, error_line);
			if (error_line.empty()) {
				config_root = root;
				loaded = true;
				ObjectConfig::_dump(config_root);
				break;
			} else {
#ifdef DEBUG_ENABLED
				print_line("Failed to load/parse resources: " + error_line);
#endif
			}
		}
	}
}

RES Resources::get_resource(const String &p_res_name) {
	if (!loaded) {
		load_config();
	}
	ERR_FAIL_COND_V_MSG(!loaded, RES(), "Resources configuration file not loaded.");
	String res_path, res_hint;
	if (_resources_loaded.count(p_res_name)) {
		res_path = _resources_loaded[p_res_name].first;
		res_hint = _resources_loaded[p_res_name].second;
	} else {
		const ObjectNode &res = config_root.get(p_res_name);
		res_path = res.name;
		res_hint = res.value;
		if (res.attribs.count("file")) {
			res_path = res.attribs.at("file").value;
		}
		ERR_FAIL_COND_V_MSG(res_path.empty(), RES(), "Resource not found.");
		_resources_loaded[p_res_name] = std::make_pair(res_path, res_hint);
	}
	return ResourceLoader::load(res_path, res_hint);
}

Resources *Resources::get_singleton() {
	return instance;
}

Resources::Resources() {
	ERR_FAIL_COND_MSG(instance != nullptr, "Singleton already exists");
	loaded = false;
	instance = this;
}

Resources::~Resources() {
	instance = nullptr;
}
