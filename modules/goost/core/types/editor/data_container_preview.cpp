/**************************************************************************/
/*  data_container_preview.cpp                                            */
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

#include "data_container_preview.h"
#include "editor/editor_node.h"

bool DataContainerPreviewGenerator::handles(const String &p_type) const {
	return p_type == "DataContainer";
}

Ref<Texture> DataContainerPreviewGenerator::generate(const Ref<Resource> &p_from, const Size2 &p_size) const {
	Ref<DataContainer> container = p_from;
	ERR_FAIL_COND_V_MSG(container.is_null(), Ref<DataContainer>(), "Invalid reference to a DataContainer object.");

	const Variant &value = container->get_value();

	Ref<Texture> tex;

	switch (value.get_type()) {
		case Variant::NIL: {
			tex = EditorNode::get_singleton()->get_class_icon("Variant");
		} break;
		case Variant::BOOL: {
			bool b = value;
			String builtin_name = Variant::get_type_name(value.get_type());
			String name = b ? "GuiChecked" : "GuiUnchecked";
			tex = EditorNode::get_singleton()->get_class_icon(name, builtin_name);
		} break;
		case Variant::COLOR: {
			Color color = value;
			Ref<Image> image;
			image.instance();
			// The icon is stretched in grid mode so it's possible to speed up
			// generation in those cases, but won't work in tree mode (default).
			// Therefore, we must still use `p_size` for the preview...
			image->create(p_size.x, p_size.y, false, Image::FORMAT_RGBA8);
			image->fill(color);
			Ref<ImageTexture> img_tex;
			img_tex.instance();
			img_tex->create_from_image(image, 0);
			tex = img_tex;
		} break;
		case Variant::OBJECT: {
			Object *obj = value;
			Ref<Texture> obj_tex = Ref<Texture>(Object::cast_to<Texture>(obj));
			if (obj_tex.is_valid()) {
				Ref<Image> image = obj_tex->get_data();
				image->resize(p_size.x, p_size.y);
				Ref<ImageTexture> img_tex;
				img_tex.instance();
				img_tex->create_from_image(image, 0);
				tex = img_tex;
			}
		} break;
		default: {
			String name = Variant::get_type_name(value.get_type());
			tex = EditorNode::get_singleton()->get_class_icon(name, "Resource");
		};
	}
	return tex;
}
