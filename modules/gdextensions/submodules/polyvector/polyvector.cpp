/**************************************************************************/
/*  polyvector.cpp                                                        */
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

#include "polyvector.h"

/// PolyVector

void PolyVector::draw_current_frame() {
	if (data_vec_file.is_null())
		return;
	uint16_t frameno = CLAMP((fps * frame_time), 0, frame_data.size() - 1);
#ifdef POLYVECTOR_DEBUG
	uint64_t debugtimer = os->get_ticks_usec();
#endif
	MeshDictionaryMap &meshdict = data_vec_file->get_mesh_dictionary();
	for (MeshInstanceMap::Element *m = mesh_display.front(); m; m = m->next())
		m->get()->set_visible(false);
	float layer_separation = 0;
	PolyVectorFrame *framedata = &frame_data[frameno];
	for (PolyVectorFrame::Element *c = framedata->front(); c; c = c->next()) {
		PolyVectorSymbol symbol = c->get();
		if (!meshdict.has(symbol.id)) {
			MeshQualityMap mqm;
			meshdict[symbol.id] = mqm;
		}
		if (!meshdict[symbol.id].has(curve_quality)) {
			PolyVectorCharacter *pvchar = &dictionary_data[symbol.id];
			Array arr;
			arr.resize(Mesh::ARRAY_MAX);
			PoolVector<Vector3> vertices;
			PoolVector<Vector3> normals;
			PoolVector<Color> colours;
#ifdef POLYVECTOR_DEBUG
			Array wireframearr;
			wireframearr.resize(Mesh::ARRAY_MAX);
			PoolVector<Vector2> wireframevertices;
#endif

			for (PolyVectorCharacter::Element *s = pvchar->front(); s; s = s->next()) {
				PolyVectorShape &shape = s->get();
				if (shape.fillcolour == nullptr)
					continue;
				std::vector<std::vector<Vector2>> polygons;
				std::vector<Vector2> tessverts;
				PoolVector<Vector2> tess = shape.path.curve.tessellate(curve_quality, max_tessellation_angle);
				if (!shape.path.closed) { // TODO: If shape is not a closed loop, store as a stroke
					shape.strokes[curve_quality].push_back(tess);
				} else { // Otherwise, triangulate
					uint32_t tess_size = tess.size();
					std::vector<Vector2> poly;
					PoolVector<Vector2>::Read tessreader = tess.read();
					for (uint32_t i = 1; i < tess_size; i++) {
						poly.push_back(tessreader[i]);
#ifdef POLYVECTOR_DEBUG
						if (debug_wireframe) {
							wireframevertices.push_back(tessreader[i - 1]);
							wireframevertices.push_back(tessreader[i]);
						}
#endif
					}
					polygons.push_back(poly);
					tessverts.insert(tessverts.end(), poly.begin(), poly.end());
					for (List<uint16_t>::Element *hole = shape.holes.front(); hole; hole = hole->next()) {
						PoolVector<Vector2> holetess = (*pvchar)[hole->get()].path.curve.tessellate(curve_quality, max_tessellation_angle);
						uint32_t holetess_size = holetess.size();
						std::vector<Vector2> holepoly;
						PoolVector<Vector2>::Read holetessreader = holetess.read();
						for (uint32_t j = 0; j < holetess_size; j++)
							holepoly.push_back(holetessreader[j]);
						polygons.push_back(holepoly);
						tessverts.insert(tessverts.end(), holepoly.begin(), holepoly.end());
					}
				}
				if (!polygons.empty()) {
					std::vector<N> indices = mapbox::earcut<N>(polygons);
					for (std::vector<N>::reverse_iterator tris = indices.rbegin(); tris != indices.rend(); tris++) { // Work through the vector in reverse to make sure the triangles' normals are facing forward
						colours.push_back(shape.fillcolour->to_linear());
						normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
						vertices.push_back(Vector3(tessverts[*tris].x, tessverts[*tris].y, 0));
#ifdef POLYVECTOR_DEBUG
						vertex_count++;
#endif
					}
				}
			}

			arr[Mesh::ARRAY_VERTEX] = vertices;
			arr[Mesh::ARRAY_NORMAL] = normals;
			arr[Mesh::ARRAY_COLOR] = colours;
			Ref<ArrayMesh> newmesh;
			newmesh.instance();
			newmesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arr);
			newmesh->surface_set_material(0, material_default);

#ifdef POLYVECTOR_DEBUG
			if (debug_wireframe) {
				wireframearr[Mesh::ARRAY_VERTEX] = wireframevertices;
				newmesh->add_surface_from_arrays(Mesh::PRIMITIVE_LINES, wireframearr);
				newmesh->surface_set_material(newmesh->get_surface_count() - 1, material_debug);
			}
#endif

			meshdict[symbol.id][curve_quality] = newmesh;
		}
		if (!mesh_display.has(symbol.depth)) {
			MeshInstance *mi = memnew(MeshInstance);
			add_child(mi);
			mi->set_owner(this);
			mesh_display[symbol.depth] = mi;
		}
		MeshInstance *mi = mesh_display[symbol.depth];
		mi->set_mesh(meshdict[symbol.id][curve_quality]);
		Transform transform;
		transform.set(
				symbol.matrix.ScaleX, symbol.matrix.Skew1, 0,
				symbol.matrix.Skew0, symbol.matrix.ScaleY, 0,
				0, 0, 1,
				position_offset.x + symbol.matrix.TranslateX, position_offset.y - symbol.matrix.TranslateY, layer_separation);
		mi->set_transform(transform);
		mi->set_visible(true);
		layer_separation += layer_depth;
	}
