#include "godot_box2d.h"

#include "fixture.h"

#include <Box2D/Box2D.h>

FixtureB2::FixtureB2(b2Fixture *entity) :
		entity(entity) {
	entity->SetUserData(this);
}

FixtureB2::~FixtureB2() {
	entity->GetBody()->DestroyFixture(entity);
}

Ref<ShapeB2> FixtureB2::get_shape() const {
	return memnew(ShapeB2(entity->GetShape(), false));
}

bool FixtureB2::is_sensor() const {
	return entity->IsSensor();
}

void FixtureB2::set_sensor(bool rhs) {
	entity->SetSensor(rhs);
}

int FixtureB2::get_filter_category() const {
	return entity->GetFilterData().categoryBits;
}

void FixtureB2::set_filter_category(int rhs) {
	auto data = entity->GetFilterData();
	data.categoryBits = (uint16)rhs;
	entity->SetFilterData(data);
}

int FixtureB2::get_filter_mask() const {
	return entity->GetFilterData().maskBits;
}

void FixtureB2::set_filter_mask(int rhs) {
	auto data = entity->GetFilterData();
	data.maskBits = (uint16)rhs;
	entity->SetFilterData(data);
}

int FixtureB2::get_filter_group() const {
	return entity->GetFilterData().groupIndex;
}

void FixtureB2::set_filter_group(int rhs) {
	auto data = entity->GetFilterData();
	data.groupIndex = (int16)rhs;
	entity->SetFilterData(data);
}

void FixtureB2::refilter() {
	entity->Refilter();
}

BodyB2 *FixtureB2::get_body() const {
	return BodyB2::get(entity->GetBody());
}

Variant FixtureB2::get_metadata() const {
	return metadata;
}

void FixtureB2::set_metadata(const Variant &rhs) {
	metadata = rhs;
}

bool FixtureB2::test_point(const Vector2 &point) const {
	return entity->TestPoint(B2(point));
}

Dictionary FixtureB2::ray_cast(const Vector2 &a, const Vector2 &b, int child) const {
	b2RayCastInput input{ B2(a), B2(b), 1.f };
	b2RayCastOutput out;
	Dictionary r;

	if (entity->RayCast(&out, input, child)) {
		r["normal"] = GD(out.normal);
		r["fraction"] = out.fraction;
	}

	return r;
}

Dictionary FixtureB2::get_mass_data() const {
	b2MassData data;
	entity->GetMassData(&data);

	Dictionary r;
	r["mass"] = data.mass;
	r["center"] = GD(data.center);
	r["inertia"] = data.I;
	return r;
}

float FixtureB2::get_density() const {
	return entity->GetDensity();
}

void FixtureB2::set_density(float rhs) {
	entity->SetDensity(rhs);
}

float FixtureB2::get_friction() const {
	return entity->GetFriction();
}

void FixtureB2::set_friction(float rhs) {
	entity->SetFriction(rhs);
}

float FixtureB2::get_restitution() const {
	return entity->GetRestitution();
}

void FixtureB2::set_restitution(float rhs) {
	entity->SetRestitution(rhs);
}

Rect2 FixtureB2::get_aabb(int child) const {
	return GD(entity->GetAABB(child));
}

void FixtureB2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_shape"), &FixtureB2::get_shape);

	BOX2D_PROPERTY_BOOL(FixtureB2, sensor);

	BOX2D_PROPERTY(FixtureB2, filter_category, Variant::INT);
	BOX2D_PROPERTY(FixtureB2, filter_mask, Variant::INT);
	BOX2D_PROPERTY(FixtureB2, filter_group, Variant::INT);

	ClassDB::bind_method(D_METHOD("refilter"), &FixtureB2::refilter);

	ClassDB::bind_method(D_METHOD("get_body"), &FixtureB2::get_body);

	BOX2D_PROPERTY(FixtureB2, metadata, Variant::NIL);

	ClassDB::bind_method(D_METHOD("test_point", "point"), &FixtureB2::test_point);
	ClassDB::bind_method(D_METHOD("ray_cast", "a", "b", "childIndex"), &FixtureB2::ray_cast, DEFVAL(0));

	ClassDB::bind_method(D_METHOD("get_mass_data"), &FixtureB2::get_mass_data);

	BOX2D_PROPERTY(FixtureB2, density, Variant::REAL);
	BOX2D_PROPERTY(FixtureB2, friction, Variant::REAL);
	BOX2D_PROPERTY(FixtureB2, restitution, Variant::REAL);

	ClassDB::bind_method(D_METHOD("get_aabb", "childIndex"), &FixtureB2::get_aabb, DEFVAL(0));
}

FixtureB2 *FixtureB2::get(const b2Fixture *o) {
	return (FixtureB2 *)o->GetUserData();
}

