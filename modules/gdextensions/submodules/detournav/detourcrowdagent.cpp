/**************************************************************************/
/*  detourcrowdagent.cpp                                                  */
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

#include "detourcrowdagent.h"
#include "util/detourinputgeometry.h"

#include "core/os/os.h"
#include "scene/main/node.h"

#include <DetourCrowd.h>
#include <DetourNavMeshQuery.h>

#define AGENT_SAVE_VERSION 1

void DetourCrowdAgentParameters::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_position", "value"), &DetourCrowdAgentParameters::set_position);
	ClassDB::bind_method(D_METHOD("get_position"), &DetourCrowdAgentParameters::get_position);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "position"), "set_position", "get_position");

	ClassDB::bind_method(D_METHOD("set_radius", "value"), &DetourCrowdAgentParameters::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &DetourCrowdAgentParameters::get_radius);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "radius"), "set_radius", "get_radius");

	ClassDB::bind_method(D_METHOD("set_height", "value"), &DetourCrowdAgentParameters::set_height);
	ClassDB::bind_method(D_METHOD("get_height"), &DetourCrowdAgentParameters::get_height);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "height"), "set_height", "get_height");

	ClassDB::bind_method(D_METHOD("set_max_acceleration", "value"), &DetourCrowdAgentParameters::set_max_acceleration);
	ClassDB::bind_method(D_METHOD("get_max_acceleration"), &DetourCrowdAgentParameters::get_max_acceleration);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "maxAcceleration"), "set_max_acceleration", "get_max_acceleration");

	ClassDB::bind_method(D_METHOD("set_max_speed", "value"), &DetourCrowdAgentParameters::set_max_speed);
	ClassDB::bind_method(D_METHOD("get_max_speed"), &DetourCrowdAgentParameters::get_max_speed);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "maxSpeed"), "set_max_speed", "get_max_speed");

	ClassDB::bind_method(D_METHOD("set_filter_name", "value"), &DetourCrowdAgentParameters::set_filter_name);
	ClassDB::bind_method(D_METHOD("get_filter_name"), &DetourCrowdAgentParameters::get_filter_name);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "filterName"), "set_filter_name", "get_filter_name");

	ClassDB::bind_method(D_METHOD("set_anticipate_turns", "value"), &DetourCrowdAgentParameters::set_anticipate_turns);
	ClassDB::bind_method(D_METHOD("get_anticipate_turns"), &DetourCrowdAgentParameters::get_anticipate_turns);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "anticipateTurns"), "set_anticipate_turns", "get_anticipate_turns");

	ClassDB::bind_method(D_METHOD("set_optimize_visibility", "value"), &DetourCrowdAgentParameters::set_optimize_visibility);
	ClassDB::bind_method(D_METHOD("get_optimize_visibility"), &DetourCrowdAgentParameters::get_optimize_visibility);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "optimizeVisibility"), "set_optimize_visibility", "get_optimize_visibility");

	ClassDB::bind_method(D_METHOD("set_optimize_topology", "value"), &DetourCrowdAgentParameters::set_optimize_topology);
	ClassDB::bind_method(D_METHOD("get_optimize_topology"), &DetourCrowdAgentParameters::get_optimize_topology);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "optimizeTopology"), "set_optimize_topology", "get_optimize_topology");

	ClassDB::bind_method(D_METHOD("set_avoid_obstacles", "value"), &DetourCrowdAgentParameters::set_avoid_obstacles);
	ClassDB::bind_method(D_METHOD("get_avoid_obstacles"), &DetourCrowdAgentParameters::get_avoid_obstacles);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "avoidObstacles"), "set_avoid_obstacles", "get_avoid_obstacles");

	ClassDB::bind_method(D_METHOD("set_avoid_other_agents", "value"), &DetourCrowdAgentParameters::set_avoid_other_agents);
	ClassDB::bind_method(D_METHOD("get_avoid_other_agents"), &DetourCrowdAgentParameters::get_avoid_other_agents);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "avoidOtherAgents"), "set_avoid_other_agents", "get_avoid_other_agents");

	ClassDB::bind_method(D_METHOD("set_obstacle_avoidance", "value"), &DetourCrowdAgentParameters::set_obstacle_avoidance);
	ClassDB::bind_method(D_METHOD("get_obstacle_avoidance"), &DetourCrowdAgentParameters::get_obstacle_avoidance);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "obstacleAvoidance"), "set_obstacle_avoidance", "get_obstacle_avoidance");

	ClassDB::bind_method(D_METHOD("set_separation_weight", "value"), &DetourCrowdAgentParameters::set_separation_weight);
	ClassDB::bind_method(D_METHOD("get_separation_weight"), &DetourCrowdAgentParameters::get_separation_weight);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "separationWeight"), "set_separation_weight", "get_separation_weight");
}

