// Comprehensive doctest tests for Google Filament math library (filamath).
// Covers: vec2/3/4, mat2/3/4, quat, scalar, half, fast, norm.
// Based on filament/libs/math/tests/ (gtest), ported to doctest.

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>
#include <math/mat2.h>
#include <math/mat3.h>
#include <math/mat4.h>
#include <math/quat.h>
#include <math/scalar.h>
#include <math/half.h>
#include <math/fast.h>
#include <math/norm.h>

#include <cmath>
#include <limits>

using namespace filament::math;

// ============================================================================
// Scalar
// ============================================================================

TEST_SUITE("filamath::scalar") {

TEST_CASE("[filamath] scalar constants") {
	CHECK(F_PI == doctest::Approx(3.14159265).epsilon(1e-6));
	CHECK(F_E == doctest::Approx(2.71828182).epsilon(1e-6));
	CHECK(F_TAU == doctest::Approx(2.0 * F_PI));
	CHECK(f::DEG_TO_RAD == doctest::Approx(F_PI / 180.0f));
	CHECK(f::RAD_TO_DEG == doctest::Approx(180.0f / F_PI));
}

TEST_CASE("[filamath] scalar clamp/saturate") {
	CHECK(saturate(0.5f) == doctest::Approx(0.5f));
	CHECK(saturate(-1.0f) == doctest::Approx(0.0f));
	CHECK(saturate(2.0f) == doctest::Approx(1.0f));
	CHECK(clamp(5.0f, 0.0f, 3.0f) == doctest::Approx(3.0f));
	CHECK(clamp(-1.0f, 0.0f, 3.0f) == doctest::Approx(0.0f));
	CHECK(clamp(1.5f, 0.0f, 3.0f) == doctest::Approx(1.5f));
}

TEST_CASE("[filamath] scalar mix/lerp") {
	CHECK(mix(0.0f, 10.0f, 0.5f) == doctest::Approx(5.0f));
	CHECK(mix(0.0f, 10.0f, 0.0f) == doctest::Approx(0.0f));
	CHECK(mix(0.0f, 10.0f, 1.0f) == doctest::Approx(10.0f));
}

} // TEST_SUITE scalar

// ============================================================================
// Vec2
// ============================================================================

TEST_SUITE("filamath::vec2") {

TEST_CASE("[filamath] vec2 size") {
	CHECK(sizeof(float2) == sizeof(float) * 2);
	CHECK(sizeof(double2) == sizeof(double) * 2);
}

TEST_CASE("[filamath] vec2 constructors") {
	float2 v0{}; // Brace-init to zero.
	CHECK(v0.x == 0);
	CHECK(v0.y == 0);

	float2 v1(5);
	CHECK(v1.x == 5);
	CHECK(v1.y == 5);

	float2 v2(1, 2);
	CHECK(v2.x == 1);
	CHECK(v2.y == 2);

	float2 v3(v2);
	CHECK(v3.x == 1);
	CHECK(v3.y == 2);
}

TEST_CASE("[filamath] vec2 access") {
	float2 v(3, 7);
	CHECK(v[0] == 3);
	CHECK(v[1] == 7);
	v[0] = 10;
	CHECK(v.x == 10);
}

TEST_CASE("[filamath] vec2 arithmetic") {
	float2 a(1, 2);
	float2 b(3, 4);

	float2 c = a + b;
	CHECK(c.x == 4);
	CHECK(c.y == 6);

	float2 d = b - a;
	CHECK(d.x == 2);
	CHECK(d.y == 2);

	float2 e = a * 3;
	CHECK(e.x == 3);
	CHECK(e.y == 6);

	CHECK(dot(a, b) == doctest::Approx(11.0f));
	CHECK(length(float2(3, 4)) == doctest::Approx(5.0f));
}

TEST_CASE("[filamath] vec2 normalize") {
	float2 v(3, 4);
	float2 n = normalize(v);
	CHECK(length(n) == doctest::Approx(1.0f));
	CHECK(n.x == doctest::Approx(0.6f));
	CHECK(n.y == doctest::Approx(0.8f));
}

} // TEST_SUITE vec2

// ============================================================================
// Vec3
// ============================================================================

