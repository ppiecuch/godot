/**************************************************************************/
/*  detourobstacle.cpp                                                    */
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

#include "detourobstacle.h"
#include <DetourTileCache.h>

#define OBSTACLE_SAVE_VERSION 1

void DetourObstacle::_bind_methods() {
	ClassDB::bind_method(D_METHOD("move", "position"), &DetourObstacle::move);
	ClassDB::bind_method(D_METHOD("destroy"), &DetourObstacle::destroy);

	ClassDB::bind_method(D_METHOD("set_position", "value"), &DetourObstacle::set_position_prop);
	ClassDB::bind_method(D_METHOD("get_position"), &DetourObstacle::get_position_prop);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "position"), "set_position", "get_position");

	ClassDB::bind_method(D_METHOD("set_dimensions", "value"), &DetourObstacle::set_dimensions);
	ClassDB::bind_method(D_METHOD("get_dimensions"), &DetourObstacle::get_dimensions);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "dimensions"), "set_dimensions", "get_dimensions");
}

DetourObstacle::DetourObstacle() :
		_type(OBSTACLE_TYPE_INVALID), _position(Vector3(0.0f, 0.0f, 0.0f)), _dimensions(Vector3(0.0f, 0.0f, 0.0f)), _destroyed(false) {
}

DetourObstacle::~DetourObstacle() {
	if (!_references.empty()) {
		ERR_PRINT("Destructor of obstacle was called with non-empty references!");
	}
}

void DetourObstacle::initialize(DetourObstacleType type, const Vector3 &position, const Vector3 &dimensions, float rotationRad) {
	_type = type;
	_position = position;
	_dimensions = dimensions;
	_rotationRad = rotationRad;
}

bool DetourObstacle::save(FileAccess *targetFile) {
	// Version
	targetFile->store_16(OBSTACLE_SAVE_VERSION);

	// Properties
	targetFile->store_16(_type);
	targetFile->store_float(_position.x);
	targetFile->store_float(_position.y);
	targetFile->store_float(_position.z);
	targetFile->store_float(_dimensions.x);
	targetFile->store_float(_dimensions.y);
	targetFile->store_float(_dimensions.z);
	targetFile->store_float(_rotationRad);
	targetFile->store_8(_destroyed);

	return true;
}

bool DetourObstacle::load(FileAccess *sourceFile) {
	// Version
	int version = sourceFile->get_16();
	if (version == OBSTACLE_SAVE_VERSION) {
		// Properties
		_type = (DetourObstacleType)sourceFile->get_16();
		_position.x = sourceFile->get_float();
		_position.y = sourceFile->get_float();
		_position.z = sourceFile->get_float();
		_dimensions.x = sourceFile->get_float();
		_dimensions.y = sourceFile->get_float();
		_dimensions.z = sourceFile->get_float();
		_rotationRad = sourceFile->get_float();
		_destroyed = sourceFile->get_8();
	} else {
		ERR_PRINT("Unable to load obstacle. Unknown version: " + itos(version));
		return false;
	}

	return true;
}

void DetourObstacle::createDetourObstacle(dtTileCache *cache) {
	dtObstacleRef ref;
	switch (_type) {
		case OBSTACLE_TYPE_CYLINDER: {
			float pos[3];
			pos[0] = _position.x;
			pos[1] = _position.y;
			pos[2] = _position.z;
			dtStatus status = cache->addObstacle(pos, _dimensions.x, _dimensions.y, &ref);
			if (dtStatusFailed(status)) {
				ERR_PRINT("createDetourObstacle: Failed to add cylinder obstacle.");
				return;
			}
			break;
		}

		case OBSTACLE_TYPE_BOX: {
			float pos[3];
			pos[0] = _position.x;
			pos[1] = _position.y;
			pos[2] = _position.z;
			float halfExtents[3];
			halfExtents[0] = _dimensions.x * 0.5f;
			halfExtents[1] = _dimensions.y * 0.5f;
			halfExtents[2] = _dimensions.z * 0.5f;
			dtStatus status = cache->addBoxObstacle(pos, halfExtents, _rotationRad, &ref);
			if (dtStatusFailed(status)) {
				ERR_PRINT("createDetourObstacle: Failed to add box obstacle.");
				return;
			}
			break;
		}

		default:
			ERR_PRINT("createDetourObstacle: Invalid obstacle type " + itos(_type));
			return;
	}

	addReference(ref, cache);
}

