/*************************************************************************/
/*  resource_importer_swf.cpp                                            */
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

#include "core/io/resource_saver.h"
#include "core/os/file_access.h"
#include "core/os/os.h"

#include "resource_importer_swf.h"

#ifdef TOOLS_ENABLED
static String _dump(const SWF::ShapeList &data) {
	String ret;
	for(const auto &s : data) {
		ret += vformat("(%s,%d", s.closed ? "c" : "o", uint64_t(s.vertices.size()));
		for (auto v : s.vertices) {
			ret += vformat(",{%f,%f}", v.anchor.x, v.anchor.y);
		}
		ret += ")\n";
	}
	return ret;
}

static String _dump(const SWF::Character &data) {
	return "[Character:"
		+ vformat(" bounds {%f,%f,%f,%f}", data.bounds.xmin, data.bounds.ymin, data.bounds.xmax, data.bounds.ymax)
		+ vformat(" shapes {%s}", _dump(data.shapes))
		+ "]";
}

#define JV(V) (bool(p_options["binary"]) ? (V) : (Math::round((V) * 100) / 100.0))

Error ResourceImporterSWF::import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	FileAccessRef swf = FileAccess::open(p_source_file, FileAccess::READ);
	ERR_FAIL_COND_V(!swf, ERR_FILE_CANT_READ);

	size_t xmllen = swf->get_len();
	ERR_FAIL_COND_V(!xmllen, ERR_CANT_OPEN);
	{
		uint8_t *swfdata = new uint8_t[xmllen];
		swf->get_buffer(swfdata, xmllen);
		ERR_FAIL_COND_V(!swfdata, ERR_INVALID_DATA);
		SWF::Parser *swfparser = new SWF::Parser();
		SWF::Error error = swfparser->parse_swf_data(swfdata, xmllen);
		switch (error) {
			case SWF::Error::OK:
				break;
			case SWF::Error::SWF_NULL_DATA:
				OS::get_singleton()->alert(String(p_source_file.ascii().get_data()) + " contains null data somehow?");
				return Error::ERR_CANT_OPEN;
			case SWF::Error::ZLIB_NOT_COMPILED:
				OS::get_singleton()->alert("zlib compression was not compiled into libshockwave.");
				return Error::ERR_CANT_OPEN;
			case SWF::Error::LZMA_NOT_COMPILED:
				OS::get_singleton()->alert("LZMA compression was not compiled into libshockwave.");
				return Error::ERR_CANT_OPEN;
			case SWF::Error::SWF_FILE_ENCRYPTED:
				OS::get_singleton()->alert(String("Password is required to decrypt %s") + p_source_file.ascii().get_data());
				return Error::ERR_FILE_NO_PERMISSION;
			default:
				OS::get_singleton()->alert(vformat("Error when opening file: code %d", error));
				return Error::ERR_CANT_OPEN;
		}

		print_verbose(">> " + p_source_file.get_basename() + " properties:");
		print_verbose(vformat("  dimensions: %dx%d", swfparser->get_properties()->dimensions.xmax,swfparser->get_properties()->dimensions.ymax));
		print_verbose(vformat("  framerate: %d", swfparser->get_properties()->framerate));
		print_verbose(vformat("  framecount: %d", swfparser->get_properties()->framecount));

		print_verbose(">> " + p_source_file.get_basename() + " parse results:");
		print_verbose(vformat("  FillStyles: %d", uint64_t(swfparser->get_dict()->FillStyles.size())));
		print_verbose(vformat("  LineStyles: %d", uint64_t(swfparser->get_dict()->LineStyles.size())));
		print_verbose(vformat("  Frames: %d", uint64_t(swfparser->get_dict()->Frames.size())));

		const real_t sc = real_t(p_options["scale"]) / 1000.0;
		SWF::Dictionary *dict = swfparser->get_dict();
		json root;
		std::map<uint16_t, uint16_t> fillstylemap, linestylemap, charactermap;
		{ // Build the library definitions first
			for (auto fsm = dict->FillStyles.begin(); fsm != dict->FillStyles.end(); fsm++) {
				json fillstylearray;
				for (SWF::FillStyleArray::iterator fs = fsm->second.begin(); fs != fsm->second.end(); fs++) {
					SWF::FillStyle fillstyle = *fs;
					fillstylemap[fs - fsm->second.begin() + 1] = fillstylemap.size() + 1;
					json fillstyledef;
					if (fillstyle.StyleType == SWF::FillStyle::Type::SOLID) {
						fillstyledef[PV_JSON_NAME_COLOUR] += fillstyle.Color.r;
						fillstyledef[PV_JSON_NAME_COLOUR] += fillstyle.Color.g;
						fillstyledef[PV_JSON_NAME_COLOUR] += fillstyle.Color.b;
						if (fillstyle.Color.a < 255)
							fillstyledef[PV_JSON_NAME_COLOUR] += fillstyle.Color.a;
					} else { // Placeholder for unsupported fill types
						fillstyledef[PV_JSON_NAME_COLOUR] += 255;
						fillstyledef[PV_JSON_NAME_COLOUR] += 0;
						fillstyledef[PV_JSON_NAME_COLOUR] += 255;
					}
					fillstylearray += fillstyledef;
				}
				root[PV_JSON_NAME_LIBRARY][PV_JSON_NAME_FILLSTYLES] += fillstylearray;
			}
			for(auto lsm=dict->LineStyles.begin(); lsm!=dict->LineStyles.end(); lsm++) {
				json linestylearray;
				for(SWF::LineStyleArray::iterator ls=lsm->second.begin(); ls!=lsm->second.end(); ls++) {
					SWF::LineStyle linestyle = *ls;
					if(linestyle.Width > 0) {
						linestylemap[ls-lsm->second.begin()+1] = linestylemap.size()+1;
						json linestyledef;
						linestyledef[PV_JSON_NAME_LINEWIDTH] = JV(linestyle.Width * sc);
						linestyledef[PV_JSON_NAME_COLOUR] += linestyle.Color.r;
						linestyledef[PV_JSON_NAME_COLOUR] += linestyle.Color.g;
						linestyledef[PV_JSON_NAME_COLOUR] += linestyle.Color.b;
						if(linestyle.Color.a < 255)
							linestyledef[PV_JSON_NAME_COLOUR] += linestyle.Color.a;
						linestylearray += linestyledef;
					}
				}
				root[PV_JSON_NAME_LIBRARY][PV_JSON_NAME_LINESTYLES] += linestylearray;
			}
			for (auto cd = dict->CharacterList.begin(); cd != dict->CharacterList.end(); cd++) {
				const uint16_t characterid = cd->first;
				const SWF::Character character = cd->second;
				if (characterid > 0) {
					charactermap[characterid] = charactermap.size() + 1;
					json characterdef;
					SWFPolygonList shapes = shape_builder(character.shapes); // Merge shapes from Flash into solid objects and calculate hole placement and fill rules
					for (auto shape = shapes.begin(); shape != shapes.end(); shape++) {
						json shapeout;
						if (shape->polygon.layer > 0)
							shapeout[PV_JSON_NAME_LAYER] = shape->polygon.layer;
						shapeout[PV_JSON_NAME_FILL] = shape->fill;
						shapeout[PV_JSON_NAME_STROKE] = shape->stroke;
						shapeout[PV_JSON_NAME_CLOSED] = shape->polygon.closed;
						if (shape->area < 0)
							points_reverse(&shape->polygon);
						for (auto v = shape->polygon.vertices.begin(); v != shape->polygon.vertices.end(); v++) {
							if (v != shape->polygon.vertices.begin()) {
								shapeout[PV_JSON_NAME_VERTICES] += JV(v->control.x * sc);
								shapeout[PV_JSON_NAME_VERTICES] += JV(v->control.y * sc);
							}
							if (shapeout[PV_JSON_NAME_CLOSED] && v == (shape->polygon.vertices.end() - 1))
								break;
							shapeout[PV_JSON_NAME_VERTICES] += JV(v->anchor.x);
							shapeout[PV_JSON_NAME_VERTICES] += JV(v->anchor.y);
						}
						for (auto h = shape->children.begin(); h != shape->children.end(); h++) {
							shapeout[PV_JSON_NAME_HOLES] += (*h);
						}
						characterdef += shapeout;
					}
					root[PV_JSON_NAME_LIBRARY][PV_JSON_NAME_CHARACTERS] += characterdef;
				}
			}
		}
		for (auto f = dict->Frames.begin(); f != dict->Frames.end(); f++) {
			SWF::DisplayList framedisplist = *f;
			json jdisplaylist;
			for (auto dl = framedisplist.begin(); dl != framedisplist.end(); dl++) {
				SWF::DisplayChar &displaychar = dl->second;
				if (displaychar.id > 0) {
					json charout;
					charout[PV_JSON_NAME_ID] = charactermap[displaychar.id - 1];
					charout[PV_JSON_NAME_DEPTH] = dl->first;
					charout[PV_JSON_NAME_TRANSFORM] += JV(displaychar.transform.TranslateX * sc);
					charout[PV_JSON_NAME_TRANSFORM] += JV(displaychar.transform.TranslateY * sc);
					if ((Math::round(displaychar.transform.ScaleX * 100) != 100 || Math::round(displaychar.transform.ScaleY * 100) != 100) || // Only add the scale and rotate transformation values if they are different from the default
							(Math::round(displaychar.transform.RotateSkew0 * 100) != 0 || Math::round(displaychar.transform.RotateSkew1 * 100) != 0)) { // If the rotation value is different but scale is not, store the scale value anyway
						charout[PV_JSON_NAME_TRANSFORM] += JV(displaychar.transform.ScaleX);
						charout[PV_JSON_NAME_TRANSFORM] += JV(displaychar.transform.ScaleY);
						if (Math::round(displaychar.transform.RotateSkew0 * 100) != 0 || Math::round(displaychar.transform.RotateSkew1 * 100) != 0) {
							charout[PV_JSON_NAME_TRANSFORM] += JV(-displaychar.transform.RotateSkew0 * sc);
							charout[PV_JSON_NAME_TRANSFORM] += JV(-displaychar.transform.RotateSkew1 * sc);
						}
					}
					if (displaychar.colourtransform.IsModified()) {
						json cxformentry;
						cxformentry += (displaychar.colourtransform.RedAddTerm / 256.0);
						cxformentry += displaychar.colourtransform.RedMultTerm;
						cxformentry += (displaychar.colourtransform.GreenAddTerm / 256.0);
						cxformentry += displaychar.colourtransform.GreenMultTerm;
						cxformentry += (displaychar.colourtransform.BlueAddTerm / 256.0);
						cxformentry += displaychar.colourtransform.BlueMultTerm;
						if (displaychar.colourtransform.AlphaAddTerm != 0 || displaychar.colourtransform.AlphaMultTerm != 1) {
							cxformentry += (displaychar.colourtransform.AlphaAddTerm / 256.0);
							cxformentry += displaychar.colourtransform.AlphaMultTerm;
						}
						charout[PV_JSON_NAME_CXFORM] = cxformentry;
					}
					jdisplaylist += charout;
				}
			}
			root[PV_JSON_NAME_FRAMES] += jdisplaylist;
		}
		root[PV_JSON_NAME_FPS] = JV(swfparser->get_properties()->framerate);
		root[PV_JSON_NAME_DIMS].push_back(int(swfparser->get_properties()->dimensions.xmax));
		root[PV_JSON_NAME_DIMS].push_back(int(swfparser->get_properties()->dimensions.ymax));

		FileAccess *pvimport = FileAccess::open(p_save_path + "." + JSONVEC_EXT, FileAccess::WRITE);
		ERR_FAIL_COND_V(!pvimport, ERR_FILE_CANT_WRITE);
		if (bool(p_options["binary"])) {
			std::vector<uint8_t> jsonout = json::to_msgpack(root);
			pvimport->store_buffer(jsonout.data(), jsonout.size());
		} else {
			std::string out = root.dump(2);
			pvimport->store_buffer((const uint8_t *)out.c_str(), out.size());
		}
		pvimport->close();
		memdelete(pvimport);

		if (swfparser)
			delete swfparser;
		if (swfdata)
			delete[] swfdata;
	}
	swf->close();
	return OK;
}

