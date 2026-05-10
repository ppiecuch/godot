/**************************************************************************/
/*  register_types.cpp                                                    */
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

#include "register_types.h"

#include "image_frames.h"
#include "image_frames_loader.h"
#include "image_frames_loader_gif.h"

#ifdef TOOLS_ENABLED
#include "editor/import/resource_importer_animated_texture.h"
#include "editor/import/resource_importer_sprite_frames.h"
#endif

static ImageFramesLoaderGIF *image_frames_loader_gif = nullptr;

static Ref<ResourceFormatLoaderImageFrames> resource_format_image_frames;
static Ref<ResourceFormatLoaderAnimatedTexture> resource_format_animated_texture;
static Ref<ResourceFormatLoaderSpriteFrames> resource_format_sprite_frames;

void register_gif_types() {
	image_frames_loader_gif = memnew(ImageFramesLoaderGIF);
	ImageFramesLoader::add_image_frames_format_loader(image_frames_loader_gif);

	resource_format_image_frames.instance();
	ResourceLoader::add_resource_format_loader(resource_format_image_frames);

	resource_format_animated_texture.instance();
	ResourceLoader::add_resource_format_loader(resource_format_animated_texture);

	resource_format_sprite_frames.instance();
	ResourceLoader::add_resource_format_loader(resource_format_sprite_frames);

	ClassDB::register_class<ImageFrames>();

#ifdef TOOLS_ENABLED
	Ref<ResourceImporterAnimatedTexture> import_animated_texture;
	import_animated_texture.instance();
	ResourceFormatImporter::get_singleton()->add_importer(import_animated_texture);

	Ref<ResourceImporterSpriteFrames> import_sprite_frames;
	import_sprite_frames.instance();
	ResourceFormatImporter::get_singleton()->add_importer(import_sprite_frames);
#endif
}

void unregister_gif_types() {
	ImageFramesLoader::remove_image_frames_format_loader(image_frames_loader_gif);
	if (image_frames_loader_gif) {
		memdelete(image_frames_loader_gif);
	}
	ResourceLoader::remove_resource_format_loader(resource_format_image_frames);
	resource_format_image_frames.unref();

	ResourceLoader::remove_resource_format_loader(resource_format_animated_texture);
	resource_format_animated_texture.unref();

	ResourceLoader::remove_resource_format_loader(resource_format_sprite_frames);
	resource_format_sprite_frames.unref();
}
