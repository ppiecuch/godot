/**************************************************************************/
/*  gdsymfonts.h                                                          */
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

#ifndef GD_SYMFONTS_H
#define GD_SYMFONTS_H

#include "core/reference.h"
#include "core/variant.h"

class GdSymbolFonts : public Reference {
	GDCLASS(GdSymbolFonts, Reference);

protected:
	static void _bind_methods();

public:
	enum FontType {
		FONT_AWESOME6,
		FONT_AWESOME6_BRANDS,
		FONT_AWESOME5,
		FONT_AWESOME5_BRANDS,
		FONT_AWESOME4,
		FONT_AWESOME7,
		FONT_AWESOME7_BRANDS,
		FONT_KENNEY,
		FONT_FONTAUDIO,
		FONT_MATERIAL_DESIGN,
		FONT_MATERIAL_DESIGN_ICONS,
		FONT_FORK_AWESOME,
		FONT_CODICONS,
		FONT_LUCIDE,
		// Material Symbols deliberately not listed here — owned by the
		// `material_symbols` submodule (axis-aware variable-font rasterisation).
		FONT_TYPE_COUNT,
	};

	// Icon lookup by name (returns UTF-8 string for use with DynamicFont)
	String get_icon(int font_type, const String &icon_name) const;

	// Check if icon name exists in the specified font
	bool has_icon(int font_type, const String &icon_name) const;

	// Get all icon names for a font
	PoolStringArray get_icon_names(int font_type) const;

	// Get number of icons in a font
	int get_icon_count(int font_type) const;

	// Get font TTF/OTF filename(s)
	String get_font_filename(int font_type) const;

	// Get codepoint range
	int get_icon_min(int font_type) const;
	int get_icon_max(int font_type) const;

	GdSymbolFonts();
};

VARIANT_ENUM_CAST(GdSymbolFonts::FontType);

#endif // GD_SYMFONTS_H
