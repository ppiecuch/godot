/**************************************************************************/
/*  resources_config.cpp                                                  */
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

#ifdef DOCTEST
#include "doctest/doctest.h"
#include "doctest/doctest_godot.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

static const String _default_locations[] = {
	"user://resources.cfg",
	"user://data/resources.cfg",
	"res://resources.cfg",
	"res://data/resources.cfg",
};
static const int _default_locations_count = sizeof(_default_locations) / sizeof(_default_locations[0]);

// Strip comments, respecting quoted strings.
// Handles both single and double quotes: anything inside matching quotes
// is preserved, including '#' characters.
static String _strip_comments(const String &line) {
	bool in_single_quote = false;
	bool in_double_quote = false;
	for (int i = 0; i < line.length(); i++) {
		CharType c = line[i];
		if (c == '\'' && !in_double_quote) {
			in_single_quote = !in_single_quote;
		} else if (c == '"' && !in_single_quote) {
			in_double_quote = !in_double_quote;
		} else if (c == '#' && !in_single_quote && !in_double_quote) {
			return line.substr(0, i);
		}
	}
	return line;
}

Resources *Resources::instance = nullptr;

static const ObjectNode _empty_node;

const ObjectNode &ObjectNode::get(const String &p_res_name) const {
	Vector<String> names = p_res_name.split(".");
	const ObjectNode *node = this;
	for (int n = 0; n < names.size(); n++) {
		const ObjectNode::Attribs::ConstElement e = node->attribs.find(names[n]);
		if (!e) {
			ERR_PRINT("ObjectNode::get: key not found: " + names[n] + " in path: " + p_res_name);
			return _empty_node;
		}
		node = &e.value();
	}
	return *node;
}

// Internal parser shared by load_config_file and load_config_string.
static ObjectNode _parse_config_lines(const Vector<String> &p_lines, String &error_string) {
	ObjectNode root("root", "root");
	Vector<ObjectNode *> nodes;
	nodes.push_back(&root);

	for (int line_num = 0; line_num < p_lines.size(); line_num++) {
		String line = _strip_comments(p_lines[line_num]).strip_edges();
		if (line.empty()) {
			continue;
		}

		int assign = line.find("=");
		if (assign >= 0) {
			String key = line.substr(0, assign).strip_edges();
			if (key.empty()) {
				WARN_PRINT(vformat("Line %d: empty key name, skipping.", line_num + 1));
				continue;
			}
			if (line.ends_with("{")) {
				String type_value = line.substr(assign + 1, line.length() - assign - 2).strip_edges();
				if (nodes[nodes.size() - 1]->attribs.has(key)) {
					WARN_PRINT(vformat("Line %d: duplicate key '%s' in section '%s', overwriting.", line_num + 1, key, nodes[nodes.size() - 1]->name));
				}
				ObjectNode node = ObjectNode(key, type_value);
				nodes[nodes.size() - 1]->attribs.insert(key, node);
				nodes.push_back(&nodes[nodes.size() - 1]->attribs[key]);
			} else {
				String val = line.substr(assign + 1).strip_edges();
				if (nodes[nodes.size() - 1]->attribs.has(key)) {
					WARN_PRINT(vformat("Line %d: duplicate key '%s' in section '%s', overwriting.", line_num + 1, key, nodes[nodes.size() - 1]->name));
				}
				ObjectNode node = ObjectNode(key, val);
				nodes[nodes.size() - 1]->attribs.insert(key, node);
			}
		} else if (line == "}") {
			if (nodes.size() > 1) {
				nodes.resize(nodes.size() - 1);
			} else {
				error_string = vformat("Line %d: unexpected '}' — too many closing braces.", line_num + 1);
				break;
			}
		}
	}

	if (error_string.empty()) {
		if (nodes.size() != 1) {
			error_string = vformat("Unexpected end of input: %d unclosed section(s).", nodes.size() - 1);
		}
	}

	if (!error_string.empty()) {
		ERR_PRINT(error_string);
		return ObjectNode();
	}

	return root;
}

ObjectNode ObjectConfig::load_config_file(const String &p_file, String &error_string) {
	FileAccessRef file = FileAccess::open(p_file, FileAccess::READ);
	if (!file) {
		error_string = "Can not open config file for reading: " + p_file;
		ERR_PRINT(error_string);
		return ObjectNode();
	}

	Vector<String> lines;
	while (!file->eof_reached()) {
		lines.push_back(file->get_line());
	}

	return _parse_config_lines(lines, error_string);
}

