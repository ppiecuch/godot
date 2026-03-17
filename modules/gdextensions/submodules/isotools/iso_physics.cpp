/**************************************************************************/
/*  iso_physics.cpp                                                       */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#include "iso_physics.h"
#include "iso_world.h"

#include "servers/physics_2d_server.h"

Dictionary IsoPhysics::iso_raycast(Object *p_world_obj, const Vector3 &p_iso_from, const Vector3 &p_iso_to,
		const Array &p_exclude, int p_collision_mask) {
	IsoWorld *p_world = Object::cast_to<IsoWorld>(p_world_obj);
	ERR_FAIL_COND_V(!p_world, Dictionary());
	ERR_FAIL_COND_V(!p_world->is_inside_tree(), Dictionary());

	Vector2 screen_from = p_world->iso_to_screen(p_iso_from);
	Vector2 screen_to = p_world->iso_to_screen(p_iso_to);

	Physics2DDirectSpaceState *space = p_world->get_world_2d()->get_direct_space_state();
	ERR_FAIL_COND_V(!space, Dictionary());

	Set<RID> exclude;
	for (int i = 0; i < p_exclude.size(); i++) {
		exclude.insert(p_exclude[i]);
	}

	Physics2DDirectSpaceState::RayResult ray_result;
	bool hit = space->intersect_ray(screen_from, screen_to, ray_result, exclude, p_collision_mask);

	Dictionary result;
	if (!hit) {
		return result;
	}

	result["position"] = ray_result.position;
	result["normal"] = ray_result.normal;
	result["collider"] = ray_result.collider;
	result["collider_id"] = ray_result.collider_id;
	result["rid"] = ray_result.rid;
	result["shape"] = ray_result.shape;

	// Add iso-converted values
	result["iso_position"] = p_world->screen_to_iso(ray_result.position, p_iso_from.z);
	result["iso_normal"] = p_world->screen_vector_to_iso(ray_result.normal);

	return result;
}

Array IsoPhysics::iso_intersect_point(Object *p_world_obj, const Vector3 &p_iso_point,
		int p_max_results, const Array &p_exclude, int p_collision_mask) {
	IsoWorld *p_world = Object::cast_to<IsoWorld>(p_world_obj);
	ERR_FAIL_COND_V(!p_world, Array());
	ERR_FAIL_COND_V(!p_world->is_inside_tree(), Array());

	Vector2 screen_point = p_world->iso_to_screen(p_iso_point);

	Physics2DDirectSpaceState *space = p_world->get_world_2d()->get_direct_space_state();
	ERR_FAIL_COND_V(!space, Array());

	Set<RID> exclude;
	for (int i = 0; i < p_exclude.size(); i++) {
		exclude.insert(p_exclude[i]);
	}

	Vector<Physics2DDirectSpaceState::ShapeResult> results;
	results.resize(p_max_results);
	int count = space->intersect_point(screen_point, results.ptrw(), p_max_results, exclude, p_collision_mask);

	Array arr;
	for (int i = 0; i < count; i++) {
		Dictionary d;
		d["collider"] = results[i].collider;
		d["collider_id"] = results[i].collider_id;
		d["rid"] = results[i].rid;
		d["shape"] = results[i].shape;
		arr.push_back(d);
	}
	return arr;
}

void IsoPhysics::_bind_methods() {
	ClassDB::bind_method(D_METHOD("iso_raycast", "world", "iso_from", "iso_to", "exclude", "collision_mask"),
			&IsoPhysics::iso_raycast, DEFVAL(Array()), DEFVAL(0x7FFFFFFF));
	ClassDB::bind_method(D_METHOD("iso_intersect_point", "world", "iso_point", "max_results", "exclude", "collision_mask"),
			&IsoPhysics::iso_intersect_point, DEFVAL(32), DEFVAL(Array()), DEFVAL(0x7FFFFFFF));
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_SUITE("[[isotools]] IsoPhysics") {
	TEST_CASE("iso_raycast returns empty dict without world") {
		IsoPhysics phys;
		Dictionary result = phys.iso_raycast(nullptr, Vector3(), Vector3(1, 0, 0));
		CHECK(result.empty());
	}

	TEST_CASE("iso_intersect_point returns empty array without world") {
		IsoPhysics phys;
		Array result = phys.iso_intersect_point(nullptr, Vector3());
		CHECK(result.empty());
	}
}

#endif // DOCTEST