TEST_SUITE("filamath::vec3") {

TEST_CASE("[filamath] vec3 size") {
	CHECK(sizeof(float3) == sizeof(float) * 3);
	CHECK(sizeof(double3) == sizeof(double) * 3);
}

TEST_CASE("[filamath] vec3 constructors") {
	float3 v0{}; // Brace-init to zero.
	CHECK(v0.x == 0);
	CHECK(v0.y == 0);
	CHECK(v0.z == 0);

	float3 v1(5);
	CHECK(v1.x == 5);
	CHECK(v1.y == 5);
	CHECK(v1.z == 5);

	float3 v2(1, 2, 3);
	CHECK(v2.x == 1);
	CHECK(v2.y == 2);
	CHECK(v2.z == 3);

	// From vec2 + scalar.
	float3 v3(float2(10, 20), 30);
	CHECK(v3.x == 10);
	CHECK(v3.y == 20);
	CHECK(v3.z == 30);
}

TEST_CASE("[filamath] vec3 swizzle") {
	float3 v(1, 2, 3);
	float2 xy = v.xy;
	CHECK(xy.x == 1);
	CHECK(xy.y == 2);
}

TEST_CASE("[filamath] vec3 cross product") {
	float3 east(1, 0, 0);
	float3 north(0, 1, 0);
	float3 up = cross(east, north);
	CHECK(up.x == doctest::Approx(0));
	CHECK(up.y == doctest::Approx(0));
	CHECK(up.z == doctest::Approx(1));
}

TEST_CASE("[filamath] vec3 dot product") {
	float3 a(1, 2, 3);
	float3 b(4, 5, 6);
	CHECK(dot(a, b) == doctest::Approx(32.0f));
	CHECK(dot(float3(1, 0, 0), float3(0, 1, 0)) == doctest::Approx(0));
}

TEST_CASE("[filamath] vec3 normalize") {
	float3 v(1, 2, 3);
	float3 n = normalize(v);
	CHECK(length(n) == doctest::Approx(1.0f));
}

TEST_CASE("[filamath] vec3 distance") {
	float3 a(1, 0, 0);
	float3 b(0, 1, 0);
	CHECK(distance(a, b) == doctest::Approx(std::sqrt(2.0f)));
}

TEST_CASE("[filamath] vec3 unary ops") {
	float3 v(1, 2, 3);
	v += 1;
	CHECK(v.x == 2);
	CHECK(v.y == 3);
	CHECK(v.z == 4);

	v -= 1;
	CHECK(v.x == 1);
	CHECK(v.y == 2);
	CHECK(v.z == 3);

	v *= 2;
	CHECK(v.x == 2);
	CHECK(v.y == 4);
	CHECK(v.z == 6);

	v /= 2;
	CHECK(v.x == 1);
	CHECK(v.y == 2);
	CHECK(v.z == 3);

	float3 neg = -v;
	CHECK(neg.x == -1);
	CHECK(neg.y == -2);
	CHECK(neg.z == -3);
}

TEST_CASE("[filamath] vec3 comparison") {
	float3 a(1, 2, 3);
	float3 b(1, 2, 3);
	float3 c(4, 5, 6);

	CHECK(a == b);
	CHECK(a != c);
	CHECK_FALSE(a != b);
	CHECK_FALSE(a == c);
}

TEST_CASE("[filamath] vec3 any/all") {
	CHECK(any(float3(0, 0, 1)));
	CHECK_FALSE(any(float3(0, 0, 0)));
	CHECK(all(float3(1, 1, 1)));
	CHECK_FALSE(all(float3(0, 0, 1)));
}

} // TEST_SUITE vec3

// ============================================================================
// Vec4
// ============================================================================

