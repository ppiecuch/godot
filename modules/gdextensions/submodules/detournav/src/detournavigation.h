/*************************************************************************/
/*  detournavigation.h                                                   */
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

#ifndef GODOTDETOURNAVIGATION_H
#define GODOTDETOURNAVIGATION_H

#include "core/reference.h"
#include "core/variant.h"

#include <atomic>
#include <map>
#include <vector>

#include "detourcrowdagent.h"
#include "detournavigationmesh.h"

class DetourInputGeometry;
class RecastContext;
class GodotDetourDebugDraw;

namespace std {
class thread;
class mutex;
} //namespace std

class DetourObstacle;
class MeshInstance;
class Material;

/// Parameters to initialize a DetourNavigation.
struct DetourNavigationParameters : public Reference {
	GD_CLASS(DetourNavigationParameters, Reference)

protected:
	static void _bind_methods();

public:
	void _init() {} // Called when .new() is called in gdscript

	Array navMeshParameters; // The number of elements in this array determines how many DetourNavigationMeshes there will be.
	int ticksPerSecond; // How many updates per second the navigation shall do in its thread.
	int maxObstacles; // The maximum amount of obstacles allowed at the same time. Obstacles beyond this amount will be rejected.
	int defaultAreaType; // The default area type to mark geometry as
};

/// Main class to initialize GodotDetour and interact with it.
class DetourNavigation : public Reference {
	GD_CLASS(DetourNavigation, Reference)

	DetourInputGeometry *_inputGeometry;
	std::vector<DetourNavigationMesh *> _navMeshes;
	std::vector<Ref<DetourCrowdAgent>> _agents;
	std::vector<Ref<DetourObstacle>> _obstacles;
	std::vector<int> _markedAreaIDs;
	std::vector<int> _removedMarkedAreaIDs;
	std::vector<int> _offMeshConnections;
	std::vector<int> _removedOffMeshConnections;

	RecastContext *_recastContext;
	GodotDetourDebugDraw *_debugDrawer;

	bool _initialized;
	int _ticksPerSecond;
	int _maxObstacles;
	int _defaultAreaType;

	std::thread *_navigationThread;
	std::atomic_bool _stopThread;
	std::mutex *_navigationMutex;

	std::map<String, int> _queryFilterIndices;

protected:
	static void _bind_methods();

public:
	void _init() {} // Called when .new() is called in gdscript

	bool isInitialized();

	// Initialize the navigation. If called on an already initialized instance, will return false.
	bool initialize(Variant inputMeshInstance, Ref<DetourNavigationParameters> parameters);

	void rebuildChangedTiles(); // Rebuilds all tiles that have changed (by marking areas).

	// Marks the area as the passed type (influencing crowds based on their area filters).
	int markConvexArea(Array vertices, float height, unsigned int areaType);

	// Removes the convex marked area with the passed id.
	void removeConvexAreaMarker(int id);

	// Creates and adds an off-mesh connection.
	int addOffMeshConnection(Vector3 from, Vector3 to, bool bidirectional, float radius, int areaType);

	void removeOffMeshConnection(int id); // Removes the off-mesh connection with the passed ID.

	bool setQueryFilter(int index, String name, Dictionary weights); // Sets the filter at the passed index.

	Ref<DetourCrowdAgent> addAgent(Ref<DetourCrowdAgentParameters> parameters); // Adds an agent to the navigation.

	void removeAgent(Ref<DetourCrowdAgent> agent); // Will remove the passed agent.

	Ref<DetourObstacle> addCylinderObstacle(Vector3 position, float radius, float height); // Add a cylindric dynamic obstacle.

	Ref<DetourObstacle> addBoxObstacle(Vector3 position, Vector3 dimensions, float rotationRad); // Add a box dynamic obstacle.

	// Creates a debug mesh for the navmesh at the passed index.
	MeshInstance *createDebugMesh(int index, bool drawCacheBounds);

	// Saves the current state of the navigation, including all marked areas, temp obstacles and agents.
	bool save(String path, bool compressed);

	// Loads an entire navigation state, including marked areas, temp obstacles and agents.
	bool load(String path, bool compressed);

	// Clears the entire navigation (all the data) and stops the navigation thread.
	// After this, a new initialize (or load) will be required.
	void clear();

	Array getAgents(); // Returns all current agents.

	Array getObstacles(); // Returns all current temporary obstacles.

	Array getMarkedAreaIDs(); // Returns all currently marked areas' IDs.

	// This function is the thread running in the background, taking care of navigation updates.
	void navigationThreadFunction();

	DetourNavigation();
	~DetourNavigation();
};

inline bool DetourNavigation::isInitialized() {
	return _initialized;
}

#endif // GODOTDETOURNAVIGATION_H
