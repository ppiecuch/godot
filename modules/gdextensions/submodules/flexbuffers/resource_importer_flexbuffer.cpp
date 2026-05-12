/**************************************************************************/
/*  resource_importer_flexbuffer.cpp                                      */
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

#include "resource_importer_flexbuffer.h"

#include "core/io/file_access_pack.h"
#include "thirdparty/flatbuffers/include/flatbuffers/flexbuffers.h"

// -----------------------------------------------------------------------
// Internal helpers — not exposed in the header.
// -----------------------------------------------------------------------

static Variant flexbuffer_to_variant(flexbuffers::Reference buffer);

static void flexbuffer_variant_add(flexbuffers::Builder &fbb, Variant variant) {
	switch (variant.get_type()) {
		case Variant::Type::NIL: {
			fbb.Null();
		} break;
		case Variant::Type::BOOL: {
			fbb.Bool((bool)variant);
		} break;
		case Variant::Type::INT: {
			fbb.Int((int64_t)variant);
		} break;
		case Variant::Type::REAL: {
			fbb.Double((double)variant);
		} break;
		case Variant::Type::STRING: {
			fbb.String(String(variant).ascii().get_data());
		} break;
		case Variant::Type::VECTOR2: {
			Vector2 v = variant;
			fbb.TypedVector([&]() {
				fbb.Double(v.x);
				fbb.Double(v.y);
			});
		} break;
		case Variant::Type::VECTOR3: {
			Vector3 v = variant;
			fbb.TypedVector([&]() {
				fbb.Double(v.x);
				fbb.Double(v.y);
				fbb.Double(v.z);
			});
		} break;
		case Variant::Type::QUAT: {
			Quat q = variant;
			fbb.TypedVector([&]() {
				fbb.Double(q.x);
				fbb.Double(q.y);
				fbb.Double(q.z);
				fbb.Double(q.w);
			});
		} break;
		case Variant::Type::POOL_BYTE_ARRAY: {
			PoolByteArray bytes = variant;
			PoolByteArray::Read r = bytes.read();
			fbb.Blob(r.ptr(), bytes.size());
		} break;
		case Variant::Type::ARRAY: {
			Array array = variant;
			fbb.Vector([&]() {
				for (int i = 0; i < array.size(); ++i) {
					flexbuffer_variant_add(fbb, array[i]);
				}
			});
		} break;
		case Variant::Type::DICTIONARY: {
			Dictionary dictionary = variant;
			Array keys = dictionary.keys();
			Array values = dictionary.values();
			fbb.Map([&]() {
				for (int i = 0; i < dictionary.size(); ++i) {
					fbb.Key(String(keys[i]).ascii().get_data());
					flexbuffer_variant_add(fbb, values[i]);
				}
			});
		} break;
		default:
			break;
	}
}

static Variant flexbuffer_to_variant(flexbuffers::Reference buffer) {
	if (buffer.IsNull()) {
		return Variant();
	}
	if (buffer.IsBool()) {
		return buffer.AsBool();
	}
	if (buffer.IsInt()) {
		return buffer.AsInt64();
	}
	if (buffer.IsUInt()) {
		return (int64_t)buffer.AsUInt64();
	}
	if (buffer.IsFloat()) {
		return buffer.AsDouble();
	}
	if (buffer.IsString()) {
		return Variant(buffer.AsString().c_str());
	}
	if (buffer.IsBlob()) {
		flexbuffers::Blob blob = buffer.AsBlob();
		PoolByteArray result;
		result.resize(blob.size());
		PoolByteArray::Write w = result.write();
		memcpy(w.ptr(), blob.data(), blob.size());
		return result;
	}
	if (buffer.IsMap()) {
		Dictionary dictionary;
		flexbuffers::Map map = buffer.AsMap();
		flexbuffers::TypedVector keys = map.Keys();
		flexbuffers::Vector values = map.Values();
		for (size_t i = 0; i < keys.size(); ++i) {
			dictionary[keys[i].AsString().c_str()] = flexbuffer_to_variant(values[i]);
		}
		return dictionary;
	}
	if (buffer.IsFixedTypedVector()) {
		Array array;
		flexbuffers::FixedTypedVector vector = buffer.AsFixedTypedVector();
		for (size_t i = 0; i < vector.size(); ++i) {
			array.append(flexbuffer_to_variant(vector[i]));
		}
		return array;
	}
	if (buffer.IsTypedVector()) {
		Array array;
		flexbuffers::TypedVector vector = buffer.AsTypedVector();
		for (size_t i = 0; i < vector.size(); ++i) {
			array.append(flexbuffer_to_variant(vector[i]));
		}
		return array;
	}
	if (buffer.IsVector()) {
		Array array;
		flexbuffers::Vector vector = buffer.AsVector();
		for (size_t i = 0; i < vector.size(); ++i) {
			array.append(flexbuffer_to_variant(vector[i]));
		}
		return array;
	}
	return Variant();
}

