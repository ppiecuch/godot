/**************************************************************************/
/*  gd_flashdb.cpp                                                        */
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

#include "gd_flashdb.h"

#include "core/io/json.h"
#include "core/os/dir_access.h"
#include "core/os/os.h"
#include "core/project_settings.h"

extern "C" {
#include "flashdb.h"
}

// =========================================================================
// FlashKVDB — Key-Value Database
// =========================================================================

FlashKVDB::FlashKVDB() :
		_db(nullptr), _initialized(false) {
	_db = (struct fdb_kvdb *)memalloc(sizeof(struct fdb_kvdb));
	memset(_db, 0, sizeof(struct fdb_kvdb));
}

FlashKVDB::~FlashKVDB() {
	close();
	if (_db) {
		memfree(_db);
		_db = nullptr;
	}
}

bool FlashKVDB::open(const String &p_name, const String &p_path, int p_sector_size, int p_max_size) {
	ERR_FAIL_COND_V(_initialized, false);
	ERR_FAIL_COND_V(p_name.empty(), false);

	_name = p_name;
	_path = ProjectSettings::get_singleton()->globalize_path(p_path);

	if (!_path.empty()) {
		DirAccess *da = DirAccess::create_for_path(_path);
		if (da) {
			da->make_dir_recursive(_path);
			memdelete(da);
		}
	}

	if (p_max_size <= 0) {
		p_max_size = p_sector_size * 16;
	}

	bool file_mode = true;
	fdb_kvdb_control(_db, FDB_KVDB_CTRL_SET_SEC_SIZE, &p_sector_size);
	fdb_kvdb_control(_db, FDB_KVDB_CTRL_SET_FILE_MODE, &file_mode);
	fdb_kvdb_control(_db, FDB_KVDB_CTRL_SET_MAX_SIZE, &p_max_size);

	_name_utf8 = p_name.utf8();
	_path_utf8 = _path.utf8();

	fdb_err_t err = fdb_kvdb_init(_db, _name_utf8.get_data(), _path_utf8.get_data(), nullptr, nullptr);
	if (err != FDB_NO_ERR) {
		ERR_PRINT(vformat("FlashKVDB: init failed for '%s' at '%s', error %d", p_name, p_path, (int)err));
		return false;
	}
	_initialized = true;
	return true;
}

void FlashKVDB::close() {
	if (_initialized && _db) {
		fdb_kvdb_deinit(_db);
		_initialized = false;
	}
}

bool FlashKVDB::is_open() const { return _initialized; }

bool FlashKVDB::set_string(const String &p_key, const String &p_value) {
	ERR_FAIL_COND_V(!_initialized, false);
	CharString key = p_key.utf8();
	CharString val = p_value.utf8();
	return fdb_kv_set(_db, key.get_data(), val.get_data()) == FDB_NO_ERR;
}

String FlashKVDB::get_string(const String &p_key, const String &p_default) const {
	ERR_FAIL_COND_V(!_initialized, p_default);
	CharString key = p_key.utf8();
	char *val = fdb_kv_get(_db, key.get_data());
	if (!val)
		return p_default;
	return String::utf8(val);
}

bool FlashKVDB::set_data(const String &p_key, const PoolByteArray &p_data) {
	ERR_FAIL_COND_V(!_initialized, false);
	CharString key = p_key.utf8();
	PoolByteArray::Read r = p_data.read();
	struct fdb_blob blob;
	fdb_blob_make(&blob, (void *)r.ptr(), p_data.size());
	return fdb_kv_set_blob(_db, key.get_data(), &blob) == FDB_NO_ERR;
}

PoolByteArray FlashKVDB::get_data(const String &p_key) const {
	PoolByteArray result;
	ERR_FAIL_COND_V(!_initialized, result);
	CharString key = p_key.utf8();
	struct fdb_kv kv;
	struct fdb_blob blob;
	fdb_kv_get_obj(_db, key.get_data(), &kv);
	if (kv.value_len == 0)
		return result;
	result.resize(kv.value_len);
	{
		PoolByteArray::Write w = result.write();
		fdb_blob_make(&blob, w.ptr(), kv.value_len);
		fdb_kv_get_blob(_db, key.get_data(), &blob);
	}
	return result;
}

bool FlashKVDB::set_value(const String &p_key, const Variant &p_value) {
	return set_string(p_key, JSON::print(p_value));
}

Variant FlashKVDB::get_value(const String &p_key, const Variant &p_default) const {
	String json = get_string(p_key, "");
	if (json.empty())
		return p_default;
	Variant result;
	String err;
	int line;
	if (JSON::parse(json, result, err, line) == OK)
		return result;
	return p_default;
}

