/**************************************************************************/
/*  gd_lite_html.cpp                                                      */
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

#include "gd_lite_html.h"

#include "core/os/os.h"
#include "scene/resources/dynamic_font.h"
#include "servers/visual_server.h"

// Shared embedded Vera.ttf from register_types.cpp (via INCBIN).
extern const uint8_t vera_ttf_data[];
extern const unsigned int vera_ttf_size;

// --- Helpers ---

GodotLiteHTML::FontInfo *GodotLiteHTML::_get_font_info(litehtml::uint_ptr hFont) {
	return m_fonts.getptr((uint64_t)hFont);
}

Color GodotLiteHTML::_to_godot_color(const litehtml::web_color &c) const {
	return Color(c.red / 255.0f, c.green / 255.0f, c.blue / 255.0f, c.alpha / 255.0f);
}

Rect2 GodotLiteHTML::_to_godot_rect(const litehtml::position &p) const {
	return Rect2(p.x - m_scroll_offset.x, p.y - m_scroll_offset.y, p.width, p.height);
}

void GodotLiteHTML::_render_document() {
	if (!m_document) {
		return;
	}
	int w = (int)get_size().x;
	if (w <= 0) {
		w = 256;
	}
	m_document->render(w);
	m_needs_render = false;
	m_needs_redraw = true;
}

// --- GDScript API ---

void GodotLiteHTML::set_html(const String &p_html) {
	m_html_text = p_html;

	CharString utf8 = p_html.utf8();
	CharString css_utf8 = m_user_css.utf8();

	m_document = litehtml::document::createFromString(
			utf8.get_data(),
			this,
			litehtml::master_css,
			css_utf8.length() > 0 ? css_utf8.get_data() : "");

	m_needs_render = true;
	m_needs_redraw = true;
	update();
}

String GodotLiteHTML::get_html() const {
	return m_html_text;
}

void GodotLiteHTML::set_user_css(const String &p_css) {
	if (m_user_css == p_css) {
		return;
	}
	m_user_css = p_css;
	if (m_html_text.length() > 0) {
		set_html(m_html_text); // Re-parse with new CSS.
	}
}

String GodotLiteHTML::get_user_css() const {
	return m_user_css;
}

void GodotLiteHTML::set_html_base_url(const String &p_url) {
	m_base_url = p_url;
}

String GodotLiteHTML::get_html_base_url() const {
	return m_base_url;
}

void GodotLiteHTML::set_default_font(const Ref<Font> &p_font) {
	m_default_font = p_font;
	if (m_html_text.length() > 0) {
		set_html(m_html_text);
	}
}

Ref<Font> GodotLiteHTML::get_default_font() const {
	return m_default_font;
}

void GodotLiteHTML::set_font_name(const String &p_name) {
	m_default_font_name = p_name;
}

String GodotLiteHTML::get_font_name() const {
	return m_default_font_name;
}

void GodotLiteHTML::set_font_size(int p_size) {
	m_default_font_size = CLAMP(p_size, 6, 128);
}

int GodotLiteHTML::get_font_size() const {
	return m_default_font_size;
}

void GodotLiteHTML::set_render_mode(RenderMode p_mode) {
	m_render_mode = p_mode;
}

GodotLiteHTML::RenderMode GodotLiteHTML::get_render_mode() const {
	return m_render_mode;
}

Ref<ImageTexture> GodotLiteHTML::get_render_texture() const {
	return m_render_texture;
}

int GodotLiteHTML::get_content_width() const {
	if (m_document) {
		return m_document->content_width();
	}
	return 0;
}

int GodotLiteHTML::get_content_height() const {
	if (m_document) {
		return m_document->content_height();
	}
	return 0;
}

void GodotLiteHTML::set_scroll_offset(const Point2 &p_offset) {
	if (m_scroll_offset == p_offset) {
		return;
	}
	m_scroll_offset = p_offset;
	m_needs_redraw = true;
	update();
}

Point2 GodotLiteHTML::get_scroll_offset() const {
	return m_scroll_offset;
}

void GodotLiteHTML::set_image(const String &p_url, const Ref<Texture> &p_texture) {
	ImageInfo info;
	info.url = p_url;
	info.texture = p_texture;
	m_images[p_url] = info;
	m_needs_redraw = true;
	update();
}

void GodotLiteHTML::render_html() {
	if (m_needs_render && m_document) {
		_render_document();
	}
	update();
}

