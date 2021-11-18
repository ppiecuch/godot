/*************************************************************************/
/*  bullets.h                                                            */
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

#ifndef BULLETS_H
#define BULLETS_H

#include "core/array.h"
#include "core/color.h"
#include "scene/2d/node_2d.h"
#include "scene/resources/material.h"
#include "scene/resources/texture.h"

#include <memory>
#include <vector>

#include "bullet_kit.h"
#include "bullets_pool.h"

class Bullets : public Node2D {
	GDCLASS(Bullets, Node2D)

private:
	// A pool internal representation with related properties.
	struct PoolKit {
		std::unique_ptr<BulletsPool> pool;
		Ref<BulletKit> bullet_kit;
		int32_t size;
		int32_t z_index;
	};
	struct PoolKitSet {
		std::vector<PoolKit> pools;
		int32_t bullets_amount;
	};
	// PoolKitSets represent PoolKits organized by their shared area.
	std::vector<PoolKitSet> pool_sets;
	// Maps each area RID to the corresponding PoolKitSet index.
	Dictionary areas_to_pool_set_indices;
	// Maps each BulletKit to the corresponding PoolKit index.
	Dictionary kits_to_set_pool_indices;

	Node *bullets_environment = nullptr;

	int32_t available_bullets = 0;
	int32_t active_bullets = 0;
	int32_t total_bullets = 0;

	Array shared_areas;
	PoolIntArray invalid_id;

	void _clear_rids();
	int32_t _get_pool_index(int32_t set_index, int32_t bullet_index);

public:
	static void _register_methods();

	Bullets();
	~Bullets();

	void _init();

	void _physics_process(float delta);

	void mount(Node *bullets_environment);
	void unmount(Node *bullets_environment);
	Node *get_bullets_environment();

	bool spawn_bullet(Ref<BulletKit> kit, Dictionary properties);
	Variant obtain_bullet(Ref<BulletKit> kit);
	bool release_bullet(Variant id);

	bool is_bullet_valid(Variant id);
	bool is_kit_valid(Ref<BulletKit> kit);

	int32_t get_available_bullets(Ref<BulletKit> kit);
	int32_t get_active_bullets(Ref<BulletKit> kit);
	int32_t get_pool_size(Ref<BulletKit> kit);
	int32_t get_z_index(Ref<BulletKit> kit);

	int32_t get_total_available_bullets();
	int32_t get_total_active_bullets();

	bool is_bullet_existing(RID area_rid, int32_t shape_index);
	Variant get_bullet_from_shape(RID area_rid, int32_t shape_index);
	Ref<BulletKit> get_kit_from_bullet(Variant id);

	void set_bullet_property(Variant id, String property, Variant value);
	Variant get_bullet_property(Variant id, String property);
};

#endif // BULLETS_H
