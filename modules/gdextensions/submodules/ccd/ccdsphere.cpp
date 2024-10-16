/**************************************************************************/
/*  ccdsphere.cpp                                                         */
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

#include "ccdsphere.h"
#include "ccdbox.h"
#include "ccdcylinder.h"

void CCDSphere::_bind_methods() {
	ClassDB::bind_method(D_METHOD("initialize"), &CCDSphere::initialize);
	ClassDB::bind_method(D_METHOD("collidesWithGJK"), &CCDSphere::collidesWithGJK);
	ClassDB::bind_method(D_METHOD("collidesWithMPR"), &CCDSphere::collidesWithMPR);
	ClassDB::bind_method(D_METHOD("collidesWithGJKAndInfo"), &CCDSphere::collidesWithGJKAndInfo);
	ClassDB::bind_method(D_METHOD("collidesWithMPRAndInfo"), &CCDSphere::collidesWithMPRAndInfo);
	ClassDB::bind_method(D_METHOD("getPosition"), &CCDSphere::getPosition);
	ClassDB::bind_method(D_METHOD("getRotation"), &CCDSphere::getRotation);
	ClassDB::bind_method(D_METHOD("getCCDType"), &CCDSphere::getCCDType);
}

CCDSphere::CCDSphere() {
	// Init sphere
	ccdSphere.type = CCD_OBJ_SPHERE;
	ccdSphere.quat = { { 0., 0., 0., 1. } };
}

void CCDSphere::initialize(Vector3 position, Quat rotation, float radius) {
	ccdSphere.pos.v[0] = position.x;
	ccdSphere.pos.v[1] = position.z;
	ccdSphere.pos.v[2] = position.y;
	ccdSphere.quat.q[0] = rotation.x;
	ccdSphere.quat.q[1] = rotation.z;
	ccdSphere.quat.q[2] = -rotation.y;
	ccdSphere.quat.q[3] = rotation.w;
	ccdSphere.radius = radius;
}

bool CCDSphere::collidesWithGJK(Variant other) {
	return CCDBase::collidesWithGJK(other);
}

bool CCDSphere::collidesWithGJKAndInfo(Variant other, Dictionary outParam) {
	return CCDBase::collidesWithGJKAndInfo(other, outParam);
}

bool CCDSphere::collidesWithMPR(Variant other) {
	return CCDBase::collidesWithMPR(other);
}

bool CCDSphere::collidesWithMPRAndInfo(Variant other, Dictionary outParam) {
	return CCDBase::collidesWithMPRAndInfo(other, outParam);
}

Vector3 CCDSphere::getPosition() const {
	return Vector3(ccdSphere.pos.v[0], ccdSphere.pos.v[2], ccdSphere.pos.v[1]);
}

Quat CCDSphere::getRotation() const {
	return Quat(ccdSphere.quat.q[0], -ccdSphere.quat.q[2], ccdSphere.quat.q[1], ccdSphere.quat.q[3]);
}