bool GodotLiteHTML::html_mouse_over(const Point2 &p_pos) {
	if (!m_document) {
		return false;
	}
	litehtml::position::vector redraw;
	int x = (int)(p_pos.x + m_scroll_offset.x);
	int y = (int)(p_pos.y + m_scroll_offset.y);
	bool changed = m_document->on_mouse_over(x, y, (int)p_pos.x, (int)p_pos.y, redraw);
	if (changed) {
		m_needs_redraw = true;
		update();
	}
	return changed;
}

bool GodotLiteHTML::html_mouse_down(const Point2 &p_pos) {
	if (!m_document) {
		return false;
	}
	litehtml::position::vector redraw;
	int x = (int)(p_pos.x + m_scroll_offset.x);
	int y = (int)(p_pos.y + m_scroll_offset.y);
	bool changed = m_document->on_lbutton_down(x, y, (int)p_pos.x, (int)p_pos.y, redraw);
	if (changed) {
		m_needs_redraw = true;
		update();
	}
	return changed;
}

bool GodotLiteHTML::html_mouse_up(const Point2 &p_pos) {
	if (!m_document) {
		return false;
	}
	litehtml::position::vector redraw;
	int x = (int)(p_pos.x + m_scroll_offset.x);
	int y = (int)(p_pos.y + m_scroll_offset.y);
	bool changed = m_document->on_lbutton_up(x, y, (int)p_pos.x, (int)p_pos.y, redraw);
	if (changed) {
		m_needs_redraw = true;
		update();
	}
	return changed;
}

bool GodotLiteHTML::html_mouse_leave() {
	if (!m_document) {
		return false;
	}
	litehtml::position::vector redraw;
	bool changed = m_document->on_mouse_leave(redraw);
	if (changed) {
		m_needs_redraw = true;
		update();
	}
	return changed;
}

// --- Notification ---

void GodotLiteHTML::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			if (!m_document) {
				break;
			}
			if (m_needs_render) {
				_render_document();
			}
			m_current_canvas = get_canvas_item();
			litehtml::position clip(0, 0, (int)get_size().x, (int)get_size().y);
			m_document->draw((litehtml::uint_ptr)this, (int)m_scroll_offset.x, (int)m_scroll_offset.y, &clip);
			m_current_canvas = RID();
			m_needs_redraw = false;
		} break;
		case NOTIFICATION_RESIZED: {
			if (m_document) {
				m_needs_render = true;
				update();
			}
		} break;
		case NOTIFICATION_ENTER_TREE: {
			if (m_document && m_needs_render) {
				update();
			}
		} break;
	}
}

// --- litehtml::document_container implementation ---

litehtml::uint_ptr GodotLiteHTML::create_font(const char *faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics *fm) {
	uint64_t id = ++m_next_font_id;

	FontInfo fi;
	fi.size = size;
	fi.weight = weight;
	fi.italic = (italic == litehtml::font_style_italic);
	fi.decoration = decoration;

	// Use default font or create from embedded vera.ttf.
	fi.font = m_default_font;
	if (fi.font.is_null()) {
		fi.font = get_font("font");
	}
	if (fi.font.is_null()) {
		// Create DynamicFont from embedded Vera TTF.
		Ref<DynamicFont> df;
		df.instance();
		Ref<DynamicFontData> fd;
		fd.instance();
		fd->set_font_ptr(vera_ttf_data, vera_ttf_size);
		df->set_font_data(fd);
		df->set_size(size);
		fi.font = df;
	}

	// Calculate metrics.
	if (fi.font.is_valid()) {
		float scale = (float)size / (fi.font->get_height() > 0 ? fi.font->get_height() : 16.0f);
		fi.metrics.height = size;
		fi.metrics.ascent = (int)(fi.font->get_ascent() * scale);
		fi.metrics.descent = (int)(fi.font->get_descent() * scale);
		fi.metrics.x_height = fi.metrics.ascent / 2;
		fi.metrics.draw_spaces = (decoration != 0);
	} else {
		fi.metrics.height = size;
		fi.metrics.ascent = (int)(size * 0.8f);
		fi.metrics.descent = (int)(size * 0.2f);
		fi.metrics.x_height = fi.metrics.ascent / 2;
		fi.metrics.draw_spaces = false;
	}

	if (fm) {
		*fm = fi.metrics;
	}

	m_fonts[id] = fi;
	return (litehtml::uint_ptr)id;
}

void GodotLiteHTML::delete_font(litehtml::uint_ptr hFont) {
	m_fonts.erase((uint64_t)hFont);
}

