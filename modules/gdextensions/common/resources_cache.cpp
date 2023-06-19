/**************************************************************************/
/*  resources_cache.cpp                                                   */
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

#include "resources_cache.h"

#include "common/gd_core.h"
#include "core/io/json.h"
#include "core/io/resource_loader.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"

#include <vector>

#ifdef DOCTEST
#include "doctest/doctest.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

static const String _cache_location = "user://.rescache";
static const String _catalog_location = "user://rescache.cfg";
static const uint64_t _min_disk_cache_size = 10000;

static ResCache *instance = nullptr;

static _FORCE_INLINE_ String _get_res_cache_path(const String &p_res_name) {
	return vformat("%s/%s.res", _cache_location, p_res_name);
}

void ResCache::_dump() const {
	if (!_cache.empty()) {
		print_line("Resource cache:");
		for (auto *E = _cache.front(); E; E = E->next()) {
			const String &key = E->key();
			print_line(vformat("  id: %s, catalog: %s, modified: %s, on_disk: %s, size: %d", key, _catalog.has(key), _catalog[key].modified, _catalog[key].on_disk, _catalog[key].size));
		}
	}
}

// Resources

void ResCache::load_config() {
	const String p = _catalog_location;
	if (FileAccess::exists(p)) {
#ifdef DEBUG_ENABLED
		print_line("Trying to load resource cache file: " + p);
#endif
		Dictionary cache = load_var(p);
		max_disk_cache = cache.get("max_disk_cache", max_disk_cache);
		max_disk_cache = MAX(_min_disk_cache_size, max_disk_cache);
	}
}

RES ResCache::get_resource(const String &p_res_name) {
	_changed = true;
	if (_cache.has(p_res_name)) {
		_catalog[p_res_name].last_access_time = OS::get_singleton()->get_system_time_secs();
		return _cache.get(p_res_name);
	} else {
		ERR_FAIL_COND_V_MSG(!_catalog.has(p_res_name), RES(), "Resource not known.");
		const String res_path = _catalog.get(p_res_name).path;
		ERR_FAIL_COND_V_MSG(!ResourceLoader::exists(res_path), RES(), "Resource not found.");
		RES res = ResourceLoader::load(res_path);
		ERR_FAIL_COND_V_MSG(!res.is_valid(), RES(), "Resource not loaded.");
		_catalog[p_res_name].last_access_time = OS::get_singleton()->get_system_time_secs();
		_cache[p_res_name] = res;
		return res;
	}
}

void ResCache::set_resource(RES p_res, const String &p_res_name) {
	_cache[p_res_name] = p_res;
	_catalog[p_res_name].modified = true;
	_catalog[p_res_name].last_access_time = OS::get_singleton()->get_system_time_secs();
	const String res_path = _catalog[p_res_name].path;
	if (!res_path.empty()) {
		DirAccess::remove_path_or_error(res_path); // invalidate
		if (!FileAccess::exists(res_path)) {
			_catalog[p_res_name].path = "";
			_catalog[p_res_name].size = 0;
			_catalog[p_res_name].on_disk = false;
		}
	}
	_changed = true;
}

void ResCache::del_resource(const String &p_res_name) {
	ERR_FAIL_COND(!_catalog.has(p_res_name));
	const String res_path = _catalog[p_res_name].path;
	if (res_path.empty()) {
		_catalog.erase(p_res_name);
	} else {
		Error err = DirAccess::remove_path_or_error(res_path); // invalidate
		if (!FileAccess::exists(res_path)) {
			_catalog.erase(p_res_name);
		} else {
			WARN_PRINT(vformat("Unable to remove resource %s (%s). %s", p_res_name, res_path, err != OK ? ("Error: " + itos(err)) : "(No error detected)"));
		}
	}
	if (_cache.has(p_res_name)) {
		_cache.erase(p_res_name);
	}
	_changed = true;
}

