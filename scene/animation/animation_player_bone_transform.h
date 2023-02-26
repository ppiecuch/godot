/**************************************************************************/
/*  animation_player_bone_transform.h                                     */
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

#ifndef ANIMATION_PLAYER_BONE_TRANSFORM_H
#define ANIMATION_PLAYER_BONE_TRANSFORM_H

#include "core/math/transform.h"
#include "core/object.h"
#include "core/ustring.h"

#define BONE_TRANSFORMER_KEY "__internal_bone_transformer"

class AnimationPlayerBoneTransform : public Object {
public:
	AnimationPlayerBoneTransform() { set_meta(BONE_TRANSFORMER_KEY, this); }
	virtual String get_bone_name(int p_bone) const = 0;
	virtual int get_bone_parent(int p_bone) const = 0;
	virtual Transform get_bone_pose(int p_bone) const = 0;
	virtual void set_bone_pose(int p_bone, const Transform &p_pose) = 0;
	virtual int find_bone(const String &p_name) const = 0;
	virtual void clear_pose() = 0;
	virtual void update_skeleton() = 0;
};

#endif // ANIMATION_PLAYER_BONE_TRANSFORM_H
