#ifndef GUI_SCENE_GENERATOR_H
#define GUI_SCENE_GENERATOR_H

#include "gui_document.h"

class GUISceneGenerator {
	String output;
	int ext_resource_id;
	int node_count;

	void _emit_node(const GUINode &p_node, const String &p_parent_path, const GUIComponent &p_comp);
	void _emit_connections(const GUINode &p_node, const String &p_node_path, const String &p_root_name);
	String _variant_to_tscn(const Variant &p_value) const;

public:
	String generate(const GUIDocument &p_doc);
};

#endif // GUI_SCENE_GENERATOR_H