uint64_t ResCache::get_cache_usage() const {
	uint64_t sz = 0;
	for (auto *E = _cache.front(); E; E = E->next()) {
		const String &key = E->key();
		if (_catalog.has(key)) {
			sz += _catalog[key].size;
		}
	}
	return sz;
}
bool ResCache::is_res_available(const String &p_res_name) const { return _catalog.has(p_res_name); }
bool ResCache::is_res_cached(const String &p_res_name) const { return _cache.has(p_res_name); }

uint64_t ResCache::get_max_disk_cache_size() const { return max_disk_cache; }
void ResCache::set_max_disk_cache_size(uint64_t p_size) {
	ERR_FAIL_COND(p_size < _min_disk_cache_size);
	max_disk_cache = p_size;
}

Error ResCache::sync() {
	if (!DirAccess::exists(_cache_location)) {
		DirAccess::create_for_path(_cache_location)->make_dir(_cache_location);
	}
	ERR_FAIL_COND_V(!DirAccess::exists(_cache_location), ERR_FILE_NOT_FOUND);
	// catalog by access time
	Vector<String> ids;
	for (auto *E = _catalog.front(); E; E = E->next()) {
		const String &key = E->key();
		const uint64_t access_time = E->value().last_access_time;
		if (access_time == 0 || ids.empty() || access_time <= _catalog[ids.back()].last_access_time) {
			ids.push_back(key);
		} else {
			for (int p = 0; p < ids.size(); p++) {
				if (access_time > _catalog[ids[p]].last_access_time) {
					ids.insert(p, key);
					break;
				}
			}
		}
	}
	unsigned cache_size = 0, cache_clean = 0;
	Array entries;
	for (const auto &key : ids) {
		const CacheEntry &entry = _catalog[key];
		if (cache_size > max_disk_cache) {
			cache_clean += _catalog[key].size;
			const String res_path = _catalog[key].path;
			if (!res_path.empty()) {
				Error err = DirAccess::remove_path_or_error(res_path); // invalidate
				if (!FileAccess::exists(res_path)) {
					if (!_cache.has(key)) {
						_catalog.erase(key); // gone forever
					} else {
						// keep the record in case the entry
						// would fit into cache later
						_catalog[key].path = "";
						_catalog[key].size = 0;
						_catalog[key].modified = true;
						_catalog[key].on_disk = false;
					}
				} else {
					// keep entry if file still exists
					entries.append(make_dict("name", key, "path", entry.path, "last_access_time", entry.last_access_time, "size", entry.size));
					WARN_PRINT(vformat("Unable to clean resource %s. %s", key, err != OK ? ("Error: " + itos(err)) : "(No error detected)"));
				}
			}
			break;
		}
		if (_catalog[key].modified) {
			ERR_CONTINUE(!_cache.has(key));
			const RES &res = _cache[key];
			ERR_CONTINUE(!res.is_valid());
			const String res_path = _get_res_cache_path(key);
			if (ResourceSaver::save(res_path, res) == OK) {
				_catalog[key].path = res_path;
				_catalog[key].size = gd_file_size(res_path);
				_catalog[key].modified = false;
				_catalog[key].on_disk = true;
			}
		}
		cache_size += _catalog[key].size;
		ERR_CONTINUE(entry.path.empty()); // res. not saved
		ERR_CONTINUE(entry.modified); // res. not synchronized
		entries.append(make_dict("name", key, "path", entry.path, "last_access_time", entry.last_access_time, "size", entry.size));
	}
	if (cache_size > max_disk_cache) {
		emit_signal("cache_full");
	}
	print_verbose("Synced " + itos(cache_size) + " bytes of cached resources.");
	if (cache_clean) {
		print_verbose("Cleaned " + itos(cache_clean) + " bytes of cached resources.");
	}
	Dictionary dict;
	dict["max_disk_cache"] = max_disk_cache;
	dict["entries"] = entries;
	return store_var(dict, _catalog_location);
}

void ResCache::clear() {
	_cache.clear();
}

void ResCache::purge() {
	if (DirAccess::exists(_cache_location)) {
		if (DirAccessRef da = DirAccess::open(_cache_location)) {
			print_verbose("Erase content of " + _cache_location + " ..");
			da->erase_contents_recursive();
			da->remove(_cache_location);
			da->remove(_catalog_location);
		}
	}
}

