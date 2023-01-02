#ifndef GD_SYMFONTS_H
#define GD_SYMFONTS_H

class GdSymbolFont : public Object {
	GDCLASS(GdSymbolFont, Object);

public:
	enum {
		FONT_AWESOME4,
		FONT_AWESOME5,
		FONT_AWESOME5BRANDS,
		FONT_KENNEY,
		FONT_AUDIO,
		FONT_MATERIAL,
	};
};

class GdSymbolFontIcon : public Object {
	GDCLASS(GdSymbolFontIcon, Object);

protected:
	static void _bind_methods();

public:
	Image get_image(int glyph);
	Image get_image(const String &glyph_name);
	Char get_char(int glyph);
	Char get_char(const String &glyph_name);

	GdSymbolFontIcon();
};

#endif // GD_SYMFONTS_H