void ResourceImporterSWF::get_import_options(List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "binary"), false));
	r_options->push_back(ImportOption(PropertyInfo(Variant::REAL, "scale"), 1));
}

ResourceImporterSWF::SWFPolygonList ResourceImporterSWF::shape_builder(SWF::ShapeList sl) {
	SWFPolygonList shapeparts;
	shapeparts.reserve(sl.size());

	for (const auto &s : sl) {
		SWFPolygon sp;
		sp.polygon = s;
		if (s.closed) {
			sp.area = shape_area(sp.polygon);
			if (!shape_area_too_small(sp.area)) {
				if (sp.area < 0)
					sp.fill = sp.polygon.fill0; // Use left fill if the shape is wound counter-clockwise
				else if (sp.area > 0)
					sp.fill = sp.polygon.fill1; // Use right fill if the shape is wound clockwise
				else
					continue;
			}
		}
		sp.stroke = sp.polygon.stroke;
		shapeparts.push_back(sp);
	}

	std::sort(shapeparts.begin(), shapeparts.end(), // Sort from smallest to largest
		[](const SWFPolygon &a, const SWFPolygon &b) { return Math::abs(a.area) < Math::abs(b.area); });

	std::set<SWFPolygonList::iterator> discardedpolygons;
	for (auto outer = shapeparts.begin(); outer != shapeparts.end(); outer++) { // For every closed polygon...
		for (auto inner = shapeparts.begin(); inner != shapeparts.end(); inner++) { // ...find the child polygons (i.e. holes)...
			if (outer == inner || !inner->polygon.closed ||
				discardedpolygons.find(inner) != discardedpolygons.end() ||
				!shape_contains_point(inner->polygon.vertices.front().anchor, outer->polygon))
				continue;
			outer->children.push_back(inner - shapeparts.begin());
			inner->has_parent = true;
			discardedpolygons.insert(inner);
		}
	}

	return shapeparts;
}

