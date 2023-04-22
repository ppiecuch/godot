#include "core/math/quat.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/project_settings.h"
#include "core/variant.h"

enum FLIGHT_TEST_MODE {
	// Calculates the time of flight based on the horizontal speed. Vertical speed is ignored.
	MODE_HORIZONTAL,
	// Calculates the time of flight that hit the elevation of the end point for the
	// second time, based on the vertical speed. Horizontal speed is ignored.
	MODE_VERTICAL_B,
	// Calculates the time of flight that hit the elevation of the end point for the
	// first time, based on the vertical speed. Horizontal speed is ignored.
	MODE_VERTICAL_A,
	// Tests the given velocity both horizontally and vertically, and outputs the time
	// of flight if the velocity is correct.
	MODE_BOTH,
};

struct ProjectileMath {
	static real_t DefaultGravity;

	// Computes the launch velocity by the given start point, end point, and coefficient
	// a of the quadratic function f(x) = ax^2 + bx + c which determines the trajectory
	// of the projectile motion.
	//
	// a: The a coefficient of the quadratic function f(x) = ax^2 + bx + c.
	// It determines the shape and speed of the trajectory, for example, -0.2 makes the
	// trajectory curvier and slower while -0.01f makes it straighter and faster. Should
	// always be negative.
	static Vector3 VelocityByA(Vector3 start, Vector3 end, real_t a) {
		auto vec = end - start;
		auto n = vec.y;
		vec.y = 0;
		auto m = vec.length();

		auto b = n / m - m * a;
		auto vx = Math::sqrt(DefaultGravity / (2 * a)); // vy + g*m/vx = (2*a*m + b) * vx
		auto vy = b * vx;
		auto direction = vec / m;
		return Vector3(direction.x * vx, vy, direction.z * vx);
	}

	// Computes the launch velocity by the given start point, end point, and launch angle
	// in degrees.
	//
	// elevationAngle: The launch angle in degrees. 0 means launch
	// horizontally. Should be from -90 (exclusive) to 90 (exclusive) and greater than
	// the elevation angle formed by start to end.
	static Vector3 VelocityByAngle(Vector3 start, Vector3 end, real_t elevationAngle) {
		auto b = Math::tan(Math::deg2rad(elevationAngle));

		auto vec = end - start;
		auto n = vec.y;
		vec.y = 0;
		auto m = vec.length();

		auto a = (n / m - b) / m;
		auto vx = Math::sqrt(DefaultGravity / (2 * a));
		auto vy = b * vx;
		auto direction = vec / m;
		return Vector3(direction.x * vx, vy, direction.z * vx);
	}

	// Computes the launch velocity by the given start point, end point, and time in
	// seconds the projectile flies from start to end. The projectile object will be
	// exactly at the end point time seconds after launch.
	//
	// time: The time in seconds you want the projectile to fly from start to end.
	static Vector3 VelocityByTime(Vector3 start, Vector3 end, real_t time) {
		return Vector3(
				(end.x - start.x) / time,
				(end.y - start.y) / time - 0.5 * DefaultGravity * time,
				(end.z - start.z) / time);
	}

	// Computes the launch velocity by the given start point, end point, and max height
	// of the projectile motion.
	//
	// heightFromEnd: The height measured from the end point (for example,
	// 1 means the max height of the trajectory is 1 meter above the end point).
	// The algorithm automatically clamps the value if it is lower than the y value
	// of start or end.
	static Vector3 VelocityByHeight(const Vector3 &start, const Vector3 &end, real_t heightFromEnd) {
		auto h = end.y + heightFromEnd - start.y;
		if (h < 0) {
			h = 0;
			heightFromEnd = start.y - end.y;
		}

		auto time = Math::sqrt(-2 * h / DefaultGravity) + Math::sqrt(-2 * heightFromEnd / DefaultGravity);
		return VelocityByTime(start, end, time);
	}

