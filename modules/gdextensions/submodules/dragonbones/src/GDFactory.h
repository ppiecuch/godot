/**************************************************************************/
/*  GDFactory.h                                                           */
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

#ifndef GDFACTORY_H
#define GDFACTORY_H

#include <string>
#include <vector>

#include <dragonBones/DragonBonesHeaders.h>

#include "GDArmatureDisplay.h"
#include "GDSlot.h"
#include "GDTextureData.h"

DRAGONBONES_NAMESPACE_BEGIN

typedef std::function<void(EventObject *)> Func_t;
class GDFactory : public BaseFactory, public IEventDispatcher {
	DRAGONBONES_DISALLOW_COPY_AND_ASSIGN(GDFactory)

protected:
	DragonBones *_dragonBonesInstance;
	GDOwnerNode *p_owner;

	mutable std::vector<std::unique_ptr<GDSlot>> _wrapperSlots;
	mutable std::vector<std::unique_ptr<GDTextureData>> _wrapperTexturesData;

public:
	GDFactory(GDOwnerNode *_p_owner);
	~GDFactory();

public:
	DragonBonesData *loadDragonBonesData(const char *_p_data_loaded, const std::string &name = "");
	TextureAtlasData *loadTextureAtlasData(const char *_p_data_loaded, Ref<Texture> *_p_atlasTexture, const std::string &name = "", float scale = 1.0f);
	GDArmatureDisplay *buildArmatureDisplay(const std::string &armatureName, const std::string &dragonBonesName = "", const std::string &skinName = "", const std::string &textureAtlasName = "") const;

	void update(float lastUpdate);
	void set_speed(float _f_speed);

	// sound IEventDispatcher
	void addDBEventListener(const std::string &type, const Func_t &listener);
	void removeDBEventListener(const std::string &type, const Func_t &listener);
	void dispatchDBEvent(const std::string &type, EventObject *value);
	bool hasDBEventListener(const std::string &type) const;

protected:
	TextureAtlasData *_buildTextureAtlasData(TextureAtlasData *textureAtlasData, void *textureAtlas) const override;
	Armature *_buildArmature(const BuildArmaturePackage &dataPackage) const override;
	Slot *_buildSlot(const BuildArmaturePackage &dataPackage, const SlotData *slotData, Armature *armature) const override;
};

DRAGONBONES_NAMESPACE_END

#endif // GDFACTORY_H