int GodotLiteHTML::text_width(const char *text, litehtml::uint_ptr hFont) {
	FontInfo *fi = _get_font_info(hFont);
	if (!fi || fi->font.is_null()) {
		return 0;
	}
	String s = String::utf8(text);
	float scale = (float)fi->size / (fi->font->get_height() > 0 ? fi->font->get_height() : 16.0f);
	return (int)(fi->font->get_string_size(s).x * scale);
}

void GodotLiteHTML::draw_text(litehtml::uint_ptr hdc, const char *text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position &pos) {
	if (!m_current_canvas.is_valid()) {
		return;
	}
	FontInfo *fi = _get_font_info(hFont);
	if (!fi || fi->font.is_null()) {
		return;
	}

	String s = String::utf8(text);
	Color c = _to_godot_color(color);
	float scale = (float)fi->size / (fi->font->get_height() > 0 ? fi->font->get_height() : 16.0f);

	// litehtml gives us the top-left of the text box; Godot draws from baseline.
	Point2 draw_pos(pos.x - m_scroll_offset.x, pos.y - m_scroll_offset.y + fi->metrics.ascent);

	// Draw text character by character with scaling applied via advance.
	float x = draw_pos.x;
	for (int i = 0; i < s.length(); i++) {
		CharType ch = s[i];
		CharType next = (i + 1 < s.length()) ? s[i + 1] : 0;
		float adv = fi->font->draw_char(m_current_canvas, Point2(x, draw_pos.y), ch, next, c) * scale;
		x += adv;
	}

	// Underline / strikethrough.
	if (fi->decoration & litehtml::font_decoration_underline) {
		float y = draw_pos.y + 2;
		VisualServer::get_singleton()->canvas_item_add_line(m_current_canvas,
				Point2(draw_pos.x, y), Point2(x, y), c, 1.0f);
	}
	if (fi->decoration & litehtml::font_decoration_linethrough) {
		float y = draw_pos.y - fi->metrics.x_height;
		VisualServer::get_singleton()->canvas_item_add_line(m_current_canvas,
				Point2(draw_pos.x, y), Point2(x, y), c, 1.0f);
	}
	if (fi->decoration & litehtml::font_decoration_overline) {
		float y = draw_pos.y - fi->metrics.ascent;
		VisualServer::get_singleton()->canvas_item_add_line(m_current_canvas,
				Point2(draw_pos.x, y), Point2(x, y), c, 1.0f);
	}
}

int GodotLiteHTML::pt_to_px(int pt) const {
	// 96 DPI standard: 1pt = 1.333px.
	return (int)((float)pt * 96.0f / 72.0f);
}

int GodotLiteHTML::get_default_font_size() const {
	return m_default_font_size;
}

const char *GodotLiteHTML::get_default_font_name() const {
	return m_default_font_name.utf8().get_data();
}

void GodotLiteHTML::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker &marker) {
	if (!m_current_canvas.is_valid()) {
		return;
	}
	Color c = _to_godot_color(marker.color);
	Rect2 rc = _to_godot_rect(marker.pos);

	switch (marker.marker_type) {
		case litehtml::list_style_type_disc: {
			// Draw filled circle.
			Vector<Point2> points;
			float cx = rc.position.x + rc.size.x / 2.0f;
			float cy = rc.position.y + rc.size.y / 2.0f;
			float r = MIN(rc.size.x, rc.size.y) / 2.0f;
			int segments = 12;
			for (int i = 0; i <= segments; i++) {
				float angle = (float)i / segments * Math_TAU;
				points.push_back(Point2(cx + cosf(angle) * r, cy + sinf(angle) * r));
			}
			Vector<Color> colors;
			colors.push_back(c);
			VisualServer::get_singleton()->canvas_item_add_polygon(m_current_canvas, points, colors);
		} break;
		case litehtml::list_style_type_circle: {
			float cx = rc.position.x + rc.size.x / 2.0f;
			float cy = rc.position.y + rc.size.y / 2.0f;
			float r = MIN(rc.size.x, rc.size.y) / 2.0f;
			int segments = 12;
			for (int i = 0; i < segments; i++) {
				float a0 = (float)i / segments * Math_TAU;
				float a1 = (float)(i + 1) / segments * Math_TAU;
				VisualServer::get_singleton()->canvas_item_add_line(m_current_canvas,
						Point2(cx + cosf(a0) * r, cy + sinf(a0) * r),
						Point2(cx + cosf(a1) * r, cy + sinf(a1) * r), c, 1.0f);
			}
		} break;
		case litehtml::list_style_type_square: {
			VisualServer::get_singleton()->canvas_item_add_rect(m_current_canvas, rc, c);
		} break;
		default: {
			// For numbered list markers, draw the text if a font is available.
			if (marker.font) {
				FontInfo *fi = _get_font_info(marker.font);
				if (fi && fi->font.is_valid()) {
					// Draw index as text.
					String idx = String::num_int64(marker.index) + ".";
					fi->font->draw(m_current_canvas,
							Point2(rc.position.x, rc.position.y + fi->metrics.ascent),
							idx, c);
				}
			}
		} break;
	}
}