TEST_SUITE("filamath::vec4") {

TEST_CASE("[filamath] vec4 size") {
	CHECK(sizeof(float4) == sizeof(float) * 4);
	CHECK(sizeof(double4) == sizeof(double) * 4);
}

TEST_CASE("[filamath] vec4 constructors") {
	double4 v1(1);
	CHECK(v1.x == 1);
	CHECK(v1.y == 1);
	CHECK(v1.z == 1);
	CHECK(v1.w == 1);

	double4 v2(1, 2, 3, 4);
	CHECK(v2.x == 1);
	CHECK(v2.y == 2);
	CHECK(v2.z == 3);
	CHECK(v2.w == 4);

	double4 v3(v2.xyz, 42);
	CHECK(v3.x == 1);
	CHECK(v3.y == 2);
	CHECK(v3.z == 3);
	CHECK(v3.w == 42);

	double4 v4(double3(v2.xy, 42), 24);
	CHECK(v4.x == 1);
	CHECK(v4.y == 2);
	CHECK(v4.z == 42);
	CHECK(v4.w == 24);

	// From two vec2s.
	double4 v5(double2(1, 2), double2(3, 4));
	CHECK(v5.x == 1);
	CHECK(v5.y == 2);
	CHECK(v5.z == 3);
	CHECK(v5.w == 4);
}

TEST_CASE("[filamath] vec4 access and swizzle") {
	double4 v(1, 2, 3, 4);
	CHECK(v[0] == 1);
	CHECK(v[1] == 2);
	CHECK(v[2] == 3);
	CHECK(v[3] == 4);

	v.xyz = double3(10, 20, 30);
	CHECK(v.x == 10);
	CHECK(v.y == 20);
	CHECK(v.z == 30);
	CHECK(v.w == 4);
}

TEST_CASE("[filamath] vec4 arithmetic") {
	double4 a(1, 2, 3, 4);
	double4 b(10, 20, 30, 40);

	double4 c = a + b;
	CHECK(c.x == 11);
	CHECK(c.y == 22);
	CHECK(c.z == 33);
	CHECK(c.w == 44);

	double4 d = b * 2;
	CHECK(d.x == 20);
	CHECK(d.y == 40);

	double4 e = 2 * b;
	CHECK(e == d);

	CHECK(dot(a, a) == doctest::Approx(30.0));
}

TEST_CASE("[filamath] vec4 abs/saturate") {
	float4 v(-1, 0.5f, 2, -3);
	float4 a = abs(v);
	CHECK(a.x == doctest::Approx(1));
	CHECK(a.y == doctest::Approx(0.5f));
	CHECK(a.z == doctest::Approx(2));
	CHECK(a.w == doctest::Approx(3));

	float4 s = saturate(v);
	CHECK(s.x == doctest::Approx(0));
	CHECK(s.y == doctest::Approx(0.5f));
	CHECK(s.z == doctest::Approx(1));
	CHECK(s.w == doctest::Approx(0));
}

} // TEST_SUITE vec4

// ============================================================================
// Mat2
// ============================================================================

TEST_SUITE("filamath::mat2") {

TEST_CASE("[filamath] mat2 identity") {
	mat2f m;
	CHECK(m[0][0] == 1);
	CHECK(m[0][1] == 0);
	CHECK(m[1][0] == 0);
	CHECK(m[1][1] == 1);
}

TEST_CASE("[filamath] mat2 trace") {
	mat2f m;
	CHECK(trace(m) == doctest::Approx(2.0f));
}

TEST_CASE("[filamath] mat2 transpose") {
	mat2f m(float2(1, 2), float2(3, 4));
	mat2f mt = transpose(m);
	CHECK(mt[0][0] == 1);
	CHECK(mt[0][1] == 3);
	CHECK(mt[1][0] == 2);
	CHECK(mt[1][1] == 4);
}

TEST_CASE("[filamath] mat2 inverse") {
	mat2f m;
	mat2f mi = inverse(m);
	CHECK(mi == m); // Inverse of identity is identity.

	mat2f m2(float2(4, 3), float2(3, 2));
	mat2f m2i = inverse(m2);
	mat2f product = m2 * m2i;
	CHECK(product[0][0] == doctest::Approx(1.0f));
	CHECK(product[0][1] == doctest::Approx(0.0f));
	CHECK(product[1][0] == doctest::Approx(0.0f));
	CHECK(product[1][1] == doctest::Approx(1.0f));
}

TEST_CASE("[filamath] mat2 determinant") {
	mat2f m(float2(4, 3), float2(3, 2));
	CHECK(det(m) == doctest::Approx(-1.0f));
}

} // TEST_SUITE mat2

// ============================================================================
// Mat3
// ============================================================================

