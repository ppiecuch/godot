#ifndef BMF_BITMAP_FONT_H
#define BMF_BITMAP_FONT_H

#include "core/int_types.h"
#include "core/image.h"
#include "core/ustring.h"
#include "core/vector.h"
#include "core/io/resource_importer.h"
#include "scene/resources/font.h"

struct BmfRGB {
	uint8_t r, g, b;
	BmfRGB() : r(0), g(0), b(0) {}
};

#define MAKRGB(p, a) ((a << 24) | (p.b << 16) | (p.g << 8) | (p.r));

struct BmfChar {
	uint8_t w, h;
	int8_t relx, rely;
	int8_t shift;
	Vector<uint8_t> d;
	BmfRGB *palette;

	uint32_t _FORCE_INLINE_ get_pixel_at(unsigned x, unsigned y, uint8_t a = 255) const { return MAKRGB(palette[d[y * w + x]], a); }
	uint32_t _FORCE_INLINE_ get_pixel_at(unsigned idx, uint8_t a = 255) const { return MAKRGB(palette[d[idx]], a); }
	BmfRGB _FORCE_INLINE_ get_color_at(unsigned idx) const { return palette[d[idx]]; }
	BmfChar() : w(0), h(0), relx(0), rely(0), shift(0), palette(nullptr) {}
};

class BmfFont {
	uint8_t used_colors, highest_color;
	BmfRGB rgb[256];
	BmfChar glyphs[256];
	uint8_t codes[256]; // index to to glyphs
	uint8_t chars[256]; // map from char code
	uint16_t num_codes;

public:
	uint8_t line_height;
	uint16_t size_over, size_under, add_space, size_inner;
	int x, y;

	uint32_t rgb_value(uint8_t i, uint8_t a = 255);
	void bmf_load(const String &fname);
	Ref<Image> get_image(uint8_t code, uint8_t a);
	Ref<Image> get_image(uint8_t code);
	Ref<BitmapFont> get_font();

	void print( uint32_t *bmp, uint8_t ch, int x, int y, int bmp_line_width);
	void print( uint32_t *bmp, const uint8_t *str, int x, int y );

	const BmfRGB *get_palette() { return &rgb[0]; }
	const BmfChar *get_char(uint8_t index = 0) { return &glyphs[index]; }
	const uint8_t *get_codes() { return &codes[0]; }
	uint16_t get_num_codes() { return num_codes; }

	BmfFont() { }
	BmfFont(const String &from_file) { bmf_load(from_file); }
};


class BmfFontImporter : public ResourceImporter {
	GDCLASS(BmfFontImporter, ResourceImporter);

public:
	virtual String get_importer_name() const;
	virtual String get_visible_name() const;
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual String get_save_extension() const;
	virtual String get_resource_type() const;

	virtual int get_preset_count() const;
	virtual String get_preset_name(int p_idx) const;

	virtual void get_import_options(List<ImportOption> *r_options, int p_preset = 0) const;
	virtual bool get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const;

	virtual Error import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files = nullptr, Variant *r_metadata = nullptr);

	BmfFontImporter() {}
};

#endif // BMF_BITMAP_FONT_H