#ifdef DOCTEST
#include "doctest/doctest.h"

static WorldB2 *make_fixture_test_world() {
	return memnew(WorldB2(memnew(b2World(b2Vec2(0.0f, -9.8f)))));
}

static BodyB2 *make_fixture_test_body(WorldB2 *w, int type = b2_staticBody) {
	b2BodyDef bdef;
	bdef.type = (b2BodyType)type;
	auto *b2b = w->get_b2()->CreateBody(&bdef);
	return memnew(BodyB2(b2b));
}

TEST_SUITE("[liquidfun] FixtureDefB2") {
	TEST_CASE("[liquidfun] FixtureDefB2 defaults match b2FixtureDef") {
		FixtureDefB2 *def = memnew(FixtureDefB2);
		CHECK(def->get_friction() == doctest::Approx(0.2f));
		CHECK(def->get_restitution() == doctest::Approx(0.0f));
		CHECK(def->get_density() == doctest::Approx(0.0f));
		CHECK(def->get_sensor() == false);
		CHECK(def->get_filter_category() == 0x0001);
		CHECK(def->get_filter_mask() == 0xFFFF);
		CHECK(def->get_filter_group() == 0);
		memdelete(def);
	}

	TEST_CASE("[liquidfun] FixtureDefB2 setters roundtrip") {
		FixtureDefB2 *def = memnew(FixtureDefB2);
		def->set_friction(0.5f);
		def->set_restitution(0.3f);
		def->set_density(1.2f);
		def->set_sensor(true);
		def->set_filter_category(0x0002);
		def->set_filter_mask(0x0004);
		def->set_filter_group(-1);
		CHECK(def->get_friction() == doctest::Approx(0.5f));
		CHECK(def->get_restitution() == doctest::Approx(0.3f));
		CHECK(def->get_density() == doctest::Approx(1.2f));
		CHECK(def->get_sensor() == true);
		CHECK(def->get_filter_category() == 0x0002);
		CHECK(def->get_filter_mask() == 0x0004);
		CHECK(def->get_filter_group() == -1);
		memdelete(def);
	}
} // TEST_SUITE("[liquidfun] FixtureDefB2")

TEST_SUITE("[liquidfun] FixtureB2") {
	TEST_CASE("[liquidfun] FixtureB2 density and body back-reference") {
		WorldB2 *w = make_fixture_test_world();
		BodyB2 *body = make_fixture_test_body(w, b2_dynamicBody);
		b2CircleShape cs;
		cs.m_p = b2Vec2(0.0f, 0.0f);
		cs.m_radius = 1.0f;
		FixtureB2 *fx = memnew(FixtureB2(body->get_b2()->CreateFixture(&cs, 2.0f)));
		CHECK(fx->get_density() == doctest::Approx(2.0f));
		CHECK(fx->get_body() == body);
		CHECK(body->get_fixture_list().size() == 1);
		memdelete(w);
	}

	TEST_CASE("[liquidfun] FixtureB2 test_point inside and outside circle") {
		WorldB2 *w = make_fixture_test_world();
		BodyB2 *body = make_fixture_test_body(w);
		b2CircleShape cs;
		cs.m_p = b2Vec2(0.0f, 0.0f);
		cs.m_radius = 1.0f;
		FixtureB2 *fx = memnew(FixtureB2(body->get_b2()->CreateFixture(&cs, 1.0f)));
		CHECK(fx->test_point(Vector2(0.0f, 0.0f)) == true);
		CHECK(fx->test_point(Vector2(0.5f, 0.0f)) == true);
		CHECK(fx->test_point(Vector2(2.0f, 0.0f)) == false);
		memdelete(w);
	}

	TEST_CASE("[liquidfun] FixtureB2 sensor flag get/set") {
		WorldB2 *w = make_fixture_test_world();
		BodyB2 *body = make_fixture_test_body(w);
		b2CircleShape cs;
		cs.m_radius = 0.5f;
		FixtureB2 *fx = memnew(FixtureB2(body->get_b2()->CreateFixture(&cs, 1.0f)));
		CHECK(fx->is_sensor() == false);
		fx->set_sensor(true);
		CHECK(fx->is_sensor() == true);
		memdelete(w);
	}

	TEST_CASE("[liquidfun] FixtureB2 filter bits get/set") {
		WorldB2 *w = make_fixture_test_world();
		BodyB2 *body = make_fixture_test_body(w);
		b2CircleShape cs;
		cs.m_radius = 0.5f;
		FixtureB2 *fx = memnew(FixtureB2(body->get_b2()->CreateFixture(&cs, 1.0f)));
		fx->set_filter_category(0x0004);
		fx->set_filter_mask(0x0008);
		fx->set_filter_group(2);
		CHECK(fx->get_filter_category() == 0x0004);
		CHECK(fx->get_filter_mask() == 0x0008);
		CHECK(fx->get_filter_group() == 2);
		memdelete(w);
	}

	TEST_CASE("[liquidfun] FixtureB2 friction and restitution get/set") {
		WorldB2 *w = make_fixture_test_world();
		BodyB2 *body = make_fixture_test_body(w, b2_dynamicBody);
		b2CircleShape cs;
		cs.m_radius = 0.5f;
		FixtureB2 *fx = memnew(FixtureB2(body->get_b2()->CreateFixture(&cs, 1.0f)));
		fx->set_friction(0.6f);
		fx->set_restitution(0.4f);
		CHECK(fx->get_friction() == doctest::Approx(0.6f));
		CHECK(fx->get_restitution() == doctest::Approx(0.4f));
		memdelete(w);
	}

	TEST_CASE("[liquidfun] FixtureB2 aabb is non-empty for circle") {
		WorldB2 *w = make_fixture_test_world();
		BodyB2 *body = make_fixture_test_body(w);
		b2CircleShape cs;
		cs.m_p = b2Vec2(0.0f, 0.0f);
		cs.m_radius = 1.0f;
		FixtureB2 *fx = memnew(FixtureB2(body->get_b2()->CreateFixture(&cs, 0.0f)));
		Rect2 aabb = fx->get_aabb();
		CHECK(aabb.size.x > 0.0f);
		CHECK(aabb.size.y > 0.0f);
		memdelete(w);
	}
} // TEST_SUITE("[liquidfun] FixtureB2")