#ifdef POLYVECTOR_DEBUG
	triangulation_time = ((os->get_ticks_usec() - debugtimer) / 1000000.0);
#endif
}

void PolyVector::clear_mesh_data() {
	if (data_vec_file.is_valid()) {
		MeshDictionaryMap &meshdict = data_vec_file->get_mesh_dictionary();
		for (MeshDictionaryMap::Element *d = meshdict.front(); d; d = d->next()) {
			for (MeshQualityMap::Element *m = d->get().front(); m; m = m->next())
				m->get().unref();
			d->get().clear();
		}
	}
}

void PolyVector::clear_mesh_instances() {
	int32_t childcount = get_child_count();
	for (int32_t i = childcount - 1; i >= 0; i--) {
		MeshInstance *mi = Node::cast_to<MeshInstance>(get_child(i));
		if (mi)
			remove_child(mi);
	}
	for (MeshInstanceMap::Element *mim = mesh_display.front(); mim; mim = mim->next())
		memdelete<MeshInstance>(mim->get());
	mesh_display.clear();
}

void PolyVector::set_vector_image(const Ref<JSONVector> &p_vector) {
	if (data_vec_file.is_valid())
		data_vec_file.unref();
	data_vec_file = p_vector;
	if (data_vec_file.is_null())
		return;
	fps = data_vec_file->get_fps();
	frame_data = data_vec_file->get_frames();
	dictionary_data = data_vec_file->get_dictionary();

	clear_mesh_instances();
	clear_mesh_data();

	set_time(0);
}
Ref<JSONVector> PolyVector::get_vector_image() const {
	return data_vec_file;
}

void PolyVector::set_time(real_t p_time) {
	frame_time = p_time;
	draw_current_frame();
}
real_t PolyVector::get_time() {
	return frame_time;
}

void PolyVector::set_curve_quality(int8_t p_quality) {
	curve_quality = p_quality;
	draw_current_frame();
}
int8_t PolyVector::get_curve_quality() {
	return curve_quality;
}

void PolyVector::set_offset(const Vector2 &p_offset) {
	position_offset = p_offset;
	draw_current_frame();
}
Vector2 PolyVector::get_offset() {
	return position_offset;
}

void PolyVector::set_layer_separation(real_t p_separation) {
	layer_depth = p_separation;
	draw_current_frame();
}
real_t PolyVector::get_layer_separation() {
	return layer_depth;
}

void PolyVector::set_albedo_colour(Color c) {
	material_default->set_albedo(c);
}
Color PolyVector::get_albedo_colour() {
	return material_default->get_albedo();
}