bool ResourceImporterSWF::shape_contains_point(SWF::Point innervertex, SWF::Shape outershape) {
	uint16_t outervertexcount = outershape.vertices.size();
	SWF::Vertex *outervertices = &outershape.vertices[0];
	bool contained = false;
	for (uint16_t outeredge = 1; outeredge < outervertexcount; outeredge++) {
		if ((outervertices[outeredge].anchor.y > innervertex.y) != (outervertices[outeredge - 1].anchor.y > innervertex.y) &&
				innervertex.x < (outervertices[outeredge - 1].anchor.x - outervertices[outeredge].anchor.x) * (innervertex.y - outervertices[outeredge].anchor.y) / (outervertices[outeredge - 1].anchor.y - outervertices[outeredge].anchor.y) + outervertices[outeredge].anchor.x)
			contained = !contained;
	}
	return contained;
}

inline bool ResourceImporterSWF::points_equal(SWF::Vertex &a, SWF::Vertex &b) {
	return (
		int32_t(Math::round(a.anchor.x * 20)) == int32_t(Math::round(b.anchor.x * 20)) &&
		int32_t(Math::round(a.anchor.y * 20)) == int32_t(Math::round(b.anchor.y * 20))
	);
}

inline void ResourceImporterSWF::points_reverse(SWF::Shape *s) {
	std::vector<SWF::Vertex> reverseverts;
	SWF::Point controlcache;
	for (std::vector<SWF::Vertex>::reverse_iterator v = s->vertices.rbegin(); v != s->vertices.rend(); v++) {
		SWF::Point newctrl = controlcache;
		controlcache = v->control;
		v->control = newctrl;
		reverseverts.push_back(*v);
	}
	s->vertices = reverseverts;
	std::swap(s->fill0, s->fill1);
}

