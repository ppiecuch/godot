/**************************************************************************/
/*  truetype_utils.h                                                      */
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

#ifndef TRUETYPE_UTILS_H
#define TRUETYPE_UTILS_H

#include "core/math/math_defs.h"
#include "core/math/rect2.h"
#include "core/variant.h"
#include "scene/2d/canvas_item.h"

#include <functional>
#include <unordered_map>
#include <vector>

typedef struct FT_FaceRec_ *FT_Face;

static const String GD_TTF_SANS = "sans-serif";
static const String GD_TTF_SERIF = "serif";
static const String GD_TTF_MONO = "monospace";

struct UnicodeBlock {
	struct range {
		std::uint32_t begin = 0;
		std::uint32_t end = 0;
		std::uint32_t get_num_glyphs() const { return end - begin + 1; }

		range() :
				begin(0), end(0) {}
		range(uint32_t be, uint32_t en) :
				begin(be), end(en) {}
	};

	static const range Space;
	static const range IdeographicSpace;
	static const range Latin;
	static const range Latin1Supplement;
	static const range LatinA;
	static const range Greek;
	static const range Cyrillic;
	static const range Arabic;
	static const range ArabicSupplement;
	static const range ArabicExtendedA;
	static const range Devanagari;
	static const range HangulJamo;
	static const range VedicExtensions;
	static const range LatinExtendedAdditional;
	static const range GreekExtended;
	static const range GeneralPunctuation;
	static const range SuperAndSubScripts;
	static const range CurrencySymbols;
	static const range LetterLikeSymbols;
	static const range NumberForms;
	static const range Arrows;
	static const range MathOperators;
	static const range MiscTechnical;
	static const range BoxDrawing;
	static const range BlockElement;
	static const range GeometricShapes;
	static const range MiscSymbols;
	static const range Dingbats;
	static const range CJKSymbolAndPunctuation;
	static const range Hiragana;
	static const range Katakana;
	static const range HangulCompatJamo;
	static const range KatakanaPhoneticExtensions;
	static const range CJKLettersAndMonths;
	static const range CJKUnified;
	static const range DevanagariExtended;
	static const range HangulExtendedA;
	static const range HangulSyllables;
	static const range HangulExtendedB;
	static const range AlphabeticPresentationForms;
	static const range ArabicPresFormsA;
	static const range ArabicPresFormsB;
	static const range KatakanaHalfAndFullwidthForms;
	static const range KanaSupplement;
	static const range RumiNumericalSymbols;
	static const range ArabicMath;
	static const range MiscSymbolsAndPictographs;
	static const range Emoticons;
	static const range TransportAndMap;
	static const range EnclosedCharacters;
	static const range Uncategorized;
	static const range AdditionalEmoticons;
	static const range AdditionalTransportAndMap;
	static const range OtherAdditionalSymbols;
	static const range Numbers;
	static const range UppercaseLatin;
	static const range LowercaseLatin;
	static const range Braces;
	static const range Symbols;
	static const range GenericSymbols;
};

struct UnicodeAlphabet {
	static const std::initializer_list<UnicodeBlock::range> Emoji;
	static const std::initializer_list<UnicodeBlock::range> Japanese;
	static const std::initializer_list<UnicodeBlock::range> Chinese;
	static const std::initializer_list<UnicodeBlock::range> Korean;
	static const std::initializer_list<UnicodeBlock::range> Arabic;
	static const std::initializer_list<UnicodeBlock::range> Devanagari;
	static const std::initializer_list<UnicodeBlock::range> Latin;
	static const std::initializer_list<UnicodeBlock::range> Greek;
	static const std::initializer_list<UnicodeBlock::range> Cyrillic;
};

struct TrueTypeFontUtilsSettings {
	String font_name;
	int font_size = 0;
	int dpi = 0;
	int index = 0;
	std::vector<UnicodeBlock::range> ranges;

	void add_ranges(std::initializer_list<UnicodeBlock::range> alphabet) { ranges.insert(ranges.end(), alphabet); }
	void add_range(const UnicodeBlock::range &range) { ranges.push_back(range); }

