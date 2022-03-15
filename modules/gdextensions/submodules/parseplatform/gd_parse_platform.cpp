/*************************************************************************/
/*  gd_parse_platform.cpp                                                */
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

#include "gd_parse_platform.h"

#include "core/bind/core_bind.h"
#include "core/io/json.h"
#include "core/project_settings.h"
#include "core/string_buffer.h"
#include "misc/cqueue.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#define LOGI(string, ...) print_line(vformat(String("(Parse Info) ") + string, ##__VA_ARGS__));
#define LOGD(string, ...) print_line(vformat(String("(Parse Debug) ") + string, ##__VA_ARGS__));

#define referr(domain, code, error) Ref<GdParseError>(memnew(GdParseError(domain, code, error)))

#define REQUESTS_QUEUE "user://request_queue.data"

struct RequestQueue {
	enum RequestType {
		GET_OBJECT_REQUEST,
		ERASE_OBJECT_REQUEST,
		QUERY_REQUEST
	};
	String query;
	String payload;
	Object *receiver;
	String slot;
	RequestType req_type;
	static RequestQueue *get_object_req(const String &p_object_id, Object *p_receiver, const String &p_slot) {
		RequestQueue *e = memnew(RequestQueue);
		e->query = p_object_id;
		e->receiver = p_receiver;
		e->slot = p_slot;
		e->req_type = GET_OBJECT_REQUEST;
		return e;
	}
};

void GdParseError::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_err_domain"), &GdParseError::get_err_domain);
	ClassDB::bind_method(D_METHOD("get_err_code"), &GdParseError::get_err_code);
	ClassDB::bind_method(D_METHOD("get_err_description"), &GdParseError::get_err_description);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "domain"), "", "get_err_domain");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "code"), "", "get_err_code");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "description"), "", "get_err_description");

	BIND_ENUM_CONSTANT(DomainUndefined);
	BIND_ENUM_CONSTANT(DomainParsePlatform);
	BIND_ENUM_CONSTANT(DomainJson);
	BIND_ENUM_CONSTANT(DomainGodot);
	BIND_ENUM_CONSTANT(DomainNetwork);
	BIND_ENUM_CONSTANT(DomainParseData);

	BIND_ENUM_CONSTANT(ParsePlatformCodeInternalServerError);
	BIND_ENUM_CONSTANT(ParsePlatformCodeConnectionFailed);
	BIND_ENUM_CONSTANT(ParsePlatformCodeObjectNotFound);

	BIND_ENUM_CONSTANT(JsonCodeFailed);

	BIND_ENUM_CONSTANT(ParseDataCodeInternal);
	BIND_ENUM_CONSTANT(ParseDataCodeNotInitialized);
	BIND_ENUM_CONSTANT(ParseDataCodeInvalidType);
}

GdParseError::GdParseError(Domain p_domain, int p_code, const String &p_description) :
		domain(p_domain), code(p_code), description(p_description) {
}

/// GdParseQuery

Variant GdParseQuery::constraints() {
	StringBuffer<> buffer;

	if (!where.empty()) {
		String json = JSON::print(where);
		if (json.empty()) {
			return Variant();
		}
		buffer.append("where=");
		buffer.append(json.percent_encode());
	}

	if (!order.empty()) {
		if (buffer.length()) {
			buffer.append("&");
		}
		buffer.append("order=");
		for (int e = 0; e < order.size(); e++) {
			Dictionary entry = order[e];
			const int direction = entry["order"];
			const String key = entry["key"];
			if (direction == DescendingOrder) {
				buffer.append("-");
			}
			buffer.append(key.percent_encode());
		}
	}

	if (limit > -1) {
		if (buffer.length()) {
			buffer.append("&");
		}
		buffer.append("limit=");
		buffer.append(String::num(limit));
	}

	if (skip > 0) {
		if (buffer.length()) {
			buffer.append("&");
		}
		buffer.append("skip=");
		buffer.append(String::num(skip));
	}

	return buffer.as_string();
}

void GdParseQuery::get_object_by_id(const String &p_object_id) {
	ERR_FAIL_COND_MSG(busy, "Operation already in progress.");
	ERR_FAIL_COND(class_name.empty());
	ERR_FAIL_COND(p_object_id.empty());

	set_busy(true);

	Ref<GdParseError> error = GdParseBackend::get_singleton()
									  ->request(HTTPClient::METHOD_GET, "classes/" + class_name + "/" + p_object_id, Variant(), this, "_get_object_by_id_finished");

	if (error) {
		set_busy(false);
		emit_signal("get_object_by_id_complete", Dictionary(), error);
	}
}

void GdParseQuery::_get_object_by_id_finished(HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, const PoolStringArray &headers, const PoolByteArray &p_data) {
	set_busy(false);

	Ref<GdParseError> error;
	Dictionary json = GdParseBackend::get_singleton()->retrieve_json_reply(p_data, p_status, p_code, HTTPClient::RESPONSE_OK, error);
	if (json.empty()) {
		emit_signal("get_object_by_id_completed", Dictionary(), error);
		return;
	}

	Ref<GdParseObject> result = memnew(GdParseObject);
	result->set_class_name(class_name);
	result->set_data(json);

	emit_signal("get_object_by_id_completed", result, error);
}

void GdParseQuery::find_objects() {
	ERR_FAIL_COND(class_name.empty());
	ERR_FAIL_COND_MSG(busy, "Operation already in progress.");

	set_busy(true);

	Variant data(constraints());

	Ref<GdParseError> error;
	if (not data.is_nil()) {
		error = GdParseBackend::get_singleton()
						->request(HTTPClient::METHOD_GET, "classes/" + class_name, data, this, "_find_objects_finished");
	}

	if (error) {
		set_busy(false);
		emit_signal("find_objects_completed", Array(), error);
	}
}