void GodotLiteHTML::load_image(const char *src, const char *baseurl, bool redraw_on_ready) {
	if (!src || !src[0]) {
		return;
	}
	String url = String::utf8(src);
	// Emit signal so GDScript can load images asynchronously.
	emit_signal("image_requested", url, baseurl ? String::utf8(baseurl) : String());
}

void GodotLiteHTML::get_image_size(const char *src, const char *baseurl, litehtml::size &sz) {
	if (!src) {
		return;
	}
	String url = String::utf8(src);
	const ImageInfo *info = m_images.getptr(url);
	if (info && info->texture.is_valid()) {
		sz.width = info->texture->get_width();
		sz.height = info->texture->get_height();
	}
}

void GodotLiteHTML::draw_background(litehtml::uint_ptr hdc, const std::vector<litehtml::background_paint> &bg) {
	if (!m_current_canvas.is_valid()) {
		return;
	}

	// Draw backgrounds in reverse order (CSS order: last = farthest).
	for (int i = (int)bg.size() - 1; i >= 0; i--) {
		const litehtml::background_paint &paint = bg[i];

		// Draw background color (only the last one has valid color).
		if (i == (int)bg.size() - 1 && paint.color.alpha > 0) {
			Rect2 rc = _to_godot_rect(paint.clip_box);
			VisualServer::get_singleton()->canvas_item_add_rect(m_current_canvas, rc, _to_godot_color(paint.color));
		}

		// Draw background image.
		if (!paint.image.empty()) {
			String url = String::utf8(paint.image.c_str());
			const ImageInfo *info = m_images.getptr(url);
			if (info && info->texture.is_valid()) {
				Rect2 dest = _to_godot_rect(paint.border_box);
				if (paint.image_size.width > 0 && paint.image_size.height > 0) {
					dest = Rect2(
							paint.position_x + paint.border_box.x - m_scroll_offset.x,
							paint.position_y + paint.border_box.y - m_scroll_offset.y,
							paint.image_size.width, paint.image_size.height);
				}
				VisualServer::get_singleton()->canvas_item_add_texture_rect(
						m_current_canvas, dest,
						info->texture->get_rid(), false, Color(1, 1, 1, 1), false);
			}
		}
	}
}

void GodotLiteHTML::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders &borders, const litehtml::position &draw_pos, bool root) {
	if (!m_current_canvas.is_valid()) {
		return;
	}

	Rect2 rc = _to_godot_rect(draw_pos);

	// Draw each border side as a line.
	if (borders.top.width > 0 && borders.top.style != litehtml::border_style_none && borders.top.style != litehtml::border_style_hidden) {
		Color c = _to_godot_color(borders.top.color);
		float y = rc.position.y + borders.top.width / 2.0f;
		VisualServer::get_singleton()->canvas_item_add_line(m_current_canvas,
				Point2(rc.position.x, y), Point2(rc.position.x + rc.size.x, y), c, (float)borders.top.width);
	}
	if (borders.bottom.width > 0 && borders.bottom.style != litehtml::border_style_none && borders.bottom.style != litehtml::border_style_hidden) {
		Color c = _to_godot_color(borders.bottom.color);
		float y = rc.position.y + rc.size.y - borders.bottom.width / 2.0f;
		VisualServer::get_singleton()->canvas_item_add_line(m_current_canvas,
				Point2(rc.position.x, y), Point2(rc.position.x + rc.size.x, y), c, (float)borders.bottom.width);
	}
	if (borders.left.width > 0 && borders.left.style != litehtml::border_style_none && borders.left.style != litehtml::border_style_hidden) {
		Color c = _to_godot_color(borders.left.color);
		float x = rc.position.x + borders.left.width / 2.0f;
		VisualServer::get_singleton()->canvas_item_add_line(m_current_canvas,
				Point2(x, rc.position.y), Point2(x, rc.position.y + rc.size.y), c, (float)borders.left.width);
	}
	if (borders.right.width > 0 && borders.right.style != litehtml::border_style_none && borders.right.style != litehtml::border_style_hidden) {
		Color c = _to_godot_color(borders.right.color);
		float x = rc.position.x + rc.size.x - borders.right.width / 2.0f;
		VisualServer::get_singleton()->canvas_item_add_line(m_current_canvas,
				Point2(x, rc.position.y), Point2(x, rc.position.y + rc.size.y), c, (float)borders.right.width);
	}
}

