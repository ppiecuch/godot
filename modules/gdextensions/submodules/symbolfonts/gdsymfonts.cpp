/**************************************************************************/
/*  gdsymfonts.cpp                                                        */
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

#include "gdsymfonts.h"

#ifdef DOCTEST
#include "doctest/doctest.h"
#include "doctest/doctest_godot.h"
#else
#define DOCTEST_CONFIG_DISABLE
#endif

#include "icon_tables.gen.h"

static_assert(sizeof(font_meta) / sizeof(FontMeta) == GdSymbolFonts::FONT_TYPE_COUNT, "font_meta size mismatch");

static const IconEntry *find_icon(const FontIconTable &table, const char *name) {
	for (int i = 0; i < table.count; i++) {
		if (strcmp(table.icons[i].name, name) == 0) {
			return &table.icons[i];
		}
	}
	return nullptr;
}

String GdSymbolFonts::get_icon(int font_type, const String &icon_name) const {
	ERR_FAIL_INDEX_V(font_type, FONT_TYPE_COUNT, String());
	CharString name = icon_name.to_lower().utf8();
	const IconEntry *entry = find_icon(icon_tables[font_type], name.get_data());
	ERR_FAIL_COND_V_MSG(!entry, String(), "Icon '" + icon_name + "' not found in font type " + itos(font_type));
	return String::utf8(entry->utf8);
}

bool GdSymbolFonts::has_icon(int font_type, const String &icon_name) const {
	ERR_FAIL_INDEX_V(font_type, FONT_TYPE_COUNT, false);
	CharString name = icon_name.to_lower().utf8();
	return find_icon(icon_tables[font_type], name.get_data()) != nullptr;
}

PoolStringArray GdSymbolFonts::get_icon_names(int font_type) const {
	ERR_FAIL_INDEX_V(font_type, FONT_TYPE_COUNT, PoolStringArray());
	const FontIconTable &table = icon_tables[font_type];
	PoolStringArray names;
	names.resize(table.count);
	for (int i = 0; i < table.count; i++) {
		names.set(i, table.icons[i].name);
	}
	return names;
}

int GdSymbolFonts::get_icon_count(int font_type) const {
	ERR_FAIL_INDEX_V(font_type, FONT_TYPE_COUNT, 0);
	return icon_tables[font_type].count;
}

String GdSymbolFonts::get_font_filename(int font_type) const {
	ERR_FAIL_INDEX_V(font_type, FONT_TYPE_COUNT, String());
	return font_meta[font_type].filename;
}

int GdSymbolFonts::get_icon_min(int font_type) const {
	ERR_FAIL_INDEX_V(font_type, FONT_TYPE_COUNT, 0);
	return font_meta[font_type].icon_min;
}

int GdSymbolFonts::get_icon_max(int font_type) const {
	ERR_FAIL_INDEX_V(font_type, FONT_TYPE_COUNT, 0);
	return font_meta[font_type].icon_max;
}

void GdSymbolFonts::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_icon", "font_type", "icon_name"), &GdSymbolFonts::get_icon);
	ClassDB::bind_method(D_METHOD("has_icon", "font_type", "icon_name"), &GdSymbolFonts::has_icon);
	ClassDB::bind_method(D_METHOD("get_icon_names", "font_type"), &GdSymbolFonts::get_icon_names);
	ClassDB::bind_method(D_METHOD("get_icon_count", "font_type"), &GdSymbolFonts::get_icon_count);
	ClassDB::bind_method(D_METHOD("get_font_filename", "font_type"), &GdSymbolFonts::get_font_filename);
	ClassDB::bind_method(D_METHOD("get_icon_min", "font_type"), &GdSymbolFonts::get_icon_min);
	ClassDB::bind_method(D_METHOD("get_icon_max", "font_type"), &GdSymbolFonts::get_icon_max);

	BIND_ENUM_CONSTANT(FONT_AWESOME6);
	BIND_ENUM_CONSTANT(FONT_AWESOME6_BRANDS);
	BIND_ENUM_CONSTANT(FONT_AWESOME5);
	BIND_ENUM_CONSTANT(FONT_AWESOME5_BRANDS);
	BIND_ENUM_CONSTANT(FONT_AWESOME4);
	BIND_ENUM_CONSTANT(FONT_AWESOME7);
	BIND_ENUM_CONSTANT(FONT_AWESOME7_BRANDS);
	BIND_ENUM_CONSTANT(FONT_KENNEY);
	BIND_ENUM_CONSTANT(FONT_FONTAUDIO);
	BIND_ENUM_CONSTANT(FONT_MATERIAL_DESIGN);
	BIND_ENUM_CONSTANT(FONT_MATERIAL_DESIGN_ICONS);
	BIND_ENUM_CONSTANT(FONT_FORK_AWESOME);
	BIND_ENUM_CONSTANT(FONT_CODICONS);
	BIND_ENUM_CONSTANT(FONT_LUCIDE);
}

GdSymbolFonts::GdSymbolFonts() {
}

#ifdef DOCTEST