TEST_SUITE("filamath::mat3") {

TEST_CASE("[filamath] mat3 identity") {
	mat3f m;
	CHECK(m[0][0] == 1);
	CHECK(m[1][1] == 1);
	CHECK(m[2][2] == 1);
	CHECK(m[0][1] == 0);
	CHECK(m[1][0] == 0);
}

TEST_CASE("[filamath] mat3 trace") {
	mat3f m;
	CHECK(trace(m) == doctest::Approx(3.0f));
}

TEST_CASE("[filamath] mat3 transpose") {
	mat3f m(float3(1, 2, 3), float3(4, 5, 6), float3(7, 8, 9));
	mat3f mt = transpose(m);
	CHECK(mt[0][0] == 1);
	CHECK(mt[0][1] == 4);
	CHECK(mt[0][2] == 7);
	CHECK(mt[1][0] == 2);
	CHECK(mt[1][1] == 5);
}

TEST_CASE("[filamath] mat3 from quaternion roundtrip") {
	quatf q = quatf::fromAxisAngle(float3(0, 0, 1), F_PI / 4.0f);
	mat3f m(q);
	// Rotating (1,0,0) by 45 degrees about Z should give (~0.707, ~0.707, 0).
	float3 v = m * float3(1, 0, 0);
	CHECK(v.x == doctest::Approx(std::cos(F_PI / 4.0f)).epsilon(1e-5));
	CHECK(v.y == doctest::Approx(std::sin(F_PI / 4.0f)).epsilon(1e-5));
	CHECK(v.z == doctest::Approx(0).epsilon(1e-5));
}

TEST_CASE("[filamath] mat3 rotation") {
	mat3f r = mat3f::rotation(F_PI / 2.0f, float3(0, 0, 1));
	float3 v = r * float3(1, 0, 0);
	CHECK(v.x == doctest::Approx(0).epsilon(1e-5));
	CHECK(v.y == doctest::Approx(1).epsilon(1e-5));
	CHECK(v.z == doctest::Approx(0).epsilon(1e-5));
}

TEST_CASE("[filamath] mat3 inverse") {
	mat3f m;
	mat3f mi = inverse(m);
	CHECK(mi == m);
}

} // TEST_SUITE mat3

// ============================================================================
// Mat4
// ============================================================================