ResCache *ResCache::get_singleton() {
	return instance;
}

void ResCache::_notification(int p_what) {
	if (p_what == NOTIFICATION_PREDELETE) {
		if (_changed) {
			_changed = sync() != OK;
		}
#if DEBUG_ENABLED
		_dump();
#endif
	}
}

void ResCache::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_resource", "name"), &ResCache::get_resource);
	ClassDB::bind_method(D_METHOD("set_resource", "res", "name"), &ResCache::set_resource);
	ClassDB::bind_method(D_METHOD("set_max_disk_cache_size", "max_size"), &ResCache::set_max_disk_cache_size);
	ClassDB::bind_method(D_METHOD("get_max_disk_cache_size"), &ResCache::get_max_disk_cache_size);
	ClassDB::bind_method(D_METHOD("sync"), &ResCache::sync);
	ClassDB::bind_method(D_METHOD("clear"), &ResCache::clear);
	ClassDB::bind_method(D_METHOD("purge"), &ResCache::purge);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_disk_cache"), "set_max_disk_cache_size", "get_max_disk_cache_size");

	ADD_SIGNAL(MethodInfo("cache_full"));
}

ResCache::ResCache() {
	ERR_FAIL_COND_MSG(instance != nullptr, "Singleton already exists");
	instance = this;
	max_disk_cache = 100000;
	_changed = false;
	load_config();
}

ResCache::~ResCache() {
	if (_changed) {
		sync();
	}
	instance = nullptr;
}

#ifdef DOCTEST

#include "core/image.h"
#include "scene/resources/texture.h"

TEST_CASE("Disk cache") {
	static const int test_data_size = 80;
	static const uint8_t test_data[80] = { // png, 32x32
		0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52,
		0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x08, 0x00, 0x00, 0x00, 0x00, 0x56, 0x11, 0x25,
		0x28, 0x00, 0x00, 0x00, 0x17, 0x49, 0x44, 0x41, 0x54, 0x38, 0xCB, 0x63, 0xFC, 0xCF, 0x80, 0x1F,
		0x30, 0x8E, 0x2A, 0x18, 0x55, 0x30, 0xAA, 0x60, 0xA4, 0x2A, 0x00, 0x00, 0xF8, 0x2D, 0x20, 0x01,
		0x4F, 0x2A, 0xA0, 0xAE, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
	};
	Ref<Image> image = memnew(Image(test_data, test_data_size));
	Ref<ImageTexture> texture = memnew(ImageTexture);
	texture->create_from_image(image);

	ResCache *res_cache = ResCache::get_singleton();

	SUBCASE("check resource access before sync") {
		res_cache->set_resource(texture, "__t1__");
		REQUIRE(res_cache->is_res_cached("__t1__"));
		REQUIRE(res_cache->is_res_available("__t1__"));
		res_cache->del_resource("__t1__");
		REQUIRE(!res_cache->is_res_cached("__t1__"));
		REQUIRE(!res_cache->is_res_available("__t1__"));
	}
	SUBCASE("check resource access after sync") {
		res_cache->set_resource(texture, "__t1__");
		REQUIRE(res_cache->is_res_cached("__t1__"));
		REQUIRE(res_cache->is_res_available("__t1__"));
		res_cache->sync();
		res_cache->del_resource("__t1__");
		REQUIRE(!res_cache->is_res_cached("__t1__"));
		REQUIRE(!res_cache->is_res_available("__t1__"));
	}
	SUBCASE("check disk cache purge") {
		res_cache->purge();
		REQUIRE(!DirAccess::exists(_cache_location));
	}
	SUBCASE("check disk cache limit") {
		res_cache->set_max_disk_cache_size(112233);
		REQUIRE(res_cache->get_max_disk_cache_size() == 112233);
	}
	SUBCASE("cleanup") {
		memdelete(res_cache);
	}
}
#endif
