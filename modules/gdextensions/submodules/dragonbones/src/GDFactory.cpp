/**************************************************************************/
/*  GDFactory.cpp                                                         */
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

#include "GDFactory.h"

#include "GDArmatureDisplay.h"
#include "GDMesh.h"
#include "GDSlot.h"
#include "GDTextureAtlasData.h"
#include "GDTextureData.h"

#include "core/os/file_access.h"
#include "core/os/os.h"

DRAGONBONES_NAMESPACE_BEGIN

GDFactory::GDFactory(GDOwnerNode *_p_owner) {
	p_owner = _p_owner;
	_dragonBonesInstance = memnew(DragonBones(this));
	_dragonBones = _dragonBonesInstance;
}

GDFactory::~GDFactory() {
	clear();

	if (_dragonBonesInstance) {
		memdelete(_dragonBonesInstance);
		_dragonBonesInstance = nullptr;
	}
}

DragonBonesData *GDFactory::loadDragonBonesData(const char *_p_data_loaded, const std::string &name) {
	if (!name.empty()) {
		const auto existedData = getDragonBonesData(name);

		if (existedData)
			return existedData;
	}
	return parseDragonBonesData(_p_data_loaded, name, 1.0f);
}

TextureAtlasData *GDFactory::loadTextureAtlasData(const char *_p_data_loaded, Ref<Texture> *_p_atlasTexture, const std::string &name, float scale) {
	return static_cast<GDTextureAtlasData *>(BaseFactory::parseTextureAtlasData(_p_data_loaded, _p_atlasTexture, name, scale));
}

GDArmatureDisplay *GDFactory::buildArmatureDisplay(const std::string &armatureName, const std::string &dragonBonesName, const std::string &skinName, const std::string &textureAtlasName) const {
	const auto armature = buildArmature(armatureName, dragonBonesName, skinName, textureAtlasName);

	if (armature != nullptr) {
		_dragonBones->getClock()->add(armature);
		return static_cast<GDArmatureDisplay *>(armature->getDisplay());
	}
	return nullptr;
}

void GDFactory::update(float lastUpdate) {
	_dragonBonesInstance->advanceTime(lastUpdate);
}

void GDFactory::set_speed(float _f_speed) {
	_dragonBonesInstance->getClock()->timeScale = _f_speed;
}

TextureAtlasData *GDFactory::_buildTextureAtlasData(TextureAtlasData *textureAtlasData, void *textureAtlas) const {
	auto textureAtlasData_ = static_cast<GDTextureAtlasData *>(textureAtlasData);

	if (textureAtlasData != nullptr)
		textureAtlasData_->setRenderTexture();
	else
		textureAtlasData_ = BaseObject::borrowObject<GDTextureAtlasData>();
	return textureAtlasData_;
}

Armature *GDFactory::_buildArmature(const BuildArmaturePackage &dataPackage) const {
	const auto armature = BaseObject::borrowObject<Armature>();
	const auto armatureDisplay = GDArmatureDisplay::create();
	armature->init(dataPackage.armature, armatureDisplay, armatureDisplay, _dragonBones);
	armatureDisplay->set_name(armature->getName().c_str());
	return armature;
}

Slot *GDFactory::_buildSlot(const BuildArmaturePackage &dataPackage, const SlotData *slotData, Armature *armature) const {
	auto slot = BaseObject::borrowObject<GDSlot>();
	auto wrapperDisplay = GDMesh::create();
	_wrapperSlots.push_back(std::unique_ptr<GDSlot>(slot));
	slot->init(slotData, armature, wrapperDisplay, wrapperDisplay);
	wrapperDisplay->set_name(slot->getName().c_str());
	return slot;
}

void GDFactory::addDBEventListener(const std::string &type, const Func_t &listener) {}
void GDFactory::removeDBEventListener(const std::string &type, const Func_t &listener) {}
void GDFactory::dispatchDBEvent(const std::string &type, EventObject *value) {
	p_owner->dispatch_snd_event(String(type.c_str()), value);
}

bool GDFactory::hasDBEventListener(const std::string &type) const {
	return true;
}

DRAGONBONES_NAMESPACE_END