TEST_SUITE("filamath::mat4") {

TEST_CASE("[filamath] mat4 size") {
	CHECK(sizeof(mat4) == sizeof(double) * 16);
	CHECK(sizeof(mat4f) == sizeof(float) * 16);
}

TEST_CASE("[filamath] mat4 identity") {
	mat4 m;
	CHECK(m[0].x == 1);
	CHECK(m[1].y == 1);
	CHECK(m[2].z == 1);
	CHECK(m[3].w == 1);
	CHECK(m[0].y == 0);
	CHECK(m[3].x == 0);
}

TEST_CASE("[filamath] mat4 constructors") {
	mat4 m1(2);
	mat4 m2(double4(2));
	mat4 m3(m2);
	CHECK(m1 == m2);
	CHECK(m2 == m3);
}

TEST_CASE("[filamath] mat4 comparison") {
	mat4 m0;
	mat4 m1(2);
	CHECK(m0 == m0);
	CHECK(m0 != m1);
	CHECK_FALSE(m0 != m0);
	CHECK_FALSE(m0 == m1);
}

TEST_CASE("[filamath] mat4 arithmetic") {
	mat4 m0;
	mat4 m1(2);
	mat4 m2(double4(2));

	m1 += m2;
	CHECK(m1 == mat4(4));

	m2 -= m1;
	CHECK(m2 == mat4(-2));

	mat4 m3(2);
	m3 *= 2;
	CHECK(m3 == mat4(4));

	m3 /= 2;
	CHECK(m3 == mat4(2));

	mat4 neg = -m0;
	CHECK(neg == mat4(-1));
}

TEST_CASE("[filamath] mat4 trace") {
	mat4 m;
	CHECK(trace(m) == doctest::Approx(4.0));
}

TEST_CASE("[filamath] mat4 transpose") {
	mat4 m1(double4(1, 2, 3, 4), double4(5, 6, 7, 8),
			double4(9, 10, 11, 12), double4(13, 14, 15, 16));
	mat4 m2(double4(1, 5, 9, 13), double4(2, 6, 10, 14),
			double4(3, 7, 11, 15), double4(4, 8, 12, 16));
	CHECK(m1 == transpose(m2));
	CHECK(m2 == transpose(m1));
}

TEST_CASE("[filamath] mat4 diag") {
	mat4 m(double4(1, 2, 3, 4), double4(5, 6, 7, 8),
			double4(9, 10, 11, 12), double4(13, 14, 15, 16));
	double4 d = diag(m);
	CHECK(d.x == 1);
	CHECK(d.y == 6);
	CHECK(d.z == 11);
	CHECK(d.w == 16);
}

TEST_CASE("[filamath] mat4 inverse identity") {
	mat4 identity;
	CHECK(inverse(identity) == identity);
}

TEST_CASE("[filamath] mat4 inverse") {
	mat4 m(double4(4, 3, 0, 0), double4(3, 2, 0, 0),
			double4(0, 0, 1, 0), double4(0, 0, 0, 1));
	mat4 mi = inverse(m);
	CHECK(mi[0][0] == doctest::Approx(-2));
	CHECK(mi[0][1] == doctest::Approx(3));
	CHECK(mi[1][0] == doctest::Approx(3));
	CHECK(mi[1][1] == doctest::Approx(-4));

	// Double inverse should return original.
	mat4 mii = inverse(mi);
	CHECK(mii[0][0] == doctest::Approx(m[0][0]));
	CHECK(mii[0][1] == doctest::Approx(m[0][1]));
	CHECK(mii[1][0] == doctest::Approx(m[1][0]));
	CHECK(mii[1][1] == doctest::Approx(m[1][1]));
}

TEST_CASE("[filamath] mat4 translation") {
	mat4f t = mat4f::translation(float3(1, 2, 3));
	float4 p = t * float4(0, 0, 0, 1);
	CHECK(p.x == doctest::Approx(1));
	CHECK(p.y == doctest::Approx(2));
	CHECK(p.z == doctest::Approx(3));
	CHECK(p.w == doctest::Approx(1));
}

TEST_CASE("[filamath] mat4 scaling") {
	mat4f s = mat4f::scaling(float3(2, 3, 4));
	float4 p = s * float4(1, 1, 1, 1);
	CHECK(p.x == doctest::Approx(2));
	CHECK(p.y == doctest::Approx(3));
	CHECK(p.z == doctest::Approx(4));
	CHECK(p.w == doctest::Approx(1));
}

TEST_CASE("[filamath] mat4 ortho") {
	mat4f o = mat4f::ortho(-1, 1, -1, 1, -1, 1);
	// Ortho maps corners to [-1,1] NDC.
	float4 p = o * float4(1, 1, -1, 1);
	CHECK(p.x == doctest::Approx(1));
	CHECK(p.y == doctest::Approx(1));
}

TEST_CASE("[filamath] mat4 upperLeft") {
	mat4f m(float4(1, 2, 3, 4), float4(5, 6, 7, 8),
			float4(9, 10, 11, 12), float4(13, 14, 15, 16));
	mat3f u = m.upperLeft();
	CHECK(u[0][0] == 1);
	CHECK(u[1][1] == 6);
	CHECK(u[2][2] == 11);
}

} // TEST_SUITE mat4

// ============================================================================
// Quaternion
// ============================================================================