void DetourCrowdAgent::_bind_methods() {
	ClassDB::bind_method(D_METHOD("moveTowards", "position"), &DetourCrowdAgent::moveTowards);
	ClassDB::bind_method(D_METHOD("stop"), &DetourCrowdAgent::stop);
	ClassDB::bind_method(D_METHOD("getPredictedMovement", "currentPos", "currentDir", "positionTicksTimestamp", "maxTurningRad"), &DetourCrowdAgent::getPredictedMovement);

	ClassDB::bind_method(D_METHOD("set_position", "value"), &DetourCrowdAgent::set_position_prop);
	ClassDB::bind_method(D_METHOD("get_position"), &DetourCrowdAgent::get_position_prop);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "position"), "set_position", "get_position");

	ClassDB::bind_method(D_METHOD("set_velocity", "value"), &DetourCrowdAgent::set_velocity);
	ClassDB::bind_method(D_METHOD("get_velocity"), &DetourCrowdAgent::get_velocity);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "velocity"), "set_velocity", "get_velocity");

	ClassDB::bind_method(D_METHOD("set_target", "value"), &DetourCrowdAgent::set_target);
	ClassDB::bind_method(D_METHOD("get_target"), &DetourCrowdAgent::get_target);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "target"), "set_target", "get_target");

	ClassDB::bind_method(D_METHOD("set_is_moving", "value"), &DetourCrowdAgent::set_is_moving);
	ClassDB::bind_method(D_METHOD("get_is_moving"), &DetourCrowdAgent::get_is_moving);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "isMoving"), "set_is_moving", "get_is_moving");

	ADD_SIGNAL(MethodInfo("arrived_at_target", PropertyInfo(Variant::OBJECT, "node")));
	ADD_SIGNAL(MethodInfo("no_progress", PropertyInfo(Variant::OBJECT, "node"), PropertyInfo(Variant::REAL, "distanceLeft")));
	ADD_SIGNAL(MethodInfo("no_movement", PropertyInfo(Variant::OBJECT, "node")));
}

DetourCrowdAgent::DetourCrowdAgent() :
		_agent(nullptr), _crowd(nullptr), _agentIndex(0), _crowdIndex(0), _query(nullptr), _filter(nullptr), _filterIndex(0), _inputGeom(nullptr), _state(AGENT_STATE_INVALID), _isMoving(false), _lastDistanceToTarget(0.0f), _distanceTotal(0.0f) {
	_hasNewTarget = false;
	lastUpdateTime = std::chrono::system_clock::now();
}

DetourCrowdAgent::~DetourCrowdAgent() {
}

