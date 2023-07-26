/**************************************************************************/
/*  bmf_bitmap_font.cpp                                                   */
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

#include "bmf_bitmap_font.h"

#include "common/gd_core.h"
#include "common/gd_pack.h"
#include "core/os/file_access.h"

#include <stdio.h>
#include <stdlib.h>

const uint32_t BMFHEADER = 0x1AD5E6E1;

// Bmf bitmap font handler:
// ------------------------
void BmfFont::print(uint32_t *bmp, uint8_t ch, int x, int y, int bmp_line_width) {
	BmfChar &g = glyphs[ch];
	for (int j = 0; j < g.h; j++) {
		for (int i = 0; i < g.w; i++) {
			bmp[(y + g.rely + j) * bmp_line_width + (x + g.relx + i)] = rgb_value(g.d[j * g.w + i]);
		}
	}
}

void BmfFont::print(uint32_t *bmp, const uint8_t *str, int x, int y) {
	uint8_t *ch = (uint8_t *)str;
	while (*ch) {
		BmfChar &g = glyphs[*ch];
		for (int j = 0; j < g.h; j++) {
			for (int i = 0; i < g.w; i++) {
				bmp[(y + g.rely + j) * g.w + (x + g.relx + i)] = rgb_value(g.d[j * g.w + i]);
			}
		}
		//inc(x,(shift+addspace)*zoom);
		ch++;
	}
}

uint32_t BmfFont::rgb_value(uint8_t i, uint8_t a) {
	return (a << 24) | (rgb[i].b << 16) | (rgb[i].g << 8) | (rgb[i].r);
}

void BmfFont::bmf_load(const String &fname) {
	if (FileAccessRef fa = FileAccess::open(fname, FileAccess::READ)) {
		uint8_t s[256];

		if (fa->get_32() == BMFHEADER) {
			const int ver = fa->get_8();
			line_height = fa->get_8();
			size_over = fa->get_8();
			size_under = fa->get_8();
			add_space = fa->get_8();
			size_inner = fa->get_8();
			used_colors = fa->get_8();
			highest_color = fa->get_8();
			fa->get_buffer(s, 4);
			const int i = fa->get_8(); // num color entries
			for (int x = 1; x <= i; x++) {
				fa->get_buffer((uint8_t *)&rgb[x], 3);
				rgb[x].r = (rgb[x].r << 2) + 3;
				rgb[x].g = (rgb[x].g << 2) + 3;
				rgb[x].b = (rgb[x].b << 2) + 3;
			}
			const int j = fa->get_8();
			fa->get_buffer(s, j);
			s[j] = 0;
			printf_verbose("bmf info: %s", s); // info string
			printf_verbose("  ver: %d", ver);
			printf_verbose("  line height: %d", line_height);
			printf_verbose("  size over: %d", size_over);
			printf_verbose("  size under: %d", size_under);
			printf_verbose("  add space: %d", add_space);
			num_codes = fa->get_16(); // num characters
			ERR_FAIL_COND(num_codes == 0);
			memset(codes, 0, 256);
			memset(chars, 0, 256);
			for (int d = 0; d < num_codes; d++) {
				const uint8_t c = fa->get_8();
				BmfChar &g = glyphs[c];
				fa->get_buffer((uint8_t *)&g, 5); // w, h, relx. rely,shift
				// printf_verbose("char '%c': %dx%d pixels\n", c, g.w, g.h);
				if (g.w > 0 && g.h > 0) {
					g.d.resize(g.w * g.h);
					fa->get_buffer(g.d.ptrw(), g.w * g.h);
					codes[d] = c;
					chars[c] = d;
				}
				glyphs[c].palette = rgb;
			}
		}
	}
}

Ref<Image> BmfFont::get_image(uint8_t code) {
	const BmfChar *ch = get_char(code);
	ERR_FAIL_NULL_V(ch, Ref<Image>());
	ERR_FAIL_COND_V(ch->w <= 0 || ch->h <= 0, Ref<Image>());
	Ref<Image> image = memnew(Image);
	PoolByteArray pixels;
	const unsigned sz = ch->w * ch->h;
	ERR_FAIL_COND_V(pixels.resize(sz * 3) != OK, Ref<Image>());
	auto pixels_write = pixels.write();
	for (int p = 0; p < sz * 3; p += 3) {
		const BmfRGB &color = ch->get_color_at(p);
		pixels_write[p + 0] = color.r;
		pixels_write[p + 1] = color.g;
		pixels_write[p + 2] = color.b;
	}
	pixels_write.release();
	image->create(ch->w, ch->h, false, Image::FORMAT_RGB8, pixels);
	return image;
}

Ref<BitmapFont> BmfFont::get_font(bool add_alpha, uint8_t alpha) {
	Vector<Ref<Image>> images;
	for (int c = 0; c < num_codes; c++) {
		images.push_back(get_image(c));
	}

	ImageMergeOptions opts;
	if (add_alpha) {
		opts.background_color = Color(0, 0, 0, alpha / 255.0);
		opts.force_atlas_channels = 4;
	}
	Dictionary atlas_info = merge_images(images, opts);
	ERR_FAIL_COND_V(atlas_info.empty(), Ref<BitmapFont>());
	Array pages = atlas_info["_generated_images"];
	ERR_FAIL_COND_V(pages.size() > 1, Ref<BitmapFont>());

	Ref<Image> atlas_image = pages[0];
	ERR_FAIL_NULL_V(atlas_image, Ref<BitmapFont>());

	Ref<BitmapFont> font = newref(BitmapFont);
	font->add_texture(atlas_image);

	Array atlas_rects = atlas_info["_rects"];
	for (int j = 0; j < atlas_rects.size(); ++j) {
		Dictionary entry = atlas_rects[j];
		ERR_CONTINUE_MSG(entry.empty(), "Empty atlas entry, Skipping!");

		const BmfChar *g = get_char(j);
		ERR_CONTINUE(g == nullptr);
		const Rect2 rc = entry["rect"];
		font->add_char(chars[j], 0, rc, Size2(g->relx, g->rely), g->shift);
	}
	font->set_height(line_height);
	return font;
}

/// BmfFontImporter

String BmfFontImporter::get_importer_name() const {
	return "bmf_font";
}
String BmfFontImporter::get_visible_name() const {
	return "BitmapFont";
}
void BmfFontImporter::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("bmf");
}
String BmfFontImporter::get_save_extension() const {
	return "font";
}
String BmfFontImporter::get_resource_type() const {
	return "BitmapFont";
}

int BmfFontImporter::get_preset_count() const {
	return 0;
}
String BmfFontImporter::get_preset_name(int p_idx) const {
	return "";
}

void BmfFontImporter::get_import_options(List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "add_alpha_channel"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "alpha"), "255"));
}
bool BmfFontImporter::get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const {
	return true;
}

Error BmfFontImporter::import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	BmfFont bmf(p_source_file);
	if (bmf.get_num_codes()) {
		const bool add_alpha = p_options["add_alpha_channel"];
		const uint8_t alpha = p_options["alpha"];
		RES fnt = bmf.get_font(add_alpha, alpha);
		String save_path = p_save_path + ".font";
		Error err = ResourceSaver::save(save_path, fnt);
		ERR_FAIL_COND_V_MSG(err != OK, err, "Cannot save Font to file '" + save_path + "'.");
		r_gen_files->push_back(save_path);
		return OK;
	} else {
		WARN_PRINT("Failed to import font file or file is empty.");
		return ERR_CANT_CREATE;
	}
}