inline real_t ResourceImporterSWF::shape_area(SWF::Shape s) {
	if (s.vertices.size() < 3)
		return 0;
	if (!s.closed)
		s.vertices.push_back(s.vertices.front());
	size_t vsize = s.vertices.size();
	SWF::Vertex *varray = &s.vertices[0];
	real_t area = 0;
	for (int i = 1; i < vsize; i++) {
		area += ((varray[i - 1].anchor.x * varray[i].anchor.y) - (varray[i].anchor.x * varray[i - 1].anchor.y));
	}
	return (area / 2);
}
#endif // TOOLS_ENABLED

RES ResourceLoaderJSONVector::load(const String &p_path, const String &p_original_path, Error *r_error) {
	if (r_error)
		*r_error = ERR_FILE_CANT_OPEN;
	FileAccess *polyvector = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V(!polyvector, RES());
	size_t jsonlength = polyvector->get_len();
	uint8_t *jsonstring = new uint8_t[jsonlength];
	polyvector->get_buffer(jsonstring, jsonlength);
	polyvector->close();
	memdelete(polyvector);

	json jsondata;
	try {
		std::vector<uint8_t> msgpak(jsonstring, jsonstring + jsonlength);
		jsondata = json::from_msgpack(msgpak);
	} catch (const json::parse_error &) {
		try { // If the data could not be parsed as MessagePack-encoded JSON, it's probably plain text
			jsondata = json::parse(jsonstring, jsonstring + jsonlength);
		} catch (const json::parse_error &e) {
			OS::get_singleton()->alert(String("JSON error: ") + e.what(), "JSON Error");
			if (r_error)
				*r_error = ERR_PARSE_ERROR;
			return RES();
		}
	}
	delete[] jsonstring;

	Ref<JSONVector> vectordata;
	vectordata.instance();

	// Load library data
	json jchardict = jsondata[PV_JSON_NAME_LIBRARY][PV_JSON_NAME_CHARACTERS];
	for (auto jci = jchardict.begin(); jci != jchardict.end(); jci++) {
		json jchar = *jci;
		uint16_t characterid = jci - jchardict.begin();
		PolyVectorCharacter pvchar;
		for (auto jsi = jchar.begin(); jsi != jchar.end(); jsi++) {
			json jshape = *jsi;
			uint16_t jshapefill = jshape[PV_JSON_NAME_FILL];
			PolyVectorShape pvshape;
			if (!jshape[PV_JSON_NAME_LAYER].is_null())
				pvshape.layer = jshape[PV_JSON_NAME_LAYER];
			if (jshapefill > 0) {
				json jcolour = jsondata[PV_JSON_NAME_LIBRARY][PV_JSON_NAME_FILLSTYLES][characterid][jshapefill - 1][PV_JSON_NAME_COLOUR];
				pvshape.fillcolour = new Color(int(jcolour[0]) / 255.0, int(jcolour[1]) / 255.0, int(jcolour[2]) / 255.0);
				if (jcolour.size() > 3) {
					pvshape.fillcolour->a = int(jcolour[3]) / 255.0;
				}
			}
			uint16_t jshapestroke = jshape[PV_JSON_NAME_STROKE];
			if(jshapestroke>0) {
				json jcolour = jsondata[PV_JSON_NAME_LIBRARY][PV_JSON_NAME_LINESTYLES][characterid][jshapestroke-1][PV_JSON_NAME_COLOUR];
				pvshape.strokecolour = new Color(int(jcolour[0])/255.0, int(jcolour[1])/255.0, int(jcolour[2])/255.0);
				if(jcolour.size() > 3) {
					pvshape.strokecolour->a = int(jcolour[3])/255.0;
				}
			}
			PolyVectorPath pvpath = verts_to_curve(jshape[PV_JSON_NAME_VERTICES]);
			pvpath.closed = jshape[PV_JSON_NAME_CLOSED];
			pvshape.path = pvpath;
			for (auto jhv = jshape[PV_JSON_NAME_HOLES].begin(); jhv != jshape[PV_JSON_NAME_HOLES].end(); jhv++) {
				pvshape.holes.push_back(*jhv);
			}
			pvchar.push_back(pvshape);
		}
		vectordata->add_character(pvchar);
	}

	// Load frame data
	for (auto jfi = jsondata[PV_JSON_NAME_FRAMES].begin(); jfi != jsondata[PV_JSON_NAME_FRAMES].end(); jfi++) {
		json jdisplaylist = *jfi;
		PolyVectorFrame frame;
		for (auto jdli = jdisplaylist.begin(); jdli != jdisplaylist.end(); jdli++) {
			json jdisplayitem = *jdli;
			PolyVectorSymbol pvom;
			pvom.depth = jdisplayitem[PV_JSON_NAME_DEPTH];
			pvom.id = jdisplayitem[PV_JSON_NAME_ID];
			if (jdisplayitem[PV_JSON_NAME_TRANSFORM].size() >= 2) {
				pvom.matrix.TranslateX = jdisplayitem[PV_JSON_NAME_TRANSFORM][0];
				pvom.matrix.TranslateY = jdisplayitem[PV_JSON_NAME_TRANSFORM][1];
			}
			if (jdisplayitem[PV_JSON_NAME_TRANSFORM].size() >= 4) {
				pvom.matrix.ScaleX = jdisplayitem[PV_JSON_NAME_TRANSFORM][2];
				pvom.matrix.ScaleY = jdisplayitem[PV_JSON_NAME_TRANSFORM][3];
			}
			if (jdisplayitem[PV_JSON_NAME_TRANSFORM].size() >= 6) {
				pvom.matrix.Skew0 = jdisplayitem[PV_JSON_NAME_TRANSFORM][4];
				pvom.matrix.Skew1 = jdisplayitem[PV_JSON_NAME_TRANSFORM][5];
			}
			json cxformarray = jdisplayitem[PV_JSON_NAME_CXFORM];
			if (cxformarray.size() > 0) {
				pvom.tint = new PolyVectorColourTransform();
				pvom.tint->RedAdd = cxformarray[0];
				pvom.tint->RedMultiplier = cxformarray[1];
				pvom.tint->GreenAdd = cxformarray[2];
				pvom.tint->GreenMultiplier = cxformarray[3];
				pvom.tint->BlueAdd = cxformarray[4];
				pvom.tint->BlueMultiplier = cxformarray[5];
				if (cxformarray.size() > 6) {
					pvom.tint->AlphaAdd = cxformarray[6];
					pvom.tint->AlphaMultiplier = cxformarray[7];
				}
			}
			frame.push_back(pvom);
		}
		vectordata->add_frame(frame);
	}

	vectordata->set_fps(jsondata[PV_JSON_NAME_FPS]);
	vectordata->set_dimensions(Vector2(jsondata[PV_JSON_NAME_DIMS][0], jsondata[PV_JSON_NAME_DIMS][1]));

	if (r_error)
		*r_error = OK;

	return vectordata;
}

