#ifndef GDISPLAY_H
#define GDISPLAY_H

#include "scene/2d/node_2d.h"
#include "scene/resources/material.h"
#include "scene/resources/texture.h"
#include "servers/visual_server.h"

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
