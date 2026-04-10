/**************************************************************************/
/*  sprite_quick_offset_plugin.cpp                                        */
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

#ifdef TOOLS_ENABLED

#include "sprite_quick_offset_plugin.h"

#include "editor/editor_node.h"
#include "scene/2d/animated_sprite.h"
#include "scene/2d/sprite.h"
#include "scene/gui/control.h"
#include "scene/gui/label.h"
#include "scene/gui/separator.h"

// ── Unicode glyphs for the 3×3 anchor grid ───────────────────────────────────
// Displayed as button labels — no image assets required.
static const char *ANCHOR_LABELS[9] = {
	/* 0 top-left  */ "\xe2\x86\x96", // ↖
	/* 1 top       */ "\xe2\x86\x91", // ↑
	/* 2 top-right */ "\xe2\x86\x97", // ↗
	/* 3 left      */ "\xe2\x86\x90", // ←
	/* 4 center    */ "\xe2\x97\x8f", // ●
	/* 5 right     */ "\xe2\x86\x92", // →
	/* 6 bot-left  */ "\xe2\x86\x99", // ↙
	/* 7 bottom    */ "\xe2\x86\x93", // ↓
	/* 8 bot-right */ "\xe2\x86\x98", // ↘
};

static const char *ANCHOR_TOOLTIPS[9] = {
	"Top-Left",
	"Top",
	"Top-Right",
	"Left",
	"Center",
	"Right",
	"Bottom-Left",
	"Bottom",
	"Bottom-Right",
};

// ── Geometry ─────────────────────────────────────────────────────────────────

// Returns the offset that, when applied to a centered Sprite/AnimatedSprite
// (or as rect_pivot_offset for Control), places the node's transform origin
// at the requested anchor position.
//
// With centered=true a Sprite is drawn so its visual center is at the node
// origin. Adding the returned offset shifts the visual center:
//   top-left  → offset = (-w/2, -h/2) → top-left corner lands on origin
//   center    → offset = (0, 0)       → unchanged
//   bot-right → offset = (+w/2, +h/2) → bottom-right corner lands on origin
Vector2 SpriteQuickOffsetPlugin::_anchor_offset(const Size2 &p_size, int p_anchor) {
	// col 0=left, 1=center, 2=right   |   row 0=top, 1=middle, 2=bottom
	const float half_w = p_size.width * 0.5f;
	const float half_h = p_size.height * 0.5f;
	static const float kx[3] = { -1.0f, 0.0f, 1.0f };
	static const float ky[3] = { -1.0f, 0.0f, 1.0f };
	const int col = p_anchor % 3;
	const int row = p_anchor / 3;
	return Vector2(kx[col] * half_w, ky[row] * half_h);
}

// ── Apply helpers ─────────────────────────────────────────────────────────────

void SpriteQuickOffsetPlugin::_apply_to_sprite(Object *p_obj, int p_anchor, UndoRedo *p_ur) {
	Sprite *s = Object::cast_to<Sprite>(p_obj);
	ERR_FAIL_NULL(s);

	Ref<Texture> tex = s->get_texture();
	if (tex.is_null()) {
		return;
	}

	// When region is enabled use the region rect size, otherwise full texture.
	Size2 size;
	if (s->is_region()) {
		size = s->get_region_rect().size;
	} else {
		size = Size2((float)tex->get_width(), (float)tex->get_height());
	}

	const bool old_centered = s->is_centered();
	const Vector2 old_offset = s->get_offset();
	const Vector2 new_offset = _anchor_offset(size, p_anchor);

	p_ur->add_do_method(s, "set_centered", true);
	p_ur->add_do_method(s, "set_offset", new_offset);
	p_ur->add_undo_method(s, "set_centered", old_centered);
	p_ur->add_undo_method(s, "set_offset", old_offset);
}

void SpriteQuickOffsetPlugin::_apply_to_animated_sprite(Object *p_obj, int p_anchor, UndoRedo *p_ur) {
	AnimatedSprite *as = Object::cast_to<AnimatedSprite>(p_obj);
	ERR_FAIL_NULL(as);

	Ref<SpriteFrames> frames = as->get_sprite_frames();
	if (frames.is_null()) {
		return;
	}
	const StringName anim = as->get_animation();
	if (!frames->has_animation(anim) || frames->get_frame_count(anim) == 0) {
		return;
	}
	Ref<Texture> tex = frames->get_frame(anim, 0);
	if (tex.is_null()) {
		return;
	}

	const Size2 size = Size2((float)tex->get_width(), (float)tex->get_height());
	const bool old_centered = as->is_centered();
	const Vector2 old_offset = as->get_offset();
	const Vector2 new_offset = _anchor_offset(size, p_anchor);

	p_ur->add_do_method(as, "set_centered", true);
	p_ur->add_do_method(as, "set_offset", new_offset);
	p_ur->add_undo_method(as, "set_centered", old_centered);
	p_ur->add_undo_method(as, "set_offset", old_offset);
}

