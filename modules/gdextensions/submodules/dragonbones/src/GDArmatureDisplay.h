/**************************************************************************/
/*  GDArmatureDisplay.h                                                   */
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

#ifndef GDARMATUREDESPLAY_H
#define GDARMATUREDESPLAY_H

#include "GDDisplay.h"
#include <dragonBones/DragonBonesHeaders.h>

DRAGONBONES_NAMESPACE_BEGIN

class GDArmatureDisplay : public GDDisplay, virtual public IArmatureProxy {
	GDCLASS(GDArmatureDisplay, GDDisplay);

private:
	GDArmatureDisplay(const GDArmatureDisplay &);

protected:
	Armature *p_armature;

public:
	GDArmatureDisplay();
	~GDArmatureDisplay();

	static GDArmatureDisplay *create() {
		return memnew(GDArmatureDisplay);
	}

	void addEvent(const std::string &_type, const std::function<void(EventObject *)> &_callback);
	void removeEvent(const std::string &_type);

	bool hasDBEventListener(const std::string &_type) const override { return true; }
	void addDBEventListener(const std::string &_type, const std::function<void(EventObject *)> &_listener) {}
	void removeDBEventListener(const std::string &_type, const std::function<void(EventObject *)> &_listener) {}
	void dispatchDBEvent(const std::string &_type, EventObject *_value);

	void dbInit(Armature *_p_armature) override;
	void dbClear() override;
	void dbUpdate() override;

	void dispose(bool disposeProxy) override;

	Armature *getArmature() const override { return p_armature; }
	Animation *getAnimation() const override { return p_armature->getAnimation(); }

	void add_parent_class(bool _b_debug, const Ref<Texture> &_m_texture_atla);
	void update_childs(bool _b_color, bool _b_blending = false);
	void update_texture_atlas(const Ref<Texture> &_m_texture_atlas);
	void update_material_inheritance(bool _b_inherit_material);

	void set_debug(bool _b_debug);
};

DRAGONBONES_NAMESPACE_END

#endif // GDARMATUREDESPLAY_H
