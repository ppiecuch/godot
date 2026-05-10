/**************************************************************************/
/*  register_image_types.cpp                                              */
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

#include "register_image_types.h"

#include "core/engine.h"

#include "drivers/png/image_loader_indexed_png.h"
#include "drivers/png/resource_saver_indexed_png.h"

#include "classes_enabled.gen.h"

static _GoostImage *_goost_image = nullptr;
static ImageLoaderIndexedPNG *image_loader_indexed_png;
static Ref<ResourceSaverIndexedPNG> resource_saver_indexed_png;

namespace goost {

void register_image_types() {
#ifdef GOOST_GoostImage
	_goost_image = memnew(_GoostImage);
	ClassDB::register_class<_GoostImage>();
	Engine::get_singleton()->add_singleton(Engine::Singleton("GoostImage", _GoostImage::get_singleton()));
#endif
#ifdef GOOST_ImageIndexed
	ClassDB::register_class<ImageIndexed>();

	image_loader_indexed_png = memnew(ImageLoaderIndexedPNG);
	ImageLoader::add_image_format_loader(image_loader_indexed_png);

	resource_saver_indexed_png.instance();
	ResourceSaver::add_resource_format_saver(resource_saver_indexed_png);
#endif
	ClassDB::register_class<ImageBlender>();
}

void unregister_image_types() {
#ifdef GOOST_GoostImage
	memdelete(_goost_image);
#endif
#ifdef GOOST_ImageIndexed
	if (image_loader_indexed_png) {
		memdelete(image_loader_indexed_png);
	}
	ResourceSaver::remove_resource_format_saver(resource_saver_indexed_png);
	resource_saver_indexed_png.unref();
#endif
}

} // namespace goost