void SpriteQuickOffsetPlugin::_apply_to_control(Object *p_obj, int p_anchor, UndoRedo *p_ur) {
	Control *c = Object::cast_to<Control>(p_obj);
	ERR_FAIL_NULL(c);

	const Size2 size = c->get_size();
	const Vector2 old_pivot = c->get_pivot_offset();
	// For Control the pivot is not relative to center — it is absolute from
	// the control's top-left corner, so col 0 → 0, col 2 → width, etc.
	const float px[3] = { 0.0f, size.width * 0.5f, size.width };
	const float py[3] = { 0.0f, size.height * 0.5f, size.height };
	const Vector2 new_pivot = Vector2(px[p_anchor % 3], py[p_anchor / 3]);

	p_ur->add_do_method(c, "set_pivot_offset", new_pivot);
	p_ur->add_undo_method(c, "set_pivot_offset", old_pivot);
}

// ── Signal handlers ───────────────────────────────────────────────────────────

void SpriteQuickOffsetPlugin::_btn_pressed() {
	// Show popup just below the toolbar button.
	const Vector2 btn_pos = toolbar_btn->get_global_rect().position;
	const float btn_h = toolbar_btn->get_size().y;
	popup->set_position(Vector2(btn_pos.x, btn_pos.y + btn_h));
	popup->popup();
}

void SpriteQuickOffsetPlugin::_anchor_selected(int p_anchor) {
	popup->hide();

	Array selected = get_editor_interface()->get_selection()->get_selected_nodes();
	if (selected.empty()) {
		return;
	}

	UndoRedo &ur = get_undo_redo();
	ur.create_action(TTR("Sprite Quick Offset"));

	for (int i = 0; i < selected.size(); ++i) {
		Object *obj = selected[i];
		if (Object::cast_to<Sprite>(obj)) {
			_apply_to_sprite(obj, p_anchor, &ur);
		} else if (Object::cast_to<AnimatedSprite>(obj)) {
			_apply_to_animated_sprite(obj, p_anchor, &ur);
		} else if (Object::cast_to<Control>(obj)) {
			_apply_to_control(obj, p_anchor, &ur);
		}
	}

	ur.commit_action();
}

// ── Construction / destruction ────────────────────────────────────────────────

void SpriteQuickOffsetPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_btn_pressed"), &SpriteQuickOffsetPlugin::_btn_pressed);
	ClassDB::bind_method(D_METHOD("_anchor_selected", "anchor"), &SpriteQuickOffsetPlugin::_anchor_selected);
}

SpriteQuickOffsetPlugin::SpriteQuickOffsetPlugin(EditorNode *p_node) {
	// ── Toolbar button ────────────────────────────────────────────────────────
	toolbar_btn = memnew(ToolButton);
	toolbar_btn->set_text(TTR("Offset"));
	toolbar_btn->set_tooltip(TTR("Sprite Quick Offset\nSet anchor / pivot of selected Sprite, AnimatedSprite or Control"));
	toolbar_btn->connect("pressed", this, "_btn_pressed");

	// ── Popup panel ───────────────────────────────────────────────────────────
	popup = memnew(PopupPanel);
	toolbar_btn->add_child(popup); // Popup subclass = always top-level, so no clipping

	VBoxContainer *vbox = memnew(VBoxContainer);
	vbox->set_anchors_and_margins_preset(Control::PRESET_WIDE, Control::PRESET_MODE_MINSIZE, 4);
	popup->add_child(vbox);

	// Small label header
	Label *header = memnew(Label);
	header->set_text(TTR("Set Anchor"));
	header->set_align(Label::ALIGN_CENTER);
	vbox->add_child(header);

	// 3×3 grid of anchor buttons
	GridContainer *grid = memnew(GridContainer);
	grid->set_columns(3);
	grid->add_constant_override("hseparation", 2);
	grid->add_constant_override("vseparation", 2);
	vbox->add_child(grid);

	const int BTN_SIZE = 32;
	for (int i = 0; i < 9; ++i) {
		Button *btn = memnew(Button);
		btn->set_text(String::utf8(ANCHOR_LABELS[i]));
		btn->set_tooltip(TTR(ANCHOR_TOOLTIPS[i]));
		btn->set_custom_minimum_size(Vector2(BTN_SIZE, BTN_SIZE));
		btn->set_flat(false);
		btn->connect("pressed", this, "_anchor_selected", varray(i));
		grid->add_child(btn);
	}

	// Size popup to fit content: 3 buttons + margins + header
	const int popup_w = BTN_SIZE * 3 + 2 * 2 + 4 * 2; // buttons + hsep + padding
	const int popup_h = BTN_SIZE * 3 + 2 * 2 + 20 + 4 * 2; // buttons + vsep + header + padding
	popup->set_custom_minimum_size(Vector2(popup_w, popup_h));

	add_control_to_container(CONTAINER_CANVAS_EDITOR_MENU, toolbar_btn);
}

SpriteQuickOffsetPlugin::~SpriteQuickOffsetPlugin() {
	if (toolbar_btn) {
		remove_control_from_container(CONTAINER_CANVAS_EDITOR_MENU, toolbar_btn);
		memdelete(toolbar_btn); // also frees popup (child)
		toolbar_btn = nullptr;
		popup = nullptr;
	}
}

#endif // TOOLS_ENABLED