bool FlashKVDB::delete_key(const String &p_key) {
	ERR_FAIL_COND_V(!_initialized, false);
	return fdb_kv_del(_db, p_key.utf8().get_data()) == FDB_NO_ERR;
}

bool FlashKVDB::has_key(const String &p_key) const {
	ERR_FAIL_COND_V(!_initialized, false);
	return fdb_kv_get(_db, p_key.utf8().get_data()) != nullptr;
}

Array FlashKVDB::get_all_keys() const {
	Array result;
	ERR_FAIL_COND_V(!_initialized, result);
	struct fdb_kv_iterator iter;
	fdb_kv_iterator_init(&iter);
	while (fdb_kv_iterate(_db, &iter)) {
		result.push_back(String::utf8(iter.curr_kv.name));
	}
	return result;
}

int FlashKVDB::get_key_count() const {
	ERR_FAIL_COND_V(!_initialized, 0);
	int count = 0;
	struct fdb_kv_iterator iter;
	fdb_kv_iterator_init(&iter);
	while (fdb_kv_iterate(_db, &iter))
		count++;
	return count;
}

void FlashKVDB::reset_defaults() {
	ERR_FAIL_COND(!_initialized);
	fdb_kv_set_default(_db);
}

void FlashKVDB::_bind_methods() {
	ClassDB::bind_method(D_METHOD("open", "name", "path", "sector_size", "max_size"), &FlashKVDB::open, DEFVAL(4096), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("close"), &FlashKVDB::close);
	ClassDB::bind_method(D_METHOD("is_open"), &FlashKVDB::is_open);
	ClassDB::bind_method(D_METHOD("set_string", "key", "value"), &FlashKVDB::set_string);
	ClassDB::bind_method(D_METHOD("get_string", "key", "default"), &FlashKVDB::get_string, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("set_data", "key", "data"), &FlashKVDB::set_data);
	ClassDB::bind_method(D_METHOD("get_data", "key"), &FlashKVDB::get_data);
	ClassDB::bind_method(D_METHOD("set_value", "key", "value"), &FlashKVDB::set_value);
	ClassDB::bind_method(D_METHOD("get_value", "key", "default"), &FlashKVDB::get_value, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("delete_key", "key"), &FlashKVDB::delete_key);
	ClassDB::bind_method(D_METHOD("has_key", "key"), &FlashKVDB::has_key);
	ClassDB::bind_method(D_METHOD("get_all_keys"), &FlashKVDB::get_all_keys);
	ClassDB::bind_method(D_METHOD("get_key_count"), &FlashKVDB::get_key_count);
	ClassDB::bind_method(D_METHOD("reset_defaults"), &FlashKVDB::reset_defaults);
}

// =========================================================================
// FlashTSDB — Time Series Database
// =========================================================================

static fdb_time_t _get_time() {
	return (fdb_time_t)(OS::get_singleton()->get_unix_time());
}

static bool _tsl_count_cb(fdb_tsl_t tsl, void *arg) {
	(*(int *)arg)++;
	return false;
}

struct _TSLCollectCtx {
	Array *out;
};

static bool _tsl_collect_cb(fdb_tsl_t tsl, void *arg) {
	_TSLCollectCtx *ctx = (_TSLCollectCtx *)arg;
	Dictionary d;
	d["time"] = (int64_t)tsl->time;
	d["status"] = (int)tsl->status;
	d["length"] = (int)tsl->log_len;
	ctx->out->push_back(d);
	return false;
}

FlashTSDB::FlashTSDB() :
		_db(nullptr), _initialized(false) {
	_db = (struct fdb_tsdb *)memalloc(sizeof(struct fdb_tsdb));
	memset(_db, 0, sizeof(struct fdb_tsdb));
}

FlashTSDB::~FlashTSDB() {
	close();
	if (_db) {
		memfree(_db);
		_db = nullptr;
	}
}

