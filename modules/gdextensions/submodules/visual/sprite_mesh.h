/*************************************************************************/
/*  sprite_mesh.h                                                        */
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

#ifndef GD_SPRITE_MESH_H
#define GD_SPRITE_MESH_H

#include "core/math/vector3.h"
#include "scene/2d/sprite.h"
#include "scene/gui/box_container.h"
#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"

class SpriteMeshLight : public Node {
	GDCLASS(SpriteMeshLight, Node);

	RID canvas_item;
	bool active;
	bool visible;
	Point2 light_position;
	real_t light_power;
	real_t light_radius;
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

class SpriteMesh : public Node2D {
	GDCLASS(SpriteMesh, Node2D);

	Ref<ArrayMesh> mesh;
	Ref<Texture> texture;
	Ref<Texture> normal_map;
	Ref<Texture> mask;

	bool auto_collision_shape;
	Vector<Vector2> mesh_outline;
	bool mesh_debug;
	bool _mesh_dirty;

	bool centered;
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

	mutable SafeFlag snapshot_done;
	Size2 snapshot_size;
	void _snapshot_done(const Variant &p_udata);
	Ref<Image> _save_mesh_snapshot(const Size2 &p_snapshot_size);
	void _save_mesh_xform(Ref<ArrayMesh> &p_mesh);

	void _get_rects(Rect2 &r_src_rect, Rect2 &r_dst_rect, bool &r_filter_clip) const;
	void _mesh_changed();

protected:
	void _notification(int p_what);
	static void _bind_methods();
	virtual void _validate_property(PropertyInfo &property) const;

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

	void _update_mesh_outline(const PoolVector3Array &p_vertices, const Transform &p_xform, const PoolIntArray &p_triangles);
	void _update_xform_values();
	void _update_mesh_xform();
	void _update_transform();

	Ref<Image> save_snapshot();

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

	void set_auto_collision_shape(bool state);
	bool is_auto_collision_shape();

	void set_centered(bool p_center);
	bool is_centered() const;
	void set_offset(const Point2 &p_offset);
	Point2 get_offset() const;

	void set_snapshot_size(const Size2 &p_snapshot_size);
	Size2 get_snapshot_size() const;

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

class SpriteMeshManager : public Node {
	GDCLASS(SpriteMeshManager, Node);

protected:
	static void _bind_methods();
	void _notification(int p_notification);

public:
	virtual String get_configuration_warning() const;

	SpriteMeshManager();
	~SpriteMeshManager();
};
#endif // GD_SPRITE_MESH_H
