/**************************************************************************/
/*  sprite_quick_offset_plugin.h                                          */
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

// SpriteQuickOffsetPlugin — canvas-editor toolbar button that lets you set
// the pivot / anchor point of a Sprite, AnimatedSprite, or Control node to
// one of 9 positions (3×3 grid: corners, edge midpoints, center).
//
// Ported from the "sprite_quick_offset" GDScript addon (K3N, v0.6.0) and
// enhanced with:
//   - Undo/redo via EditorUndoRedoManager
//   - AnimatedSprite region-size awareness
//   - Keyboard shortcut (no shortcut in original)
//   - Control pivot support unchanged from original

#ifndef SPRITE_QUICK_OFFSET_PLUGIN_H
#define SPRITE_QUICK_OFFSET_PLUGIN_H

#ifdef TOOLS_ENABLED

#include "editor/editor_plugin.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/grid_container.h"
#include "scene/gui/popup.h"
#include "scene/gui/tool_button.h"

class SpriteQuickOffsetPlugin : public EditorPlugin {
	GDCLASS(SpriteQuickOffsetPlugin, EditorPlugin);

public:
	// Anchor positions (row-major: 0=top-left … 8=bottom-right).
	enum AnchorType {
		ANCHOR_TOP_LEFT = 0,
		ANCHOR_TOP = 1,
		ANCHOR_TOP_RIGHT = 2,
		ANCHOR_LEFT = 3,
		ANCHOR_CENTER = 4,
		ANCHOR_RIGHT = 5,
		ANCHOR_BOTTOM_LEFT = 6,
		ANCHOR_BOTTOM = 7,
		ANCHOR_BOTTOM_RIGHT = 8,
	};

private:
	ToolButton *toolbar_btn = nullptr; // lives in CONTAINER_CANVAS_EDITOR_MENU
	PopupPanel *popup = nullptr; // child of toolbar_btn (Popup = always top-level)

	// ── Signal handlers ───────────────────────────────────────────────────────
	void _btn_pressed();
	void _anchor_selected(int p_anchor);

	// ── Per-type apply helpers ─────────────────────────────────────────────────
	// Each helper records undo/redo actions into the passed UndoRedo and applies
	// them immediately so the viewport refreshes before commit_action().
	void _apply_to_sprite(Object *p_obj, int p_anchor, UndoRedo *p_ur);
	void _apply_to_animated_sprite(Object *p_obj, int p_anchor, UndoRedo *p_ur);
	void _apply_to_control(Object *p_obj, int p_anchor, UndoRedo *p_ur);

	// ── Geometry helper ────────────────────────────────────────────────────────
	// Returns the offset Vector2 that places the node origin at p_anchor when
	// centered=true (Sprite / AnimatedSprite) or the pivot offset (Control).
	// p_size is the full texture / control size.
	static Vector2 _anchor_offset(const Size2 &p_size, int p_anchor);

protected:
	static void _bind_methods();

public:
	virtual String get_name() const { return "SpriteQuickOffset"; }

	SpriteQuickOffsetPlugin(EditorNode *p_node);
	~SpriteQuickOffsetPlugin();
};

#endif // TOOLS_ENABLED
#endif // SPRITE_QUICK_OFFSET_PLUGIN_H
