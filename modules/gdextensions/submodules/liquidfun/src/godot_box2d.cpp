
#include "godot_box2d.h"

#include <Box2D/Box2D.h>

Box2D *Box2D::singleton = NULL;

Box2D::Box2D() {
	ERR_FAIL_COND(singleton);
	singleton = this;
}

Box2D::~Box2D() {
	singleton = NULL;
}

WorldB2 *Box2D::world(const Vector2 &gravity) {
	auto o = memnew(b2World(B2(gravity)));
	return memnew(WorldB2(o));
}

BodyB2 *Box2D::body(WorldB2 *world, int type, const Transform2D &xf) {
	ERR_FAIL_NULL_V(world, NULL);

	static b2BodyDef def;
	def.type = (b2BodyType)type;
	def.position = B2(xf.get_origin());
	def.angle = xf.get_rotation();

	auto *o = world->get_b2()->CreateBody(&def);
	return memnew(BodyB2(o));
}

FixtureB2 *Box2D::fixture(BodyB2 *body, const ShapeB2 *shape, float density) {
	ERR_FAIL_NULL_V(body, NULL);
	ERR_FAIL_NULL_V(shape, NULL);

	auto *o = body->get_b2()->CreateFixture(shape->get_b2(), density);
	return memnew(FixtureB2(o));
}

Ref<ShapeB2> Box2D::circle(const Vector2 &offset, float radius) {
	auto o = memnew(b2CircleShape);
	o->m_p = B2(offset);
	o->m_radius = radius;
	return memnew(ShapeB2(o, true));
}

Ref<ShapeB2> Box2D::box(const Vector2 &extents, const Vector2 &offset, float angle) {
	auto o = memnew(b2PolygonShape);
	o->SetAsBox(extents.x, extents.y, B2(offset), angle);
	return memnew(ShapeB2(o, true));
}

Ref<ShapeB2> Box2D::poly(const PoolVector2Array &vertices) {
	auto r = vertices.read();
	auto o = memnew(b2PolygonShape);
	o->Set((b2Vec2 *)r.ptr(), vertices.size());
	return memnew(ShapeB2(o, true));
}

Ref<ShapeB2> Box2D::edge(const Vector2 &a, const Vector2 &b) {
	auto o = memnew(b2EdgeShape);
	o->Set(B2(a), B2(b));
	return memnew(ShapeB2(o, true));
}

Ref<ShapeB2> Box2D::chain(const PoolVector2Array &vertices, bool loop) {
	auto r = vertices.read();
	auto o = memnew(b2ChainShape);
	if (loop)
		o->CreateLoop((b2Vec2 *)r.ptr(), vertices.size());
	else
		o->CreateChain((b2Vec2 *)r.ptr(), vertices.size());
	return memnew(ShapeB2(o, true));
}

bool Box2D::overlap_fixtures(FixtureB2 *a, FixtureB2 *b) {
	ERR_FAIL_NULL_V(a, false);
	ERR_FAIL_NULL_V(b, false);

	b2Fixture *fa = a->get_b2();
	b2Fixture *fb = b->get_b2();

	b2Body *ba = fa->GetBody();
	b2Body *bb = fb->GetBody();

	return b2TestOverlap(fa->GetShape(), 0, fb->GetShape(), 0, ba->GetTransform(), bb->GetTransform());
}

bool Box2D::overlap_shapes(ShapeB2 *a, ShapeB2 *b, const Transform2D &xf_a, const Transform2D &xf_b) {
	ERR_FAIL_NULL_V(a, false);
	ERR_FAIL_NULL_V(b, false);

	return b2TestOverlap(a->get_b2(), 0, b->get_b2(), 0, B2(xf_a), B2(xf_b));
}

void Box2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("world", "gravity"), &Box2D::world);
	ClassDB::bind_method(D_METHOD("body", "world", "type", "transform"), &Box2D::body, DEFVAL(Transform2D()));
	ClassDB::bind_method(D_METHOD("fixture", "body", "shape", "density"), &Box2D::fixture);

	ClassDB::bind_method(D_METHOD("circle", "offset", "radius"), &Box2D::circle);
	ClassDB::bind_method(D_METHOD("box", "extents", "offset", "angle"), &Box2D::box, DEFVAL(Vector2()), DEFVAL(.0f));
	ClassDB::bind_method(D_METHOD("poly", "vertices"), &Box2D::poly);
	ClassDB::bind_method(D_METHOD("edge", "a", "b"), &Box2D::edge);
	ClassDB::bind_method(D_METHOD("chain", "vertices", "loop"), &Box2D::chain, DEFVAL(false));

	ClassDB::bind_method(D_METHOD("overlap_fixtures", "a", "b"), &Box2D::overlap_fixtures);
	ClassDB::bind_method(D_METHOD("overlap_shapes", "a", "b", "xf_a", "xf_b"), &Box2D::overlap_shapes);
}

