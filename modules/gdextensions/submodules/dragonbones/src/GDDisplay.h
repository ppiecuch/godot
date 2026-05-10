/**************************************************************************/
/*  GDDisplay.h                                                           */
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

#ifndef GDISPLAY_H
#define GDISPLAY_H

#include "scene/2d/node_2d.h"
#include "scene/resources/material.h"
#include "scene/resources/texture.h"
#include "servers/visual_server.h"

#include <dragonBones/DragonBonesHeaders.h>

DRAGONBONES_USING_NAME_SPACE;

class GDOwnerNode : public Node2D {
	GDCLASS(GDOwnerNode, Node2D);

public:
	Color modulate;

public:
	GDOwnerNode() { modulate = Color(1, 1, 1, 1); }
	virtual ~GDOwnerNode() {}

	virtual void set_modulate(const Color &col) {
		modulate = col;
	}

	virtual void dispatch_event(const String &type, const EventObject *value) = 0;
	virtual void dispatch_snd_event(const String &type, const EventObject *value) = 0;
};

class GDDisplay : public Node2D {
	GDCLASS(GDDisplay, Node2D);

private:
	GDDisplay(const GDDisplay &);

	Ref<CanvasItemMaterial> canvas_mat;

public:
	Ref<Texture> texture;
	Color modulate;
	GDOwnerNode *p_owner;
	bool b_debug;

public:
	GDDisplay() {
		modulate = Color(1, 1, 1, 1);
		p_owner = nullptr;
		b_debug = false;
		canvas_mat.instance();
	}
	virtual ~GDDisplay() {}

	virtual void set_modulate(const Color &col) {
		modulate = col;
	}

	void set_blend_mode(CanvasItem::BlendMode blend) {
		canvas_mat->set_blend_mode((CanvasItemMaterial::BlendMode)blend);
		set_material(canvas_mat);
	}
};

#endif // GDISPLAY_H