bool FlashTSDB::open(const String &p_name, const String &p_path, int p_max_len, int p_sector_size, int p_max_size) {
	ERR_FAIL_COND_V(_initialized, false);
	_name = p_name;
	_path = ProjectSettings::get_singleton()->globalize_path(p_path);

	DirAccess *da = DirAccess::create_for_path(_path);
	if (da) {
		da->make_dir_recursive(_path);
		memdelete(da);
	}

	if (p_max_size <= 0) {
		p_max_size = p_sector_size * 16;
	}

	bool file_mode = true;
	fdb_tsdb_control(_db, FDB_TSDB_CTRL_SET_SEC_SIZE, &p_sector_size);
	fdb_tsdb_control(_db, FDB_TSDB_CTRL_SET_FILE_MODE, &file_mode);
	fdb_tsdb_control(_db, FDB_TSDB_CTRL_SET_MAX_SIZE, &p_max_size);

	_name_utf8 = p_name.utf8();
	_path_utf8 = _path.utf8();

	fdb_err_t err = fdb_tsdb_init(_db, _name_utf8.get_data(), _path_utf8.get_data(), _get_time, p_max_len, nullptr);
	if (err != FDB_NO_ERR) {
		ERR_PRINT(vformat("FlashTSDB: init failed for '%s', error %d", p_name, (int)err));
		return false;
	}
	_initialized = true;
	return true;
}

void FlashTSDB::close() {
	if (_initialized && _db) {
		fdb_tsdb_deinit(_db);
		_initialized = false;
	}
}

bool FlashTSDB::is_open() const { return _initialized; }

bool FlashTSDB::append_string(const String &p_data) {
	ERR_FAIL_COND_V(!_initialized, false);
	CharString utf8 = p_data.utf8();
	struct fdb_blob blob;
	fdb_blob_make(&blob, (void *)utf8.get_data(), utf8.length());
	return fdb_tsl_append(_db, &blob) == FDB_NO_ERR;
}

bool FlashTSDB::append_data(const PoolByteArray &p_data) {
	ERR_FAIL_COND_V(!_initialized, false);
	PoolByteArray::Read r = p_data.read();
	struct fdb_blob blob;
	fdb_blob_make(&blob, (void *)r.ptr(), p_data.size());
	return fdb_tsl_append(_db, &blob) == FDB_NO_ERR;
}

bool FlashTSDB::append_value(const Variant &p_value) {
	return append_string(JSON::print(p_value));
}

Array FlashTSDB::get_all_records() const {
	Array result;
	ERR_FAIL_COND_V(!_initialized, result);
	_TSLCollectCtx ctx;
	ctx.out = &result;
	fdb_tsl_iter(_db, _tsl_collect_cb, &ctx);
	return result;
}

Array FlashTSDB::get_records_by_time(int64_t p_from, int64_t p_to) const {
	Array result;
	ERR_FAIL_COND_V(!_initialized, result);
	_TSLCollectCtx ctx;
	ctx.out = &result;
	fdb_tsl_iter_by_time(_db, (fdb_time_t)p_from, (fdb_time_t)p_to, _tsl_collect_cb, &ctx);
	return result;
}

int FlashTSDB::get_record_count() const {
	ERR_FAIL_COND_V(!_initialized, 0);
	int count = 0;
	fdb_tsl_iter(_db, _tsl_count_cb, &count);
	return count;
}

void FlashTSDB::clean() {
	ERR_FAIL_COND(!_initialized);
	fdb_tsl_clean(_db);
}

void FlashTSDB::set_rollover(bool p_enable) {
	ERR_FAIL_COND(!_initialized);
	fdb_tsdb_control(_db, FDB_TSDB_CTRL_SET_ROLLOVER, &p_enable);
}

void FlashTSDB::_bind_methods() {
	ClassDB::bind_method(D_METHOD("open", "name", "path", "max_len", "sector_size", "max_size"), &FlashTSDB::open, DEFVAL(256), DEFVAL(4096), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("close"), &FlashTSDB::close);
	ClassDB::bind_method(D_METHOD("is_open"), &FlashTSDB::is_open);
	ClassDB::bind_method(D_METHOD("append_string", "data"), &FlashTSDB::append_string);
	ClassDB::bind_method(D_METHOD("append_data", "data"), &FlashTSDB::append_data);
	ClassDB::bind_method(D_METHOD("append_value", "value"), &FlashTSDB::append_value);
	ClassDB::bind_method(D_METHOD("get_all_records"), &FlashTSDB::get_all_records);
	ClassDB::bind_method(D_METHOD("get_records_by_time", "from", "to"), &FlashTSDB::get_records_by_time);
	ClassDB::bind_method(D_METHOD("get_record_count"), &FlashTSDB::get_record_count);
	ClassDB::bind_method(D_METHOD("clean"), &FlashTSDB::clean);
	ClassDB::bind_method(D_METHOD("set_rollover", "enable"), &FlashTSDB::set_rollover);
}

// =========================================================================
// Tests
// =========================================================================

#ifdef DOCTEST
#include "doctest/doctest.h"
#include "doctest/doctest_godot.h"

static int _test_counter = 0;

static String _test_path(const char *name) {
	// Use counter + ticks to guarantee unique paths across tests and runs
	return vformat("user://flashdb_test_%s_%d_%d", name, ++_test_counter, OS::get_singleton()->get_ticks_msec());
}

