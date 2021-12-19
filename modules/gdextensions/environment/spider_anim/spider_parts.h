/*************************************************************************/
/*  spider_parts.h                                                       */
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

#ifndef GD_SPIDER_PARTS_H
#define GD_SPIDER_PARTS_H

#include "spider_sprite.h"

const String __spider_parts = "__spider_parts";

class Ped : public SpiderSprite {
	GDCLASS(Ped, SpiderSprite)

public:
	void init_geometry();
	Ped();
};

class Leg1 : public SpiderSprite {
	GDCLASS(Leg1, SpiderSprite)

public:
	void init_geometry();
	Leg1();
};

class Leg2 : public SpiderSprite {
	GDCLASS(Leg2, SpiderSprite)

public:
	void init_geometry();
	Leg2();
};

class Leg2b : public SpiderSprite {
	GDCLASS(Leg2b, SpiderSprite)

public:
	void init_geometry();
	Leg2b();
};

class Leg3 : public SpiderSprite {
	GDCLASS(Leg3, SpiderSprite)

public:
	void init_geometry();
	Leg3();
};

class Leg4 : public SpiderSprite {
	GDCLASS(Leg4, SpiderSprite)

public:
	void init_geometry();
	Leg4();
};

class Body : public SpiderSprite {
	GDCLASS(Body, SpiderSprite)

public:
	SpiderSprite *sprite = 0;
	SpiderSprite *light = 0;

public:
	void init_geometry();
	Body();
};

class Body2 : public SpiderSprite {
	GDCLASS(Body2, SpiderSprite)

public:
	SpiderSprite *sprite = 0;

public:
	void init_geometry();
	Body2();
};

class Back : public SpiderSprite {
	GDCLASS(Back, SpiderSprite)

public:
	SpiderSprite *sprite = 0;
	SpiderSprite *light = 0;

public:
	void init_geometry();
	Back();
};

#endif // GD_SPIDER_PARTS_H