#endif // DOCTEST

FixtureDefB2::FixtureDefB2() :
		def(memnew(b2FixtureDef)) {
}

FixtureDefB2::~FixtureDefB2() {
	memdelete(def);
}

FixtureB2 *FixtureDefB2::instance(BodyB2 *body) {
	ERR_FAIL_NULL_V(body, NULL);
	auto *o = body->get_b2()->CreateFixture(def);

	auto *fixture = memnew(FixtureB2(o));
	fixture->set_metadata(metadata);

	return fixture;
}

Ref<ShapeB2> FixtureDefB2::get_shape() const {
	return shape;
}

void FixtureDefB2::set_shape(const Ref<ShapeB2> &rhs) {
	def->shape = rhs.is_null() ? NULL : rhs->get_b2();
	shape = rhs;
}

float FixtureDefB2::get_friction() const {
	return def->friction;
}

void FixtureDefB2::set_friction(float rhs) {
	def->friction = rhs;
}

float FixtureDefB2::get_restitution() const {
	return def->restitution;
}

void FixtureDefB2::set_restitution(float rhs) {
	def->restitution = rhs;
}

float FixtureDefB2::get_density() const {
	return def->density;
}

void FixtureDefB2::set_density(float rhs) {
	def->density = rhs;
}

bool FixtureDefB2::get_sensor() const {
	return def->isSensor;
}

void FixtureDefB2::set_sensor(bool rhs) {
	def->isSensor = rhs;
}

int FixtureDefB2::get_filter_category() const {
	return def->filter.categoryBits;
}

void FixtureDefB2::set_filter_category(int rhs) {
	def->filter.categoryBits = (uint16)rhs;
}

int FixtureDefB2::get_filter_mask() const {
	return def->filter.maskBits;
}

void FixtureDefB2::set_filter_mask(int rhs) {
	def->filter.maskBits = (uint16)rhs;
}

int FixtureDefB2::get_filter_group() const {
	return def->filter.groupIndex;
}

void FixtureDefB2::set_filter_group(int rhs) {
	def->filter.groupIndex = (int16)rhs;
}

Variant FixtureDefB2::get_metadata() const {
	return metadata;
}

void FixtureDefB2::set_metadata(const Variant &rhs) {
	metadata = rhs;
}

void FixtureDefB2::_bind_methods() {
	ClassDB::bind_method(D_METHOD("instance", "body"), &FixtureDefB2::instance);

	BOX2D_PROPERTY(FixtureDefB2, shape, Variant::OBJECT);
	BOX2D_PROPERTY(FixtureDefB2, friction, Variant::REAL);
	BOX2D_PROPERTY(FixtureDefB2, restitution, Variant::REAL);
	BOX2D_PROPERTY(FixtureDefB2, density, Variant::REAL);
	BOX2D_PROPERTY(FixtureDefB2, sensor, Variant::BOOL);
	BOX2D_PROPERTY(FixtureDefB2, filter_category, Variant::INT);
	BOX2D_PROPERTY(FixtureDefB2, filter_mask, Variant::INT);
	BOX2D_PROPERTY(FixtureDefB2, filter_group, Variant::INT);
}
