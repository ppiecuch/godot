/**************************************************************************/
/*  register.cpp                                                          */
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

#include "register.h"

#include "bullet.h"
#include "bullet_server.h"
#include "bullet_server_relay.h"
#include "bullet_spawner.h"
#include "bullet_type.h"
#include "core/class_db.h"
#include "core/engine.h"

BulletServerRelay *_bullet_server_relay = nullptr;

void register_qurobullet() {
	_bullet_server_relay = memnew(BulletServerRelay);

	ClassDB::register_class<Bullet>();
	ClassDB::register_class<BulletType>();
	ClassDB::register_class<BulletServer>();
	ClassDB::register_class<BulletServerRelay>();
	ClassDB::register_class<BulletSpawner>();

	Engine::get_singleton()->add_singleton(Engine::Singleton("BulletServerRelay", _bullet_server_relay));
}

void unregister_qurobullet() {
	if (_bullet_server_relay) {
		memdelete(_bullet_server_relay);
	}
}