static Vector<uint8_t> variant_to_flexbuffer(Variant variant) {
	flexbuffers::Builder fbb;
	flexbuffer_variant_add(fbb, variant);
	fbb.Finish();
	std::vector<uint8_t> std_vector = fbb.GetBuffer();
	Vector<uint8_t> godot_bytes;
	godot_bytes.resize(std_vector.size());
	memcpy(godot_bytes.ptrw(), std_vector.data(), std_vector.size());
	return godot_bytes;
}

static Variant flexbuffer_to_variant(Vector<uint8_t> p_buffer) {
	std::vector<uint8_t> std_vector(p_buffer.size());
	memcpy(std_vector.data(), p_buffer.ptr(), p_buffer.size());
	return flexbuffer_to_variant(flexbuffers::GetRoot(std_vector));
}

// -----------------------------------------------------------------------
// FlexbuffersData
// -----------------------------------------------------------------------

void FlexbuffersData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_data", "data"), &FlexbuffersData::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &FlexbuffersData::get_data);
	ClassDB::bind_method(D_METHOD("set_flexbuffers", "flexbuffers"), &FlexbuffersData::set_flexbuffers);
	ClassDB::bind_method(D_METHOD("get_flexbuffers"), &FlexbuffersData::get_flexbuffers);

	ADD_PROPERTY(PropertyInfo(Variant::NIL, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR | PROPERTY_USAGE_INTERNAL), "set_data", "get_data");
}

Variant FlexbuffersData::get_data() const {
	return data;
}

void FlexbuffersData::set_data(Variant p_data) {
	data = p_data;
}

Vector<uint8_t> FlexbuffersData::get_flexbuffers() const {
	return variant_to_flexbuffer(data);
}

void FlexbuffersData::set_flexbuffers(Vector<uint8_t> p_buffer) {
	data = flexbuffer_to_variant(p_buffer);
}

// -----------------------------------------------------------------------
// ResourceImporterFlexbuffers
// -----------------------------------------------------------------------

String ResourceImporterFlexbuffers::get_importer_name() const {
	return "FLEXBUFFERS";
}

String ResourceImporterFlexbuffers::get_visible_name() const {
	return "Flexbuffers";
}

void ResourceImporterFlexbuffers::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("flexbin");
}

String ResourceImporterFlexbuffers::get_save_extension() const {
	return "res";
}

String ResourceImporterFlexbuffers::get_resource_type() const {
	return "FlexbuffersData";
}

int ResourceImporterFlexbuffers::get_preset_count() const {
	return 0;
}

String ResourceImporterFlexbuffers::get_preset_name(int p_idx) const {
	return String();
}

void ResourceImporterFlexbuffers::get_import_options(List<ImportOption> *r_options, int p_preset) const {
}

bool ResourceImporterFlexbuffers::get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const {
	return true;
}

Error ResourceImporterFlexbuffers::import(const String &p_source_file, const String &p_save_path,
		const Map<StringName, Variant> &p_options,
		List<String> *r_platform_variants,
		List<String> *r_gen_files,
		Variant *r_metadata) {
	FileAccess *file = FileAccess::create(FileAccess::ACCESS_RESOURCES);
	ERR_FAIL_COND_V(!file, FAILED);
	Vector<uint8_t> array = file->get_file_as_array(p_source_file);
	memdelete(file);

	Ref<FlexbuffersData> flexbuffer_data;
	flexbuffer_data.instance();
	flexbuffer_data->set_flexbuffers(array);
	return ResourceSaver::save(p_save_path + ".res", flexbuffer_data);
}

