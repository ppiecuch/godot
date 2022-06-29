/*************************************************************************/
/*  resource_importer_swf.h                                              */
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

#ifndef RESOURCE_IMPORTER_SWF_H
#define RESOURCE_IMPORTER_SWF_H

#include "core/io/resource_importer.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "scene/3d/mesh_instance.h"
#include "scene/resources/curve.h"

#include <map>
#include <set>
#include <vector>

#include "libshockwave/swfparser.h"

using N = uint32_t;
#include "json/json.h"
using json = nlohmann::json;

#define RISWF_SHAPE_AREA_THRESHOLD 0.1

struct PolyVectorMatrix {
	float TranslateX = 0;
	float TranslateY = 0;
	float ScaleX = 1;
	float ScaleY = 1;
	float Skew0 = 0;
	float Skew1 = 0;
};
struct PolyVectorColourTransform {
	float RedAdd = 0;
	float GreenAdd = 0;
	float BlueAdd = 0;
	float AlphaAdd = 0;
	float RedMultiplier = 1;
	float GreenMultiplier = 1;
	float BlueMultiplier = 1;
	float AlphaMultiplier = 1;
};
struct PolyVectorPath {
	bool closed;
	Curve2D curve;
	PolyVectorPath() {}
	PolyVectorPath(const PolyVectorPath &in) {
		*this = in;
	}
	void operator=(const PolyVectorPath &in) {
		closed = in.closed;
		curve.clear_points();
		for (uint16_t pt = 0; pt < in.curve.get_point_count(); pt++)
			curve.add_point(in.curve.get_point_position(pt), in.curve.get_point_in(pt), in.curve.get_point_out(pt));
	}
};
struct PolyVectorShape {
	~PolyVectorShape() {
		if (fillcolour == NULL)
			delete fillcolour;
		if (strokecolour == NULL)
			delete strokecolour;
	}
	uint8_t layer;
	PolyVectorPath path;
	List<uint16_t> holes;
	Color *fillcolour = NULL;
	Color *strokecolour = NULL;

	Map<uint16_t, List<PoolVector2Array>> strokes;
};
typedef List<PolyVectorShape> PolyVectorCharacter;

struct PolyVectorSymbol {
	~PolyVectorSymbol() {
		if (tint == NULL)
			delete tint;
	}
	uint16_t id = 0;
	uint16_t depth = 0;
	PolyVectorMatrix matrix;
	PolyVectorColourTransform *tint = NULL;
};
typedef List<PolyVectorSymbol> PolyVectorFrame;

typedef Map<uint16_t, Ref<ArrayMesh>> MeshQualityMap;
typedef Map<uint16_t, MeshQualityMap> MeshDictionaryMap;
typedef Map<uint16_t, MeshInstance *> MeshInstanceMap;

#define JSONVEC_EXT "vec.json"

#ifdef TOOLS_ENABLED
class ResourceImporterSWF : public ResourceImporter {
	GDCLASS(ResourceImporterSWF, ResourceImporter)

	struct SWFPolygon;
	typedef std::vector<SWFPolygon> SWFPolygonList;
	struct SWFPolygon {
		SWF::Shape polygon;
		real_t area = 0;
		uint16_t fill;
		uint16_t stroke;
		bool has_parent = false;
		std::list<uint16_t> children;
	};
	SWFPolygonList shape_builder(SWF::ShapeList);
	bool shape_contains_point(SWF::Point, SWF::Shape);
	_FORCE_INLINE_ bool points_equal(SWF::Vertex &, SWF::Vertex &);
	_FORCE_INLINE_ void points_reverse(SWF::Shape *);
	_FORCE_INLINE_ bool shape_area_too_small(real_t a) { return (abs(a) < RISWF_SHAPE_AREA_THRESHOLD); }
	real_t shape_area(SWF::Shape);
	real_t shape_area(SWF::ShapeList::iterator i) { return this->shape_area(*i); }

public:
	virtual String get_importer_name() const { return "JSONVector"; }
	virtual String get_visible_name() const { return "PolyVector"; }
	virtual void get_recognized_extensions(List<String> *p_extensions) const { p_extensions->push_back("swf"); }
	virtual String get_save_extension() const { return JSONVEC_EXT; }
	virtual String get_resource_type() const { return "JSONVector"; }
	virtual bool get_option_visibility(const String &, const Map<StringName, Variant> &) const { return true; }
	virtual int get_preset_count() const { return 0; }
	virtual String get_preset_name(int p_idx) const { return String(); }
	virtual void get_import_options(List<ImportOption> *r_options, int p_preset = 0) const;

