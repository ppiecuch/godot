/**************************************************************************/
/*  bullet_server.h                                                       */
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

#ifndef BULLETSERVER_H
#define BULLETSERVER_H

#include "bullet.h"
#include "bullet_type.h"
#include "core/os/os.h"
#include "scene/2d/node_2d.h"
#include <vector>

class BulletServer : public Node2D {
	GDCLASS(BulletServer, Node2D);

public:
	enum AreaMode {
		VIEWPORT,
		MANUAL,
		INFINITE,
	};

private:
	int bullet_pool_size;

	bool pop_on_collide;
	float max_lifetime;

	Vector<Bullet *> live_bullets;
	Vector<Bullet *> dead_bullets;

	AreaMode play_area_mode;
	Rect2 play_area_rect;
	float play_area_margin;
	bool play_area_allow_incoming;

	bool relay_autoconnect;

	void _process_bullets(float delta);

	void _init_bullets();
	void _create_bullet();

	void _update_play_area();

protected:
	static void _bind_methods();
	void _notification(int p_what);
	void _validate_property(PropertyInfo &property) const;

public:
	BulletServer();
	~BulletServer();

	void spawn_bullet(const Ref<BulletType> &p_type, const Vector2 &p_position, const Vector2 &p_direction);
	void spawn_volley(const Ref<BulletType> &p_type, const Vector2 &p_position, const Array &p_volley);

	void clear_bullets();
	int get_live_bullet_count();

	Array get_live_bullets();
	Array get_live_bullet_positions();

	void set_bullet_pool_size(int p_size);
	int get_bullet_pool_size() const;

	void set_pop_on_collide(bool p_enabled);
	bool get_pop_on_collide() const;

	void set_max_lifetime(float p_time);
	float get_max_lifetime() const;

	void set_play_area_mode(AreaMode p_mode);
	AreaMode get_play_area_mode() const;

	void set_play_area_rect(const Rect2 &p_rect);
	Rect2 get_play_area_rect() const;

	void set_play_area_margin(float p_margin);
	float get_play_area_margin() const;

	void set_play_area_allow_incoming(bool p_enabled);
	bool get_play_area_allow_incoming() const;

	void set_relay_autoconnect(bool p_enabled);
	bool get_relay_autoconnect() const;
};

VARIANT_ENUM_CAST(BulletServer::AreaMode)

#endif