TEST_CASE("[SymbolFonts] Font metadata") {
	GdSymbolFonts sf;

	SUBCASE("font filenames are valid strings") {
		for (int i = 0; i < GdSymbolFonts::FONT_TYPE_COUNT; i++) {
			String filename = sf.get_font_filename(i);
			CHECK_FALSE(filename.empty());
			CHECK_MESSAGE(filename.find(".") != -1, "Font filename should have extension");
		}
	}

	SUBCASE("codepoint ranges are valid") {
		for (int i = 0; i < GdSymbolFonts::FONT_TYPE_COUNT; i++) {
			int min_cp = sf.get_icon_min(i);
			int max_cp = sf.get_icon_max(i);
			CHECK(min_cp > 0);
			CHECK(max_cp >= min_cp);
		}
	}

	SUBCASE("icon counts are positive") {
		for (int i = 0; i < GdSymbolFonts::FONT_TYPE_COUNT; i++) {
			CHECK(sf.get_icon_count(i) > 0);
		}
	}

	SUBCASE("invalid font type returns safely") {
		EXPECT_ERROR(CHECK(sf.get_font_filename(-1).empty()));
		EXPECT_ERROR(CHECK(sf.get_font_filename(999).empty()));
		EXPECT_ERROR(CHECK(sf.get_icon_count(-1) == 0));
		EXPECT_ERROR(CHECK(sf.get_icon_min(999) == 0));
	}
}

TEST_CASE("[SymbolFonts] Icon lookup") {
	GdSymbolFonts sf;

	SUBCASE("FA6 known icons") {
		CHECK(sf.has_icon(GdSymbolFonts::FONT_AWESOME6, "address_book"));
		CHECK(sf.has_icon(GdSymbolFonts::FONT_AWESOME6, "heart"));
		CHECK(sf.has_icon(GdSymbolFonts::FONT_AWESOME6, "star"));

		String icon = sf.get_icon(GdSymbolFonts::FONT_AWESOME6, "heart");
		CHECK_FALSE(icon.empty());
	}

	SUBCASE("case insensitive lookup") {
		CHECK(sf.has_icon(GdSymbolFonts::FONT_AWESOME6, "HEART"));
		CHECK(sf.has_icon(GdSymbolFonts::FONT_AWESOME6, "Heart"));
		String lower = sf.get_icon(GdSymbolFonts::FONT_AWESOME6, "heart");
		String upper = sf.get_icon(GdSymbolFonts::FONT_AWESOME6, "HEART");
		CHECK(lower == upper);
	}

	SUBCASE("nonexistent icon returns empty") {
		CHECK_FALSE(sf.has_icon(GdSymbolFonts::FONT_AWESOME6, "nonexistent_icon_xyz"));
	}

	SUBCASE("Kenney icons") {
		CHECK(sf.has_icon(GdSymbolFonts::FONT_KENNEY, "home"));
		CHECK(sf.has_icon(GdSymbolFonts::FONT_KENNEY, "check"));
		String icon = sf.get_icon(GdSymbolFonts::FONT_KENNEY, "home");
		CHECK_FALSE(icon.empty());
	}

	SUBCASE("Material Design icons") {
		CHECK(sf.has_icon(GdSymbolFonts::FONT_MATERIAL_DESIGN, "home"));
		CHECK(sf.get_icon_count(GdSymbolFonts::FONT_MATERIAL_DESIGN) > 2000);
	}

	SUBCASE("Codicons icons") {
		CHECK(sf.has_icon(GdSymbolFonts::FONT_CODICONS, "add"));
		CHECK(sf.has_icon(GdSymbolFonts::FONT_CODICONS, "repo"));
	}

	SUBCASE("Lucide icons") {
		CHECK(sf.has_icon(GdSymbolFonts::FONT_LUCIDE, "house"));
		CHECK(sf.get_icon_count(GdSymbolFonts::FONT_LUCIDE) > 1000);
	}
}

TEST_CASE("[SymbolFonts] Icon names listing") {
	GdSymbolFonts sf;

	SUBCASE("get_icon_names returns correct count") {
		PoolStringArray names = sf.get_icon_names(GdSymbolFonts::FONT_KENNEY);
		CHECK(names.size() == sf.get_icon_count(GdSymbolFonts::FONT_KENNEY));
		CHECK(names.size() > 200);
	}

	SUBCASE("names are non-empty lowercase strings") {
		PoolStringArray names = sf.get_icon_names(GdSymbolFonts::FONT_FONTAUDIO);
		for (int i = 0; i < names.size(); i++) {
			CHECK_FALSE(names[i].empty());
			CHECK(names[i] == names[i].to_lower());
		}
	}
}