void GdParseQuery::_find_objects_finished(HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, const PoolStringArray &headers, const PoolByteArray &p_data) {
	set_busy(false);

	Ref<GdParseError> error;
	Dictionary json = GdParseBackend::get_singleton()->retrieve_json_reply(p_data, p_status, p_code, HTTPClient::RESPONSE_OK, error);
	if (json.empty()) {
		emit_signal("find_objects_completed", Array(), error);
		return;
	}

	Array objects;
	for (const Variant &result : Array(json["results"])) {
		Ref<GdParseObject> object = memnew(GdParseObject);
		object->set_class_name(class_name);
		object->set_data(result);

		objects.append(result);
	}

	emit_signal("find_objects_completed", objects, error);
}

void GdParseQuery::add_where(const String &p_op, const String &p_key, const Variant &p_what) {
	ERR_FAIL_COND(p_key.empty());
	ERR_FAIL_COND(p_what.is_nil());

	Dictionary value = where[p_key];
	value[p_op] = p_what;
	where[p_key] = value;
}

void GdParseQuery::add_order(const String &p_key, SortOrder p_sort_order) {
	ERR_FAIL_COND(p_key.empty());

	Dictionary entry;
	entry["order"] = p_sort_order;
	entry["key"] = p_key;
	order.append(entry);
}

String GdParseQuery::get_class_name() const {
	return class_name;
}

void GdParseQuery::set_class_name(const String &p_class_name) {
	class_name = p_class_name;
}

int GdParseQuery::get_limit() const {
	return limit;
}

void GdParseQuery::set_limit(int p_limit) {
	limit = p_limit;
}

int GdParseQuery::get_skip() const {
	return skip;
}

void GdParseQuery::set_skip(int p_skip) {
	skip = p_skip;
}

bool GdParseQuery::is_busy() const {
	return busy;
}

void GdParseQuery::set_busy(bool p_busy) {
	if (busy != p_busy) {
		busy = p_busy;
		emit_signal("busy_changed");
	}
}

void GdParseQuery::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_class_name"), &GdParseQuery::set_class_name);
	ClassDB::bind_method(D_METHOD("get_class_name"), &GdParseQuery::get_class_name);
	ClassDB::bind_method(D_METHOD("set_limit"), &GdParseQuery::set_limit);
	ClassDB::bind_method(D_METHOD("get_limit"), &GdParseQuery::get_limit);
	ClassDB::bind_method(D_METHOD("set_skip"), &GdParseQuery::set_skip);
	ClassDB::bind_method(D_METHOD("get_skip"), &GdParseQuery::get_skip);
	ClassDB::bind_method(D_METHOD("set_busy"), &GdParseQuery::set_busy);
	ClassDB::bind_method(D_METHOD("is_busy"), &GdParseQuery::is_busy);

	ClassDB::bind_method(D_METHOD("find_objects"), &GdParseQuery::find_objects);

	ClassDB::bind_method(D_METHOD("_find_objects_finished"), &GdParseQuery::_find_objects_finished);
	ClassDB::bind_method(D_METHOD("_get_object_by_id_finished"), &GdParseQuery::_get_object_by_id_finished);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "class_name"), "set_class_name", "get_class_name");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "limit"), "set_limit", "get_limit");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "skip"), "set_skip", "get_skip");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "busy"), "set_busy", "is_busy");

	ADD_SIGNAL(MethodInfo("busy_changed"));
	ADD_SIGNAL(MethodInfo("find_objects_completed", PropertyInfo(Variant::ARRAY, "results"), PropertyInfo(Variant::OBJECT, "error")));
	ADD_SIGNAL(MethodInfo("get_object_by_id_completed", PropertyInfo(Variant::OBJECT, "result"), PropertyInfo(Variant::OBJECT, "error")));

	BIND_ENUM_CONSTANT(AscendingOrder);
	BIND_ENUM_CONSTANT(DescendingOrder);
}

GdParseQuery::GdParseQuery() {
	busy = false;
}

/// GdParseObject

static Dictionary filter_map(const Dictionary &map) {
	Dictionary result = map.duplicate();

	result.erase("objectId");
	result.erase("createdAt");
	result.erase("updatedAt");

	return result;
}

static Dictionary merge_map(const Dictionary &base, const Dictionary &other) {
	Dictionary result = base.duplicate();

	const Variant *next = nullptr;
	while (const Variant *key = other.next(next)) {
		result[*key] = other[*key];
		next = key;
	}

	return result;
}

static Dictionary diff_map(const Dictionary &base, const Dictionary &other) {
	Dictionary result;

	const Variant *next = nullptr;
	while (const Variant *key = other.next(next)) {
		const Variant &base_value = base[*key];
		const Variant &other_value = other[*key];
		if (base_value != other_value) {
			result[*key] = other_value;
		}
		next = key;
	}

	return result;
}

void GdParseObject::save() {
	ERR_FAIL_COND_MSG(busy, "Operation already in progress.");
	ERR_FAIL_COND(class_name.empty());

	set_busy(true);

	if (get_object_id().empty()) {
		_create_object();
	} else {
		_update_object();
	}
}

void GdParseObject::erase() {
	ERR_FAIL_COND_MSG(busy, "Operation already in progress.");
	ERR_FAIL_COND(class_name.empty());
	ERR_FAIL_COND(get_object_id().empty());

	set_busy(true);

	Ref<GdParseError> error = GdParseBackend::get_singleton()
									  ->request(HTTPClient::METHOD_DELETE, "classes/" + class_name + "/" + get_object_id(), Variant(), this, "_erase_finished");

	if (error) {
		set_busy(false);
		emit_signal("erase_completed", error);
	}
}

