/**************************************************************************/
/*  sprite_mesh.h                                                         */
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

#ifndef GD_SPRITE_MESH_H
#define GD_SPRITE_MESH_H

#include "core/math/vector3.h"
#include "scene/2d/sprite.h"
#include "scene/gui/box_container.h"
#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"

// BEGIN Mesh-light definition and its editor

class SpriteMeshLight : public Node {
	GDCLASS(SpriteMeshLight, Node);

	RID canvas_item;
	bool active;
	bool visible;
	Point2 light_position;
	real_t light_power;
	real_t light_radius;
	Color ambient;
	Color diffuse;
	Color specular;
	bool debug_outline;

	void _update();

protected:
	static void _bind_methods();
	void _notification(int p_notification);

public:
#ifdef TOOLS_ENABLED
	bool _edit_is_selected_on_click(const Point2 &p_point) const;
	Dictionary _edit_get_state() const;
	void _edit_set_state(const Dictionary &p_state);
	bool _edit_state_changed(const Dictionary &p_state) const;
	Transform2D _edit_get_global_transform() const;
	Transform2D _edit_get_transform() const;
#endif

	virtual String get_configuration_warning() const;

	void set_active(bool p_state);
	bool is_active() const;
	void set_visible(bool p_visible);
	bool is_visible() const;
	void set_light_position(const Point2 &p_pos);
	Point2 get_light_position() const;
	void set_light_power(real_t p_power);
	real_t get_light_power() const;
	void set_light_radius(real_t p_radius);
	real_t get_light_radius() const;
	void set_debug_outline(bool p_state);
	bool is_debug_outline() const;
	void set_light_ambient(Color p_ambient);
	Color get_light_ambient() const;
	void set_light_diffuse(Color p_diffuse);
	Color get_light_diffuse() const;
	void set_light_specular(Color p_specular);
	Color get_light_specular() const;

	SpriteMeshLight();
	~SpriteMeshLight();
};

class CanvasItemEditor;

class SpriteMeshLightEditor : public HBoxContainer {
	GDCLASS(SpriteMeshLightEditor, HBoxContainer);

	enum {
		NONE,
		DRAG_RESIZE,
		DRAG_MOVE,
	};

	CanvasItemEditor *canvas_item_editor;
	EditorNode *editor;
	SpriteMeshLight *node;

	int dragging;
	Point2 _handles[4]; // left,right,top,bottom

protected:
	Dictionary undo_redo_state;
	UndoRedo *undo_redo;

public:
	bool forward_gui_input(const Ref<InputEvent> &p_event);
	void forward_canvas_draw_over_viewport(Control *p_overlay);

	void edit(SpriteMeshLight *p_node);
	SpriteMeshLightEditor(EditorNode *p_editor);
};

class SpriteMeshLightEditorPlugin : public EditorPlugin {
	GDCLASS(SpriteMeshLightEditorPlugin, EditorPlugin);

	SpriteMeshLightEditor *node_editor;

public:
	virtual bool forward_canvas_gui_input(const Ref<InputEvent> &p_event);
	virtual void forward_canvas_draw_over_viewport(Control *p_overlay);

	virtual String get_name() const { return "2D"; }
	bool has_main_screen() const { return false; }
	virtual void edit(Object *p_object);
	virtual bool handles(Object *p_object) const;
	virtual void make_visible(bool p_visible);

	SpriteMeshLightEditorPlugin(EditorNode *p_node);
	~SpriteMeshLightEditorPlugin();
};

// END

// BEGIN 3D Mesh and its editor

class SpriteMesh;

struct SpriteMeshSnapshot {
	SpriteMesh *owner;
	RID scenario, viewport, viewport_texture, canvas, canvas_item;

	void _create();
	void _trigger(const String &filepath);
	Ref<Image> _get_image();
	void _destroy();

	void done();

	~SpriteMeshSnapshot() {
		if (viewport.is_valid()) {
			_destroy();
		}
	}
};

class SpriteMesh : public Node2D {
	GDCLASS(SpriteMesh, Node2D);

	Ref<Mesh> drawer, mesh;
	Ref<Texture> texture;
	Ref<Texture> normal_map;
	Ref<Texture> mask;
	int submesh_selected;