TEST_CASE("[SymbolFonts] Font version variants") {
	GdSymbolFonts sf;

	SUBCASE("FA4 FA5 FA6 FA7 have distinct filenames") {
		String fa4 = sf.get_font_filename(GdSymbolFonts::FONT_AWESOME4);
		String fa5 = sf.get_font_filename(GdSymbolFonts::FONT_AWESOME5);
		String fa6 = sf.get_font_filename(GdSymbolFonts::FONT_AWESOME6);
		String fa7 = sf.get_font_filename(GdSymbolFonts::FONT_AWESOME7);
		CHECK(fa4 != fa5);
		CHECK(fa6 != fa7);
		CHECK(fa4.find("webfont") != -1);
		CHECK(fa7.find("woff2") != -1);
	}

	SUBCASE("FA4 has smaller range than FA6") {
		CHECK(sf.get_icon_max(GdSymbolFonts::FONT_AWESOME4) < sf.get_icon_max(GdSymbolFonts::FONT_AWESOME6));
	}

	SUBCASE("brands variants have separate icon sets") {
		CHECK(sf.get_icon_count(GdSymbolFonts::FONT_AWESOME6_BRANDS) > 400);
		CHECK(sf.get_icon_count(GdSymbolFonts::FONT_AWESOME7_BRANDS) > 400);
		// Brands have different icons than solid
		CHECK(sf.has_icon(GdSymbolFonts::FONT_AWESOME6_BRANDS, "github"));
		CHECK(sf.has_icon(GdSymbolFonts::FONT_AWESOME7_BRANDS, "github"));
	}

	SUBCASE("each font version has unique icon count") {
		int fa4_count = sf.get_icon_count(GdSymbolFonts::FONT_AWESOME4);
		int fa5_count = sf.get_icon_count(GdSymbolFonts::FONT_AWESOME5);
		int fa6_count = sf.get_icon_count(GdSymbolFonts::FONT_AWESOME6);
		int fa7_count = sf.get_icon_count(GdSymbolFonts::FONT_AWESOME7);
		CHECK(fa4_count < fa5_count);
		CHECK(fa5_count < fa6_count);
		CHECK(fa6_count < fa7_count);
	}
}

TEST_CASE("[SymbolFonts] UTF-8 icon values") {
	GdSymbolFonts sf;

	SUBCASE("icon values are valid UTF-8") {
		String heart = sf.get_icon(GdSymbolFonts::FONT_AWESOME6, "heart");
		CHECK(heart.length() > 0);
		// UTF-8 encoded icon should be a single Unicode codepoint
		CHECK(heart.length() == 1);
	}

	SUBCASE("different icons produce different values") {
		String heart = sf.get_icon(GdSymbolFonts::FONT_AWESOME6, "heart");
		String star = sf.get_icon(GdSymbolFonts::FONT_AWESOME6, "star");
		CHECK(heart != star);
	}

	SUBCASE("same icon name across fonts may differ") {
		// "heart" exists in FA5 and FA6 — values should be the same codepoint
		String fa5 = sf.get_icon(GdSymbolFonts::FONT_AWESOME5, "heart");
		String fa6 = sf.get_icon(GdSymbolFonts::FONT_AWESOME6, "heart");
		CHECK_FALSE(fa5.empty());
		CHECK_FALSE(fa6.empty());
		CHECK(fa5 == fa6); // same codepoint U+f004 across versions
	}

	SUBCASE("Material Design icons are valid") {
		String home = sf.get_icon(GdSymbolFonts::FONT_MATERIAL_DESIGN, "home");
		CHECK(home.length() == 1);
	}

	SUBCASE("Fork Awesome icons") {
		CHECK(sf.has_icon(GdSymbolFonts::FONT_FORK_AWESOME, "heart"));
		CHECK(sf.get_icon_count(GdSymbolFonts::FONT_FORK_AWESOME) > 700);
	}

	SUBCASE("Fontaudio icons") {
		CHECK(sf.has_icon(GdSymbolFonts::FONT_FONTAUDIO, "speaker"));
		CHECK(sf.get_icon_count(GdSymbolFonts::FONT_FONTAUDIO) > 100);
	}

	SUBCASE("MDI icons") {
		CHECK(sf.get_icon_count(GdSymbolFonts::FONT_MATERIAL_DESIGN_ICONS) > 7000);
		CHECK(sf.has_icon(GdSymbolFonts::FONT_MATERIAL_DESIGN_ICONS, "account"));
	}
}

TEST_CASE("[SymbolFonts] Edge cases") {
	GdSymbolFonts sf;

	SUBCASE("invalid font type for has_icon") {
		EXPECT_ERROR(CHECK_FALSE(sf.has_icon(-1, "heart")));
		EXPECT_ERROR(CHECK_FALSE(sf.has_icon(999, "heart")));
	}

	SUBCASE("empty icon name") {
		CHECK_FALSE(sf.has_icon(GdSymbolFonts::FONT_AWESOME6, ""));
	}

	SUBCASE("get_icon on invalid type returns empty") {
		String result;
		EXPECT_ERROR(result = sf.get_icon(-1, "heart"));
		CHECK(result.empty());
	}

	SUBCASE("get_icon_names on invalid type returns empty array") {
		PoolStringArray names;
		EXPECT_ERROR(names = sf.get_icon_names(-1));
		CHECK(names.size() == 0);
	}

	SUBCASE("get_icon_max on invalid type returns 0") {
		EXPECT_ERROR(CHECK(sf.get_icon_max(-1) == 0));
		EXPECT_ERROR(CHECK(sf.get_icon_max(GdSymbolFonts::FONT_TYPE_COUNT) == 0));
	}
}

#endif // DOCTEST