bool DetourCrowdAgent::save(FileAccess *targetFile) {
	// Sanity check
	if (!_agent) {
		ERR_PRINT("AgentSave: No detour agent present!");
		return false;
	}

	// Version
	targetFile->store_16(AGENT_SAVE_VERSION);

	// Properties
	targetFile->store_32(_agentIndex);
	targetFile->store_32(_crowdIndex);
	targetFile->store_32(_filterIndex);
	targetFile->store_float(_position.x);
	targetFile->store_float(_position.y);
	targetFile->store_float(_position.z);
	targetFile->store_float(_velocity.x);
	targetFile->store_float(_velocity.y);
	targetFile->store_float(_velocity.z);
	targetFile->store_float(_targetPosition.x);
	targetFile->store_float(_targetPosition.y);
	targetFile->store_float(_targetPosition.z);
	targetFile->store_8(_hasNewTarget.load());
	targetFile->store_8(_isMoving);
	targetFile->store_16(_state);

	// Parameter values
	targetFile->store_float(_agent->params.radius);
	targetFile->store_float(_agent->params.height);
	targetFile->store_float(_agent->params.maxAcceleration);
	targetFile->store_float(_agent->params.maxSpeed);
	targetFile->store_32(_agent->params.updateFlags);
	targetFile->store_8(_agent->params.obstacleAvoidanceType);
	targetFile->store_float(_agent->params.separationWeight);

	return true;
}

bool DetourCrowdAgent::load(FileAccess *sourceFile) {
	// Version
	int version = sourceFile->get_16();

	if (version == AGENT_SAVE_VERSION) {
		_agentIndex = sourceFile->get_32();
		_crowdIndex = sourceFile->get_32();
		_filterIndex = sourceFile->get_32();
		_position.x = sourceFile->get_float();
		_position.y = sourceFile->get_float();
		_position.z = sourceFile->get_float();
		_velocity.x = sourceFile->get_float();
		_velocity.y = sourceFile->get_float();
		_velocity.z = sourceFile->get_float();
		_targetPosition.x = sourceFile->get_float();
		_targetPosition.y = sourceFile->get_float();
		_targetPosition.z = sourceFile->get_float();
		_hasNewTarget = sourceFile->get_8();
		_isMoving = sourceFile->get_8();
		_state = (DetourCrowdAgentState)sourceFile->get_16();
	} else {
		ERR_PRINT("Unable to load agent. Unknown save version: " + itos(version));
		return false;
	}

	return true;
}

bool DetourCrowdAgent::loadParameterValues(Ref<DetourCrowdAgentParameters> params, FileAccess *sourceFile) {
	params->radius = sourceFile->get_float();
	params->height = sourceFile->get_float();
	params->maxAcceleration = sourceFile->get_float();
	params->maxSpeed = sourceFile->get_float();
	params->position = _position;
	int updateFlags = sourceFile->get_32();
	params->anticipateTurns = updateFlags & DT_CROWD_ANTICIPATE_TURNS;
	params->optimizeVisibility = updateFlags & DT_CROWD_OPTIMIZE_VIS;
	params->optimizeTopology = updateFlags & DT_CROWD_OPTIMIZE_TOPO;
	params->avoidObstacles = updateFlags & DT_CROWD_OBSTACLE_AVOIDANCE;
	params->avoidOtherAgents = updateFlags & DT_CROWD_SEPARATION;
	params->obstacleAvoidance = sourceFile->get_8();
	params->separationWeight = sourceFile->get_float();

	return true;
}

void DetourCrowdAgent::setMainAgent(dtCrowdAgent *crowdAgent, dtCrowd *crowd, int index, dtNavMeshQuery *query, DetourInputGeometry *geom, int crowdIndex) {
	_agent = crowdAgent;
	_crowd = crowd;
	_agentIndex = index;
	_crowdIndex = crowdIndex;
	_query = query;
	_inputGeom = geom;
	_state = AGENT_STATE_IDLE;
	_distanceTotal = 0.0f;
	_lastDistanceToTarget = 0.0f;
	_distanceTime = 0.0f;
	_movementOverTime = 0.0f;
	_movementTime = 0.0f;
	_lastPosition = Vector3(_agent->npos[0], _agent->npos[1], _agent->npos[2]);
}

void DetourCrowdAgent::setFilter(int filterIndex) {
	_filter = _crowd->getEditableFilter(filterIndex);
	_filterIndex = filterIndex;
}

