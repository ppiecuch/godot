/**************************************************************************/
/*  object_db_snapshot.cpp                                                */
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

#include "object_db_snapshot.h"

#include "core/io/json.h"
#include "core/os/file_access.h"
#include "core/os/memory.h"
#include "core/print_string.h"
#include "core/reference.h"
#include "core/resource.h"
#include "core/script_language.h"
#include "scene/main/node.h"

struct _SortByCountDesc {
	bool operator()(const Pair<String, int> &a, const Pair<String, int> &b) const {
		return a.second > b.second;
	}
};

static void _collect_object_ids(Object *p_obj, void *p_user_data) {
	Vector<ObjectID> *ids = static_cast<Vector<ObjectID> *>(p_user_data);
	ids->push_back(p_obj->get_instance_id());
}

Dictionary ObjectDBSnapshot::take_snapshot() {
	// Phase 1: Collect all object IDs while the DB is locked.
	Vector<ObjectID> ids;
	ObjectDB::debug_objects(_collect_object_ids, &ids);

	// Phase 2: Gather info from each object (DB unlocked).
	Array objects_array;

	for (int i = 0; i < ids.size(); i++) {
		Object *obj = ObjectDB::get_instance(ids[i]);
		if (!obj) {
			continue;
		}

		Dictionary obj_dict;
		obj_dict["id"] = ids[i];
		obj_dict["class"] = obj->get_class();

		// Script path.
		ScriptInstance *si = obj->get_script_instance();
		if (si && si->get_script().is_valid()) {
			Ref<Script> script = si->get_script();
			if (script.is_valid()) {
				obj_dict["script_path"] = script->get_path();
			}
		}

		// Reference info.
		Reference *ref = Object::cast_to<Reference>(obj);
		if (ref) {
			obj_dict["is_reference"] = true;
			obj_dict["ref_count"] = ref->reference_get_count();
		}

		// Resource info.
		Resource *res = Object::cast_to<Resource>(obj);
		if (res) {
			String path = res->get_path();
			if (!path.empty()) {
				obj_dict["resource_path"] = path;
			}
		}

		// Node info.
		Node *node = Object::cast_to<Node>(obj);
		if (node) {
			obj_dict["is_node"] = true;
			obj_dict["node_name"] = node->get_name();
			obj_dict["is_inside_tree"] = node->is_inside_tree();
			if (node->is_inside_tree()) {
				obj_dict["node_path"] = String(node->get_path());
			}
			obj_dict["has_parent"] = node->get_parent() != nullptr;
			obj_dict["child_count"] = node->get_child_count();

			// Orphan: not inside tree and no parent.
			if (!node->is_inside_tree() && !node->get_parent()) {
				obj_dict["is_orphan"] = true;
			}
		}

		objects_array.push_back(obj_dict);
	}

	Dictionary snapshot;
	snapshot["timestamp"] = OS::get_singleton()->get_unix_time();
	snapshot["object_count"] = objects_array.size();
	snapshot["mem_usage"] = (uint64_t)Memory::get_mem_usage();
	snapshot["mem_max_usage"] = (uint64_t)Memory::get_mem_max_usage();
	snapshot["objects"] = objects_array;

	return snapshot;
}

Error ObjectDBSnapshot::save_to_file(const String &p_path) {
	Dictionary snapshot = take_snapshot();
	String json = JSON::print(snapshot, "\t", false);

	Error err;
	FileAccess *f = FileAccess::open(p_path, FileAccess::WRITE, &err);
	if (err != OK || !f) {
		ERR_FAIL_V_MSG(err != OK ? err : ERR_CANT_CREATE, "Cannot open file for writing: " + p_path);
	}

	f->store_string(json);
	f->close();
	memdelete(f);

	return OK;
}

void ObjectDBSnapshot::print_summary() {
	Dictionary snapshot = take_snapshot();

	print_line("=== ObjectDB Snapshot Summary ===");
	print_line("Total objects: " + itos(snapshot["object_count"]));
	print_line("Memory usage: " + String::humanize_size((uint64_t)snapshot["mem_usage"]));
	print_line("Memory peak:  " + String::humanize_size((uint64_t)snapshot["mem_max_usage"]));

	// Count objects by class.
	Array objects = snapshot["objects"];
	HashMap<String, int> class_counts;
	int total_nodes = 0;
	int orphan_nodes = 0;
	int total_refs = 0;

	for (int i = 0; i < objects.size(); i++) {
		Dictionary obj = objects[i];
		String cls = obj["class"];

		if (class_counts.has(cls)) {
			class_counts[cls]++;
		} else {
			class_counts[cls] = 1;
		}

		if (obj.has("is_node")) {
			total_nodes++;
			if (obj.has("is_orphan")) {
				orphan_nodes++;
			}
		}
		if (obj.has("is_reference")) {
			total_refs++;
		}
	}

	print_line("Total nodes: " + itos(total_nodes) + " (orphans: " + itos(orphan_nodes) + ")");
	print_line("Total references: " + itos(total_refs));

	// Sort classes by count descending.
	Vector<Pair<String, int>> sorted_classes;
	const String *K = nullptr;
	while ((K = class_counts.next(K))) {
		sorted_classes.push_back(Pair<String, int>(*K, class_counts[*K]));
	}
	sorted_classes.sort_custom<_SortByCountDesc>();

	print_line("\nObjects by class (top 20):");
	int limit = MIN(sorted_classes.size(), 20);
	for (int i = 0; i < limit; i++) {
		print_line("  " + sorted_classes[i].first + ": " + itos(sorted_classes[i].second));
	}
	if (sorted_classes.size() > 20) {
		print_line("  ... and " + itos(sorted_classes.size() - 20) + " more classes");
	}

	print_line("=================================");
}