// FlashDB logs sector-format messages during first open; suppress in tests.
static bool _test_open_kv(FlashKVDB &db, const char *name, const String &path,
		int sector_size = 4096, int max_size = 0) {
	bool ok;
	SUPPRESS_OUTPUT(ok = db.open(name, path, sector_size, max_size));
	return ok;
}

static bool _test_open_ts(FlashTSDB &db, const char *name, const String &path,
		int max_len = 128) {
	bool ok;
	SUPPRESS_OUTPUT(ok = db.open(name, path, max_len));
	return ok;
}

static void _test_cleanup(const String &path) {
	String abs_path = ProjectSettings::get_singleton()->globalize_path(path);
	DirAccess *da = DirAccess::open(abs_path);
	if (da) {
		da->list_dir_begin();
		String f;
		while ((f = da->get_next()) != "") {
			if (!da->current_is_dir()) {
				da->remove(abs_path.plus_file(f));
			}
		}
		da->list_dir_end();
		da->remove(abs_path);
		memdelete(da);
	}
}

TEST_SUITE("[[flashdb]] FlashKVDB") {
	TEST_CASE("[flashdb] default state") {
		FlashKVDB db;
		CHECK_FALSE(db.is_open());
		EXPECT_ERROR(CHECK(db.get_key_count() == 0)); // expected: !_initialized
	}

	TEST_CASE("[flashdb] open and close") {
		FlashKVDB db;
		CHECK(_test_open_kv(db, "test", _test_path("kv_open")));
		CHECK(db.is_open());
		db.close();
		CHECK_FALSE(db.is_open());
	}

	TEST_CASE("[flashdb] set and get string") {
		FlashKVDB db;
		REQUIRE(_test_open_kv(db, "test", _test_path("kv_str")));
		CHECK(db.set_string("name", "Godot"));
		CHECK(db.get_string("name") == "Godot");
		CHECK(db.get_string("missing", "fallback") == "fallback");
		db.close();
	}

	TEST_CASE("[flashdb] overwrite string") {
		FlashKVDB db;
		REQUIRE(_test_open_kv(db, "test", _test_path("kv_ow")));
		db.set_string("k", "first");
		CHECK(db.get_string("k") == "first");
		db.set_string("k", "second");
		CHECK(db.get_string("k") == "second");
		db.close();
	}

	TEST_CASE("[flashdb] delete key") {
		FlashKVDB db;
		REQUIRE(_test_open_kv(db, "test", _test_path("kv_del")));
		db.set_string("temp", "val");
		CHECK(db.has_key("temp"));
		CHECK(db.delete_key("temp"));
		CHECK_FALSE(db.has_key("temp"));
		db.close();
	}

	TEST_CASE("[flashdb] binary data roundtrip") {
		FlashKVDB db;
		REQUIRE(_test_open_kv(db, "test", _test_path("kv_bin")));
		PoolByteArray data;
		data.resize(4);
		{
			PoolByteArray::Write w = data.write();
			w[0] = 0xDE;
			w[1] = 0xAD;
			w[2] = 0xBE;
			w[3] = 0xEF;
		}
		CHECK(db.set_data("bin", data));
		PoolByteArray out = db.get_data("bin");
		REQUIRE(out.size() == 4);
		{
			PoolByteArray::Read r = out.read();
			CHECK(r[0] == 0xDE);
			CHECK(r[3] == 0xEF);
		}
		db.close();
	}

	TEST_CASE("[flashdb] variant int roundtrip") {
		FlashKVDB db;
		REQUIRE(_test_open_kv(db, "test", _test_path("kv_vi")));
		db.set_value("n", 42);
		CHECK((int)db.get_value("n") == 42);
		db.close();
	}

	TEST_CASE("[flashdb] variant string roundtrip") {
		FlashKVDB db;
		REQUIRE(_test_open_kv(db, "test", _test_path("kv_vs")));
		db.set_value("s", "hello");
		CHECK(String(db.get_value("s")) == "hello");
		db.close();
	}

	TEST_CASE("[flashdb] get_value default for missing") {
		FlashKVDB db;
		REQUIRE(_test_open_kv(db, "test", _test_path("kv_def")));
		CHECK(db.get_value("x", 99) == Variant(99));
		db.close();
	}

	TEST_CASE("[flashdb] get_all_keys") {
		FlashKVDB db;
		REQUIRE(_test_open_kv(db, "test", _test_path("kv_keys")));
		db.set_string("a", "1");
		db.set_string("b", "2");
		CHECK(db.get_all_keys().size() == 2);
		db.close();
	}

	TEST_CASE("[flashdb] get_key_count") {
		FlashKVDB db;
		String path = _test_path("kv_cnt");
		REQUIRE(_test_open_kv(db, "kv_cnt", path));
		CHECK(db.get_key_count() == 0);
		db.set_string("x", "1");
		CHECK(db.get_key_count() == 1);
		db.close();
		_test_cleanup(path);
	}

	TEST_CASE("[flashdb] ops on closed db are safe") {
		FlashKVDB db;
		EXPECT_ERROR({
			CHECK_FALSE(db.set_string("k", "v")); // expected: !_initialized
			CHECK(db.get_string("k", "d") == "d");
			CHECK_FALSE(db.delete_key("k"));
			CHECK_FALSE(db.has_key("k"));
		});
	}

	TEST_CASE("[flashdb] empty name rejected") {
		FlashKVDB db;
		EXPECT_ERROR(CHECK_FALSE(db.open("", _test_path("kv_empty")))); // expected: p_name.empty()
	}

	TEST_CASE("[flashdb] double open rejected") {
		FlashKVDB db;
		REQUIRE(_test_open_kv(db, "test", _test_path("kv_dbl")));
		EXPECT_ERROR(CHECK_FALSE(db.open("test2", _test_path("kv_dbl2")))); // expected: _initialized
		db.close();
	}
}

