#ifndef GUI_DOCUMENT_H
#define GUI_DOCUMENT_H

#include "core/ustring.h"
#include "core/variant.h"
#include "core/vector.h"

// Intermediate representation for parsed .gui files.

struct GUIProperty {
	String name;
	Variant value; // resolved literal value
	String binding_expr; // non-empty if this is a binding (e.g. "root.title")

	bool is_binding() const { return !binding_expr.empty(); }
};

struct GUISignalConnection {
	String signal_name; // e.g. "pressed"
	String target; // e.g. "root", "parent", or node ref
	String method; // e.g. "on_click"
};

struct GUISignalDecl {
	String name;
	Vector<Pair<String, String>> args; // (name, type) pairs
};

struct GUIPropertyDecl {
	enum Visibility {
		VIS_PRIVATE,
		VIS_IN,
		VIS_OUT,
		VIS_IN_OUT,
	};
	Visibility visibility;
	String type;
	String name;
	Variant default_value;
};

struct GUINode {
	String type; // Godot class name: "Button", "VBoxContainer", etc.
	String name; // from #name syntax, or auto-generated
	Vector<GUIProperty> properties;
	Vector<GUISignalConnection> connections;
	Vector<GUINode> children;
	bool is_children_slot; // true if this is an @children marker

	GUINode() :
			is_children_slot(false) {}
};

struct GUIComponent {
	String name; // component name
	String inherits; // base Godot class
	Vector<GUIPropertyDecl> property_decls;
	Vector<GUISignalDecl> signal_decls;
	GUINode root; // root node (type == inherits)
};

struct GUIDocument {
	enum Target {
		TARGET_SCENE,
		TARGET_GDSCRIPT,
		TARGET_RUNTIME,
	};
	Target target;
	String script_path;
	Vector<GUIComponent> components;

	GUIDocument() :
			target(TARGET_SCENE) {}
};

#endif // GUI_DOCUMENT_H
