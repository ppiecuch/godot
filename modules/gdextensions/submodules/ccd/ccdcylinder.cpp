/**************************************************************************/
/*  ccdcylinder.cpp                                                       */
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

/*
 * Copyright (c) 2019 Jan Drabner jd at jdrabner.eu
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "ccdcylinder.h"
#include "ccdbox.h"
#include "ccdsphere.h"

void CCDCylinder::_bind_methods() {
	ClassDB::bind_method(D_METHOD("initialize"), &CCDCylinder::initialize);
	ClassDB::bind_method(D_METHOD("collidesWithGJK"), &CCDCylinder::collidesWithGJK);
	ClassDB::bind_method(D_METHOD("collidesWithMPR"), &CCDCylinder::collidesWithMPR);
	ClassDB::bind_method(D_METHOD("collidesWithGJKAndInfo"), &CCDCylinder::collidesWithGJKAndInfo);
	ClassDB::bind_method(D_METHOD("collidesWithMPRAndInfo"), &CCDCylinder::collidesWithMPRAndInfo);
	ClassDB::bind_method(D_METHOD("getPosition"), &CCDCylinder::getPosition);
	ClassDB::bind_method(D_METHOD("getRotation"), &CCDCylinder::getRotation);
	ClassDB::bind_method(D_METHOD("getCCDType"), &CCDCylinder::getCCDType);
}

CCDCylinder::CCDCylinder() :
		CCDBase() {
	// Init sphere
	ccdCylinder.type = CCD_OBJ_CYL;
	ccdCylinder.quat = { { 0., 0., 0., 1. } };
}

void CCDCylinder::initialize(Vector3 position, Quat rotation, float radius, float height) {
	ccdCylinder.pos.v[0] = position.x;
	ccdCylinder.pos.v[1] = position.z;
	ccdCylinder.pos.v[2] = position.y;
	ccdCylinder.quat.q[0] = rotation.x;
	ccdCylinder.quat.q[1] = rotation.z;
	ccdCylinder.quat.q[2] = -rotation.y;
	ccdCylinder.quat.q[3] = rotation.w;
	ccdCylinder.radius = radius;
	ccdCylinder.height = height;
}

bool CCDCylinder::collidesWithGJK(Variant other) {
	return CCDBase::collidesWithGJK(other);
}

bool CCDCylinder::collidesWithGJKAndInfo(Variant other, Dictionary outParam) {
	return CCDBase::collidesWithGJKAndInfo(other, outParam);
}

bool CCDCylinder::collidesWithMPR(Variant other) {
	return CCDBase::collidesWithMPR(other);
}

bool CCDCylinder::collidesWithMPRAndInfo(Variant other, Dictionary outParam) {
	return CCDBase::collidesWithMPRAndInfo(other, outParam);
}

Vector3 CCDCylinder::getPosition() const {
	return Vector3(ccdCylinder.pos.v[0], ccdCylinder.pos.v[2], ccdCylinder.pos.v[1]);
}

Quat CCDCylinder::getRotation() const {
	return Quat(ccdCylinder.quat.q[0], -ccdCylinder.quat.q[2], ccdCylinder.quat.q[1], ccdCylinder.quat.q[3]);
}