	static const int LIGHTS_NUM = 4;
	struct {
		NodePath light_nodes[LIGHTS_NUM];
		bool is_empty() const {
			for (int l = 0; l < LIGHTS_NUM; l++) {
				if (!light_nodes[l].is_empty()) {
					return false;
				}
			}
			return true;
		}
		int get_num_lights() const {
			int n = 0;
			for (int l = 0; l < LIGHTS_NUM; l++) {
				n += !light_nodes[l].is_empty();
			}
			return n;
		}
		bool has_light(const NodePath &node_path) const {
			for (int l = 0; l < LIGHTS_NUM; l++) {
				if (light_nodes[l] == node_path) {
					return true;
				}
			}
			return false;
		}
		_FORCE_INLINE_ bool has_light(int index) const {
			return !light_nodes[index].is_empty();
		}
		_FORCE_INLINE_ void set_light(int index, const NodePath &node_path) {
			light_nodes[index] = node_path;
		}
		_FORCE_INLINE_ NodePath get_light(int index) const {
			return light_nodes[index];
		}
	} lights;

	bool auto_collision_shape;
	Vector<Vector2> mesh_outline;
	bool mesh_debug;
	bool _mesh_dirty;

	bool centered, flipped;
	Point2 offset;

	int selected_frame;
	// Description of frames to be precalculated:
	// ------------------------------------------
	// - repcounter: current frame
	// - repcount: total number of frames
	// ------------------------------------------
	// [
	//    {
	//       "rotate":[10, 10, 10],
	//       "scale":[10, 10, 10]
	//    },
	//    {
	//       "repeat":"10",
	//       "rotate":["sin(repcounter)", 10, 10],
	//       "scale":["360/repcount", 10, 10]
	//    }
	// ]
	String frames_builder;

	struct _FrameInfo {
		Basis xform;
		int surface_nr;
		AABB bbox;
	};
	Vector<_FrameInfo> _frames;

	Basis mesh_xform;
	Vector3 _mesh_angle;
	Vector3 _mesh_scale;

	SpriteMeshSnapshot snapshot;
	void _snapshot_done(const Variant &p_udata);
	void _mesh_changed();

	void _get_rects(Rect2 &r_src_rect, Rect2 &r_dst_rect, bool &r_filter_clip) const;
	void _refresh_properties();
	void _update_lights();
	SpriteMeshLight *_get_light_node(int p_index);
	void _update_mesh_outline(const PoolVector3Array &p_vertices, const Transform &p_xform, const PoolIntArray &p_triangles);
	void _update_mesh_changes();
	void _update_components_from_xform();
	void _update_xform_from_components();
	void _update_mesh();

protected:
	bool _get(const StringName &p_path, Variant &r_ret) const;
	bool _set(const StringName &p_path, const Variant &p_value);
	void _get_property_list(List<PropertyInfo> *p_list) const;
	void _notification(int p_what);
	static void _bind_methods();

public:
#ifdef TOOLS_ENABLED
	virtual Dictionary _edit_get_state() const;
	virtual void _edit_set_state(const Dictionary &p_state);
	bool _edit_state_changed(const Dictionary &p_state) const;

	virtual void _edit_set_pivot(const Point2 &p_pivot);
	virtual Point2 _edit_get_pivot() const;
	virtual bool _edit_use_pivot() const;
	virtual bool _edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const;

	virtual Rect2 _edit_get_rect() const;
	virtual bool _edit_use_rect() const;
#endif

	void save_snapshot(const String &p_filepath = "");
	void save_snapshot_mesh(Ref<ArrayMesh> p_mesh);

	void set_mesh(const Ref<Mesh> &p_mesh);
	Ref<Mesh> get_mesh() const;

	void set_mesh_texture(const Ref<Texture> &p_texture);
	Ref<Texture> get_mesh_texture() const;
	void set_mesh_normal_map(const Ref<Texture> &p_texture);
	Ref<Texture> get_mesh_normal_map() const;
	void set_mesh_mask(const Ref<Texture> &p_texture);
	Ref<Texture> get_mesh_mask() const;

	void set_selected_frame(int p_frame);
	int get_selected_frame() const;
	void set_frames_builder(const String &p_input);
	String get_frames_builder() const;

	void set_mesh_rotation(const Vector3 &p_radians);
	Vector3 get_mesh_rotation() const;
	void set_mesh_scale(const Vector3 &p_scale);
	Vector3 get_mesh_scale() const;
	void set_mesh_orientation(const Basis &p_basis);
	Basis get_mesh_orientation() const;