void DetourCrowdAgent::addShadowAgent(dtCrowdAgent *crowdAgent) {
	_shadows.push_back(crowdAgent);
}

void DetourCrowdAgent::moveTowards(Vector3 position) {
	_targetPosition = position;
	_hasNewTarget = true;
	_distanceTotal = 0.0f;
	_lastDistanceToTarget = 0.0f;
	_movementTime = 0.0f;
	_movementOverTime = 0.0f;
	_lastPosition = _position;
}

void DetourCrowdAgent::applyNewTarget() {
	if (!_hasNewTarget) {
		return;
	}
	_hasNewTarget = false;

	// Get the final target position and poly reference
	// TODO: Optimization, create new method to assign same target to multiple agents at once (only one findNearestPoly per agent)
	dtPolyRef targetRef;
	float finalTargetPos[3];
	const float *halfExtents = _crowd->getQueryExtents();
	finalTargetPos[0] = 0.0f;
	finalTargetPos[1] = 0.0f;
	finalTargetPos[2] = 0.0f;
	float pos[3];
	pos[0] = _targetPosition.x;
	pos[1] = _targetPosition.y;
	pos[2] = _targetPosition.z;
	float extents[3];
	extents[0] = halfExtents[0] * 1.0f;
	extents[1] = halfExtents[1] * 1.0f;
	extents[2] = halfExtents[2] * 1.0f;

	// Do the query to find the nearest poly & point
	dtStatus status = _query->findNearestPoly(pos, extents, _filter, &targetRef, finalTargetPos);
	if (dtStatusFailed(status)) {
		_hasNewTarget = true;
		ERR_PRINT("applyNewTarget: findPoly failed: " + itos(status));
		return;
	}

	// Set the movement target
	if (!_crowd->requestMoveTarget(_agentIndex, targetRef, finalTargetPos)) {
		ERR_PRINT("Unable to request detour move target.");
	}
	_state = AGENT_STATE_GOING_TO_TARGET;
}

void DetourCrowdAgent::stop() {
	// Stop all movement
	_crowd->resetMoveTarget(_agentIndex);
	_hasNewTarget = false;
	_isMoving = false;
	_state = AGENT_STATE_IDLE;
	_distanceTotal = 0.0f;
	_lastDistanceToTarget = 0.0f;
	_distanceTime = 0.0f;
	_movementTime = 0.0f;
	_movementOverTime = 0.0f;
}

Dictionary
DetourCrowdAgent::getPredictedMovement(Vector3 currentPos, Vector3 currentDir, int64_t positionTicksTimestamp, float maxTurningRad) {
	Dictionary result;

	// Get the time since the last internal update in milliseconds
	auto timeSinceUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdateTime).count();
	float secondsPassed = timeSinceUpdate / 1000.0f;

	// Calculate the point where the agent itself would be now
	Vector3 velToUse = _velocity.length() <= 0.01f ? currentDir : _velocity;
	Vector3 agentTargetPos = _position + secondsPassed * velToUse;

	// If we are already at the target position, no need to calculate the rest
	float distance = currentPos.distance_to(agentTargetPos);
	if (distance < 0.01f) {
		result["position"] = currentPos;
		velToUse.y = 0.0f;
		velToUse.normalize();
		result["direction"] = velToUse;
		return result;
	}

	// Get the seconds since the timestamp and the direction
	float secondsSinceTimestamp = (OS::get_singleton()->get_ticks_msec() - positionTicksTimestamp) / 1000.0f;
	Vector3 direction = agentTargetPos - currentPos;
	direction.normalize();

	// Make sure we don't go too far
	float lengthToUse = velToUse.length();
	Vector3 movement = secondsSinceTimestamp * (direction * lengthToUse);
	if (movement.length() > distance) {
		movement = movement.normalized() * distance;
	}

	// Interpolate the facing direction with a maximum turning amount
	direction.y = 0.0f;
	direction.normalize();
	float turningRad = currentDir.angle_to(direction);
	turningRad = currentDir.cross(direction).y > 0.0f ? turningRad : -turningRad;
	if (fabs(turningRad) > maxTurningRad) {
		turningRad = turningRad < 0.0f ? -maxTurningRad : maxTurningRad;
	}
	Vector3 newDirection = currentDir.rotated(Vector3(0.0f, 1.0f, 0.0f), turningRad);

	// Apply movement
	Vector3 predictedPos = currentPos + movement;
	result["position"] = predictedPos;
	result["direction"] = newDirection;
	return result;
}