void GodotLiteHTML::set_caption(const char *caption) {
	if (caption) {
		emit_signal("caption_changed", String::utf8(caption));
	}
}

void GodotLiteHTML::set_base_url(const char *base_url) {
	if (base_url) {
		m_base_url = String::utf8(base_url);
	}
}

void GodotLiteHTML::link(const std::shared_ptr<litehtml::document> &doc, const litehtml::element::ptr &el) {
	// CSS link elements — could load external stylesheets.
}

void GodotLiteHTML::on_anchor_click(const char *url, const litehtml::element::ptr &el) {
	if (url) {
		emit_signal("anchor_clicked", String::utf8(url));
	}
}

void GodotLiteHTML::set_cursor(const char *cursor) {
	if (!cursor) {
		return;
	}
	String c = String::utf8(cursor);
	if (c == "pointer") {
		set_default_cursor_shape(CURSOR_POINTING_HAND);
	} else if (c == "text") {
		set_default_cursor_shape(CURSOR_IBEAM);
	} else {
		set_default_cursor_shape(CURSOR_ARROW);
	}
}

void GodotLiteHTML::transform_text(litehtml::string &text, litehtml::text_transform tt) {
	String s = String::utf8(text.c_str());
	switch (tt) {
		case litehtml::text_transform_capitalize: {
			s = s.capitalize();
		} break;
		case litehtml::text_transform_uppercase: {
			s = s.to_upper();
		} break;
		case litehtml::text_transform_lowercase: {
			s = s.to_lower();
		} break;
		default:
			return;
	}
	CharString utf8 = s.utf8();
	text = utf8.get_data();
}

void GodotLiteHTML::import_css(litehtml::string &text, const litehtml::string &url, litehtml::string &baseurl) {
	// External CSS import — emit signal for GDScript to handle.
	emit_signal("css_import_requested", String::utf8(url.c_str()), String::utf8(baseurl.c_str()));
}

void GodotLiteHTML::set_clip(const litehtml::position &pos, const litehtml::border_radiuses &bdr_radius) {
	ClipInfo ci;
	ci.pos = pos;
	ci.radius = bdr_radius;
	m_clips.push_back(ci);
}

void GodotLiteHTML::del_clip() {
	if (m_clips.size() > 0) {
		m_clips.resize(m_clips.size() - 1);
	}
}

void GodotLiteHTML::get_client_rect(litehtml::position &client) const {
	client.x = 0;
	client.y = 0;
	client.width = (int)get_size().x;
	client.height = (int)get_size().y;
}

litehtml::element::ptr GodotLiteHTML::create_element(const char *tag_name, const litehtml::string_map &attributes, const std::shared_ptr<litehtml::document> &doc) {
	return nullptr; // Use default element creation.
}

void GodotLiteHTML::get_media_features(litehtml::media_features &media) const {
	media.type = litehtml::media_type_screen;
	media.width = (int)get_size().x;
	media.height = (int)get_size().y;
	media.device_width = (int)OS::get_singleton()->get_window_size().x;
	media.device_height = (int)OS::get_singleton()->get_window_size().y;
	media.color = 8;
	media.color_index = 0;
	media.monochrome = 0;
	media.resolution = 96;
}

void GodotLiteHTML::get_language(litehtml::string &language, litehtml::string &culture) const {
	String locale = OS::get_singleton()->get_locale();
	CharString utf8 = locale.utf8();
	language = utf8.get_data();
	culture = utf8.get_data();
}

// --- Bindings ---

