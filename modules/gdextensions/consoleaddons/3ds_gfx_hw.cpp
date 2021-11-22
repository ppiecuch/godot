/*************************************************************************/
/*  3ds_gfx_hw.cpp                                                       */
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

#include "3ds_gfx_hw.h"

#include <nds/arm9/trig_lut.h>
#include <nds/background.h>
#include <nds/sprite.h>

static const int SPRITE_DMA_CHANNEL = 3;

typedef struct {
	int oam_id;
	int width;
	int height;
	int angle;
	SpriteEntry *entry;
} SpriteInfo;

void NdsSprite::_update_oam(OAMTable *p_oam) {
	DC_FlushAll();
	dmaCopyHalfWords(SPRITE_DMA_CHANNEL, p_oam->oamBuffer, OAM, SPRITE_COUNT * sizeof(SpriteEntry));
}

void NdsSprite::_init_oam(OAMTable *p_oam) {
	for (int i = 0; i < SPRITE_COUNT; i++) {
		p_oam->oamBuffer[i].attribute[0] = ATTR0_DISABLED;
		p_oam->oamBuffer[i].attribute[1] = 0;
		p_oam->oamBuffer[i].attribute[2] = 0;
	}
	for (int i = 0; i < MATRIX_COUNT; i++) {
		p_oam->matrixBuffer[i].hdx = 1 << 8;
		p_oam->matrixBuffer[i].hdy = 0;
		p_oam->matrixBuffer[i].vdx = 0;
		p_oam->matrixBuffer[i].vdy = 1 << 8;
	}
	_update_oam(oam);
}

void NdsSprite::set_visibility(SpriteEntry *p_sprite_entry, bool p_hidden, bool p_affine, bool p_double_bound) {
	if (p_hidden) {
		p_sprite_entry->isRotateScale = false; // Bit 9 off
		p_sprite_entry->isHidden = true; // Bit 8 on
	} else {
		if (p_affine) {
			p_sprite_entry->isRotateScale = true;
			p_sprite_entry->isSizeDouble = p_double_bound;
		} else {
			p_sprite_entry->isHidden = false;
		}
	}
}