	virtual Error import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files = NULL, Variant *r_metadata = NULL);

	ResourceImporterSWF() {}
};
#endif

class ResourceLoaderJSONVector : public ResourceFormatLoader {
	PolyVectorPath verts_to_curve(json);

public:
	virtual RES load(const String &p_path, const String &p_original_path = "", Error *r_error = NULL);
	virtual void get_recognized_extensions(List<String> *p_extensions) const { p_extensions->push_back(JSONVEC_EXT); }
	virtual String get_resource_type(const String &p_path) const {
		if (p_path.get_extension().to_lower() == JSONVEC_EXT)
			return "JSONVector";
		return "";
	}
	virtual bool handles_type(const String &p_type) const { return (p_type == "JSONVector"); }
};

class JSONVector : public Resource {
	GDCLASS(JSONVector, Resource);
	OBJ_SAVE_TYPE(JSONVector);
	RES_BASE_EXTENSION(JSONVEC_EXT);

	real_t fps;
	Vector2 dimensions;
	List<PolyVectorCharacter> dictionary;
	List<PolyVectorFrame> frames;

	MeshDictionaryMap mesh_dictionary;

public:
	void add_character(PolyVectorCharacter p_data) { dictionary.push_back(p_data); }
	PolyVectorCharacter get_character(uint16_t i) { return dictionary[i]; }
	List<PolyVectorCharacter> get_dictionary() { return dictionary; }

	void add_frame(PolyVectorFrame p_data) { frames.push_back(p_data); }
	PolyVectorFrame get_frame(uint16_t i) { return frames[i]; }
	List<PolyVectorFrame> get_frames() { return frames; }

	void set_fps(real_t f) { fps = f; }
	real_t get_fps() { return fps; }
	void set_dimensions(const Vector2 &dim) { dimensions = dim; }
	Vector2 get_dimensions() { return dimensions; }

	MeshDictionaryMap &get_mesh_dictionary() { return mesh_dictionary; }

	JSONVector() {}
};

#define PV_JSON_NAME_FPS "fps"
#define PV_JSON_NAME_DIMS "dim"
#define PV_JSON_NAME_LIBRARY "lib"
#define PV_JSON_NAME_CHARACTERS "chr"
#define PV_JSON_NAME_LAYER "lyr"
#define PV_JSON_NAME_FILL "fil"
#define PV_JSON_NAME_STROKE "stk"
#define PV_JSON_NAME_CLOSED "clo"
#define PV_JSON_NAME_VERTICES "ver"
#define PV_JSON_NAME_HOLES "hol"
#define PV_JSON_NAME_FILLSTYLES "fis"
#define PV_JSON_NAME_COLOUR "col"
#define PV_JSON_NAME_LINESTYLES "lis"
#define PV_JSON_NAME_LINEWIDTH "liw"
#define PV_JSON_NAME_FRAMES "frm"
#define PV_JSON_NAME_ID "id"
#define PV_JSON_NAME_DEPTH "dep"
#define PV_JSON_NAME_TRANSFORM "xf"
#define PV_JSON_NAME_CXFORM "cx"

#endif // RESOURCE_IMPORTER_SWF_H