void GodotLiteHTML::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_html", "html"), &GodotLiteHTML::set_html);
	ClassDB::bind_method(D_METHOD("get_html"), &GodotLiteHTML::get_html);

	ClassDB::bind_method(D_METHOD("set_user_css", "css"), &GodotLiteHTML::set_user_css);
	ClassDB::bind_method(D_METHOD("get_user_css"), &GodotLiteHTML::get_user_css);

	ClassDB::bind_method(D_METHOD("set_html_base_url", "url"), &GodotLiteHTML::set_html_base_url);
	ClassDB::bind_method(D_METHOD("get_html_base_url"), &GodotLiteHTML::get_html_base_url);

	ClassDB::bind_method(D_METHOD("set_default_font", "font"), &GodotLiteHTML::set_default_font);
	ClassDB::bind_method(D_METHOD("get_default_font"), &GodotLiteHTML::get_default_font);

	ClassDB::bind_method(D_METHOD("set_font_name", "name"), &GodotLiteHTML::set_font_name);
	ClassDB::bind_method(D_METHOD("get_font_name"), &GodotLiteHTML::get_font_name);

	ClassDB::bind_method(D_METHOD("set_font_size", "size"), &GodotLiteHTML::set_font_size);
	ClassDB::bind_method(D_METHOD("get_font_size"), &GodotLiteHTML::get_font_size);

	ClassDB::bind_method(D_METHOD("set_render_mode", "mode"), &GodotLiteHTML::set_render_mode);
	ClassDB::bind_method(D_METHOD("get_render_mode"), &GodotLiteHTML::get_render_mode);

	ClassDB::bind_method(D_METHOD("get_render_texture"), &GodotLiteHTML::get_render_texture);
	ClassDB::bind_method(D_METHOD("get_content_width"), &GodotLiteHTML::get_content_width);
	ClassDB::bind_method(D_METHOD("get_content_height"), &GodotLiteHTML::get_content_height);

	ClassDB::bind_method(D_METHOD("set_scroll_offset", "offset"), &GodotLiteHTML::set_scroll_offset);
	ClassDB::bind_method(D_METHOD("get_scroll_offset"), &GodotLiteHTML::get_scroll_offset);

	ClassDB::bind_method(D_METHOD("set_image", "url", "texture"), &GodotLiteHTML::set_image);
	ClassDB::bind_method(D_METHOD("render_html"), &GodotLiteHTML::render_html);

	ClassDB::bind_method(D_METHOD("html_mouse_over", "position"), &GodotLiteHTML::html_mouse_over);
	ClassDB::bind_method(D_METHOD("html_mouse_down", "position"), &GodotLiteHTML::html_mouse_down);
	ClassDB::bind_method(D_METHOD("html_mouse_up", "position"), &GodotLiteHTML::html_mouse_up);
	ClassDB::bind_method(D_METHOD("html_mouse_leave"), &GodotLiteHTML::html_mouse_leave);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "html", PROPERTY_HINT_MULTILINE_TEXT), "set_html", "get_html");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "user_css", PROPERTY_HINT_MULTILINE_TEXT), "set_user_css", "get_user_css");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "base_url"), "set_html_base_url", "get_html_base_url");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "default_font", PROPERTY_HINT_RESOURCE_TYPE, "Font"), "set_default_font", "get_default_font");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "default_font_name"), "set_font_name", "get_font_name");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "default_font_size", PROPERTY_HINT_RANGE, "6,128,1"), "set_font_size", "get_font_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "render_mode", PROPERTY_HINT_ENUM, "Direct,Texture"), "set_render_mode", "get_render_mode");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "scroll_offset"), "set_scroll_offset", "get_scroll_offset");

	ADD_SIGNAL(MethodInfo("anchor_clicked", PropertyInfo(Variant::STRING, "url")));
	ADD_SIGNAL(MethodInfo("image_requested", PropertyInfo(Variant::STRING, "url"), PropertyInfo(Variant::STRING, "base_url")));
	ADD_SIGNAL(MethodInfo("caption_changed", PropertyInfo(Variant::STRING, "caption")));
	ADD_SIGNAL(MethodInfo("css_import_requested", PropertyInfo(Variant::STRING, "url"), PropertyInfo(Variant::STRING, "base_url")));
	ADD_SIGNAL(MethodInfo("cursor_changed", PropertyInfo(Variant::STRING, "cursor")));

	BIND_ENUM_CONSTANT(RENDER_DIRECT);
	BIND_ENUM_CONSTANT(RENDER_TEXTURE);
}

GodotLiteHTML::GodotLiteHTML() {
	m_needs_render = false;
	m_needs_redraw = false;
	m_render_mode = RENDER_DIRECT;
	m_next_font_id = 0;
	m_default_font_name = "serif";
	m_default_font_size = 16;
}

GodotLiteHTML::~GodotLiteHTML() {
	m_document.reset();
	m_fonts.clear();
	m_images.clear();
}