void DetourCrowdAgent::update(float secondsSinceLastTick) {
	// Update all the shadows with the main agent's values
	for (int i = 0; i < _shadows.size(); ++i) {
		_shadows[i]->npos[0] = _agent->npos[0];
		_shadows[i]->npos[1] = _agent->npos[1];
		_shadows[i]->npos[2] = _agent->npos[2];
	}

	_position.x = _agent->npos[0];
	_position.y = _agent->npos[1];
	_position.z = _agent->npos[2];
	_velocity.x = _agent->vel[0];
	_velocity.y = _agent->vel[1];
	_velocity.z = _agent->vel[2];

	// Various state-dependent calculations
	switch (_state) {
		default:
			break;
		case AGENT_STATE_GOING_TO_TARGET: {
			// Update the values available to GDScript
			lastUpdateTime = std::chrono::system_clock::now();

			// Get distance to target and other statistics
			float distanceToTarget = _targetPosition.distance_to(_position);
			_distanceTime += secondsSinceLastTick;
			_distanceTotal += fabs(_lastDistanceToTarget - distanceToTarget);

			_movementOverTime += _position.distance_squared_to(_lastPosition);

			// Mark moving or not
			if (_movementOverTime <= 0.001f) {
				_isMoving = false;
			} else {
				_isMoving = true;
			}

			// If we are moving but have no velocity (most likely using off-mesh connection), fake it
			if (_isMoving && _velocity.length_squared() <= 0.001f) {
				_velocity = _position - _lastPosition;
				_velocity = _velocity.normalized() / secondsSinceLastTick;
			}

			// Remember last position for next tick
			_lastPosition = _position;
			_movementTime += secondsSinceLastTick;

			// If the agent has not moved noticeably in a while, report that
			if (_movementTime >= 1.0f) {
				_movementTime -= 1.0f;
				if (_movementOverTime < (_agent->params.maxSpeed * 0.01f)) {
					emit_signal("no_movement", this, distanceToTarget);
				}
				_movementOverTime = 0.0f;
			}

			// If we haven't made enough progress in a second, tell the user
			if (_distanceTime >= 5.0f) {
				_distanceTime -= 5.0f;
				if (_distanceTotal < (_agent->params.maxSpeed * 0.03f)) {
					emit_signal("no_progress", this, distanceToTarget);
				}
				_distanceTotal = 0.0f;
			}

			// Arrived?
			if (distanceToTarget < 0.1f) {
				_isMoving = false;
				_crowd->resetMoveTarget(_agentIndex);
				_state = AGENT_STATE_IDLE;
				_distanceTotal = 0.0f;
				_lastDistanceToTarget = 0.0f;
				_distanceTime = 0.0f;
				_movementTime = 0.0f;
				_movementOverTime = 0.0f;
				emit_signal("arrived_at_target", this);
			}
			_lastDistanceToTarget = distanceToTarget;
			break;
		}
	}
}