void GdParseObject::_create_object() {
	Ref<GdParseError> error;

	error = GdParseBackend::get_singleton()
					->request(HTTPClient::METHOD_POST, "classes/" + class_name, filter_map(data), this, "_create_object_finished");

	if (error) {
		set_busy(false);
		emit_signal("save_completed", error);
	}
}

void GdParseObject::_update_object() {
	Ref<GdParseError> error;

	error = GdParseBackend::get_singleton()
					->request(HTTPClient::METHOD_PUT, "classes/" + class_name + "/" + get_object_id(), diff_map(filter_map(snapshot), filter_map(data)), this, "_update_object_finished");

	if (error) {
		set_busy(false);
		emit_signal("save_completed", error);
	}
}

void GdParseObject::_update_object_finished(HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, const PoolStringArray &headers, const PoolByteArray &p_data) {
	set_busy(false);

	Ref<GdParseError> error;
	Dictionary response;

	response = GdParseBackend::get_singleton()->retrieve_json_reply(p_data, p_status, p_code, HTTPClient::RESPONSE_OK, error);

	if (error) {
		emit_signal("save_completed", error);
	}

	set_data(merge_map(snapshot, merge_map(filter_map(data), response)));
	emit_signal("save_completed", error);
}

void GdParseObject::_create_object_finished(HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, const PoolStringArray &headers, const PoolByteArray &p_data) {
	Ref<GdParseError> error;
	Dictionary response;

	set_busy(false);

	response = GdParseBackend::get_singleton()->retrieve_json_reply(p_data, p_status, p_code, HTTPClient::RESPONSE_CREATED, error);

	if (error) {
		emit_signal("save_completed", error);
		return;
	}

	set_data(merge_map(filter_map(data), response));
	emit_signal("save_completed", error);
}

void GdParseObject::_erase_finished(HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, const PoolStringArray &headers, const PoolByteArray &p_data) {
	set_busy(false);

	Ref<GdParseError> error;
	Dictionary json = GdParseBackend::get_singleton()->retrieve_json_reply(p_data, p_status, p_code, HTTPClient::RESPONSE_OK, error);

	if (error) {
		emit_signal("erase_completed", false, error);
		return;
	}

	set_data(Dictionary());

	emit_signal("erase_completed", true);
}

String GdParseObject::get_object_id() const {
	return snapshot.has("objectId") ? snapshot.get_valid("objectId") : "";
}

Dictionary GdParseObject::get_created_at() const {
	return GdParseBackend::get_singleton()->get_datetime_from_string(snapshot.get_valid("createdAt"));
}

Dictionary GdParseObject::get_updated_at() const {
	return GdParseBackend::get_singleton()->get_datetime_from_string(snapshot.get_valid("updatedAt"));
}

String GdParseObject::get_class_name() const {
	return class_name;
}

void GdParseObject::set_class_name(const String &p_class_name) {
	class_name = p_class_name;
}

Dictionary GdParseObject::get_data() const {
	return data;
}

void GdParseObject::set_data(const Dictionary &p_data) {
	bool changed_data = false;
	bool changed_object_id = false;
	bool changed_created_at = false;
	bool changed_updated_at = false;

	if (snapshot != p_data) {
		changed_data = true;
	}
	if (snapshot.get_valid("objectId") != p_data.get_valid("objectId")) {
		changed_object_id = true;
	}
	if (snapshot.get_valid("createdAt") != p_data.get_valid("createdAt")) {
		changed_created_at = true;
	}
	if (snapshot.get_valid("updatedAt") != p_data.get_valid("updatedAt")) {
		changed_updated_at = true;
	}

	data = GdParseBackend::get_singleton()->from_raw_data(p_data);
	snapshot = p_data;

	if (changed_data) {
		emit_signal("data_changed");
	}
	if (changed_object_id) {
		emit_signal("object_id_changed");
	}
	if (changed_created_at) {
		emit_signal("created_at_changed");
	}
	if (changed_updated_at) {
		emit_signal("updated_at_changed");
	}
}

bool GdParseObject::is_busy() const {
	return busy;
}

void GdParseObject::set_busy(bool p_busy) {
	busy = p_busy;
}

void GdParseObject::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_busy", "state"), &GdParseObject::set_busy);
	ClassDB::bind_method(D_METHOD("is_busy"), &GdParseObject::is_busy);
	ClassDB::bind_method(D_METHOD("set_data", "data"), &GdParseObject::set_data);
	ClassDB::bind_method(D_METHOD("get_data"), &GdParseObject::get_data);
	ClassDB::bind_method(D_METHOD("set_class_name", "class_name"), &GdParseObject::set_class_name);
	ClassDB::bind_method(D_METHOD("get_class_name"), &GdParseObject::get_class_name);

	ClassDB::bind_method(D_METHOD("save"), &GdParseObject::save);
	ClassDB::bind_method(D_METHOD("erase"), &GdParseObject::erase);

	ClassDB::bind_method(D_METHOD("get_object_id"), &GdParseObject::get_object_id);
	ClassDB::bind_method(D_METHOD("get_created_at"), &GdParseObject::get_created_at);
	ClassDB::bind_method(D_METHOD("get_updated_at"), &GdParseObject::get_updated_at);

	ClassDB::bind_method(D_METHOD("_create_object_finished"), &GdParseObject::_create_object_finished);
	ClassDB::bind_method(D_METHOD("_update_object_finished"), &GdParseObject::_update_object_finished);
	ClassDB::bind_method(D_METHOD("_erase_finished"), &GdParseObject::_erase_finished);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "busy"), "set_busy", "is_busy");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "class_name"), "set_class_name", "get_class_name");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "data"), "set_data", "get_data");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "object_id"), "", "get_object_id");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "created_at"), "", "get_created_at");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "updated_at"), "", "get_updated_at");

	ADD_SIGNAL(MethodInfo("data_changed"));
	ADD_SIGNAL(MethodInfo("object_id_changed"));
	ADD_SIGNAL(MethodInfo("created_at_changed"));
	ADD_SIGNAL(MethodInfo("updated_at_changed"));
	ADD_SIGNAL(MethodInfo("busy_changed"));

	ADD_SIGNAL(MethodInfo("save_completed"));
	ADD_SIGNAL(MethodInfo("erase_completed"));
}

