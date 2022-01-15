/*************************************************************************/
/*  gdlibrary.cpp                                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
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

#include "bullets.h"
#include "kits/basic_bullet_kit.h"
#include "kits/dynamic_bullet_kit.h"
#include "kits/following_bullet_kit.h"
#include "kits/following_dynamic_bullet_kit.h"

void register_bullet_kit() {
	ClassDB::register_class<Bullet>();
	ClassDB::register_class<BulletKit>();
	ClassDB::register_class<Bullets>();

	// Default Bullet Kits.
	ClassDB::register_class<BasicBulletKit>();

	ClassDB::register_class<FollowingBullet>();
	ClassDB::register_class<FollowingBulletKit>();

	ClassDB::register_class<DynamicBullet>();
	ClassDB::register_class<DynamicBulletKit>();

	ClassDB::register_class<FollowingDynamicBullet>();
	ClassDB::register_class<FollowingDynamicBulletKit>();

	// Custom Bullet Kits.
	//register_class<CustomBulletKit>();
}
