#ifndef DOCUMENT_CONTAINER_H
#define DOCUMENT_CONTAINER_H

#include "litehtml.h"

class DocumentContainer : public litehtml::document_container {
public:
	
	litehtml::uint_ptr	create_font(const char* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics* fm) override;
	void delete_font(litehtml::uint_ptr hFont) override;
	int	text_width(const char* text, litehtml::uint_ptr hFont) override;
	void draw_text(litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) override;
	int pt_to_px(int pt) const override;
	int get_default_font_size() const override;
	const char*	get_default_font_name() const override;
	void draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) override;
	void load_image(const char* src, const char* baseurl, bool redraw_on_ready) override;
	void get_image_size(const char* src, const char* baseurl, litehtml::size& sz) override;
	void draw_background(litehtml::uint_ptr hdc, const std::vector<litehtml::background_paint>& bg) override;
	void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) override;
	void set_caption(const char* caption) override;
	void set_base_url(const char* base_url) override;
	void link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) override;
	void on_anchor_click(const char* url, const litehtml::element::ptr& el) override;
	void set_cursor(const char* cursor) override;
	void transform_text(litehtml::string& text, litehtml::text_transform tt) override;
	void import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) override;
	void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius) override;
	void del_clip() override;
	void get_client_rect(litehtml::position& client) const override;
	litehtml::element::ptr create_element( const char* tag_name,
	void get_media_features(litehtml::media_features& media) const override;
	void get_language(litehtml::string& language, litehtml::string& culture) const override;
	// litehtml::string resolve_color(const litehtml::string& /*color*/) const { return litehtml::string(); }
	void split_text(const char* text, const std::function<void(const char*)>& on_word, const std::function<void(const char*)>& on_space);	
};

#endif