TEST_SUITE("filamath::quat") {

TEST_CASE("[filamath] quat size") {
	CHECK(sizeof(quat) == sizeof(double) * 4);
	CHECK(sizeof(quatf) == sizeof(float) * 4);
}

TEST_CASE("[filamath] quat constructors") {
	quat q0;
	CHECK(q0.x == 0);
	CHECK(q0.y == 0);
	CHECK(q0.z == 0);
	CHECK(q0.w == 0);

	quat q1(1);
	CHECK(q1.x == 0);
	CHECK(q1.y == 0);
	CHECK(q1.z == 0);
	CHECK(q1.w == 1);

	quat q2(1, 2, 3, 4); // w, x, y, z.
	CHECK(q2.x == 2);
	CHECK(q2.y == 3);
	CHECK(q2.z == 4);
	CHECK(q2.w == 1);

	quat q3(q2);
	CHECK(q3.x == 2);
	CHECK(q3.y == 3);
	CHECK(q3.z == 4);
	CHECK(q3.w == 1);
}

TEST_CASE("[filamath] quat access") {
	quat q(1, 2, 3, 4);
	q.x = 10;
	q.y = 20;
	q.z = 30;
	q.w = 40;
	CHECK(q[0] == 10);
	CHECK(q[1] == 20);
	CHECK(q[2] == 30);
	CHECK(q[3] == 40);
}

TEST_CASE("[filamath] quat comparison") {
	quat a(1, 2, 3, 4);
	quat b(10, 20, 30, 40);
	CHECK(a == a);
	CHECK(a != b);
	CHECK_FALSE(a != a);
	CHECK_FALSE(a == b);
}

TEST_CASE("[filamath] quat arithmetic") {
	quat a(1, 2, 3, 4);
	quat b(10, 20, 30, 40);

	quat c = a + b;
	CHECK(c.x == 22);
	CHECK(c.y == 33);
	CHECK(c.z == 44);
	CHECK(c.w == 11);

	quat d = b * 2;
	CHECK(d.x == 40);
	CHECK(d.y == 60);
	CHECK(d.z == 80);
	CHECK(d.w == 20);
}

TEST_CASE("[filamath] quat conjugate") {
	quat q(1, 2, 3, 4);
	quat qc = conj(q);
	CHECK(qc.x == -2);
	CHECK(qc.y == -3);
	CHECK(qc.z == -4);
	CHECK(qc.w == 1);

	CHECK(~q == qc);
	CHECK(length(q) == doctest::Approx(length(qc)));
}

TEST_CASE("[filamath] quat normalize") {
	quat q(1, 2, 3, 4);
	quat qn = normalize(q);
	CHECK(length(qn) == doctest::Approx(1.0));
	CHECK(dot(qn, qn) == doctest::Approx(1.0));
}

TEST_CASE("[filamath] quat length") {
	quat q(1, 2, 3, 4);
	CHECK(length(q) == doctest::Approx(std::sqrt(30.0)));
}

TEST_CASE("[filamath] quat fromAxisAngle") {
	quat q = quat::fromAxisAngle(double3(0, 0, 1), F_PI / 2.0);
	// Rotating (1,0,0) by 90 degrees about Z should give (0,1,0).
	double3 v = q * double3(1, 0, 0);
	CHECK(v.x == doctest::Approx(0).epsilon(1e-10));
	CHECK(v.y == doctest::Approx(1).epsilon(1e-10));
	CHECK(v.z == doctest::Approx(0).epsilon(1e-10));
}

TEST_CASE("[filamath] quat slerp") {
	quat qa = quat::fromAxisAngle(double3(0, 0, 1), 0);
	quat qb = quat::fromAxisAngle(double3(0, 0, 1), F_PI / 2.0);
	quat qs = slerp(qa, qb, 0.5);
	quat qr = quat::fromAxisAngle(double3(0, 0, 1), F_PI / 4.0);
	CHECK(qr.x == doctest::Approx(qs.x).epsilon(1e-10));
	CHECK(qr.y == doctest::Approx(qs.y).epsilon(1e-10));
	CHECK(qr.z == doctest::Approx(qs.z).epsilon(1e-10));
	CHECK(qr.w == doctest::Approx(qs.w).epsilon(1e-10));
}

TEST_CASE("[filamath] quat nlerp") {
	quat qa = quat::fromAxisAngle(double3(0, 0, 1), 0);
	quat qb = quat::fromAxisAngle(double3(0, 0, 1), F_PI / 2.0);
	quat qs = nlerp(qa, qb, 0.5);
	quat qr = quat::fromAxisAngle(double3(0, 0, 1), F_PI / 4.0);
	CHECK(qr.x == doctest::Approx(qs.x).epsilon(1e-10));
	CHECK(qr.y == doctest::Approx(qs.y).epsilon(1e-10));
	CHECK(qr.z == doctest::Approx(qs.z).epsilon(1e-10));
	CHECK(qr.w == doctest::Approx(qs.w).epsilon(1e-10));
}

TEST_CASE("[filamath] quat to mat4 roundtrip") {
	quat q = quat::fromAxisAngle(double3(0, 0, 1), F_PI / 2.0);
	quat q2 = mat4(q).toQuaternion();
	CHECK(q.x == doctest::Approx(q2.x).epsilon(1e-10));
	CHECK(q.y == doctest::Approx(q2.y).epsilon(1e-10));
	CHECK(q.z == doctest::Approx(q2.z).epsilon(1e-10));
	CHECK(q.w == doctest::Approx(q2.w).epsilon(1e-10));
}

TEST_CASE("[filamath] quat log/exp roundtrip") {
	quat q = quat::fromAxisAngle(double3(0, 0, 1), F_PI / 2.0);
	quat q2 = log(exp(q));
	CHECK(q.x == doctest::Approx(q2.x).epsilon(1e-10));
	CHECK(q.y == doctest::Approx(q2.y).epsilon(1e-10));
	CHECK(q.z == doctest::Approx(q2.z).epsilon(1e-10));
	CHECK(q.w == doctest::Approx(q2.w).epsilon(1e-10));
}

TEST_CASE("[filamath] quat pow") {
	quat q = quat::fromAxisAngle(double3(0, 0, 1), F_PI / 4.0);
	quat qq = q * q;
	quat q2 = pow(q, 2);
	CHECK(qq.x == doctest::Approx(q2.x).epsilon(1e-12));
	CHECK(qq.y == doctest::Approx(q2.y).epsilon(1e-12));
	CHECK(qq.z == doctest::Approx(q2.z).epsilon(1e-12));
	CHECK(qq.w == doctest::Approx(q2.w).epsilon(1e-12));
}

TEST_CASE("[filamath] quat NaN safety") {
	// Near-identical quaternions should not produce NaN in slerp.
	quatf qa = {0.5f, 0.5f, 0.5f, 0.5f};
	quatf qb = {0.49995f, 0.49998f, 0.49998f, 0.49995f};
	quatf qs = slerp(qa, qb, 0.034934f);
	CHECK_FALSE(std::isnan(qs.x));
	CHECK_FALSE(std::isnan(qs.y));
	CHECK_FALSE(std::isnan(qs.z));
	CHECK_FALSE(std::isnan(qs.w));
}

} // TEST_SUITE quat