PolyVectorPath ResourceLoaderJSONVector::verts_to_curve(json jverts) {
	PolyVectorPath pvpath;
	if (jverts.size() > 2) {
		Vector2 inctrldelta, outctrldelta, quadcontrol;
		Vector2 anchor(jverts[0], jverts[1]);
		Vector2 firstanchor = anchor;
		for (auto jvi = jverts.begin() + 2; jvi != jverts.end(); jvi++) {
			real_t vert = *jvi;
			switch (((jvi - jverts.begin() + 2) % 4)) {
				case 0: {
					quadcontrol.x = vert;
				} break;
				case 1: {
					quadcontrol.y = vert;
					outctrldelta = (quadcontrol - anchor) * (2.0 / 3.0);
					pvpath.curve.add_point(anchor, inctrldelta, outctrldelta);
				} break;
				case 2: {
					anchor.x = vert;
				} break;
				case 3: {
					anchor.y = vert;
					inctrldelta = (quadcontrol - anchor) * (2.0 / 3.0);
				} break;
			}
		}
		if (pvpath.closed) {
			inctrldelta = (quadcontrol - firstanchor) * (2.0 / 3.0);
			pvpath.curve.add_point(firstanchor, inctrldelta, Vector2(0, 0));
		}
	}
	return pvpath;
}
