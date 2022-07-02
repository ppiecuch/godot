/*************************************************************************/
/*  resource_importer_flac.cpp                                           */
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

#include "resource_importer_flac.h"

#include "core/io/resource_saver.h"
#include "core/os/file_access.h"
#include "scene/resources/texture.h"

String ResourceImporterFLAC::get_importer_name() const {
	return "flac";
}

String ResourceImporterFLAC::get_visible_name() const {
	return "FLAC";
}
void ResourceImporterFLAC::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("flac");
}

String ResourceImporterFLAC::get_save_extension() const {
	return "flacstr";
}

String ResourceImporterFLAC::get_resource_type() const {
	return "AudioStreamFLAC";
}

bool ResourceImporterFLAC::get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const {
	return true;
}

int ResourceImporterFLAC::get_preset_count() const {
	return 0;
}
String ResourceImporterFLAC::get_preset_name(int p_idx) const {
	return String();
}

void ResourceImporterFLAC::get_import_options(List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "loop"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::REAL, "loop_offset"), 0));
}

Error ResourceImporterFLAC::import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	bool loop = p_options["loop"];
	float loop_offset = p_options["loop_offset"];

	FileAccess *f = FileAccess::open(p_source_file, FileAccess::READ);

	ERR_FAIL_COND_V(!f, ERR_CANT_OPEN);

	size_t len = f->get_len();

	PoolVector<uint8_t> data;
	data.resize(len);
	PoolVector<uint8_t>::Write w = data.write();

	f->get_buffer(w.ptr(), len);

	memdelete(f);

	Ref<AudioStreamFLAC> flac_stream;
	flac_stream.instance();

	flac_stream->set_data(data);
	ERR_FAIL_COND_V(!flac_stream->get_data().size(), ERR_FILE_CORRUPT);
	flac_stream->set_loop(loop);
	flac_stream->set_loop_offset(loop_offset);

	return ResourceSaver::save(p_save_path + ".flacstr", flac_stream);
}

ResourceImporterFLAC::ResourceImporterFLAC() {
}
