#ifndef GUI_LOADER_H
#define GUI_LOADER_H

#include "core/reference.h"
#include "gui_document.h"
#include "gui_parser.h"
#include "gui_scene_generator.h"
#include "gui_tokenizer.h"

class GUILoader : public Reference {
	GDCLASS(GUILoader, Reference);

	String last_error;

	Error _parse_source(const String &p_source, GUIDocument &r_doc);
	Node *_build_node_tree(const GUINode &p_node, Node *p_root) const;

protected:
	static void _bind_methods();

public:
	// Compile .gui source text to .tscn text.
	String compile_to_scene(const String &p_source);

	// Load .gui file from path and return a live Node tree.
	Node *load_file(const String &p_path);

	// Load .gui source text and return a live Node tree.
	Node *load_source(const String &p_source);

	String get_last_error() const { return last_error; }
};

#endif // GUI_LOADER_H
