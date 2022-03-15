/*************************************************************************/
/*  gd_parse_platform.h                                                  */
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

#ifndef GD_PARSE_PLATFORM_H
#define GD_PARSE_PLATFORM_H

#include "core/object.h"
#include "scene/main/http_request.h"

#define GODOTPARSE_MAJOR 0
#define GODOTPARSE_MINOR 0
#define GODOTPARSE_PATCH 1

#define GP_STR(x) #x
#define GODOTPARSE_VERSION   \
	GP_STR(GODOTPARSE_MAJOR) \
	"." GP_STR(GODOTPARSE_MINOR) "." GP_STR(GODOTPARSE_PATCH)
#define GODOTPARSE_AGENT vformat("godot-parse (%s)", GODOTPARSE_VERSION)

/// GdParseError

class GdParseError : public Reference {
	GDCLASS(GdParseError, Reference);

public:
	enum Domain {
		DomainUndefined,
		DomainParsePlatform,
		DomainJson,
		DomainGodot,
		DomainNetwork,
		DomainParseData,
	};

	enum Code {
		ParsePlatformCodeInternalServerError,
		ParsePlatformCodeConnectionFailed,
		ParsePlatformCodeObjectNotFound,

		JsonCodeFailed,

		ParseDataCodeInternal,
		ParseDataCodeNotInitialized,
		ParseDataCodeInvalidType,
	};

private:
	Domain domain;
	int code;
	String description;

protected:
	static void _bind_methods();

public:
	Domain get_err_domain() const { return domain; }
	int get_err_code() const { return code; }
	String get_err_description() const { return description; }

	GdParseError(Domain p_domain, int p_code, const String &p_description);
};

VARIANT_ENUM_CAST(GdParseError::Domain);
VARIANT_ENUM_CAST(GdParseError::Code);

/// GdParseObject

class GdParseObject : public Reference {
	GDCLASS(GdParseObject, Reference);

	String class_name;
	Dictionary data;
	Dictionary snapshot;
	bool busy;

private:
	void _create_object();
	void _update_object();

	void _create_object_finished(HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, const PoolStringArray &headers, const PoolByteArray &p_data);
	void _update_object_finished(HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, const PoolStringArray &headers, const PoolByteArray &p_data);
	void _erase_finished(HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, const PoolStringArray &headers, const PoolByteArray &p_data);

protected:
	static void _bind_methods();

public:
	Dictionary get_data() const;
	void set_data(const Dictionary &p_data);
	/// managing object properties
	String get_class_name() const;
	void set_class_name(const String &p_class_name);
	bool is_busy() const;
	void set_busy(bool p_busy);

	String get_object_id() const;
	Dictionary get_created_at() const;
	Dictionary get_updated_at() const;

	/// saving an object
	void save();
	/// deleting objects
	void erase();

	GdParseObject();
};

/// GdParseQuery

class GdParseQuery : public Reference {
	GDCLASS(GdParseQuery, Reference);

	String class_name;
	Dictionary where;
	Array order;
	int limit;
	int skip;
	bool busy;

public:
	enum SortOrder {
		AscendingOrder,
		DescendingOrder,
	};

private:
	void add_where(const String &p_op, const String &p_key, const Variant &p_what);
	void add_order(const String &p_key, SortOrder p_sort_order);

	Variant constraints();

	void _find_objects_finished(HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, const PoolStringArray &headers, const PoolByteArray &p_data);
	void _get_object_by_id_finished(HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, const PoolStringArray &headers, const PoolByteArray &p_data);

protected:
	static void _bind_methods();

public:
	/// managing object properties
	String get_class_name() const;
	void set_class_name(const String &p_class_name);
	bool is_busy() const;
	void set_busy(bool p_busy);
	/// pagination
	int get_limit() const;
	void set_limit(int p_limit);
	int get_skip() const;
	void set_skip(int p_skip);

	/// adding basic constraints
	void where_less_than(const String &p_key, const Variant &p_what);
	void where_less_than_or_equal_to(const String &p_key, const Variant &p_what);
	void where_greater_than(const String &p_key, const Variant &p_what);
	void where_greater_than_or_equal_to(const String &p_key, const Variant &p_what);
	void where_not_equal_to(const String &p_key, const Variant &p_what);

	/// sorting
	void order_by_ascending(const String &p_key);
	void add_ascending_order(const String &p_key);
	void order_by_descending(const String &p_key);
	void add_descending_order(const String &p_key);

	/// getting objects by id
	void get_object_by_id(const String &p_object_id);
	/// finding objects as specified
	void find_objects();

	GdParseQuery();
};

VARIANT_ENUM_CAST(GdParseQuery::SortOrder);

/// GdParseBackend

struct Queue;
class GdParseBackend : public Object {
	GDCLASS(GdParseBackend, Object);

	HTTPRequest *http;
	String parse_server_url;
	String application_id;
	String master_key;
	bool trace;

	Queue *_queue;
	Ref<GdParseQuery> _query;

	void _check_requests_queue();
	void _query_status_changed();
	void _request_finished(HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, const PoolStringArray &headers, const PoolByteArray &p_data);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	static GdParseBackend *get_singleton();

	HTTPRequest *get_http() const;
	void configure_http(Node *p_http);
	String get_application_id() const;
	void set_application_id(const String &p_app_id);
	String get_master_key() const;
	void set_master_key(const String &p_master_key);
	String get_parse_server_url() const;
	void set_parse_server_url(const String &p_parse_server_url);
	bool get_trace() const;
	void set_trace(bool p_trace);

	Ref<GdParseError> request(HTTPClient::Method p_op, const String &p_url, const Variant &p_variant, Object *p_receiver, const String &p_slot);
	Dictionary retrieve_json_reply(const PoolByteArray &p_data, HTTPRequest::Result p_status, HTTPClient::ResponseCode p_code, HTTPClient::ResponseCode p_expected_code, Ref<GdParseError> &p_error);

	/// object creation
	Ref<GdParseObject> create_object(const Dictionary &p_data = Dictionary(), const String &p_class_name = "");
	Ref<GdParseError> load_object(const String &p_object_id, Object *p_receiver, const String &p_slot = "_object_loaded");
	Ref<GdParseError> load_objects(const Ref<GdParseQuery> &p_query, Object *p_receiver, const String &p_slot = "_objects_loaded");

	/// helpers
	Dictionary from_raw_data(const Dictionary &p_data);
	static Variant jsonify(const Variant &p_data);
	static Variant objectify(const Variant &p_json);
	static Dictionary get_datetime_from_string(const String &p_iso_datetime);
	static String string_from_datetime(const Dictionary &p_datetime);
	static void debug_json(const String &p_message, const Variant &p_json);

	GdParseBackend();
	~GdParseBackend();
};

#endif // GD_PARSE_PLATFORM_H
