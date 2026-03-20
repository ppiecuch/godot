/**************************************************************************/
/*  detourcrowdagent.h                                                    */
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

#ifndef GODOTDETOURCROWDAGENT_H
#define GODOTDETOURCROWDAGENT_H

#include "core/math/vector3.h"
#include "core/os/file_access.h"
#include "core/reference.h"

#include <atomic>
#include <chrono>
#include <map>
#include <vector>

struct dtCrowdAgent;
class dtCrowd;
class dtNavMeshQuery;
class dtQueryFilter;
class DetourInputGeometry;
class DetourNavigationMesh;

// Parameters to initialize a DetourCrowdAgent.
struct DetourCrowdAgentParameters : public Reference {
	GDCLASS(DetourCrowdAgentParameters, Reference)

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

	void set_position(const Vector3 &p_value) { position = p_value; }
	Vector3 get_position() const { return position; }
	void set_radius(float p_value) { radius = p_value; }
	float get_radius() const { return radius; }
	void set_height(float p_value) { height = p_value; }
	float get_height() const { return height; }
	void set_max_acceleration(float p_value) { maxAcceleration = p_value; }
	float get_max_acceleration() const { return maxAcceleration; }
	void set_max_speed(float p_value) { maxSpeed = p_value; }
	float get_max_speed() const { return maxSpeed; }
	void set_filter_name(const String &p_value) { filterName = p_value; }
	String get_filter_name() const { return filterName; }
	void set_anticipate_turns(bool p_value) { anticipateTurns = p_value; }
	bool get_anticipate_turns() const { return anticipateTurns; }
	void set_optimize_visibility(bool p_value) { optimizeVisibility = p_value; }
	bool get_optimize_visibility() const { return optimizeVisibility; }
	void set_optimize_topology(bool p_value) { optimizeTopology = p_value; }
	bool get_optimize_topology() const { return optimizeTopology; }
	void set_avoid_obstacles(bool p_value) { avoidObstacles = p_value; }
	bool get_avoid_obstacles() const { return avoidObstacles; }
	void set_avoid_other_agents(bool p_value) { avoidOtherAgents = p_value; }
	bool get_avoid_other_agents() const { return avoidOtherAgents; }
	void set_obstacle_avoidance(int p_value) { obstacleAvoidance = p_value; }
	int get_obstacle_avoidance() const { return obstacleAvoidance; }
	void set_separation_weight(float p_value) { separationWeight = p_value; }
	float get_separation_weight() const { return separationWeight; }
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
	GDCLASS(DetourCrowdAgent, Reference)

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

	bool save(FileAccess *targetFile); // Will save this agent's current state to the passed file.
	bool load(FileAccess *sourceFile); // Loads the agent from the file.

	// Loads agent parameters from file
	bool loadParameterValues(Ref<DetourCrowdAgentParameters> params, FileAccess *sourceFile);

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

	void set_position_prop(const Vector3 &p_value) { _position = p_value; }
	Vector3 get_position_prop() const { return _position; }
	void set_velocity(const Vector3 &p_value) { _velocity = p_value; }
	Vector3 get_velocity() const { return _velocity; }
	void set_target(const Vector3 &p_value) { _targetPosition = p_value; }
	Vector3 get_target() const { return _targetPosition; }
	void set_is_moving(bool p_value) { _isMoving = p_value; }
	bool get_is_moving() const { return _isMoving; }

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
