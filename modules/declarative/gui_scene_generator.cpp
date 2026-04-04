#include "gui_scene_generator.h"

#include "gui_parser.h"
#include "gui_tokenizer.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

String GUISceneGenerator::_variant_to_tscn(const Variant &p_value) const {
	switch (p_value.get_type()) {
		case Variant::BOOL:
			return p_value.operator bool() ? "true" : "false";
		case Variant::INT:
			return itos(p_value.operator int());
		case Variant::REAL:
			return rtos(p_value.operator real_t());
		case Variant::STRING:
			return "\"" + String(p_value).xml_escape() + "\"";
		default:
			return String(p_value);
	}
}

void GUISceneGenerator::_emit_node(const GUINode &p_node, const String &p_parent_path, const GUIComponent &p_comp) {
	String node_name = p_node.name.empty() ? p_node.type : p_node.name;
	String node_path;

	if (p_parent_path.empty()) {
		// Root node
		node_path = ".";
		output += "\n[node name=\"" + node_name + "\" type=\"" + p_node.type + "\"]\n";
	} else {
		node_path = (p_parent_path == ".") ? node_name : (p_parent_path + "/" + node_name);
		output += "\n[node name=\"" + node_name + "\" type=\"" + p_node.type + "\" parent=\"" + p_parent_path + "\"]\n";
	}

	// Emit properties
	for (int i = 0; i < p_node.properties.size(); i++) {
		const GUIProperty &prop = p_node.properties[i];
		if (prop.is_binding()) {
			// Bindings can't be represented in .tscn directly — emit as metadata comment
			output += "// binding: " + prop.name + " = " + prop.binding_expr + "\n";
		} else {
			output += prop.name + " = " + _variant_to_tscn(prop.value) + "\n";
		}
	}

	// Handle script attachment on root node
	if (p_parent_path.empty() && !p_comp.name.empty()) {
		// If a script is specified at document level, it will be attached via ext_resource
	}

	// Recurse into children
	for (int i = 0; i < p_node.children.size(); i++) {
		const GUINode &child = p_node.children[i];
		if (child.is_children_slot) {
			// Emit a placeholder Control for the slot
			output += "\n[node name=\"_slot\" type=\"Control\" parent=\"" + node_path + "\"]\n";
		} else {
			_emit_node(child, node_path, p_comp);
		}
	}
}

void GUISceneGenerator::_emit_connections(const GUINode &p_node, const String &p_node_path, const String &p_root_name) {
	for (int i = 0; i < p_node.connections.size(); i++) {
		const GUISignalConnection &conn = p_node.connections[i];
		String from_path = p_node_path;

		// Resolve target
		String to_path = "."; // default: root
		if (conn.target != "root") {
			to_path = conn.target;
		}

		output += "\n[connection signal=\"" + conn.signal_name + "\" from=\"" + from_path +
				"\" to=\"" + to_path + "\" method=\"" + conn.method + "\"]\n";
	}

	// Recurse
	for (int i = 0; i < p_node.children.size(); i++) {
		const GUINode &child = p_node.children[i];
		if (child.is_children_slot)
			continue;
		String child_name = child.name.empty() ? child.type : child.name;
		String child_path = (p_node_path == ".") ? child_name : (p_node_path + "/" + child_name);
		_emit_connections(child, child_path, p_root_name);
	}
}

String GUISceneGenerator::generate(const GUIDocument &p_doc) {
	output = "";
	ext_resource_id = 0;
	node_count = 0;

	if (p_doc.components.empty())
		return "";

	// Use the first component as the main scene.
	const GUIComponent &comp = p_doc.components[0];

	// Count load steps (external resources needed)
	int load_steps = 1; // base
	bool has_script = !p_doc.script_path.empty();
	if (has_script)
		load_steps++;

	output += "[gd_scene load_steps=" + itos(load_steps) + " format=2]\n";

	// External resources
	if (has_script) {
		ext_resource_id++;
		output += "\n[ext_resource path=\"" + p_doc.script_path + "\" type=\"Script\" id=" + itos(ext_resource_id) + "]\n";
	}

	// Emit node tree
	_emit_node(comp.root, "", comp);

	// Attach script to root
	if (has_script) {
		// Insert script reference after root node's first line
		// We need to go back and add it — simpler: just append after root properties
		output += "script = ExtResource( 1 )\n";
	}

	// Emit signal connections
	_emit_connections(comp.root, ".", comp.name);

	return output;
}