b2Vec2 B2(const Vector2 &v) { return b2Vec2(v.x, v.y); }
Vector2 GD(const b2Vec2 &v) { return Vector2(v.x, v.y); }

b2AABB B2(const Rect2 &v) { return b2AABB{ B2(v.position), B2(v.position + v.size) }; }
Rect2 GD(const b2AABB &v) { return Rect2(GD(v.lowerBound), GD(v.upperBound - v.lowerBound)); }

b2Transform B2(const Transform2D &v) {
	b2Transform xf;
	xf.p.x = v.elements[2].x;
	xf.p.y = v.elements[2].y;
	Vector2 rot = v.elements[0].normalized();
	xf.q.c = rot.x;
	xf.q.s = rot.y;
	return xf;
}

Transform2D GD(const b2Transform &v) {
	return Transform2D(v.q.c, v.q.s, -v.q.s, v.q.c, v.p.x, v.p.y);
}

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[liquidfun] math conversions") {
	TEST_CASE("[liquidfun] B2/GD Vec2 roundtrip") {
		b2Vec2 bv = B2(Vector2(3.0f, 4.0f));
		CHECK(bv.x == doctest::Approx(3.0f));
		CHECK(bv.y == doctest::Approx(4.0f));
		Vector2 v = GD(b2Vec2(7.0f, -2.0f));
		CHECK(v.x == doctest::Approx(7.0f));
		CHECK(v.y == doctest::Approx(-2.0f));
	}

	TEST_CASE("[liquidfun] B2/GD Rect2 roundtrip") {
		b2AABB aabb = B2(Rect2(1.0f, 2.0f, 3.0f, 4.0f));
		CHECK(aabb.lowerBound.x == doctest::Approx(1.0f));
		CHECK(aabb.lowerBound.y == doctest::Approx(2.0f));
		CHECK(aabb.upperBound.x == doctest::Approx(4.0f));
		CHECK(aabb.upperBound.y == doctest::Approx(6.0f));
		Rect2 back = GD(aabb);
		CHECK(back.position.x == doctest::Approx(1.0f));
		CHECK(back.position.y == doctest::Approx(2.0f));
		CHECK(back.size.x == doctest::Approx(3.0f));
		CHECK(back.size.y == doctest::Approx(4.0f));
	}

	TEST_CASE("[liquidfun] B2 Transform2D identity") {
		b2Transform bt = B2(Transform2D());
		CHECK(bt.p.x == doctest::Approx(0.0f));
		CHECK(bt.p.y == doctest::Approx(0.0f));
		CHECK(bt.q.c == doctest::Approx(1.0f));
		CHECK(bt.q.s == doctest::Approx(0.0f));
	}

	TEST_CASE("[liquidfun] GD b2Transform identity") {
		b2Transform xf;
		xf.SetIdentity();
		Transform2D gd = GD(xf);
		CHECK(gd.elements[0].x == doctest::Approx(1.0f));
		CHECK(gd.elements[0].y == doctest::Approx(0.0f));
		CHECK(gd.elements[1].x == doctest::Approx(0.0f));
		CHECK(gd.elements[1].y == doctest::Approx(1.0f));
		CHECK(gd.elements[2].x == doctest::Approx(0.0f));
		CHECK(gd.elements[2].y == doctest::Approx(0.0f));
	}

	TEST_CASE("[liquidfun] B2 Transform2D 90-degree rotation extracts correct cos/sin") {
		Transform2D xf(Math_PI / 2.0f, Vector2(3.0f, 4.0f));
		b2Transform bt = B2(xf);
		CHECK(bt.p.x == doctest::Approx(3.0f));
		CHECK(bt.p.y == doctest::Approx(4.0f));
		CHECK(bt.q.c == doctest::Approx(0.0f)); // cos(π/2) ≈ 0; scale=1 handles near-zero
		CHECK(bt.q.s == doctest::Approx(1.0f));
	}

	TEST_CASE("[liquidfun] GD/B2 Transform2D roundtrip at 45 degrees") {
		Transform2D xf(Math_PI / 4.0f, Vector2(5.0f, 7.0f));
		Transform2D back = GD(B2(xf));
		CHECK(back.elements[0].x == doctest::Approx(xf.elements[0].x));
		CHECK(back.elements[0].y == doctest::Approx(xf.elements[0].y));
		CHECK(back.elements[1].x == doctest::Approx(xf.elements[1].x));
		CHECK(back.elements[1].y == doctest::Approx(xf.elements[1].y));
		CHECK(back.elements[2].x == doctest::Approx(5.0f));
		CHECK(back.elements[2].y == doctest::Approx(7.0f));
	}
} // TEST_SUITE("[liquidfun] math conversions")

#endif // DOCTEST