	void set_mesh_rotation_x_degrees(float p_degrees);
	float get_mesh_rotation_x_degrees() const;
	void set_mesh_rotation_y_degrees(float p_degrees);
	float get_mesh_rotation_y_degrees() const;
	void set_mesh_rotation_z_degrees(float p_degrees);
	float get_mesh_rotation_z_degrees() const;

	void set_mesh_light(int p_index, const NodePath &p_path);
	NodePath get_mesh_light(int p_index) const;

	void set_auto_collision_shape(bool p_state);
	bool is_auto_collision_shape();

	void set_centered(bool p_center);
	bool is_centered() const;
	void set_flipped_vert(bool p_flipped);
	bool is_flipped_vert() const;
	void set_offset(const Point2 &p_offset);
	Point2 get_offset() const;

	Rect2 get_rect() const;
	virtual Rect2 get_anchorable_rect() const;

	AABB get_mesh_aabb() const;

	SpriteMesh();
};

#ifdef TOOLS_ENABLED

// Reference:
// ----------
// 1. https://github.com/ChaosWitchNikol/godot-plugin-rect-extents-2d

#include "editor/editor_plugin.h"

class SpriteMeshEditor : public Control {
	GDCLASS(SpriteMeshEditor, Control);

	enum {
		NONE,
		DRAG_RESIZE,
		DRAG_ROTATE_X,
		DRAG_ROTATE_Y,
		DRAG_ROTATE_Z,
		DRAG_SCALE_X,
		DRAG_SCALE_Y,
		DRAG_SCALE_Z,
		DRAG_SCALE_XY,
		DRAG_SCALE_YZ,
		DRAG_SCALE_XZ,
	};
	int dragging;
	Point2 mouse_dragging_start;
	Vector2 mouse_dragging_dist;
	real_t _dragging_change;

	CanvasItemEditor *canvas_item_editor;
	EditorPlugin *plugin;
	EditorNode *editor;
	SpriteMesh *node;

	struct {
		bool enabled;
		RID item;
		RID mesh3d;
		Ref<ArrayMesh> mesh;
	} origin_indicator;

	struct {
		RID item;
		RID mesh3d;
		Ref<ArrayMesh> mesh;
	} rotate_gizmo[3], scale_gizmo[3], scale_plane_gizmo[3];

	void _init_indicators();
	void _finish_indicators();
	void _init_gizmo_instance();
	void _finish_gizmo_instances();
	Color _get_color(const StringName &p_name, const StringName &p_theme_type = StringName()) const;

	enum {
		GIZMO_BASE_LAYER = 27,
		GIZMO_ORIGIN_LAYER = 25,
	};

	bool _project_settings_change_pending;
	int _update_dragging_info(const Point2 &gpoint);
	real_t _get_gizmo_scale() const;

protected:
	static void _bind_methods();
	void _notification(int p_what);

	void _project_settings_changed();
	void _node_removed(Node *p_node);

	Dictionary undo_redo_state;
	UndoRedo *undo_redo;

public:
	bool forward_gui_input(const Ref<InputEvent> &p_event);
	void forward_canvas_draw_over_viewport(Control *p_overlay);

	void update_transform_gizmo_view();
	void edit(SpriteMesh *p_node);

	SpriteMeshEditor(EditorPlugin *p_plugin, EditorNode *p_node);
	~SpriteMeshEditor();
};

class SpriteMeshEditorPlugin : public EditorPlugin {
	GDCLASS(SpriteMeshEditorPlugin, EditorPlugin);

	SpriteMeshEditor *sprite_mesh_editor;

public:
	virtual String get_name() const { return "2D"; }
	bool has_main_screen() const { return false; }
	virtual void make_visible(bool p_visible);
	virtual bool handles(Object *p_object) const { return Object::cast_to<SpriteMesh>(p_object) != nullptr; }
	virtual void edit(Object *p_object);

	virtual bool forward_canvas_gui_input(const Ref<InputEvent> &p_event);
	virtual void forward_canvas_draw_over_viewport(Control *p_overlay);

	SpriteMeshEditorPlugin(EditorNode *p_node);
	~SpriteMeshEditorPlugin();
};

#endif // TOOLS_ENABLED

// END

#endif // GD_SPRITE_MESH_H