// --- Helper for scene generator tests ---
#ifdef DOCTEST
static String _compile_gui(const String &p_source) {
	GUITokenizer tk;
	if (tk.tokenize(p_source) != OK)
		return "";
	GUIParser parser;
	GUIDocument doc;
	if (parser.parse(tk.get_tokens(), doc) != OK)
		return "";
	GUISceneGenerator gen;
	return gen.generate(doc);
}

// --- Doctests ---

TEST_CASE("[GUISceneGenerator] minimal scene output") {
	String src = "@target: scene\ncomponent Foo inherits Control {\n}\n";
	String tscn = _compile_gui(src);
	CHECK(tscn.find("[gd_scene") != -1);
	CHECK(tscn.find("format=2") != -1);
	CHECK(tscn.find("[node name=\"Foo\" type=\"Control\"]") != -1);
}

TEST_CASE("[GUISceneGenerator] child node output") {
	String src = "@target: scene\n"
				 "component App inherits VBoxContainer {\n"
				 "  Label {\n"
				 "    text: \"Hello\"\n"
				 "  }\n"
				 "}\n";
	String tscn = _compile_gui(src);
	CHECK(tscn.find("[node name=\"App\" type=\"VBoxContainer\"]") != -1);
	CHECK(tscn.find("[node name=\"Label\" type=\"Label\" parent=\".\"]") != -1);
	CHECK(tscn.find("text = \"Hello\"") != -1);
}

TEST_CASE("[GUISceneGenerator] named node output") {
	String src = "@target: scene\n"
				 "component App inherits Control {\n"
				 "  Button #my_btn {\n"
				 "    text: \"Click\"\n"
				 "  }\n"
				 "}\n";
	String tscn = _compile_gui(src);
	CHECK(tscn.find("[node name=\"my_btn\" type=\"Button\" parent=\".\"]") != -1);
}

TEST_CASE("[GUISceneGenerator] signal connection output") {
	String src = "@target: scene\n"
				 "component App inherits Control {\n"
				 "  Button {\n"
				 "    pressed => root.on_click()\n"
				 "  }\n"
				 "}\n";
	String tscn = _compile_gui(src);
	CHECK(tscn.find("[connection signal=\"pressed\"") != -1);
	CHECK(tscn.find("to=\".\"") != -1);
	CHECK(tscn.find("method=\"on_click\"") != -1);
}

TEST_CASE("[GUISceneGenerator] script ext_resource") {
	String src = "@target: scene\n@script: \"res://my.gd\"\n"
				 "component App inherits Control {\n}\n";
	String tscn = _compile_gui(src);
	CHECK(tscn.find("[ext_resource path=\"res://my.gd\" type=\"Script\"") != -1);
	CHECK(tscn.find("script = ExtResource( 1 )") != -1);
}

TEST_CASE("[GUISceneGenerator] integer property") {
	String src = "@target: scene\n"
				 "component App inherits Control {\n"
				 "  Label { align: 1 }\n"
				 "}\n";
	String tscn = _compile_gui(src);
	CHECK(tscn.find("align = 1") != -1);
}

TEST_CASE("[GUISceneGenerator] boolean property") {
	String src = "@target: scene\n"
				 "component App inherits Control {\n"
				 "  LineEdit { secret: true }\n"
				 "}\n";
	String tscn = _compile_gui(src);
	CHECK(tscn.find("secret = true") != -1);
}

TEST_CASE("[GUISceneGenerator] children slot") {
	String src = "@target: scene\n"
				 "component Card inherits PanelContainer {\n"
				 "  VBoxContainer {\n"
				 "    @children\n"
				 "  }\n"
				 "}\n";
	String tscn = _compile_gui(src);
	CHECK(tscn.find("[node name=\"_slot\" type=\"Control\" parent=\"VBoxContainer\"]") != -1);
}

TEST_CASE("[GUISceneGenerator] nested hierarchy paths") {
	String src = "@target: scene\n"
				 "component App inherits VBoxContainer {\n"
				 "  HBoxContainer {\n"
				 "    Button { text: \"A\" }\n"
				 "  }\n"
				 "}\n";
	String tscn = _compile_gui(src);
	CHECK(tscn.find("parent=\"HBoxContainer\"") != -1);
}

#endif // DOCTEST
