/**************************************************************************/
/*  ccdbase.h                                                             */
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

#ifndef CCDBASE_H
#define CCDBASE_H

#include "core/dictionary.h"
#include "core/math/quat.h"
#include "core/object.h"
#include "core/variant.h"

#include "ccd/ccd.h"
#include "testsuites/support.h"

class CCDBase : public Object {
	GDCLASS(CCDBase, Object)

protected:
	static ccd_t ccd;
	static bool ccdInitialized;

public:
	enum CCDType {
		CCDTYPE_INVALID = -1,
		CCDTYPE_BOX,
		CCDTYPE_SPHERE,
		CCDTYPE_CYLINDER,
		NUM_CCD_TYPES
	};

	/// Returns true if this object collides with the passed one.
	bool collidesWithGJK(Variant other);

	/// Returns true if this object collides with the passed one.
	/// Will also fill the outParam with details about the collision.
	bool collidesWithGJKAndInfo(Variant other, Dictionary outParam);

	/// Returns true if this object collides with the passed one.
	bool collidesWithMPR(Variant other);

	/// Returns true if this object collides with the passed one.
	/// Will also fill the outParam with details about the collision.
	bool collidesWithMPRAndInfo(Variant other, Dictionary outParam);

	virtual Vector3 getPosition() const { return Vector3(0.0, 0.0, 0.0); }

	virtual Quat getRotation() const { return Quat(0.0, 0.0, 0.0, 1.0); }

	virtual void *getCCDStruct() const { return nullptr; }

	virtual int getCCDType() const { return CCDTYPE_INVALID; }

	CCDBase();
};

#endif // CCDBASE_H