void DetourObstacle::addReference(unsigned int ref, dtTileCache *cache) {
	_references[cache] = ref;
}

void DetourObstacle::move(Vector3 position) {
	// Iterate over all tile caches
	for (auto const &it : _references) {
		// Remove the obstacle
		it.first->removeObstacle(it.second);

		// Add the obstacle again at a new position
		_position = position;
		createDetourObstacle(it.first);
	}
}

void DetourObstacle::destroy() {
	// Iterate over all tile caches
	for (auto const &it : _references) {
		it.first->removeObstacle(it.second);
	}
	_references.clear();
	_destroyed = true;
}

#ifdef DOCTEST
#include "core/os/dir_access.h"
#include "doctest/doctest.h"

TEST_SUITE("[[detournav]] DetourObstacle") {
	TEST_CASE("[detournav] default construction") {
		DetourObstacle obs;
		CHECK(obs.getType() == OBSTACLE_TYPE_INVALID);
		CHECK(obs.isDestroyed() == false);
		CHECK(obs.get_position_prop() == Vector3(0, 0, 0));
		CHECK(obs.get_dimensions() == Vector3(0, 0, 0));
	}

	TEST_CASE("[detournav] initialize cylinder obstacle") {
		DetourObstacle obs;
		obs.initialize(OBSTACLE_TYPE_CYLINDER, Vector3(1, 2, 3), Vector3(5, 10, 0), 0.0f);
		CHECK(obs.getType() == OBSTACLE_TYPE_CYLINDER);
		CHECK(obs.get_position_prop() == Vector3(1, 2, 3));
		CHECK(obs.get_dimensions() == Vector3(5, 10, 0));
	}

	TEST_CASE("[detournav] initialize box obstacle") {
		DetourObstacle obs;
		obs.initialize(OBSTACLE_TYPE_BOX, Vector3(10, 20, 30), Vector3(2, 4, 6), 1.57f);
		CHECK(obs.getType() == OBSTACLE_TYPE_BOX);
		CHECK(obs.get_position_prop() == Vector3(10, 20, 30));
		CHECK(obs.get_dimensions() == Vector3(2, 4, 6));
	}

	TEST_CASE("[detournav] obstacle property setters and getters") {
		DetourObstacle obs;
		obs.set_position_prop(Vector3(5, 10, 15));
		CHECK(obs.get_position_prop() == Vector3(5, 10, 15));

		obs.set_dimensions(Vector3(1, 2, 3));
		CHECK(obs.get_dimensions() == Vector3(1, 2, 3));
	}

	TEST_CASE("[detournav] obstacle destroy without references") {
		DetourObstacle obs;
		obs.initialize(OBSTACLE_TYPE_CYLINDER, Vector3(1, 2, 3), Vector3(5, 10, 0), 0.0f);
		CHECK(obs.isDestroyed() == false);
		obs.destroy();
		CHECK(obs.isDestroyed() == true);
	}

	TEST_CASE("[detournav] obstacle save and load roundtrip") {
		// Create and initialize
		DetourObstacle original;
		original.initialize(OBSTACLE_TYPE_BOX, Vector3(1.5f, 2.5f, 3.5f), Vector3(4.0f, 5.0f, 6.0f), 0.785f);

		// Save to a temporary file
		String path = "user://test_obstacle_roundtrip.bin";
		FileAccess *f = FileAccess::open(path, FileAccess::WRITE);
		REQUIRE(f != nullptr);
		CHECK(original.save(f));
		f->close();
		memdelete(f);

		// Load into a new obstacle
		DetourObstacle loaded;
		f = FileAccess::open(path, FileAccess::READ);
		REQUIRE(f != nullptr);
		CHECK(loaded.load(f));
		f->close();
		memdelete(f);

		CHECK(loaded.getType() == OBSTACLE_TYPE_BOX);
		CHECK(loaded.get_position_prop().x == doctest::Approx(1.5f));
		CHECK(loaded.get_position_prop().y == doctest::Approx(2.5f));
		CHECK(loaded.get_position_prop().z == doctest::Approx(3.5f));
		CHECK(loaded.get_dimensions().x == doctest::Approx(4.0f));
		CHECK(loaded.get_dimensions().y == doctest::Approx(5.0f));
		CHECK(loaded.get_dimensions().z == doctest::Approx(6.0f));

		// Cleanup
		DirAccess *dir = DirAccess::create_for_path("user://");
		if (dir) {
			dir->remove(path);
			memdelete(dir);
		}
	}
}
#endif