TEST_SUITE("[[flashdb]] FlashTSDB") {
	TEST_CASE("[flashdb] default state") {
		FlashTSDB db;
		CHECK_FALSE(db.is_open());
		EXPECT_ERROR(CHECK(db.get_record_count() == 0)); // expected: !_initialized
	}

	TEST_CASE("[flashdb] open and close") {
		FlashTSDB db;
		CHECK(_test_open_ts(db, "ts", _test_path("ts_open")));
		CHECK(db.is_open());
		db.close();
		CHECK_FALSE(db.is_open());
	}

	TEST_CASE("[flashdb] append and count") {
		FlashTSDB db;
		String path = _test_path("ts_app");
		REQUIRE(_test_open_ts(db, "ts_app", path, 128));
		CHECK(db.append_string("ev1"));
		CHECK(db.append_string("ev2"));
		CHECK(db.get_record_count() == 2);
		db.close();
		_test_cleanup(path);
	}

	TEST_CASE("[flashdb] append binary") {
		FlashTSDB db;
		String path = _test_path("ts_bin");
		REQUIRE(_test_open_ts(db, "ts_bin", path, 64));
		PoolByteArray data;
		data.resize(4);
		{
			PoolByteArray::Write w = data.write();
			w[0] = 1;
			w[1] = 2;
			w[2] = 3;
			w[3] = 4;
		}
		CHECK(db.append_data(data));
		CHECK(db.get_record_count() == 1);
		db.close();
		_test_cleanup(path);
	}

	TEST_CASE("[flashdb] append variant") {
		FlashTSDB db;
		String path = _test_path("ts_var");
		REQUIRE(_test_open_ts(db, "ts_var", path, 256));
		CHECK(db.append_value(42));
		CHECK(db.append_value("msg"));
		CHECK(db.get_record_count() == 2);
		db.close();
		_test_cleanup(path);
	}

	TEST_CASE("[flashdb] records have timestamp") {
		FlashTSDB db;
		String path = _test_path("ts_time");
		REQUIRE(_test_open_ts(db, "ts_time", path, 128));
		db.append_string("log");
		Array recs = db.get_all_records();
		REQUIRE(recs.size() == 1);
		Dictionary r = recs[0];
		CHECK(r.has("time"));
		CHECK(r.has("status"));
		CHECK((int64_t)r["time"] > 0);
		db.close();
		_test_cleanup(path);
	}

	TEST_CASE("[flashdb] clean removes all") {
		FlashTSDB db;
		String path = _test_path("ts_clean");
		REQUIRE(_test_open_ts(db, "ts_clean", path, 128));
		db.append_string("a");
		db.append_string("b");
		CHECK(db.get_record_count() == 2);
		SUPPRESS_OUTPUT(db.clean()); // sector reformat logs
		CHECK(db.get_record_count() == 0);
		db.close();
		_test_cleanup(path);
	}

	TEST_CASE("[flashdb] ops on closed db are safe") {
		FlashTSDB db;
		EXPECT_ERROR({
			CHECK_FALSE(db.append_string("x")); // expected: !_initialized
			CHECK(db.get_record_count() == 0);
			CHECK(db.get_all_records().size() == 0);
		});
	}
}

#endif // DOCTEST
