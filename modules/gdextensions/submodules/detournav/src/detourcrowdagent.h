/*************************************************************************/
/*  detourcrowdagent.h                                                   */
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

#ifndef GODOTDETOURCROWDAGENT_H
#define GODOTDETOURCROWDAGENT_H

#include "core/math/vector3.h"
#include "core/os/file_access.h"
#include "core/reference.h"

#include <atomic>
#include <chrono>
#include <map>
#include <vector>

class dtCrowdAgent;
class dtCrowd;
class dtNavMeshQuery;
class dtQueryFilter;
class DetourInputGeometry;
class DetourNavigationMesh;

// Parameters to initialize a DetourCrowdAgent.
struct DetourCrowdAgentParameters : public Reference {
	GD_CLASS(DetourCrowdAgentParameters, Reference)

protected:
	static void _bind_methods();

public:
	void _init() {} // Called when .new() is called in gdscript

	Vector3 position;

	// These two parameters will determine into which navigation mesh & crowd this agent will be put.
	// Make sure your DetourNavigationMesh supports the radius & height.
	float radius;
	float height;

	float maxAcceleration;
	float maxSpeed;

	String filterName; // The filter to use

	// Check more in-depth descriptions of the optimizations here:
	// http://digestingduck.blogspot.com/2010/11/path-corridor-optimizations.html
	bool anticipateTurns; // If this agent should anticipate turns and move accordingly.
	bool optimizeVisibility; // Optimize walked path based on visibility. Strongly recommended.
	bool optimizeTopology; // If shorter paths should be attempted under certain circumstances. Also recommended.

	bool avoidObstacles; // If this agent should try to avoid obstacles (dynamic obstacles).
	bool avoidOtherAgents; // If this agent should avoid other agents.
	int obstacleAvoidance; // How much this agent should avoid obstacles. 0 - 3, with 0 being low and 3 high avoidance.
	float separationWeight; // How strongly the other agents should try to avoid this agent (if they have avoidOtherAgents set).
};

// Different states that an agent can be in
enum DetourCrowdAgentState {
	AGENT_STATE_INVALID = -1,
	AGENT_STATE_IDLE,
	AGENT_STATE_GOING_TO_TARGET,
	NUM_AGENT_STATES
};

// A single agent in a crowd.
class DetourCrowdAgent : public Reference {
	GD_CLASS(DetourCrowdAgent, Reference)

	dtCrowdAgent *_agent;
	dtCrowd *_crowd;
	int _agentIndex;
	int _crowdIndex;
	dtNavMeshQuery *_query;
	dtQueryFilter *_filter;
	int _filterIndex;
	DetourInputGeometry *_inputGeom;
	std::vector<dtCrowdAgent *> _shadows;

	Vector3 _position;
	Vector3 _velocity;
	Vector3 _targetPosition;
	std::atomic_bool _hasNewTarget;
	DetourCrowdAgentState _state;

	bool _isMoving;
	float _lastDistanceToTarget;
	float _distanceTotal;
	float _distanceTime;
	Vector3 _lastPosition;
	float _movementTime;
	float _movementOverTime;

	std::chrono::system_clock::time_point lastUpdateTime;

protected:
	static void _bind_methods();

public:
	void _init() {} // Called when .new() is called in gdscript

	bool save(FileAccessRef targetFile); // Will save this agent's current state to the passed file.
	bool load(FileAccessRef sourceFile); // Loads the agent from the file.

	// Loads agent parameters from file
	bool loadParameterValues(Ref<DetourCrowdAgentParameters> params, FileAccessRef sourceFile);

	// Sets this agent's main crowd agent.
	void setMainAgent(dtCrowdAgent *crowdAgent, dtCrowd *crowd, int index, dtNavMeshQuery *query, DetourInputGeometry *geom, int crowdIndex);

	void setFilter(int filterIndex); // Sets the filter this agent will use.

	int getFilterIndex(); // Return the index of the filter.

	int getCrowdIndex(); // Return the index of the crowd (= index of navmesh).

	bool isMoving(); // True if the agent is currently moving.

	Vector3 getTargetPosition(); // The target position for this agent (doesn't necessarily mean that it is currently moving).

	// Adds the passed agent as a shadow agent that will be updated with the main agent's values regularly.
	void addShadowAgent(dtCrowdAgent *crowdAgent);

	void moveTowards(Vector3 position); // The agent will start moving as close as possible towards the passed position.

	void applyNewTarget(); // Will fill the passed vector with the current movement target, THEN RESET IT.

	void stop(); // Stops moving entirely.

	// Returns a prediction of the movement, based on the passed position and the last updated agent position and velocity.
	Dictionary getPredictedMovement(Vector3 currentPos, Vector3 currentDir, int64_t positionTicksTimestamp, float maxTurningRad);

	void update(float secondsSinceLastTick); // Will update the shadows with the current values from the primary crowd.

	// Removes the agent from all crowds it is in and frees all associated memory.
	void destroy();

	DetourCrowdAgent();
	~DetourCrowdAgent();
};

inline int DetourCrowdAgent::getFilterIndex() {
	return _filterIndex;
}

inline int DetourCrowdAgent::getCrowdIndex() {
	return _crowdIndex;
}

inline bool DetourCrowdAgent::isMoving() {
	return _isMoving;
}

inline Vector3 DetourCrowdAgent::getTargetPosition() {
	return _targetPosition;
}

#endif // GODOTDETOURCROWDAGENT_H
