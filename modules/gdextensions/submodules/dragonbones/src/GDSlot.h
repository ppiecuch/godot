/**************************************************************************/
/*  GDSlot.h                                                              */
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

#ifndef GDSLOT_H
#define GDSLOT_H

#include "GDDisplay.h"
#include "scene/2d/node_2d.h"
#include <dragonBones/DragonBonesHeaders.h>
#include <memory>

DRAGONBONES_NAMESPACE_BEGIN

class GDSlot : public Slot {
	BIND_CLASS_TYPE_A(GDSlot);

private:
	float _textureScale;
	GDDisplay *_renderDisplay;

public:
	virtual void _updateVisible() override;
	virtual void _updateBlendMode() override;
	virtual void _updateColor() override;

protected:
	virtual void _initDisplay(void *value, bool isRetain) override;
	virtual void _disposeDisplay(void *value, bool isRelease) override;
	virtual void _onUpdateDisplay() override;
	virtual void _addDisplay() override;
	virtual void _replaceDisplay(void *value, bool isArmatureDisplay) override;
	virtual void _removeDisplay() override;
	virtual void _updateZOrder() override;

	virtual void _updateFrame() override;
	virtual void _updateMesh() override;
	virtual void _updateTransform() override;
	virtual void _identityTransform() override;

	virtual void _onClear() override;

	static void __get_uv_pt(Point2 &_pt, bool _is_rot, float _u, float _v, const Rectangle &_reg, const TextureAtlasData *_p_atlas);
};

DRAGONBONES_NAMESPACE_END

#endif // GDSLOT_H
