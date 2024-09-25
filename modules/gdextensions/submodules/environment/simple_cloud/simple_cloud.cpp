/**************************************************************************/
/*  simple_cloud.cpp                                                      */
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

#include "core/math/vector2.h"
#include "core/math/vector3.h"

struct SimpleRotator {
	Vector3 rotation = Vector3(0, 1, 0);
	real_t speed = 1;

	void update(real_t delta) {
		transform.Rotate(rotation * speed * delta);
	}
};

struct SimpleTextureRotator {
	Vector2 rotation = Vector2(1, 1);
	Vector2 _tiling;
	real_t speedMultiplier = 1;

	void Start() {
		_tiling = GetComponent<Renderer>().material.mainTextureScale;
	}

	void Update(real_t delta) {
		Vector2 _tempVector2 = GetComponent<Renderer>().material.mainTextureOffset;
		Vector2 modification = rotation * delta * speedMultiplier;
		modification.Scale(_tiling);
		_tempVector2 += modification;

		if (_tempVector2.x > 1) {
			_tempVector2.x -= 1;
		}
		if (_tempVector2.x < -1) {
			_tempVector2.x += 1;
		}
		if (_tempVector2.y > 1) {
			_tempVector2.y -= 1;
		}
		if (_tempVector2.y < -1) {
			_tempVector2.y += 1;
		}

		GetComponent<Renderer>().material.mainTextureOffset = _tempVector2;
	}

	void FindNewStartPosition() {
		GetComponent<Renderer>().material.mainTextureOffset = new Vector2(Random.value, Random.value);
	}
}

class CloudFaker {
	real_t minWindStrength = 0.0005f;
	real_t maxWindStrength = 0.0015f;

	real_t minWindRotation = 0.025f;
	real_t maxWindRotation = 0.075f;

	bool limitWindDirection;
	real_t WindDirection = 0;
	real_t WindDirectionVariance = 180f;

	void InitComponents() {
		real_t windSpeed = Random.Range(minWindStrength, maxWindStrength);

		//if (Random.value > 0.5) {
		//  windSpeed = -windSpeed;
		//}

		GetComponent<SimpleTextureRotator>().speedMultiplier = windSpeed;
		GetComponent<SimpleTextureRotator>().FindNewStartPosition();

		real_t windRotation = Random.Range(minWindRotation, maxWindRotation);

		if (Random.value > 0.5) {
			windRotation = -windRotation;
		}

		GetComponent<SimpleRotator>().speed = windRotation;
		real_t targetRotation;
		if (limitWindDirection) {
			targetRotation = Random.Range(WindDirection - WindDirectionVariance, WindDirection + WindDirectionVariance);
		} else {
			targetRotation = Random.Range(0, 360);
		}
		var localEulerAngles = transform.localEulerAngles;
		// var y = localEulerAngles.y - targetRotation;
		localEulerAngles = (new Vector3(localEulerAngles.x, targetRotation, localEulerAngles.z));
		transform.localEulerAngles = localEulerAngles;
	}

	void Update() {
		if (DebugReCalc) {
			DebugReCalc = false;
			InitComponents();
		}
	}

public:
	bool DebugReCalc;

	void Start() {
		InitComponents();
	}

	void RecalcNow() {
		InitComponents();
	}
};