	// Computes the two angle results by the given start point, end point, and launch
	// speed. Returns false if out of reach.
	//
	// speed: The launch speed of the projectile object.
	// lowAngle: The lower angle that satisfies the conditions, or 0 if the method returns false.
	// highAngle: The higher angle that satisfies the conditions, or 0 if the method returns false.
	static bool AnglesBySpeed(Vector3 start, Vector3 end, real_t speed, real_t &lowAngle, real_t &highAngle) {
		auto vec = end - start;
		auto n = vec.y;
		vec.y = 0;
		auto m = vec.length();

		// Note that the b and c here are of the quadratic equation that calculates
		// the b value of the quadratic function of the projectile motion.
		auto b = (2 * speed * speed) / (m * DefaultGravity);
		auto c = -(b * n / m) + 1;
		auto delta = b * b - 4 * c;

		if (delta < 0) {
			lowAngle = highAngle = 0;
			return false;
		}

		auto deltaRoot = Math::sqrt(delta);
		lowAngle = Math::atan(Math::rad2deg((-b - deltaRoot) * 0.5));
		highAngle = Math::atan(Math::rad2deg((-b + deltaRoot) * 0.5));
		return true;
	}

	// Computes the two velocity results by the given start point, end point, and launch
	// speed. Returns false if out of reach.
	//
	// speed: The launch speed of the projectile object.
	// lowAngleV: The lower-angle velocity that satisfies the conditions, or (0, 0, 0) if the method returns false.
	// highAngleV: The higher-angle velocity that satisfies the conditions, or (0, 0, 0) if the method returns false.
	static bool VelocitiesBySpeed(Vector3 start, Vector3 end, real_t speed, Vector3 &lowAngleV, Vector3 &highAngleV) {
		real_t lowAngle, highAngle;
		if (!AnglesBySpeed(start, end, speed, lowAngle, highAngle)) {
			lowAngleV = Vector3();
			highAngleV = Vector3();
			return false;
		}

		auto dirXZ = end - start;
		dirXZ.y = 0;
		dirXZ.normalize();
		auto right = Vector3::DOWN.cross(dirXZ);

		auto lowDir = Quat(right, lowAngle).xform(dirXZ);
		lowAngleV = lowDir * speed;

		auto highDir = Quat(right, highAngle).xform(dirXZ);
		highAngleV = speed * highDir;

		return true;
	}

	// Computes the position of the projectile at the given time counted from the moment
	// the projectile is at origin.
	//
	// time: The time counted from the moment the projectile is at origin.
	// gAcceleration: Gravitational acceleration, equals the magnitude of gravity.
	static Vector3 PositionAtTime(Vector3 origin, Vector3 originVelocity, real_t time, real_t gAcceleration) {
		auto vy = originVelocity.y + time * gAcceleration;
		auto py = 0.5 * time * (originVelocity.y + vy);
		auto displacement = Vector3(time * originVelocity.x, py, time * originVelocity.z);
		return origin + displacement;
	}

	// Computes the trajectory points of the projectile and stores them into the buffer.
	//
	// distance: To calculate the positions to how far, from origin and ignoring height.
	// count: How many positions to calculate, including the origin and end.
	// gAcceleration: Gravitational acceleration, equals the magnitude of gravity (normally equals DefaultGravity).
	// positions: The buffer to store the calculated positions.
	static void Positions(Vector3 origin, Vector3 originVelocity, real_t distance, int count, real_t gAcceleration, PoolVector3Array positions) {
		auto vxz = originVelocity;
		vxz.y = 0;

		real_t timeInterval = distance / vxz.length() / (count - 1);
		auto y = 0.5 * gAcceleration * timeInterval;
		positions[0] = origin;

		for (int i = 1; i < positions.size(); i++) {
			positions[i] = origin + i * timeInterval * Vector3(originVelocity.x, originVelocity.y + i * y, originVelocity.z);
		}
	}