	TrueTypeFontUtilsSettings(const String &name, int size) :
			font_name(name), font_size(size) {}
};

struct TrueTypePath {
	struct Command {
		enum Type {
			MoveTo,
			LineTo,
			CurveTo,
			BezierTo,
			QuadBezierTo,
			Close
		};

		Type type;
		Point2 to, cp1, cp2;
		real_t radius_x, radius_y, angle_begin, angle_end;

		Command(Type type); // for Close
		Command(Type type, const Point2 &p); // for LineTo and CurveTo
		Command(Type type, const Point2 &p, const Point2 &cp1, const Point2 &cp2); // for BezierTo
		Command(Type type, const Point2 &centre, real_t radius_x, real_t radius_y, real_t angle_begin, real_t angle_end); // for Arc
	};

	std::vector<Command> commands;

	void add_command(const Command &cmd) {
		if ((commands.empty() || commands.back().type == Command::Close) && cmd.type != Command::MoveTo) {
			commands.push_back(Command(Command::MoveTo, cmd.to));
		}
		commands.push_back(cmd);
	}

	void translate(const Vector2 &p) {
		for (int j = 0; j < int(commands.size()); j++) {
			commands[j].to += p;
			if (commands[j].type == Command::BezierTo || commands[j].type == Command::QuadBezierTo) {
				commands[j].cp1 += p, commands[j].cp2 += p;
			}
		}
	}

	void scale(real_t x, real_t y) {
		for (int j = 0; j < int(commands.size()); j++) {
			commands[j].to.x *= x, commands[j].to.y *= y;
			if (commands[j].type == Command::BezierTo || commands[j].type == Command::QuadBezierTo) {
				commands[j].cp1.x *= x, commands[j].cp1.y *= y;
				commands[j].cp2.x *= x, commands[j].cp2.y *= y;
			}
		}
	}

	bool empty() const { return commands.empty(); }

	void move_to(const Point2 &pt) { add_command(Command(Command::MoveTo, pt)); }
	void line_to(const Point2 &pt) { add_command(Command(Command::LineTo, pt)); }
	void curve_to(const Point2 &pt) { add_command(Command(Command::CurveTo, pt)); }
	void bezier_to(const Point2 &cp1, const Point2 &cp2, const Point2 &pt) { add_command(Command(Command::BezierTo, pt, cp1, cp2)); }
	void quad_bezier_to(const Point2 &cp1, const Point2 &cp2, const Point2 &pt) { add_command(Command(Command::QuadBezierTo, pt, cp1, cp2)); }
	void close() { add_command(Command(Command::Close)); }
};

class TrueTypeFontUtils {
	std::shared_ptr<struct FT_FaceRec_> face;
	static bool init_libraries();
	static void finish_libraries();

protected:
	std::vector<TrueTypePath> char_outlines;
	std::vector<TrueTypePath> char_outlines_non_vflipped;

	real_t line_height;
	real_t ascender_height;
	real_t descender_height;
	real_t space_size;
	real_t letter_spacing;
	int direction; // 1 or -1
	uint32_t tab_width; // number of spaces per tab
	real_t font_unit_scale;

	Rect2 glyph_bbox;

	struct glyph_props {
		size_t character_index;
		uint32_t glyph;
		real_t height;
		real_t width;
		real_t bearing_x, bearing_y;
		real_t advance;
	};

	std::vector<glyph_props> cps; // properties for each character

	TrueTypeFontUtilsSettings settings;
	std::unordered_map<uint32_t, size_t> glyph_index_map;

	real_t get_kerning(uint32_t left_c, uint32_t right_c) const;
	Vector<Vector<Point2>> get_character_as_points(const TrueTypePath &outlines, bool vflip, real_t simplify_amt = 0, int resample_count = 0) const;
	glyph_props load_glyph(uint32_t utf8) const;
	const glyph_props &get_glyph_properties(uint32_t glyph) const;
	void iterate_string(const String &str, real_t x, real_t y, bool vflipped, std::function<void(uint32_t, Point2)> f) const;
	size_t index_for_glyph(uint32_t glyph) const;

public:
	enum TextProperties {
		TEXT_SPACESIZE,
		TEXT_TABWIDTH,
		TEXT_LETTERSPACING,
		TEXT_DIRECTION,
		TEXT_FONTUNITSCALE,
	};