// ============================================================================
// Half precision
// ============================================================================

TEST_SUITE("filamath::half") {

TEST_CASE("[filamath] half size") {
	CHECK(sizeof(half) == 2);
	CHECK(sizeof(half2) == 4);
	CHECK(sizeof(half3) == 6);
	CHECK(sizeof(half4) == 8);
}

TEST_CASE("[filamath] half conversion") {
	half h(1.0f);
	float f = float(h);
	CHECK(f == doctest::Approx(1.0f));

	half h2(0.5f);
	CHECK(float(h2) == doctest::Approx(0.5f));
}

TEST_CASE("[filamath] half zero and special") {
	half zero(0.0f);
	CHECK(float(zero) == doctest::Approx(0.0f));

	half neg(-1.0f);
	CHECK(float(neg) == doctest::Approx(-1.0f));
}

} // TEST_SUITE half

// ============================================================================
// Fast math
// ============================================================================

TEST_SUITE("filamath::fast") {

TEST_CASE("[filamath] fast ilog2") {
	CHECK(fast::ilog2(1.0f) == 0);
	CHECK(fast::ilog2(2.0f) == 1);
	CHECK(fast::ilog2(4.0f) == 2);
	CHECK(fast::ilog2(8.0f) == 3);
}

TEST_CASE("[filamath] fast isqrt approximation") {
	float v = fast::isqrt(4.0f);
	CHECK(v == doctest::Approx(0.5f).epsilon(0.02f));

	v = fast::isqrt(1.0f);
	CHECK(v == doctest::Approx(1.0f).epsilon(0.02f));
}

} // TEST_SUITE fast

// ============================================================================
// Norm (pack/unpack)
// ============================================================================

TEST_SUITE("filamath::norm") {

TEST_CASE("[filamath] snorm16 roundtrip") {
	int16_t packed = packSnorm16(0.5f);
	float unpacked = unpackSnorm16(packed);
	CHECK(unpacked == doctest::Approx(0.5f).epsilon(0.001f));
}

TEST_CASE("[filamath] unorm16 roundtrip") {
	uint16_t packed = packUnorm16(0.5f);
	float unpacked = unpackUnorm16(packed);
	CHECK(unpacked == doctest::Approx(0.5f).epsilon(0.001f));
}

TEST_CASE("[filamath] snorm16 extremes") {
	CHECK(unpackSnorm16(packSnorm16(1.0f)) == doctest::Approx(1.0f).epsilon(0.001f));
	CHECK(unpackSnorm16(packSnorm16(-1.0f)) == doctest::Approx(-1.0f).epsilon(0.001f));
	CHECK(unpackSnorm16(packSnorm16(0.0f)) == doctest::Approx(0.0f).epsilon(0.001f));
}

TEST_CASE("[filamath] unorm16 extremes") {
	CHECK(unpackUnorm16(packUnorm16(1.0f)) == doctest::Approx(1.0f).epsilon(0.001f));
	CHECK(unpackUnorm16(packUnorm16(0.0f)) == doctest::Approx(0.0f).epsilon(0.001f));
}

} // TEST_SUITE norm