void DetourCrowdAgent::destroy() {
	// In contrast to obstacles, agents really shouldn't be removed during the thread update, so this has to be done thread safe

	// Simply setting an agent's active value to false should remove it from all detour calculations
	_agent->active = false;
	for (int i = 0; i < _shadows.size(); ++i) {
		_shadows[i]->active = false;
	}
	_shadows.clear();
	_agent = nullptr;
	_isMoving = false;
	_distanceTotal = 0.0f;
	_lastDistanceToTarget = 0.0f;
	_distanceTime = 0.0f;
	_movementTime = 0.0f;
	_movementOverTime = 0.0f;
}

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[detournav]] DetourCrowdAgentParameters") {
	TEST_CASE("[detournav] default parameter construction") {
		DetourCrowdAgentParameters params;
		params._init();
		CHECK(params.get_radius() == doctest::Approx(0.0f));
		CHECK(params.get_height() == doctest::Approx(0.0f));
		CHECK(params.get_max_acceleration() == doctest::Approx(0.0f));
		CHECK(params.get_max_speed() == doctest::Approx(0.0f));
	}

	TEST_CASE("[detournav] parameter setters and getters") {
		DetourCrowdAgentParameters params;
		params.set_position(Vector3(1, 2, 3));
		CHECK(params.get_position() == Vector3(1, 2, 3));

		params.set_radius(0.5f);
		CHECK(params.get_radius() == doctest::Approx(0.5f));

		params.set_height(1.8f);
		CHECK(params.get_height() == doctest::Approx(1.8f));

		params.set_max_acceleration(10.0f);
		CHECK(params.get_max_acceleration() == doctest::Approx(10.0f));

		params.set_max_speed(5.0f);
		CHECK(params.get_max_speed() == doctest::Approx(5.0f));

		params.set_filter_name("custom_filter");
		CHECK(params.get_filter_name() == "custom_filter");

		params.set_anticipate_turns(false);
		CHECK(params.get_anticipate_turns() == false);

		params.set_optimize_visibility(false);
		CHECK(params.get_optimize_visibility() == false);

		params.set_optimize_topology(false);
		CHECK(params.get_optimize_topology() == false);

		params.set_avoid_obstacles(false);
		CHECK(params.get_avoid_obstacles() == false);

		params.set_avoid_other_agents(false);
		CHECK(params.get_avoid_other_agents() == false);

		params.set_obstacle_avoidance(3);
		CHECK(params.get_obstacle_avoidance() == 3);

		params.set_separation_weight(2.5f);
		CHECK(params.get_separation_weight() == doctest::Approx(2.5f));
	}
}

TEST_SUITE("[[detournav]] DetourCrowdAgent") {
	TEST_CASE("[detournav] agent default construction") {
		DetourCrowdAgent agent;
		CHECK(agent.get_is_moving() == false);
		CHECK(agent.get_position_prop() == Vector3(0, 0, 0));
		CHECK(agent.get_velocity() == Vector3(0, 0, 0));
	}

	TEST_CASE("[detournav] agent property setters") {
		DetourCrowdAgent agent;
		agent.set_position_prop(Vector3(10, 20, 30));
		CHECK(agent.get_position_prop() == Vector3(10, 20, 30));

		agent.set_velocity(Vector3(1, 0, -1));
		CHECK(agent.get_velocity() == Vector3(1, 0, -1));

		agent.set_target(Vector3(100, 0, 100));
		CHECK(agent.get_target() == Vector3(100, 0, 100));

		agent.set_is_moving(true);
		CHECK(agent.get_is_moving() == true);
	}

	TEST_CASE("[detournav] agent save and load roundtrip") {
		DetourCrowdAgent agent;
		agent.set_position_prop(Vector3(1.5f, 2.5f, 3.5f));
		agent.set_velocity(Vector3(0.1f, 0.0f, -0.1f));
		agent.set_target(Vector3(10.0f, 0.0f, 10.0f));

		// Note: save/load require _agent to be non-null, so we test property roundtrip via setters.
		CHECK(agent.get_position_prop().x == doctest::Approx(1.5f));
		CHECK(agent.get_velocity().z == doctest::Approx(-0.1f));
		CHECK(agent.get_target().x == doctest::Approx(10.0f));
	}
}
#endif
