/**************************************************************************/
/*  detourobstacle.h                                                      */
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

#ifndef GODOTDETOUROBSTACLE_H
#define GODOTDETOUROBSTACLE_H

#include "core/math/vector3.h"
#include "core/os/file_access.h"
#include "core/reference.h"

#include <map>

class dtTileCache;

// Obstacle types
enum DetourObstacleType {
	OBSTACLE_TYPE_INVALID = -1,
	OBSTACLE_TYPE_CYLINDER,
	OBSTACLE_TYPE_BOX,
	NUM_OBSTACLE_TYPES
};

/// A single obstacle. Can be moved around or destroyed.
class DetourObstacle : public Reference {
	GD_CLASS(DetourObstacle, Reference)

	DetourObstacleType _type;

	Vector3 _position;
	Vector3 _dimensions; // In case of cylinder, x = radius, y = height, z = unused
	float _rotationRad;
	bool _destroyed;

	std::map<dtTileCache *, unsigned int> _references;

protected:
	static void _bind_methods();

public:
	void _init() {} // Called when .new() is called in gdscript

	DetourObstacleType getType() const; // Returns the type of this obstacle.

	void initialize(DetourObstacleType type, const Vector3 &position, const Vector3 &dimensions, float rotationRad);

	bool save(FileAccessRef targetFile); // Will save this obstacle's current state to the passed file.
	bool load(FileAccessRef sourceFile); // Loads the obstacle from the file.

	void createDetourObstacle(dtTileCache *cache); // Create the obstacle using the passed tile cache. Will also remember the reference.

	void addReference(unsigned int ref, dtTileCache *cache); // Pass the reference of this obstacle and the navmesh it is in.
	void move(Vector3 position); // Move this obstacle to a new position.

	void destroy(); // Destroy this obstacle, removing it from all navmeshes.

	bool isDestroyed(); // Returns true if this was destroyed.

	DetourObstacle();
	~DetourObstacle();
};

inline DetourObstacleType DetourObstacle::getType() const {
	return _type;
}
inline bool DetourObstacle::isDestroyed() {
	return _destroyed;
}

#endif // GODOTDETOUROBSTACLE_H
