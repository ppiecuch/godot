/**************************************************************************/
/*  resources_cache.h                                                     */
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

#ifndef RESOURCES_CACHE_H
#define RESOURCES_CACHE_H

#include "core/map.h"
#include "core/reference.h"
#include "core/resource.h"
#include "core/variant.h"

class ResCache : public Object {
	GDCLASS(ResCache, Object);

	struct CacheEntry {
		String path;
		uint64_t size = 0;
		uint64_t last_access_time = 0;
		bool modified = false;
		bool on_disk = false;
	};

	uint64_t max_disk_cache;

	Map<String, RES> _cache;
	Map<String, CacheEntry> _catalog;
	bool _changed;

	void _dump() const;
	void load_config();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	static ResCache *get_singleton();

	RES get_resource(const String &p_res_name);
	void set_resource(RES p_res, const String &p_res_name);
	void del_resource(const String &p_res_name);

	uint64_t get_cache_usage() const;
	bool is_res_available(const String &p_res_name) const;
	bool is_res_cached(const String &p_res_name) const;

	void set_max_disk_cache_size(uint64_t p_size);
	uint64_t get_max_disk_cache_size() const;

	Error sync();
	void clear();
	void purge();

	ResCache();
	~ResCache();
};

#define _CACHE_GET(N) ResCache::get_singleton()->get_resource(N)
#define _CACHE_ADD(N, R) ResCache::get_singleton()->set_resource(R, N)
#define _CACHE_DEL(N) ResCache::get_singleton()->del_resource(N)
#define _CACHE_HAS(N) ResCache::get_singleton()->is_res_available(N)

#endif // RESOURCES_CACHE_H