ObjectNode ObjectConfig::load_config_string(const String &p_content, String &error_string) {
	Vector<String> lines = p_content.split("\n");
	return _parse_config_lines(lines, error_string);
}

Variant ObjectConfig::load_json_file(const String &p_file) {
	FileAccessRef file = FileAccess::create_for_path(p_file);
	ERR_FAIL_COND_V_MSG(!file, Variant(), "Failed to open file: " + p_file);

	Error err;
	String json_string = file->get_file_as_string(p_file, &err);
	ERR_FAIL_COND_V_MSG(err != OK, Variant(), "Can not read json file: " + p_file);

	String error_string;
	int error_line;
	Variant data;
	err = JSON::parse(json_string, data, error_string, error_line);
	ERR_FAIL_COND_V_MSG(err != OK, Variant(), "Can not parse JSON: " + error_string + " on line " + itos(error_line) + " in " + p_file);

	return data;
}

void ObjectConfig::print_tree(const ObjectNode &p_node, int level) {
	String pad = String(level ? "|" : "+").rpad(level * 2);
	String brn = String("|").rpad(level * 2, "-");
	print_line(vformat((p_node.value == "Section" ? brn : pad) + "%s = %s", p_node.name, p_node.value));
	for (ObjectNode::Attribs::ConstElement e = p_node.attribs.front(); e; e = e.next()) {
		print_tree(e.value(), level + 1);
	}
}

// Resources

