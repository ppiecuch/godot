/**************************************************************************/
/*  gd_lite_html.h                                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/

#ifndef GD_LITE_HTML_H
#define GD_LITE_HTML_H

#include "scene/gui/control.h"
#include "scene/resources/font.h"
#include "scene/resources/texture.h"

#include <litehtml.h>

class GodotLiteHTML : public Control, public litehtml::document_container {
	GDCLASS(GodotLiteHTML, Control);

public:
	enum RenderMode {
		RENDER_DIRECT, // Draw directly to canvas via _draw()
		RENDER_TEXTURE, // Render to ImageTexture (for non-Control uses)
	};

private:
	// HTML document state.
	litehtml::document::ptr m_document;
	String m_html_text;
	String m_user_css;
	String m_base_url;
	bool m_needs_render;
	bool m_needs_redraw;

	// Rendering state.
	RenderMode m_render_mode;
	RID m_current_canvas;
	Ref<ImageTexture> m_render_texture;
	Ref<Image> m_render_image;

	// Font management.
	struct FontInfo {
		Ref<Font> font;
		int size;
		int weight;
		bool italic;
		unsigned int decoration;
		litehtml::font_metrics metrics;
	};
	HashMap<uint64_t, FontInfo> m_fonts;
	uint64_t m_next_font_id;

	String m_default_font_name;
	int m_default_font_size;
	Ref<Font> m_default_font;

	// Image management.
	struct ImageInfo {
		Ref<Texture> texture;
		String url;
	};
	HashMap<String, ImageInfo> m_images;

	// Clipping stack.
	struct ClipInfo {
		litehtml::position pos;
		litehtml::border_radiuses radius;
	};
	Vector<ClipInfo> m_clips;

	// Scroll position.
	Point2 m_scroll_offset;

	// Internal helpers.
	FontInfo *_get_font_info(litehtml::uint_ptr hFont);
	Color _to_godot_color(const litehtml::web_color &c) const;
	Rect2 _to_godot_rect(const litehtml::position &p) const;
	void _render_document();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	// GDScript API.
	void set_html(const String &p_html);
	String get_html() const;

	void set_user_css(const String &p_css);
	String get_user_css() const;

	void set_html_base_url(const String &p_url);
	String get_html_base_url() const;

	void set_default_font(const Ref<Font> &p_font);
	Ref<Font> get_default_font() const;

	void set_font_name(const String &p_name);
	String get_font_name() const;

	void set_font_size(int p_size);
	int get_font_size() const;

	void set_render_mode(RenderMode p_mode);
	RenderMode get_render_mode() const;

	Ref<ImageTexture> get_render_texture() const;

	int get_content_width() const;
	int get_content_height() const;

	void set_scroll_offset(const Point2 &p_offset);
	Point2 get_scroll_offset() const;

	void set_image(const String &p_url, const Ref<Texture> &p_texture);
	void render_html();

	// Mouse input forwarding.
	bool html_mouse_over(const Point2 &p_pos);
	bool html_mouse_down(const Point2 &p_pos);
	bool html_mouse_up(const Point2 &p_pos);
	bool html_mouse_leave();

	// --- litehtml::document_container interface ---
	litehtml::uint_ptr create_font(const char *faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics *fm) override;
	void delete_font(litehtml::uint_ptr hFont) override;
	int text_width(const char *text, litehtml::uint_ptr hFont) override;
	void draw_text(litehtml::uint_ptr hdc, const char *text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position &pos) override;
	int pt_to_px(int pt) const override;
	int get_default_font_size() const override;
	const char *get_default_font_name() const override;
	void draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker &marker) override;
	void load_image(const char *src, const char *baseurl, bool redraw_on_ready) override;
	void get_image_size(const char *src, const char *baseurl, litehtml::size &sz) override;
	void draw_background(litehtml::uint_ptr hdc, const std::vector<litehtml::background_paint> &bg) override;
	void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders &borders, const litehtml::position &draw_pos, bool root) override;
	void set_caption(const char *caption) override;
	void set_base_url(const char *base_url) override;
	void link(const std::shared_ptr<litehtml::document> &doc, const litehtml::element::ptr &el) override;
	void on_anchor_click(const char *url, const litehtml::element::ptr &el) override;
	void set_cursor(const char *cursor) override;
	void transform_text(litehtml::string &text, litehtml::text_transform tt) override;
	void import_css(litehtml::string &text, const litehtml::string &url, litehtml::string &baseurl) override;
	void set_clip(const litehtml::position &pos, const litehtml::border_radiuses &bdr_radius) override;
	void del_clip() override;
	void get_client_rect(litehtml::position &client) const override;
	litehtml::element::ptr create_element(const char *tag_name, const litehtml::string_map &attributes, const std::shared_ptr<litehtml::document> &doc) override;
	void get_media_features(litehtml::media_features &media) const override;
	void get_language(litehtml::string &language, litehtml::string &culture) const override;

	GodotLiteHTML();
	~GodotLiteHTML();
};

VARIANT_ENUM_CAST(GodotLiteHTML::RenderMode);

#endif // GD_LITE_HTML_H
