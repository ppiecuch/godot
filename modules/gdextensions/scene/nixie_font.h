#ifndef GD_NIXIE_FONT_H
#define GD_NIXIE_FONT_H

#include "scene/2d/node_2d.h"
#include "scene/resources/texture.h"

class NixieFont : public Node2D {

	GDCLASS(NixieFont, Node2D);

private:
	Ref<Texture>  texture;
	String draw_text;

	struct CharControl {
		int age;     // displaing age
		int phase;   // current step
		int alt;     // alternate char overlay
		int alt_age; // next alternate char
	};
	PoolVector<CharControl> _control;
	PoolVector<Rect2> _frames;
	int _age;

	void _draw();

public:
	void set_text(String p_text);

	NixieFont();
	~NixieFont();
};

#endif // GD_NIXIE_FONT_H