void PolyVector::set_material_unshaded(bool t) {
	material_default->set_flag(SpatialMaterial::FLAG_UNSHADED, t);
}
bool PolyVector::get_material_unshaded() {
	return material_default->get_flag(SpatialMaterial::FLAG_UNSHADED);
}

void PolyVector::set_max_tessellation_angle(real_t f) {
	max_tessellation_angle = f;
}
real_t PolyVector::get_max_tessellation_angle() {
	return max_tessellation_angle;
}

AABB PolyVector::get_aabb() const {
	AABB aabbfull;
	aabbfull.set_position(Vector3(-0.5, -0.5, -0.5));
	aabbfull.set_size(Vector3(1, 1, 1));
	//for(MeshInstanceMap::Element *mi=mesh_display.front(); mi; mi=mi->next()) {
	//	MeshInstance *meshinstance = mi->get();
	//	aabbfull.expand_to(meshinstance->get_translation());
	//	aabbfull.expand_to(meshinstance->get_aabb().get_size()*meshinstance->get_scale());
	//}
	return aabbfull;
}

PoolVector<Face3> PolyVector::get_faces(uint32_t p_usage_flags) const {
	PoolVector<Face3> allfaces;
	for (MeshInstanceMap::Element *mi = mesh_display.front(); mi; mi = mi->next()) {
		allfaces.append_array(mi->get()->get_mesh()->get_faces());
	}
	return allfaces;
}

#ifdef POLYVECTOR_DEBUG
void PolyVector::set_debug_wireframe(bool b) {
	debug_wireframe = b;
	clear_mesh_instances();
	clear_mesh_data();
	draw_current_frame();
}
bool PolyVector::get_debug_wireframe() {
	return debug_wireframe;
}

double PolyVector::get_triangulation_time() {
	return triangulation_time;
}
double PolyVector::get_mesh_update_time() {
	return mesh_update_time;
}
uint32_t PolyVector::get_vertex_count() {
	return vertex_count;
}
#endif // POLYVECTOR_DEBUG

void PolyVector::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_vector_image"), &PolyVector::set_vector_image);
	ClassDB::bind_method(D_METHOD("get_vector_image"), &PolyVector::get_vector_image);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "vector", PROPERTY_HINT_RESOURCE_TYPE, "JSONVector"), "set_vector_image", "get_vector_image");

	ADD_GROUP("Display", "");
	ClassDB::bind_method(D_METHOD("set_time"), &PolyVector::set_time);
	ClassDB::bind_method(D_METHOD("get_time"), &PolyVector::get_time);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "timecode", PROPERTY_HINT_RANGE, "0.0, 1000.0, 0.01, 0.0"), "set_time", "get_time");

	ClassDB::bind_method(D_METHOD("set_curve_quality"), &PolyVector::set_curve_quality);
	ClassDB::bind_method(D_METHOD("get_curve_quality"), &PolyVector::get_curve_quality);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "curve_quality", PROPERTY_HINT_RANGE, "0,10,1,2"), "set_curve_quality", "get_curve_quality");

	ClassDB::bind_method(D_METHOD("set_material_unshaded"), &PolyVector::set_material_unshaded);
	ClassDB::bind_method(D_METHOD("get_material_unshaded"), &PolyVector::get_material_unshaded);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "unshaded"), "set_material_unshaded", "get_material_unshaded");

	ADD_GROUP("Adjustments", "");
	ClassDB::bind_method(D_METHOD("set_offset"), &PolyVector::set_offset);
	ClassDB::bind_method(D_METHOD("get_offset"), &PolyVector::get_offset);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "offset"), "set_offset", "get_offset");

	ClassDB::bind_method(D_METHOD("set_layer_separation"), &PolyVector::set_layer_separation);
	ClassDB::bind_method(D_METHOD("get_layer_separation"), &PolyVector::get_layer_separation);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "layer_separation", PROPERTY_HINT_RANGE, "0.0, 1.0, 0.0"), "set_layer_separation", "get_layer_separation");

	ClassDB::bind_method(D_METHOD("set_albedo_colour"), &PolyVector::set_albedo_colour);
	ClassDB::bind_method(D_METHOD("get_albedo_colour"), &PolyVector::get_albedo_colour);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "albedo_colour"), "set_albedo_colour", "get_albedo_colour");

	ADD_GROUP("Advanced", "");
	ClassDB::bind_method(D_METHOD("set_max_tessellation_angle"), &PolyVector::set_max_tessellation_angle);
	ClassDB::bind_method(D_METHOD("get_max_tessellation_angle"), &PolyVector::get_max_tessellation_angle);
	ADD_PROPERTY(PropertyInfo(Variant::REAL, "max_tessellation_angle", PROPERTY_HINT_RANGE, "1.0, 10.0, 0.1, 4.0"), "set_max_tessellation_angle", "get_max_tessellation_angle");

