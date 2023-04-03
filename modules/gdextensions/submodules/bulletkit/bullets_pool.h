/**************************************************************************/
/*  bullets_pool.h                                                        */
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

#ifndef BULLETS_POOL_H
#define BULLETS_POOL_H

#include "core/color.h"
#include "scene/2d/canvas_item.h"
#include "scene/main/canvas_layer.h"
#include "scene/resources/material.h"
#include "scene/resources/texture.h"

#include "bullet.h"
#include "bullet_kit.h"

class BulletsPool {
protected:
	int32_t *shapes_to_indices = nullptr;
	int32_t available_bullets = 0;
	int32_t active_bullets = 0;
	int32_t bullets_to_handle = 0;
	bool collisions_enabled;

	CanvasLayer *canvas_layer;
	Viewport *viewport;
	RID canvas_parent;
	RID canvas_item;
	RID shared_area;
	int32_t starting_shape_index;

	Rect2 active_rect;

	template <typename T>
	void _swap(T &a, T &b) {
		T t = a;
		a = b;
		b = t;
	}

public:
	int32_t pool_size = 0;
	int32_t set_index = -1;

	BulletsPool();
	virtual ~BulletsPool();

	virtual void _init(Node *parent_hint, RID shared_area, int32_t starting_shape_index,
			int32_t set_index, Ref<BulletKit> kit, int32_t pool_size, int32_t z_index) = 0;

	int32_t get_available_bullets();
	int32_t get_active_bullets();

	virtual int32_t _process(float delta) = 0;

	virtual void spawn_bullet(Dictionary properties) = 0;
	virtual BulletID obtain_bullet() = 0;
	virtual bool release_bullet(BulletID id) = 0;
	virtual bool is_bullet_valid(BulletID id) = 0;

	virtual bool is_bullet_existing(int32_t shape_index) = 0;
	virtual BulletID get_bullet_from_shape(int32_t shape_index) = 0;

	virtual void set_bullet_property(BulletID id, String property, Variant value) = 0;
	virtual Variant get_bullet_property(BulletID id, String property) = 0;
};

template <class Kit, class BulletType>
class AbstractBulletsPool : public BulletsPool {
protected:
	Ref<Kit> kit;
	BulletType **bullets = nullptr;

	virtual inline void _init_bullet(BulletType *bullet);
	virtual inline void _enable_bullet(BulletType *bullet);
	virtual inline void _disable_bullet(BulletType *bullet);
	virtual inline bool _process_bullet(BulletType *bullet, float delta);

	inline void _release_bullet(int32_t index);

public:
	AbstractBulletsPool() {}
	virtual ~AbstractBulletsPool();

	virtual void _init(Node *parent_hint, RID shared_area, int32_t starting_shape_index,
			int32_t set_index, Ref<BulletKit> kit, int32_t pool_size, int32_t z_index) override;

	virtual int32_t _process(float delta) override;

	virtual void spawn_bullet(Dictionary properties) override;
	virtual BulletID obtain_bullet() override;
	virtual bool release_bullet(BulletID id) override;
	virtual bool is_bullet_valid(BulletID id) override;

	virtual bool is_bullet_existing(int32_t shape_index) override;
	virtual BulletID get_bullet_from_shape(int32_t shape_index) override;

	virtual void set_bullet_property(BulletID id, String property, Variant value) override;
	virtual Variant get_bullet_property(BulletID id, String property) override;
};

#include "bullets_pool.inl"

#endif // BULLETS_POOL_H
