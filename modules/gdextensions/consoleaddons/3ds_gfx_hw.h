/*************************************************************************/
/*  3ds_gfx_hw.h                                                         */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
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

#ifndef 3DS_GFX_HW_H
#define 3DS_GFX_HW_H

#include "scene/2d/node_2d.h"

struct OAMTable;
struct SpriteEntry;

enum NdsScreen {
	SCREEN_TOP,
	SCREEN_BOTTOM,
}

enum NdsMode5Background {
	MODE5_BG0,
	MODE5_BG1,
	MODE5_BG2,
	MODE5_BG3,
}

enum NdsBackgroundBitmapSize {
	BITMAP_128x128,
	BITMAP_256x256,
	BITMAP_512x256,
	BITMAP_512x512,
}

class NdsSprite : public Node2D {
	GDCLASS(NdsSprite, Node2D);

	int oam_id;

	void _update_oam(OAMTable *p_oam);
	void _init_oam(OAMTable *p_oam);
	void _set_visibility(SpriteEntry *p_sprite_entry, bool p_hidden, bool p_affine, bool p_double_bound);

public:
	NdsSprite();
	~NdsSprite();
};

class NdsBackground : public Node2D {
	GDCLASS(NdsSprite, Node2D);

public:
	NdsBackground();
	~NdsBackground();
};

#endif // 3DS_GFX_HW_H