	// Tests if a projectile at start can use the vertical velocity (y) of startVelocity
	// to hit the elevation (y) of end, if true, outputs the time of flight based on the
	// vertical speed. Horizontal speed is ignored.
	//
	// startVelocity: The velocity at the start point, or launch velocity.
	// timesOfFlight: The time results that a projectile fly from start to end with the launch velocity startVelocity.
	static bool VerticalFlightTest(Vector3 start, Vector3 end, Vector3 startVelocity, Vector2 &timesOfFlight) {
		timesOfFlight = Vector2(-1, -1);
		auto a = 0.5 * DefaultGravity;
		auto b = startVelocity.y;
		auto c = start.y - end.y;

		auto delta = b * b - 4 * a * c;
		if (delta < 0) {
			return false;
		}

		auto ta = (-b + Math::sqrt(delta)) / (2 * a);
		auto tb = (-b - Math::sqrt(delta)) / (2 * a);
		timesOfFlight = Vector2(ta, tb);
		return true;
	}

	// Tests if a projectile at start can use startVelocity to hit end, and outputs the
	// time of flight.
	//
	// startVelocity: The velocity at the start point, or launch velocity.
	// testMode: FLIGHT_TEST_MODE enum.
	// timeOfFlight: The time that a projectile fly from start to end with the launch velocity startVelocity.
	static bool FlightTest(Vector3 start, Vector3 end, Vector3 startVelocity, FLIGHT_TEST_MODE testMode, real_t &timeOfFlight) {
		auto dXZ = end - start;
		auto sqrDistance = dXZ.x * dXZ.x + dXZ.z * dXZ.z;
		auto sqrSpeed = startVelocity.x * startVelocity.x + startVelocity.z * startVelocity.z;
		auto testT = Math::sqrt(sqrDistance / sqrSpeed);

		if (testMode == MODE_HORIZONTAL) {
			timeOfFlight = testT;
			if (sqrSpeed == 0) {
				return (sqrDistance == 0);
			}
			return true;
		}

		if (testMode == MODE_BOTH) {
			if (Math::is_nan(testT)) {
				testMode = MODE_VERTICAL_B;
			} else {
				auto vy = startVelocity.y + testT * DefaultGravity;
				auto py = 0.5 * testT * (startVelocity.y + vy) + start.y;
				timeOfFlight = testT;
				return Math::abs(py - end.y) < 0.04;
			}
		}

		if (testMode == MODE_VERTICAL_B || testMode == MODE_VERTICAL_A) {
			Vector2 results;
			if (VerticalFlightTest(start, end, startVelocity, results)) {
				if (testMode == MODE_VERTICAL_B) {
					timeOfFlight = results.y;
				} else {
					timeOfFlight = results.x;
				}
				return timeOfFlight >= 0;
			}
		}

		timeOfFlight = -1;
		return false;
	}

	// Computes how far a projectile that uses the given speed at start can reach at the
	// given elevation endElevation. Returns -1f if can't reach the elevation.
	//
	// <param name="endElevation">The elevation (y) of the target point you want the
	// projectile motion to hit or pass through.
	// <param name="speed">The launch speed of the projectile object.
	static real_t ElevationalReach(Vector3 start, real_t endElevation, real_t speed) {
		auto n = endElevation - start.y;
		auto bm = (2 * speed * speed) / DefaultGravity;
		auto invSqr = 4 / (bm * bm + 4 * bm * n);

		if (invSqr <= 0) {
			return -1;
		}

		return Math::sqrt(1 / invSqr);
	}

	// Computes how far a projectile that uses the given speed at start can reach at the
	// given elevation endElevation, and outputs the corresponding launch angle. Returns
	// -1f if can't reach the elevation.
	//
	// endElevation: The elevation (y) of the target point you want the
	// projectile motion to hit or pass through.
	// speed: The launch speed of the projectile object.
	// angle: The angle that satisfies the conditions.
	static real_t ElevationalReach(Vector3 start, real_t endElevation, real_t speed, real_t &angle) {
		auto n = endElevation - start.y;
		auto bm = (2 * speed * speed) / DefaultGravity;
		auto invSqr = 4 / (bm * bm + 4 * bm * n);

		if (invSqr <= 0) {
			angle = 0;
			return -1;
		}

		auto m = Math::sqrt(1 / invSqr);
		auto b = bm / m;
		angle = Math::atan(Math::rad2deg(-b * 0.5));
		return m;
	}
};

real_t ProjectileMath::DefaultGravity = GLOBAL_DEF("physics/3d/default_gravity", 9.8);