	static const glyph_props InvalidProps;

	void set_text_property(TextProperties prop, Variant value);
	Variant get_text_property(TextProperties prop) const;

	// Loads a font, and allows you to set the following parameters: the filename, the size, if the font is anti-aliased,
	// if it has a full character set, if you need it to have contours (for getStringPoints) and parameters that control
	// the simplification amount for those contours and the dpi of the font.
	//
	// \filename The name of the font file to load.
	// \font_size The size in pixels to load the font.
	// \anti_aliased true if the font should be anti-aliased.
	// \full_character_set true if the full character set should be cached.
	// \dpi the dots per inch used to specify rendering size.
	// returns true if the font was loaded correctly.
	bool load(const String &filename, int font_size, bool full_character_set = true, int dpi = 96);
	bool load(const TrueTypeFontUtilsSettings &settings);

	bool is_loaded() const; // Has the font been loaded successfully?

	bool has_full_character_set() const; // If the font was allocated with a full character set.
	std::size_t get_num_characters() const; // Number of characters in loaded character set.
	int get_size() const; // Size of font, set when font was loaded.
	real_t get_line_height() const; // Computes line height based on font size.

	// The ascender is the vertical distance from the baseline to the highest "character" coordinate.
	// The meaning of "character" coordinate depends on the font. Some fonts take accents into account,
	// others do not, and still others define it simply to be the highest coordinate over all glyphs.
	real_t get_ascender_height() const;

	// The descender is the vertical distance from the baseline to the lowest "character" coordinate.
	// The meaning of "character" coordinate depends on the font. Some fonts take accents into account,
	// others do not, and still others define it simply to be the lowest coordinate over all glyphs.
	// This value will be negative for descenders below the baseline (which is typical).
	real_t get_descender_height() const;

	// The global bounding box is the rectangle inside of which all glyphs in the font can fit.
	// Glyphs are drawn starting from (0,0) in the returned box (though note that the box can
	// extend in any direction out from the origin).
	const Rect2 &get_glyph_bbox() const;

	// The bounding box of a string as a rectangle.
	Rect2 get_string_bounding_box(const String &str, real_t x, real_t y, bool vflip = true) const;

	// Return vector of glyph contours
	//
	// \simplify_amt the amount to simplify the vector contours. Larger number means more simplified.
	Vector<Vector<Point2>> get_character_as_points(uint32_t character, bool vflip = true, real_t simplify_amt = 0, int resample_count = 0) const;
	Vector<Vector<Point2>> get_string_as_points(const String &str, bool vflip = true, real_t simplify_amt = 0) const;
	TrueTypePath get_character_as_path(uint32_t character, bool vflip = true) const;
	Point2 get_first_glyph_pos_for_texture(const String &str, bool vflip) const;
	bool is_valid_glyph(uint32_t) const;

	static real_t int26p6_to_dbl(long p) { return real_t(p) / 64.0; }
	static Point2 int26p6_to_dbl(long px, long py) { return Point2(real_t(px) / 64.0, real_t(py) / 64.0); }
	static _FORCE_INLINE_ int dbl_to_int26p6(real_t p) { return int(p * 64.0 + 0.5); }

	TrueTypeFontUtils &operator=(const TrueTypeFontUtils &mom);
	TrueTypeFontUtils &operator=(TrueTypeFontUtils &&mom);

	TrueTypeFontUtils(const TrueTypeFontUtils &mom);
	TrueTypeFontUtils(TrueTypeFontUtils &&mom);

#if TOOLS_ENABLED
	void draw_demo(CanvasItem *p_canvas, int frame_num);
#endif

	TrueTypeFontUtils();
	~TrueTypeFontUtils();
};

#endif // TRUETYPE_UTILS_H
