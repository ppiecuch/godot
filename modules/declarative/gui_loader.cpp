#include "gui_loader.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#include "core/class_db.h"
#include "core/os/file_access.h"
#include "core/project_settings.h"
#include "scene/main/node.h"

Error GUILoader::_parse_source(const String &p_source, GUIDocument &r_doc) {
	GUITokenizer tokenizer;
	Error err = tokenizer.tokenize(p_source);
	if (err != OK) {
		last_error = "Tokenizer: " + tokenizer.get_error();
		return err;
	}

	GUIParser parser;
	err = parser.parse(tokenizer.get_tokens(), r_doc);
	if (err != OK) {
		last_error = "Parser: " + parser.get_error();
		return err;
	}

	return OK;
}

Node *GUILoader::_build_node_tree(const GUINode &p_node, Node *p_root) const {
	Node *node = nullptr;

	if (p_node.is_children_slot) {
		// Create a placeholder Control for the children slot.
		node = Object::cast_to<Node>(ClassDB::instance("Control"));
		if (node) {
			node->set_name("_slot");
			node->set_meta("_is_gui_slot", true);
		}
		return node;
	}

	// Instantiate the node by class name.
	node = Object::cast_to<Node>(ClassDB::instance(p_node.type));
	if (!node) {
		ERR_PRINT("GUILoader: Unknown class '" + p_node.type + "'");
		return nullptr;
	}

	String name = p_node.name.empty() ? p_node.type : p_node.name;
	node->set_name(name);

	// Set properties.
	for (int i = 0; i < p_node.properties.size(); i++) {
		const GUIProperty &prop = p_node.properties[i];
		if (prop.is_binding()) {
			// Simple bindings: if it's "root.property_name", resolve from root's meta or properties.
			// For Phase 1, store as metadata for later resolution.
			node->set_meta("_bind_" + prop.name, prop.binding_expr);
		} else {
			node->set(prop.name, prop.value);
		}
	}

	// Wire signal connections.
	Node *root = p_root ? p_root : node;
	for (int i = 0; i < p_node.connections.size(); i++) {
		const GUISignalConnection &conn = p_node.connections[i];
		Node *target = root; // "root" target
		if (conn.target != "root" && conn.target != "self") {
			// Try to find named node — deferred until tree is complete.
			// For now, default to root.
			target = root;
		}
		// Defer connection — the target method may be defined in a script attached later.
		node->connect(conn.signal_name, target, conn.method, Vector<Variant>(), Object::CONNECT_DEFERRED);
	}

	// Build children recursively.
	for (int i = 0; i < p_node.children.size(); i++) {
		Node *child = _build_node_tree(p_node.children[i], root);
		if (child) {
			node->add_child(child);
		}
	}

	return node;
}

String GUILoader::compile_to_scene(const String &p_source) {
	last_error = "";
	GUIDocument doc;
	Error err = _parse_source(p_source, doc);
	if (err != OK)
		return "";

	GUISceneGenerator gen;
	return gen.generate(doc);
}

Node *GUILoader::load_file(const String &p_path) {
	last_error = "";

	// Read file.
	String resolved_path = p_path;
	if (p_path.begins_with("res://")) {
		resolved_path = ProjectSettings::get_singleton()->globalize_path(p_path);
	}

	FileAccess *f = FileAccess::open(p_path, FileAccess::READ);
	if (!f) {
		last_error = "Cannot open file: " + p_path;
		return nullptr;
	}
	String source = f->get_as_utf8_string();
	f->close();
	memdelete(f);

	return load_source(source);
}

Node *GUILoader::load_source(const String &p_source) {
	last_error = "";
	GUIDocument doc;
	Error err = _parse_source(p_source, doc);
	if (err != OK)
		return nullptr;

	if (doc.components.empty()) {
		last_error = "No components in document";
		return nullptr;
	}

	// Build the first component's node tree.
	const GUIComponent &comp = doc.components[0];
	Node *root = _build_node_tree(comp.root, nullptr);

	if (!root) {
		last_error = "Failed to build node tree";
		return nullptr;
	}

	// If a script path is specified, try to load and attach it.
	if (!doc.script_path.empty()) {
		Ref<Script> script = ResourceLoader::load(doc.script_path);
		if (script.is_valid()) {
			root->set_script(script.get_ref_ptr());
		} else {
			WARN_PRINT("GUILoader: Could not load script '" + doc.script_path + "'");
		}
	}

	// Add custom signals from component declarations.
	// Note: In GDScript, signals are typically declared in the script.
	// For runtime-created nodes without scripts, we store signal info as metadata.
	for (int i = 0; i < comp.signal_decls.size(); i++) {
		const GUISignalDecl &sig = comp.signal_decls[i];
		root->set_meta("_signal_" + sig.name, true);
	}

	return root;
}

void GUILoader::_bind_methods() {
	ClassDB::bind_method(D_METHOD("compile_to_scene", "source"), &GUILoader::compile_to_scene);
	ClassDB::bind_method(D_METHOD("load_file", "path"), &GUILoader::load_file);
	ClassDB::bind_method(D_METHOD("load_source", "source"), &GUILoader::load_source);
	ClassDB::bind_method(D_METHOD("get_last_error"), &GUILoader::get_last_error);
}