GdParseObject::GdParseObject() {
	busy = false;
}

/// GdParseBackend

namespace Utils {
_FORCE_INLINE_ static String bytearray_to_string(const PoolByteArray &p_data) {
	String s;
	if (p_data.size() >= 0) {
		PoolByteArray::Read r = p_data.read();
		CharString cs;
		cs.resize(p_data.size() + 1);
		memcpy(cs.ptrw(), r.ptr(), p_data.size());
		cs[p_data.size()] = 0;
		s = cs.get_data();
	}
	return s;
}
_FORCE_INLINE_ static PoolByteArray string_to_bytearray(const String &p_string) {
	CharString charstr = p_string.utf8();
	PoolByteArray retval;
	size_t len = charstr.length();
	retval.resize(len);
	PoolByteArray::Write w = retval.write();
	memcpy(w.ptr(), charstr.ptr(), len);
	w.release();
	return retval;
}
#define SECOND_KEY "second"
#define MINUTE_KEY "minute"
#define HOUR_KEY "hour"
#define DAY_KEY "day"
#define MONTH_KEY "month"
#define YEAR_KEY "year"
#define WEEKDAY_KEY "weekday"
#define TIMEZONE_KEY "timezone"
#define DST_KEY "dst"
// Parses an ISO-8601 date string to a datetime dictionary that can be parsed by Godot.
_FORCE_INLINE_ static Dictionary parse_iso_datetime(const String &p_iso_datetime) {
	ERR_FAIL_COND_V_MSG(p_iso_datetime.find("T") == -1, Dictionary(), "Invalid iso date string (missing T)");

	const Vector<String> &parts = p_iso_datetime.split("T");
	ERR_FAIL_COND_V_MSG(parts.size() != 2, Dictionary(), "Invalid iso date string (missing date/time part)");
	const Vector<String> &date = parts[0].split("-");
	ERR_FAIL_COND_V_MSG(date.size() != 3, Dictionary(), "Invalid iso date string (malformed date part)");
	const Vector<String> &time = p_iso_datetime.ends_with("Z")
			? parts[1].trim_suffix("Z").split(":")
			: parts[1].split(p_iso_datetime.find("+") >= 0 ? "+" : "-")[0].split(":");
	ERR_FAIL_COND_V_MSG(time.size() != 3, Dictionary(), "Invalid iso date string (malformed time part)");
#if DEBUG_ENABLED
#define _CHECK(k, s)                                                  \
	if (!s.is_numeric()) {                                            \
		WARN_PRINT(vformat("(%s) '%s' is not numeric value", #k, s)); \
	}
	_CHECK(YEAR, date[0]);
	_CHECK(MONTH, date[1]);
	_CHECK(DAY, date[2]);
	_CHECK(HOUR, time[0]);
	_CHECK(MINUTE, time[1]);
	_CHECK(SECOND, time[2]);
#undef _CHECK
#endif
	Dictionary dict;
	dict[YEAR_KEY] = date[0].to_int();
	dict[MONTH_KEY] = date[1].to_int();
	dict[DAY_KEY] = date[2].to_int();
	dict[HOUR_KEY] = time[0].to_int();
	dict[MINUTE_KEY] = time[1].to_int();
	dict[SECOND_KEY] = time[2].to_int();

	if (p_iso_datetime.ends_with("Z")) {
		dict[TIMEZONE_KEY] = "Z";
	} else {
		const int pp = parts[1].find("+"), mp = parts[1].find("-");
		if (pp >= 0 && mp >= 0) {
			WARN_PRINT("Malformed timezone info - ignoring section");
		} else if (pp >= 0 || mp >= 0) {
			const Vector<String> &tz = parts[1].substr(MAX(pp, mp)).split(":");
			if (tz.size() > 0) {
				if (!tz[0].is_numeric()) {
					WARN_PRINT("Malformed timezone info - ignoring section");
				} else {
					const int tz_h = tz[0].to_int();
					if (tz.size() > 1 && !tz[1].is_numeric()) {
						WARN_PRINT("Malformed timezone info - ignoring part");
						dict[TIMEZONE_KEY] = tz_h * 60;
					} else {
						dict[TIMEZONE_KEY] = tz_h * 60 + SGN(tz_h) * tz[1].to_int();
					}
				}
			}
		}
	}
	return dict;
}
_FORCE_INLINE_ String datetime_to_iso_string(const Dictionary &p_datetime) {
	String timezone = "";
	if (p_datetime.has(TIMEZONE_KEY)) {
		if (p_datetime[TIMEZONE_KEY].get_type() == Variant::INT || p_datetime[TIMEZONE_KEY].get_type() == Variant::REAL) {
			const int tz = p_datetime[TIMEZONE_KEY];
			const int tz_h = tz / 60, tz_m = Math::abs(tz % 60);
			timezone = (tz >= 0 ? "+" : "") + itos(tz_h).pad_zeros(2) + (tz_m > 0 ? ":" + itos(tz_m).pad_zeros(2) : "");
		} else if (p_datetime[TIMEZONE_KEY].get_type() == Variant::STRING) {
			timezone = p_datetime[TIMEZONE_KEY];
		} else {
			WARN_PRINT("Unknown timezone format - ignoring.");
		}
	}
	return itos(p_datetime[YEAR_KEY]).pad_zeros(2) +
			"-" +
			itos(p_datetime[MONTH_KEY]).pad_zeros(2) +
			"-" +
			itos(p_datetime[DAY_KEY]).pad_zeros(2) +
			"T" +
			itos(p_datetime[HOUR_KEY]).pad_zeros(2) +
			":" +
			itos(p_datetime[MINUTE_KEY]).pad_zeros(2) +
			":" +
			itos(p_datetime[SECOND_KEY]).pad_zeros(2) +
			timezone;
}
_FORCE_INLINE_ bool is_datetime(const Dictionary &p_data) {
	return (
			p_data.has_all(array(HOUR_KEY, MINUTE_KEY, SECOND_KEY)) || p_data.has_all(array(DAY_KEY, MONTH_KEY, YEAR_KEY)));
}
} //namespace Utils

namespace Builder {
struct Url {
	String _scheme = "https";
	String _host;
	String _port;
	String _path;
	Url &scheme(const String &p_scheme) {
		_scheme = _scheme.empty() ? "https" : p_scheme;
		return *this;
	}
	Url &host(const String &p_host) {
		_host = p_host;
		return *this;
	}
	Url &port(int &p_port) {
		_port = p_port == -1 ? "" : String::num(p_port);
		return *this;
	}
	Url &path(const String &p_path) {
		_path = p_path;
		return *this;
	}
	String build() { return vformat("%s://%s%s%s", _scheme, _host, _port.empty() ? "" : ":" + _port, _path.begins_with("/") ? _path : "/" + _path); }
};
struct Request {
	String _url;
	HTTPClient::Method _method = HTTPClient::METHOD_GET;
	Vector<String> _headers;
	PoolStringArray _query;
	PoolByteArray _payload;
	Object *_receiver = nullptr;
	String _slot;
	Request &url(const String &p_url) {
		_url = p_url;
		return *this;
	}
	Request &header(const String &p_header) {
		_headers.push_back(p_header);
		return *this;
	}
	Request &header(const String &p_name, const String &p_value) {
		_headers.push_back(p_name + ":" + p_value);
		return *this;
	}
	Request &query_parameter(const String &p_name, const String &p_value) {
		_query.push_back(p_name + "=" + p_value);
		return *this;
	}
	Request method(HTTPClient::Method p_method, const PoolByteArray &p_payload = PoolByteArray()) {
		_method = p_method;
		_payload = p_payload;
		return *this;
	}
	Request slot(Object *p_receiver, const String &p_slot) {
		_receiver = p_receiver;
		_slot = p_slot;
		return *this;
	}
	Error make(HTTPRequest *p_client, bool trace = false) {
		if (trace) {
			LOGI("Sending request: %s", _url);
		}
		p_client->cancel_request();
		if (_receiver && !_slot.empty()) {
			p_client->connect("request_completed", _receiver, _slot);
		}
		return p_client->request(_url, _headers, true, _method, Utils::bytearray_to_string(_payload));
	}
};
} //namespace Builder

Variant GdParseBackend::jsonify(const Variant &p_data) {
	Variant::Type data_type = p_data.get_type();

	if (data_type == Variant::POOL_BYTE_ARRAY) {
		Dictionary result;
		result["__type"] = "Bytes";
		const PoolByteArray data = p_data;
		result["base64"] = _Marshalls::get_singleton()->raw_to_base64(data);
		return result;
	}
	if (data_type == Variant::ARRAY) {
		Array result;
		for (const Variant &variant : Array(p_data)) {
			Variant json = jsonify(variant);
			if (json.is_nil()) {
				return json;
			}
			result.append(json);
		}
		return result;
	}
	if (data_type == Variant::DICTIONARY) {
		if (Utils::is_datetime(p_data)) {
			Dictionary result;
			result["__type"] = "Date";
			result["iso"] = Utils::datetime_to_iso_string(p_data);
			return result;
		} else {
			Dictionary result;
			const Dictionary map = p_data;
			for (const auto &key : map.keys()) {
				Variant json = jsonify(map[key]);
				if (json.is_nil()) {
					return json;
				}
				result[key] = json;
			}
			return result;
		}
	}

	return p_data;
}

Dictionary GdParseBackend::from_raw_data(const Dictionary &p_data) {
	Dictionary result;
	const Variant *next = nullptr;
	while (const Variant *key = p_data.next(next)) {
		result[*key] = objectify(p_data[*key]);
		next = key;
	}
	return result;
}

Variant GdParseBackend::objectify(const Variant &p_json) {
	Variant::Type data_type = p_json.get_type();

	if (data_type == Variant::ARRAY) {
		Array result;
		for (const Variant &variant : Array(p_json)) {
			result.append(objectify(variant));
		}
		return result;
	}

	if (data_type != Variant::DICTIONARY) {
		return p_json;
	}

	const Dictionary map = p_json;
	if (!map.has("__type")) {
		Dictionary result;
		const Variant *next = nullptr;
		while (const Variant *key = map.next(next)) {
			result[*key] = objectify(map[*key]);
			next = key;
		}
		return result;
	}

	String type = map["__type"];
	if (type == "Date") {
		return Utils::parse_iso_datetime(map["iso"]);
	}
	if (type == "Bytes") {
		const String data = map["base64"];
		return _Marshalls::get_singleton()->base64_to_raw(data);
	}

	ERR_PRINT("Parse invalid type: " + type);
	return Variant();
}

Ref<GdParseError> GdParseBackend::request(HTTPClient::Method p_op, const String &p_url, const Variant &p_variant, Object *p_receiver, const String &p_slot) {
	ERR_FAIL_COND_V(p_url.empty(), Ref<GdParseError>());
	ERR_FAIL_NULL_V(p_receiver, Ref<GdParseError>());
	ERR_FAIL_COND_V(p_slot.empty(), Ref<GdParseError>());

	// Check that application id and api key are set
	if (application_id.empty() || master_key.empty()) {
		return referr(GdParseError::DomainParsePlatform, GdParseError::ParseDataCodeNotInitialized, "ApplicationId or APIKey not set");
	}

	Ref<GdParseError> error;

	// Dispatch according to method
	switch (p_op) {
		case HTTPClient::METHOD_GET: {
			String query = p_variant;
			if (!query.empty()) {
				error = referr(GdParseError::DomainParsePlatform, GdParseError::ParseDataCodeInternal, "Missing data");
			} else {
				if (trace) {
					LOGD("Response query: %s", query);
				}
				Error ret = Builder::Request()
									.url(parse_server_url + p_url + "?" + query)
									.method(p_op)
									.header("Content-Type", "application/json")
									.header("X-Parse-Application-Id", application_id)
									.header("X-Parse-REST-API-Key", master_key)
									.header("User-Agent", GODOTPARSE_AGENT)
									.slot(p_receiver, p_slot)
									.make(http, trace);
				if (ret != OK) {
					error = referr(GdParseError::DomainParsePlatform, ret, "Cannot start network request");
				}
			}
		} break;

		case HTTPClient::METHOD_POST:
		case HTTPClient::METHOD_PUT: {
			Dictionary json = p_variant;
			if (json.empty()) {
				error = referr(GdParseError::DomainParsePlatform, GdParseError::ParseDataCodeInternal, "Missing data");
			} else {
				if (trace) {
					LOGD("Parse JSON: %s", json);
				}
				Error ret = Builder::Request()
									.url(parse_server_url + p_url)
									.method(p_op, Utils::string_to_bytearray(JSON::print(json)))
									.header("Content-Type", "application/json")
									.header("X-Parse-Application-Id", application_id)
									.header("X-Parse-REST-API-Key", master_key)
									.header("User-Agent", GODOTPARSE_AGENT)
									.slot(p_receiver, p_slot)
									.make(http, trace);
				if (ret != OK) {
					error = referr(GdParseError::DomainParsePlatform, ret, "Cannot start network request");
				}
			}
		} break;

		case HTTPClient::METHOD_DELETE: {
			Error ret = Builder::Request()
								.url(parse_server_url + p_url)
								.method(p_op)
								.header("X-Parse-Application-Id", application_id)
								.header("X-Parse-REST-API-Key", master_key)
								.header("User-Agent", GODOTPARSE_AGENT)
								.slot(p_receiver, p_slot)
								.make(http, trace);
			if (ret != OK) {
				error = referr(GdParseError::DomainParsePlatform, ret, "Cannot start network request");
			};
		} break;

		default: {
			error = referr(GdParseError::DomainParsePlatform, GdParseError::ParseDataCodeInternal, "Unknown, undefined or unimplemented operation");
		} break;
	}

	return error;
}

Dictionary GdParseBackend::retrieve_json_reply(const PoolByteArray &p_data, HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, HTTPClient::ResponseCode p_expected_code, Ref<GdParseError> &p_error) {
	if (p_status != HTTPRequest::RESULT_SUCCESS) {
		if (trace) {
			LOGD("Reply: error %s", p_status);
		}
		p_error = referr(GdParseError::DomainNetwork, p_status, "Network failed");
		return Dictionary();
	}

	if (p_data.empty()) {
		p_error = referr(GdParseError::DomainNetwork, p_status, "Response data are missing");
		return Dictionary();
	}

	const String raw_data = Utils::bytearray_to_string(p_data);
	if (trace) {
		LOGD("Reply: %s (%d bytes)", raw_data, p_data.size());
	}

	int err_line;
	String err_message;
	Variant json_data;
	if (JSON::parse(raw_data, json_data, err_message, err_line) != OK) {
		p_error = referr(GdParseError::DomainJson, GdParseError::JsonCodeFailed, err_message);
		return Dictionary();
	}
	Dictionary json = json_data;
	if (json.empty()) {
		p_error = referr(GdParseError::DomainParseData, GdParseError::ParseDataCodeInternal, "JSON data are empty");
		return Dictionary();
	}

	if (p_expected_code != 0 && p_expected_code != p_code) {
		p_error = referr(GdParseError::DomainParsePlatform, json.has("code") ? json.get_valid("code") : Variant(0), json.get_valid("error"));
		return Dictionary();
	}

	return json;
}

Ref<GdParseObject> GdParseBackend::create_object(const Dictionary &p_data, const String &p_class_name) {
	Ref<GdParseObject> object = memnew(GdParseObject);
	object->set_data(p_data);
	object->set_class_name(p_class_name);
	return object;
}

Ref<GdParseError> GdParseBackend::load_object(const String &p_object_id, Object *p_receiver, const String &p_slot) {
	Ref<GdParseError> error;
	if (_query->is_busy()) {
		// queue request
		QueueData data = { RequestQueue::get_object_req(p_object_id, p_receiver, p_slot), sizeof(RequestQueue) };
		if (queue_push(_queue, &data) != LIBQUEUE_SUCCESS) {
			ERR_PRINT("Failed to queue the request - request is skipped.");
			error = referr(GdParseError::DomainGodot, ERR_SKIP, "Cannot queue network request");
		}
	} else {
		_query->get_object_by_id(p_object_id);
	}
	return error;
}

Ref<GdParseError> GdParseBackend::load_objects(const Ref<GdParseQuery> &p_query, Object *p_receiver, const String &p_slot) {
	Ref<GdParseError> error;
	return error;
}

String GdParseBackend::get_parse_server_url() const {
	return parse_server_url;
}

void GdParseBackend::set_parse_server_url(const String &p_parse_server_url) {
	parse_server_url = p_parse_server_url;
	if (not parse_server_url.ends_with("/")) {
		parse_server_url += "/"; // make sure server url is ending with /
	}
}

HTTPRequest *GdParseBackend::get_http() const {
	return http;
}

void GdParseBackend::configure_http(Node *p_http) {
	if (http) {
		http->disconnect("request_completed", this, "_request_finished");
	}
	if (HTTPRequest *req = cast_to<HTTPRequest>(p_http)) {
		http = req;
		if (http) {
			http->connect("request_completed", this, "_request_finished");
		}
	} else {
		http = nullptr;
	}
}

String GdParseBackend::get_application_id() const {
	return application_id;
}

void GdParseBackend::set_application_id(const String &p_app_id) {
	application_id = p_app_id;
}

String GdParseBackend::get_master_key() const {
	return master_key;
}

void GdParseBackend::set_master_key(const String &p_master_key) {
	master_key = p_master_key;
}

bool GdParseBackend::get_trace() const {
	return trace;
}

void GdParseBackend::set_trace(bool p_trace) {
	trace = p_trace;
}

static GdParseBackend *instance = nullptr;

GdParseBackend *GdParseBackend::get_singleton() {
	return instance;
}

Dictionary GdParseBackend::get_datetime_from_string(const String &p_iso_datetime) {
	return Utils::parse_iso_datetime(p_iso_datetime);
}

String GdParseBackend::string_from_datetime(const Dictionary &p_datetime) {
	return Utils::datetime_to_iso_string(p_datetime);
}

void GdParseBackend::_request_finished(HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, const PoolStringArray &headers, const PoolByteArray &p_data) {
	// check requests queue
	QueueData data;
	int64_t count;
	if (queue_count(_queue, &count) == LIBQUEUE_SUCCESS) {
		if (count > 0) {
			if (queue_pop(_queue, &data) == LIBQUEUE_SUCCESS) {
				// new request from queue
			} else {
				WARN_PRINT("Failed to fetch request from queue.");
			}
		}
	} else {
		WARN_PRINT("Failed to determine queue size.");
	}
}

void GdParseBackend::_notification(int p_what) {
}

void GdParseBackend::_bind_methods() {
	ClassDB::bind_method(D_METHOD("configure_http", "httprequest"), &GdParseBackend::configure_http);
	ClassDB::bind_method(D_METHOD("get_http"), &GdParseBackend::get_http);
	ClassDB::bind_method(D_METHOD("set_application_id", "appid"), &GdParseBackend::set_application_id);
	ClassDB::bind_method(D_METHOD("get_application_id"), &GdParseBackend::get_application_id);
	ClassDB::bind_method(D_METHOD("set_master_key", "key"), &GdParseBackend::set_master_key);
	ClassDB::bind_method(D_METHOD("get_master_key"), &GdParseBackend::get_master_key);
	ClassDB::bind_method(D_METHOD("set_parse_server_url", "url"), &GdParseBackend::set_parse_server_url);
	ClassDB::bind_method(D_METHOD("get_parse_server_url"), &GdParseBackend::get_parse_server_url);
	ClassDB::bind_method(D_METHOD("set_trace", "trace"), &GdParseBackend::set_trace);
	ClassDB::bind_method(D_METHOD("get_trace"), &GdParseBackend::get_trace);

	ClassDB::bind_method(D_METHOD("create_object", "data"), &GdParseBackend::create_object, DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("load_object", "object_id"), &GdParseBackend::load_object);
	ClassDB::bind_method(D_METHOD("load_objects", "query"), &GdParseBackend::load_objects);

	ClassDB::bind_method(D_METHOD("_request_finished"), &GdParseBackend::_request_finished);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "application_id"), "set_application_id", "get_application_id");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "master_key"), "set_master_key", "get_master_key");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "parse_server"), "set_parse_server_url", "get_parse_server_url");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "trace"), "set_trace", "get_trace");
}