void Resources::load_config() {
	for (int i = 0; i < _default_locations_count; i++) {
		const String &p = _default_locations[i];
		if (FileAccess::exists(p)) {
#ifdef DEBUG_ENABLED
			print_line("Trying to load resource file: " + p);
#endif
			String error_line;
			const ObjectNode &root = ObjectConfig::load_config_file(p, error_line);
			if (error_line.empty()) {
				config_root = root;
				loaded = true;
#ifdef DEBUG_ENABLED
				ObjectConfig::print_tree(config_root);
#endif
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
	const OrderedHashMap<String, Pair<String, String>>::Element cached = _resources_loaded.find(p_res_name);
	if (cached) {
		res_path = cached.value().first;
		res_hint = cached.value().second;
	} else {
		const ObjectNode &res = config_root.get(p_res_name);
		res_path = res.name;
		res_hint = res.value;
		if (res.attribs.has("file")) {
			res_path = res.attribs["file"].value;
		}
		ERR_FAIL_COND_V_MSG(res_path.empty(), RES(), "Resource not found: " + p_res_name);
		_resources_loaded.insert(p_res_name, Pair<String, String>(res_path, res_hint));
	}
	return ResourceLoader::load(res_path, res_hint);
}

void Resources::reload_config() {
	config_root = ObjectNode();
	_resources_loaded.clear();
	loaded = false;
	load_config();
}

void Resources::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_resource", "resource_name"), &Resources::get_resource);
	ClassDB::bind_method(D_METHOD("reload_config"), &Resources::reload_config);
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

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

#ifdef DOCTEST

TEST_SUITE("resources_config") {
	// -----------------------------------------------------------------------
	// ObjectNode::get
	// -----------------------------------------------------------------------

	TEST_CASE("ObjectNode::get returns correct node for simple key") {
		ObjectNode root("root", "root");
		root.attribs.insert("alpha", ObjectNode("alpha", "100"));
		root.attribs.insert("beta", ObjectNode("beta", "200"));

		const ObjectNode &n = root.get("alpha");
		CHECK(n.name == "alpha");
		CHECK(n.value == "100");
	}

	TEST_CASE("ObjectNode::get returns correct node for dotted path") {
		ObjectNode root("root", "root");
		ObjectNode child("child", "Section");
		child.attribs.insert("key", ObjectNode("key", "deep_value"));
		root.attribs.insert("parent", child);

		const ObjectNode &n = root.get("parent.key");
		CHECK(n.name == "key");
		CHECK(n.value == "deep_value");
	}

	TEST_CASE("ObjectNode::get returns empty node on missing key") {
		ObjectNode root("root", "root");
		root.attribs.insert("exists", ObjectNode("exists", "yes"));

		EXPECT_ERROR({
			const ObjectNode &n = root.get("missing");
			CHECK(n.name == "");
			CHECK(n.value == "");
		});
	}

	TEST_CASE("ObjectNode::get returns empty node on missing nested key") {
		ObjectNode root("root", "root");
		root.attribs.insert("a", ObjectNode("a", "Section"));

		EXPECT_ERROR({
			const ObjectNode &n = root.get("a.b.c");
			CHECK(n.name == "");
			CHECK(n.value == "");
		});
	}

	TEST_CASE("ObjectNode::get does not mutate tree on missing key") {
		ObjectNode root("root", "root");
		root.attribs.insert("a", ObjectNode("a", "val"));
		int before = root.attribs.size();

		EXPECT_ERROR(root.get("nonexistent"));
		CHECK(root.attribs.size() == before);
	}

	// -----------------------------------------------------------------------
	// Comment stripping
	// -----------------------------------------------------------------------

	TEST_CASE("_strip_comments: basic comment removal") {
		String err;
		ObjectNode root = ObjectConfig::load_config_string("key = value # this is a comment", err);
		CHECK(err.empty());
		CHECK(root.attribs["key"].value == "value");
	}

	TEST_CASE("_strip_comments: hash inside double quotes is preserved") {
		String err;
		ObjectNode root = ObjectConfig::load_config_string("file = \"data/color#red.png\"", err);
		CHECK(err.empty());
		CHECK(root.attribs["file"].value == "\"data/color#red.png\"");
	}

	TEST_CASE("_strip_comments: hash inside single quotes is preserved") {
		String err;
		ObjectNode root = ObjectConfig::load_config_string("file = 'path#with#hashes'", err);
		CHECK(err.empty());
		CHECK(root.attribs["file"].value == "'path#with#hashes'");
	}

	TEST_CASE("_strip_comments: full-line comment produces no keys") {
		String err;
		ObjectNode root = ObjectConfig::load_config_string("# just a comment\nkey = val", err);
		CHECK(err.empty());
		CHECK(root.attribs.size() == 1);
		CHECK(root.attribs.has("key"));
	}

	// -----------------------------------------------------------------------
	// Parser: basic key-value pairs
	// -----------------------------------------------------------------------

	TEST_CASE("Parser: single key-value pair") {
		String err;
		ObjectNode root = ObjectConfig::load_config_string("name = hello", err);
		CHECK(err.empty());
		CHECK(root.attribs.size() == 1);
		CHECK(root.attribs["name"].value == "hello");
	}

	TEST_CASE("Parser: multiple key-value pairs") {
		String err;
		ObjectNode root = ObjectConfig::load_config_string("a = 1\nb = 2\nc = 3", err);
		CHECK(err.empty());
		CHECK(root.attribs.size() == 3);
		CHECK(root.attribs["a"].value == "1");
		CHECK(root.attribs["b"].value == "2");
		CHECK(root.attribs["c"].value == "3");
	}

	TEST_CASE("Parser: whitespace is trimmed from keys and values") {
		String err;
		ObjectNode root = ObjectConfig::load_config_string("  key  =  value  ", err);
		CHECK(err.empty());
		CHECK(root.attribs["key"].value == "value");
	}

	TEST_CASE("Parser: empty lines are ignored") {
		String err;
		ObjectNode root = ObjectConfig::load_config_string("\n\na = 1\n\nb = 2\n\n", err);
		CHECK(err.empty());
		CHECK(root.attribs.size() == 2);
	}

	TEST_CASE("Parser: empty input produces empty root") {
		String err;
		ObjectNode root = ObjectConfig::load_config_string("", err);
		CHECK(err.empty());
		CHECK(root.attribs.size() == 0);
	}

	// -----------------------------------------------------------------------
	// Parser: sections (nested braces)
	// -----------------------------------------------------------------------

	TEST_CASE("Parser: section with children") {
		String err;
		String input = "game = Section {\n  width = 800\n  height = 600\n}";
		ObjectNode root = ObjectConfig::load_config_string(input, err);
		CHECK(err.empty());
		CHECK(root.attribs.has("game"));

		const ObjectNode &game = root.attribs["game"];
		CHECK(game.value == "Section");
		CHECK(game.attribs.size() == 2);
		CHECK(game.attribs["width"].value == "800");
		CHECK(game.attribs["height"].value == "600");
	}

	TEST_CASE("Parser: nested sections") {
		String err;
		String input =
				"outer = Section {\n"
				"  inner = Section {\n"
				"    val = deep\n"
				"  }\n"
				"}";
		ObjectNode root = ObjectConfig::load_config_string(input, err);
		CHECK(err.empty());

		const ObjectNode &deep = root.get("outer.inner.val");
		CHECK(deep.value == "deep");
	}

	TEST_CASE("Parser: multiple sibling sections") {
		String err;
		String input =
				"sec1 = Section {\n"
				"  a = 1\n"
				"}\n"
				"sec2 = Section {\n"
				"  b = 2\n"
				"}";
		ObjectNode root = ObjectConfig::load_config_string(input, err);
		CHECK(err.empty());
		CHECK(root.attribs.size() == 2);
		CHECK(root.get("sec1.a").value == "1");
		CHECK(root.get("sec2.b").value == "2");
	}

	// -----------------------------------------------------------------------
	// Parser: realistic config (matches format from file header comment)
	// -----------------------------------------------------------------------

	TEST_CASE("Parser: realistic game config") {
		String err;
		String input =
				"game = Section {\n"
				"  axe_sprite = SpriteAnimation {\n"
				"    center_x = 16\n"
				"    center_y = 33\n"
				"    div_x = 9\n"
				"    div_y = 1\n"
				"    frame_height = 42\n"
				"    frame_width = 39\n"
				"    surface = game.axe_surf\n"
				"  }\n"
				"  axe_surf = Sprite {\n"
				"    file = data/gfx/pickaxe.png\n"
				"  }\n"
				"}";
		ObjectNode root = ObjectConfig::load_config_string(input, err);
		CHECK(err.empty());

		const ObjectNode &sprite = root.get("game.axe_sprite");
		CHECK(sprite.value == "SpriteAnimation");
		CHECK(sprite.attribs["center_x"].value == "16");
		CHECK(sprite.attribs["div_x"].value == "9");
		CHECK(sprite.attribs["surface"].value == "game.axe_surf");

		const ObjectNode &surf = root.get("game.axe_surf");
		CHECK(surf.value == "Sprite");
		CHECK(surf.attribs["file"].value == "data/gfx/pickaxe.png");
	}

	// -----------------------------------------------------------------------
	// Parser: error handling
	// -----------------------------------------------------------------------

	TEST_CASE("Parser: too many closing braces reports line number") {
		String err;
		String input = "a = 1\n}\n}";
		EXPECT_ERROR(ObjectConfig::load_config_string(input, err));
		CHECK(!err.empty());
		CHECK(err.find("Line 2") >= 0);
	}

	TEST_CASE("Parser: unclosed section reports unclosed count") {
		String err;
		String input = "sec = Section {\n  a = 1\n";
		EXPECT_ERROR(ObjectConfig::load_config_string(input, err));
		CHECK(!err.empty());
		CHECK(err.find("unclosed") >= 0);
	}

	TEST_CASE("Parser: deeply unclosed sections reports correct count") {
		String err;
		String input =
				"a = Section {\n"
				"  b = Section {\n"
				"    c = Section {\n";
		EXPECT_ERROR(ObjectConfig::load_config_string(input, err));
		CHECK(!err.empty());
		CHECK(err.find("3 unclosed") >= 0);
	}

	// -----------------------------------------------------------------------
	// Parser: duplicate key warnings
	// -----------------------------------------------------------------------

	TEST_CASE("Parser: duplicate key overwrites with last value") {
		String err;
		String input = "key = first\nkey = second";
		ObjectNode root;
		EXPECT_ERROR(root = ObjectConfig::load_config_string(input, err));
		CHECK(err.empty()); // no error, just warning
		CHECK(root.attribs["key"].value == "second");
	}

	TEST_CASE("Parser: duplicate section key overwrites") {
		String err;
		String input =
				"sec = Section {\n"
				"  a = 1\n"
				"}\n"
				"sec = Section {\n"
				"  b = 2\n"
				"}";
		ObjectNode root;
		EXPECT_ERROR(root = ObjectConfig::load_config_string(input, err));
		CHECK(err.empty());
		// Second 'sec' overwrites first
		CHECK(root.attribs["sec"].attribs.has("b"));
	}

	// -----------------------------------------------------------------------
	// Parser: edge cases
	// -----------------------------------------------------------------------

	TEST_CASE("Parser: value with equals sign") {
		String err;
		String input = "expr = a=b+c";
		ObjectNode root = ObjectConfig::load_config_string(input, err);
		CHECK(err.empty());
		// First '=' is the delimiter, rest is the value
		CHECK(root.attribs["expr"].value == "a=b+c");
	}

	TEST_CASE("Parser: comment-only lines are skipped") {
		String err;
		String input = "# comment 1\n# comment 2\nkey = val\n# comment 3";
		ObjectNode root = ObjectConfig::load_config_string(input, err);
		CHECK(err.empty());
		CHECK(root.attribs.size() == 1);
		CHECK(root.attribs["key"].value == "val");
	}

	TEST_CASE("Parser: section with typed value") {
		String err;
		String input = "sprite = Texture {\n  file = res://icon.png\n}";
		ObjectNode root = ObjectConfig::load_config_string(input, err);
		CHECK(err.empty());
		CHECK(root.attribs["sprite"].value == "Texture");
		CHECK(root.attribs["sprite"].attribs["file"].value == "res://icon.png");
	}

	TEST_CASE("Parser: only whitespace and comments") {
		String err;
		String input = "   \n  # nothing here\n   \n# more nothing\n";
		ObjectNode root = ObjectConfig::load_config_string(input, err);
		CHECK(err.empty());
		CHECK(root.attribs.size() == 0);
	}

	TEST_CASE("Parser: closing brace with trailing whitespace") {
		String err;
		String input = "sec = Section {\n  a = 1\n  }  ";
		ObjectNode root = ObjectConfig::load_config_string(input, err);
		CHECK(err.empty());
		CHECK(root.attribs["sec"].attribs["a"].value == "1");
	}

	TEST_CASE("Parser: empty key name is skipped") {
		String err;
		String input = " = value";
		ObjectNode root;
		EXPECT_ERROR(root = ObjectConfig::load_config_string(input, err));
		CHECK(err.empty());
		CHECK(root.attribs.size() == 0);
	}

	// -----------------------------------------------------------------------
	// ObjectNode construction
	// -----------------------------------------------------------------------

	TEST_CASE("ObjectNode default construction") {
		ObjectNode node;
		CHECK(node.name == "");
		CHECK(node.value == "");
		CHECK(node.attribs.size() == 0);
	}

	TEST_CASE("ObjectNode construction with args") {
		ObjectNode node("myname", "myvalue");
		CHECK(node.name == "myname");
		CHECK(node.value == "myvalue");
		CHECK(node.attribs.size() == 0);
	}

	// -----------------------------------------------------------------------
	// load_config_string vs load_config_file equivalence
	// -----------------------------------------------------------------------

	TEST_CASE("load_config_string root node has name=root value=root") {
		String err;
		ObjectNode root = ObjectConfig::load_config_string("a = 1", err);
		CHECK(err.empty());
		CHECK(root.name == "root");
		CHECK(root.value == "root");
	}

	// -----------------------------------------------------------------------
	// Complex integration scenario
	// -----------------------------------------------------------------------

	TEST_CASE("Parser: complex multi-level config with comments and quotes") {
		String err;
		String input =
				"# Game resources configuration\n"
				"game = Section {\n"
				"  title = \"My Game #1\" # inline comment\n"
				"  gfx = Section {\n"
				"    player = Sprite {\n"
				"      file = data/gfx/player.png\n"
				"      width = 32\n"
				"      height = 64\n"
				"    }\n"
				"    enemy = Sprite {\n"
				"      file = data/gfx/enemy.png\n"
				"    }\n"
				"  }\n"
				"  snd = Section {\n"
				"    jump = AudioStream {\n"
				"      file = data/sfx/jump.wav\n"
				"    }\n"
				"  }\n"
				"}\n"
				"ui = Section {\n"
				"  font = res://fonts/main.ttf\n"
				"}";
		ObjectNode root = ObjectConfig::load_config_string(input, err);
		CHECK(err.empty());

		// Check structure
		CHECK(root.attribs.size() == 2);

		// Dotted path access
		const ObjectNode &player = root.get("game.gfx.player");
		CHECK(player.value == "Sprite");
		CHECK(player.attribs["file"].value == "data/gfx/player.png");
		CHECK(player.attribs["width"].value == "32");

		const ObjectNode &jump = root.get("game.snd.jump");
		CHECK(jump.value == "AudioStream");
		CHECK(jump.attribs["file"].value == "data/sfx/jump.wav");

		// Title with hash inside quotes is preserved
		const ObjectNode &title = root.get("game.title");
		CHECK(title.value == "\"My Game #1\"");

		// UI section
		const ObjectNode &font = root.get("ui.font");
		CHECK(font.value == "res://fonts/main.ttf");
	}
}

#endif // DOCTEST
