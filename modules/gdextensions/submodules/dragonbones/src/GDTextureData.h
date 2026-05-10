/**************************************************************************/
/*  GDTextureData.h                                                       */
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

#ifndef GDTEXTUREDATA_H
#define GDTEXTUREDATA_H

#include "core/math/rect2.h"
#include "core/math/transform_2d.h"
#include "core/math/vector2.h"
#include "scene/resources/texture.h"
#include <dragonBones/DragonBonesHeaders.h>
#include <memory>

DRAGONBONES_NAMESPACE_BEGIN

class GDTextureData : public TextureData {
	BIND_CLASS_TYPE_B(GDTextureData);

public:
	Rect2i textureRect;

public:
	GDTextureData() {
		_onClear();
	}

	virtual ~GDTextureData() {
		_onClear();
	}

	void _onClear() override {
		TextureData::_onClear();
	}
};

DRAGONBONES_NAMESPACE_END

#endif // GDTEXTUREDATA_H