GdParseBackend::GdParseBackend() {
	if (not(_queue = queue_open(ProjectSettings::get_singleton()->globalize_path(REQUESTS_QUEUE).utf8().c_str()))) {
		WARN_PRINT("Failed to open requests queue at: " + ProjectSettings::get_singleton()->globalize_path(REQUESTS_QUEUE));
	}
	_query = Ref<GdParseQuery>(memnew(GdParseQuery));
	http = nullptr;
	parse_server_url = "http://localhost:1337/parse/";
	application_id = "AppId";
	master_key = "ParseMasterKey";
	trace = false;
	instance = this;
}

GdParseBackend::~GdParseBackend() {
	instance = nullptr;
	if (http) {
		http->cancel_request();
	}
	_queue = nullptr;
	if (_queue) {
		if (queue_close(_queue) != LIBQUEUE_SUCCESS) {
			WARN_PRINT("Failed to close requests queue.");
		}
	}
}

#ifdef DOCTEST
TEST_CASE("Utils functions") {
	SUBCASE("parse_iso_datetime") {
		Dictionary ret1 = Utils::parse_iso_datetime("2021-11-18T00:02:40");
		REQUIRE(int(ret1[YEAR_KEY]) == 2021);
		REQUIRE(int(ret1[MONTH_KEY]) == 11);
		REQUIRE(int(ret1[DAY_KEY]) == 18);
		REQUIRE(int(ret1[HOUR_KEY]) == 0);
		REQUIRE(int(ret1[MINUTE_KEY]) == 2);
		REQUIRE(int(ret1[SECOND_KEY]) == 40);
		REQUIRE_FALSE(ret1.has(TIMEZONE_KEY));
		REQUIRE(Utils::datetime_to_iso_string(ret1) == "2021-11-18T00:02:40");
		Dictionary ret2 = Utils::parse_iso_datetime("2021-11-18T00:02:40Z");
		REQUIRE(int(ret2[YEAR_KEY]) == 2021);
		REQUIRE(int(ret2[MONTH_KEY]) == 11);
		REQUIRE(int(ret2[DAY_KEY]) == 18);
		REQUIRE(int(ret2[HOUR_KEY]) == 0);
		REQUIRE(int(ret2[MINUTE_KEY]) == 2);
		REQUIRE(int(ret2[SECOND_KEY]) == 40);
		REQUIRE(ret2[TIMEZONE_KEY] == "Z");
		REQUIRE(Utils::datetime_to_iso_string(ret2) == "2021-11-18T00:02:40Z");
		Dictionary ret3 = Utils::parse_iso_datetime("2021-11-18T00:02:40+10:00");
		REQUIRE(int(ret3[YEAR_KEY]) == 2021);
		REQUIRE(int(ret3[MONTH_KEY]) == 11);
		REQUIRE(int(ret3[DAY_KEY]) == 18);
		REQUIRE(int(ret3[HOUR_KEY]) == 0);
		REQUIRE(int(ret3[MINUTE_KEY]) == 2);
		REQUIRE(int(ret3[SECOND_KEY]) == 40);
		REQUIRE(int(ret3[TIMEZONE_KEY]) == 10 * 60);
		REQUIRE(Utils::datetime_to_iso_string(ret3) == "2021-11-18T00:02:40+10");
		Dictionary ret4 = Utils::parse_iso_datetime("2021-11-18T00:02:40-09:30");
		REQUIRE(int(ret4[YEAR_KEY]) == 2021);
		REQUIRE(int(ret4[MONTH_KEY]) == 11);
		REQUIRE(int(ret4[DAY_KEY]) == 18);
		REQUIRE(int(ret4[HOUR_KEY]) == 0);
		REQUIRE(int(ret4[MINUTE_KEY]) == 2);
		REQUIRE(int(ret4[SECOND_KEY]) == 40);
		REQUIRE(int(ret4[TIMEZONE_KEY]) == -(9 * 60 + 30));
		REQUIRE(Utils::datetime_to_iso_string(ret4) == "2021-11-18T00:02:40-09:30");
	}
	SUBCASE("datetime_to_iso_string") {
		Dictionary datetime_dict;
		datetime_dict[YEAR_KEY] = 2021;
		datetime_dict[MONTH_KEY] = 11;
		datetime_dict[DAY_KEY] = 18;
		datetime_dict[HOUR_KEY] = 0;
		datetime_dict[MINUTE_KEY] = 2;
		datetime_dict[SECOND_KEY] = 40;
		REQUIRE(Utils::datetime_to_iso_string(datetime_dict) == "2021-11-18T00:02:40");
		datetime_dict[TIMEZONE_KEY] = "Z";
		REQUIRE(Utils::datetime_to_iso_string(datetime_dict) == "2021-11-18T00:02:40Z");
		datetime_dict[TIMEZONE_KEY] = 2 * 60;
		REQUIRE(Utils::datetime_to_iso_string(datetime_dict) == "2021-11-18T00:02:40+02");
		datetime_dict[TIMEZONE_KEY] = -4 * 60;
		REQUIRE(Utils::datetime_to_iso_string(datetime_dict) == "2021-11-18T00:02:40-04");
		datetime_dict[TIMEZONE_KEY] = -3 * 60 - 30;
		REQUIRE(Utils::datetime_to_iso_string(datetime_dict) == "2021-11-18T00:02:40-03:30");
		datetime_dict[TIMEZONE_KEY] = "+00:30";
		REQUIRE(Utils::datetime_to_iso_string(datetime_dict) == "2021-11-18T00:02:40+00:30");
	}
}