#ifdef POLYVECTOR_DEBUG
	ADD_GROUP("Debug", "");
	ClassDB::bind_method(D_METHOD("set_debug_wireframe"), &PolyVector::set_debug_wireframe);
	ClassDB::bind_method(D_METHOD("get_debug_wireframe"), &PolyVector::get_debug_wireframe);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_wireframe"), "set_debug_wireframe", "get_debug_wireframe");

	ClassDB::bind_method(D_METHOD("get_triangulation_time"), &PolyVector::get_triangulation_time);
	ClassDB::bind_method(D_METHOD("get_mesh_update_time"), &PolyVector::get_mesh_update_time);
	ClassDB::bind_method(D_METHOD("get_vertex_count"), &PolyVector::get_vertex_count);
#endif
}

PolyVector::PolyVector() {
	frame_time = 0;
	position_offset = Vector2(0, 0);
	curve_quality = 2;
	layer_depth = 0;
	max_tessellation_angle = 4;

	material_default.instance();
	material_default->set_flag(SpatialMaterial::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	material_default->set_cull_mode(SpatialMaterial::CULL_DISABLED);

#ifdef POLYVECTOR_DEBUG
	os = OS::get_singleton();
	material_debug.instance();
	debug_wireframe = false;
	triangulation_time = 0;
	mesh_update_time = 0;
#endif
}

PolyVector::~PolyVector() {
	clear_mesh_data();
	if (!material_default.is_null())
		material_default.unref();
	data_vec_file.unref();
}

/// PolyVector2D

#ifdef TOOLS_ENABLED
void PolyVector2D::_edit_set_position(const Point2 &p_position) {}
Point2 PolyVector2D::_edit_get_position() const {
	return Point2();
}
void PolyVector2D::_edit_set_scale(const Size2 &p_scale) {}
Size2 PolyVector2D::_edit_get_scale() const {
	return viewSize;
}
#endif

Transform2D PolyVector2D::get_transform() const {
	return Transform2D();
}

void PolyVector2D::set_vector_image(const Ref<JSONVector> &p_vector) {
	if (data_vec_file.is_valid())
		data_vec_file.unref();
	data_vec_file = p_vector;
	if (data_vec_file.is_null())
		return;
	fps = data_vec_file->get_fps();
	frame_data = data_vec_file->get_frames();
	dictionary_data = data_vec_file->get_dictionary();

	set_time(0);
}
Ref<JSONVector> PolyVector2D::get_vector_image() const {
	return data_vec_file;
}

void PolyVector2D::set_time(real_t p_time) {
	frame_time = p_time;
	// draw_current_frame();
}
real_t PolyVector2D::get_time() {
	return frame_time;
}

void PolyVector2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PARENTED:
		case NOTIFICATION_ENTER_TREE: {
		} break;
		case NOTIFICATION_PROCESS: {
			update();
		} break;
		case NOTIFICATION_DRAW: {
		} break;
	}
}

void PolyVector2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_vector_image"), &PolyVector::set_vector_image);
	ClassDB::bind_method(D_METHOD("get_vector_image"), &PolyVector::get_vector_image);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "vector", PROPERTY_HINT_RESOURCE_TYPE, "JSONVector"), "set_vector_image", "get_vector_image");
}

PolyVector2D::PolyVector2D() {
	viewSize = Size2(100, 100);
}