// -----------------------------------------------------------------------
// Doctests
// -----------------------------------------------------------------------

#ifdef DOCTEST
#include "doctest/doctest.h"

TEST_CASE("[flexbuffers] round-trip null") {
	Variant v;
	CHECK(flexbuffer_to_variant(variant_to_flexbuffer(v)).get_type() == Variant::NIL);
}

TEST_CASE("[flexbuffers] round-trip bool") {
	CHECK((bool)flexbuffer_to_variant(variant_to_flexbuffer(Variant(true))) == true);
	CHECK((bool)flexbuffer_to_variant(variant_to_flexbuffer(Variant(false))) == false);
}

TEST_CASE("[flexbuffers] round-trip int") {
	CHECK((int64_t)flexbuffer_to_variant(variant_to_flexbuffer(Variant(42))) == 42);
	CHECK((int64_t)flexbuffer_to_variant(variant_to_flexbuffer(Variant(-1000))) == -1000);
}

TEST_CASE("[flexbuffers] round-trip float") {
	double v = 3.14159;
	double v2 = (double)flexbuffer_to_variant(variant_to_flexbuffer(Variant(v)));
	CHECK(v2 == doctest::Approx(3.14159).epsilon(0.0001));
}

TEST_CASE("[flexbuffers] round-trip String") {
	Variant v = String("hello flexbuffers");
	CHECK(String(flexbuffer_to_variant(variant_to_flexbuffer(v))) == "hello flexbuffers");
}

TEST_CASE("[flexbuffers] round-trip Dictionary") {
	Dictionary d;
	d["x"] = 10;
	d["y"] = -5;
	Variant v2 = flexbuffer_to_variant(variant_to_flexbuffer(d));
	CHECK(v2.get_type() == Variant::DICTIONARY);
	Dictionary d2 = v2;
	CHECK((int64_t)d2["x"] == 10);
	CHECK((int64_t)d2["y"] == -5);
}

TEST_CASE("[flexbuffers] round-trip heterogeneous Array") {
	Array a;
	a.push_back(Variant(1));
	a.push_back(Variant(String("two")));
	a.push_back(Variant(true));
	Variant v2 = flexbuffer_to_variant(variant_to_flexbuffer(a));
	CHECK(v2.get_type() == Variant::ARRAY);
	Array a2 = v2;
	CHECK(a2.size() == 3);
	CHECK((int64_t)a2[0] == 1);
	CHECK(String(a2[1]) == "two");
	CHECK((bool)a2[2] == true);
}

TEST_CASE("[flexbuffers] round-trip PoolByteArray blob") {
	PoolByteArray b;
	b.resize(4);
	{
		PoolByteArray::Write w = b.write();
		w[0] = 0xDE;
		w[1] = 0xAD;
		w[2] = 0xBE;
		w[3] = 0xEF;
	}
	Variant v2 = flexbuffer_to_variant(variant_to_flexbuffer(b));
	CHECK(v2.get_type() == Variant::POOL_BYTE_ARRAY);
	PoolByteArray b2 = v2;
	CHECK(b2.size() == 4);
	CHECK(b2[0] == 0xDE);
	CHECK(b2[3] == 0xEF);
}

TEST_CASE("[flexbuffers] round-trip nested Dictionary/Array") {
	Dictionary outer;
	Array items;
	items.push_back(Variant(1));
	items.push_back(Variant(2));
	outer["items"] = items;
	outer["count"] = 2;

	Variant v2 = flexbuffer_to_variant(variant_to_flexbuffer(outer));
	Dictionary d2 = v2;
	CHECK((int64_t)d2["count"] == 2);
	Array items2 = d2["items"];
	CHECK(items2.size() == 2);
	CHECK((int64_t)items2[0] == 1);
	CHECK((int64_t)items2[1] == 2);
}

TEST_CASE("[flexbuffers] FlexbuffersData set/get round-trip") {
	FlexbuffersData fd;
	Dictionary d;
	d["key"] = String("value");
	fd.set_data(d);

	Vector<uint8_t> buf = fd.get_flexbuffers();
	CHECK(buf.size() > 0);

	FlexbuffersData fd2;
	fd2.set_flexbuffers(buf);
	Dictionary d2 = fd2.get_data();
	CHECK(String(d2["key"]) == "value");
}

#endif // DOCTEST