TEST_CASE("Map operations") {
	SUBCASE("filter_map") {
		Dictionary map1;
		map1["1"] = "1";
		map1["2"] = "2";
		map1["3"] = "3";
		Dictionary ret = filter_map(map1);
		REQUIRE(ret.has_all(array("1", "2", "3")));
		REQUIRE(ret["2"] == "2");
		REQUIRE(ret["2"] == "2");
		REQUIRE(ret["3"] == "3");
	}
	SUBCASE("merge_map") {
		Dictionary map1;
		map1["1"] = "1";
		map1["2"] = "2";
		Dictionary map2;
		map2["1"] = "2";
		map2["3"] = "3";
		Dictionary ret1 = merge_map(map1, map2);
		REQUIRE(ret1["1"] == "2");
		REQUIRE(ret1["2"] == "2");
		REQUIRE(ret1["3"] == "3");
		Dictionary ret2 = merge_map(map2, map1);
		REQUIRE(ret2["1"] == "1");
		REQUIRE(ret2["2"] == "2");
		REQUIRE(ret2["3"] == "3");
	}
	SUBCASE("diff_map") {
		Dictionary map1;
		map1["1"] = "1";
		map1["2"] = "2";
		Dictionary map2;
		map2["1"] = "2";
		map2["3"] = "3";
		Dictionary ret1 = diff_map(map1, map2);
		REQUIRE(ret1.has_all(array("1", "3")));
		REQUIRE(ret1["1"] == "2");
		REQUIRE(ret1["3"] == "3");
		Dictionary ret2 = diff_map(map2, map1);
		REQUIRE(ret2.has_all(array("1", "2")));
		REQUIRE(ret2["1"] == "1");
		REQUIRE(ret2["2"] == "2");
	}
}
#endif // DOCTEST
