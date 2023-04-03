/**************************************************************************/
/*  resources_cchecpp                                                     */
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

#include "core/io/json.h"
#include "core/io/resource_loader.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"
#include "common/gd_core.h"

#include <vector>

static const String _cache_location = "user://.rescache";
static const String _catalog_location = "user://rescache.cfg";

static ResCache *instance = nullptr;

static _FORCE_INLINE_ String _get_res_cache_path(const String &p_res_name) {
	return vformat("%s/%s.res", _cache_location, p_res_name);
}

void ResCache::_dump() const {
	print_line("Resource cache:");
	for (auto *E = _cache.front(); E; E = E->next()) {
		const String &key = E->key();
		print_line(vformat("  id: %s, catalog: %s, modified: %s, size: %d", key, _catalog.has(key), _catalog[key].modified, _catalog[key].size));
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
		max_size = cache.get("max_size", max_size);
	}
}

RES ResCache::get_resource(const String &p_res_name) {
	if (_cache.has(p_res_name)) {
		_catalog[p_res_name].last_access_time = OS::get_singleton()->get_system_time_secs();
		return _cache.get(p_res_name);
	} else {
		ERR_FAIL_COND_V_MSG(!_catalog.has(p_res_name), RES(), "Resource not known.");
		const String res_path = _catalog.get(p_res_name).path;
		ERR_FAIL_COND_V_MSG(!ResourceLoader::exists(res_path), RES(), "Resource not found.");
		RES res =  ResourceLoader::load(res_path);
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
	if (!res_path.empty() && FileAccess::exists(res_path)) {
		DirAccess::remove_file_or_error(res_path); // invalidate
	}
}

size_t ResCache::get_cache_size() const {
	size_t sz = 0;
	for (auto *E = _cache.front(); E; E = E->next()) {
		const String &key = E->key();
		if (_catalog.has(key)) {
			sz += _catalog[key].size;
		}
	}
	return sz;
}
size_t ResCache::get_cache_size_limit() const { return max_size; }
bool ResCache::is_res_available(const String &p_res_name) const { return _catalog.has(p_res_name); }
bool ResCache::is_res_cached(const String &p_res_name) const { return _cache.has(p_res_name); }

Error ResCache::sync() {
	Dictionary dict;
	Array entries;
	dict["max_size"] = max_size;
	for (auto *E = _catalog.front(); E; E = E->next()) {
		const String &key = E->key();
		const CacheEntry &entry = E->value();
		ERR_CONTINUE(entry.path.empty());
		ERR_CONTINUE(entry.modified);
		entries.append(helper::dict("name", key, "path", entry.path, "last_access_time", entry.last_access_time, "size", entry.size));
	}
	dict["entries"] = entries;
    return store_var(dict, _catalog_location);
}

Error ResCache::save() {
	if (!DirAccess::exists(_cache_location)) {
		DirAccess::create_for_path(_cache_location)->make_dir(_cache_location);
	}
	ERR_FAIL_COND_V(!DirAccess::exists(_cache_location), ERR_FILE_NOT_FOUND);
	for (auto *E = _cache.front(); E; E = E->next()) {
		const String &key = E->key();
		ERR_CONTINUE(!_catalog.has(key));
		const RES &res = E->value();
		ERR_CONTINUE(!res.is_valid());
		if (_catalog[key].modified) {
			const String res_path = _get_res_cache_path(key);
			if (ResourceSaver::save(res_path, res) == OK) {
				_catalog[key].path = res_path;
				_catalog[key].size = gd_file_size(res_path);
				_catalog[key].modified = false;
			}
		}
	}
	sync();
	return OK;
}

void ResCache::purge() {
	_cache.clear();
}

ResCache *ResCache::get_singleton() {
	return instance;
}

void ResCache::_notification(int p_what) {
	if (p_what == NOTIFICATION_PREDELETE) {
		save();
#if DEBUG_ENABLED
		_dump();
#endif
	}
}

void ResCache::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_resource", "name"), &ResCache::get_resource);
	ClassDB::bind_method(D_METHOD("set_resource", "res", "name"), &ResCache::set_resource);
	ClassDB::bind_method(D_METHOD("sync"), &ResCache::sync);
	ClassDB::bind_method(D_METHOD("save"), &ResCache::save);
	ClassDB::bind_method(D_METHOD("purge"), &ResCache::purge);
}

ResCache::ResCache() {
	ERR_FAIL_COND_MSG(instance != nullptr, "Singleton already exists");
	instance = this;
	max_size = 100000;
	load_config();
}

ResCache::~ResCache() {
	save();
	instance = nullptr;
}
