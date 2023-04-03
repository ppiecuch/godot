/**************************************************************************/
/*  spider_parts.cpp                                                      */
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

#include "spider_parts.h"

void Ped::init_geometry() {
	SpiderSprite::init_geometry(theme[part], Rect2(-6.05, -12.85, 37.9, 23.5));
}
Ped::Ped() :
		SpiderSprite(SpiderBodyPed) {
	set_name("_spider_ped");
	init_geometry();
}

void Leg1::init_geometry() {
	SpiderSprite::init_geometry(theme[part], Rect2(-21.6, -13.5, 128.9, 26));
}
Leg1::Leg1() :
		SpiderSprite(SpiderLegTop) {
	set_name("_spider_leg1");
	init_geometry();
}

void Leg2::init_geometry() {
	SpiderSprite::init_geometry(theme[part], Rect2(-13.9, -12.8, 126.95, 25.9));
}
Leg2::Leg2() :
		SpiderSprite(SpiderLegMiddle) {
	set_name("_spider_leg2");
	init_geometry();
}

void Leg2b::init_geometry() {
	SpiderSprite::init_geometry(theme[part], Rect2(-12.5, -8, 120, 15));
}
Leg2b::Leg2b() :
		SpiderSprite(SpiderLegBottom) {
	set_name("_spider_leg2b");
	init_geometry();
}

void Leg3::init_geometry() {
	SpiderSprite::init_geometry(theme[part], Rect2(-27, -24, 140, 46.5));
}
Leg3::Leg3() :
		SpiderSprite(SpiderLegTopShadow) {
	set_name("_spider_leg3");
	init_geometry();
}

void Leg4::init_geometry() {
	SpiderSprite::init_geometry(theme[part], Rect2(-13.5, -12, 118, 26.9));
}
Leg4::Leg4() :
		SpiderSprite(SpiderLegBottomShadow) {
	set_name("_spider_leg4");
	init_geometry();
}

void Body::init_geometry() {
	sprite = make_sprite_spider(SpiderBodyFront, Rect2(-43.2, -37, 89.45, 75.7));
	light = make_sprite_spider(SpiderBodyBackTop, Rect2(4, -31.45, 84, 64), Point2(-35.6, -1.2), 0.66);

	sprite->add_child(light);
	add_child(sprite);
}
Body::Body() :
		SpiderSprite(SpiderSpriteGroupNode) {
	set_name("_spider_body");
	init_geometry();
}

void Body2::init_geometry() {
	sprite = make_sprite_spider(SpiderBodyShadow, Rect2(-101.5, -38.85, 137.5, 82.75));
	sprite->set_color(Color::named("white"));

	add_child(sprite);
}
Body2::Body2() :
		SpiderSprite(SpiderSpriteGroupNode) {
	set_name("_spider_body2");
	init_geometry();
}

void Back::init_geometry() {
	sprite = make_sprite_spider(SpiderBodyBack, -87.55, -32, 92, 71.25);
	light = make_sprite_spider(SpiderBodyBackTop, Rect2(4.0, -31.45, 84, 64), Point2(-87.8, 3.6));
	light->set_alpha(0.8);

	sprite->add_child(light);
	add_child(sprite);
}
Back::Back() :
		SpiderSprite(SpiderSpriteGroupNode) {
	set_name("_spider_back");
	init_geometry();
}
