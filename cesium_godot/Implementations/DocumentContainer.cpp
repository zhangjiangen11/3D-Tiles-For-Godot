#include "DocumentContainer.h"
#include "godot_cpp/classes/font.hpp"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/text_line.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/core/memory.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/rect2.hpp"
#include "litehtml/background.h"
#include "litehtml/document.h"
#include "litehtml/os_types.h"
#include "litehtml/types.h"
#include <cstdint>
#include <cstdio>


constexpr inline uint32_t HashFnv1a(const char *data, const uint32_t length)
{
	// NOLINTNEXTLINE
	uint32_t hash = 0x811c9dc5;
	const uint32_t prime = 0x1000193;

	for (uint32_t i = 0; i < length; ++i)
	{
		uint8_t value = data[i];
		hash = hash ^ value;
		hash *= prime;
	}

	return hash;
}


constexpr inline uint32_t HashFnv1a(const std::string_view &data)
{
	return HashFnv1a(data.data(), static_cast<uint32_t>(data.size()));
}


struct FontHandle {
	Ref<Font> font;
	float ascent;
	float descent;
	float height;
};


const char*	DocumentContainer::get_default_font_name() const {
	return Control::get_theme_default_font()->get_font_name().utf8().get_data();
}


void DocumentContainer::set_html_stl(const std::string_view& html) {
	this->m_document = litehtml::document::createFromString(html.data(), this);
	this->m_document->render(this->get_size().width);
	this->queue_redraw();
}

void DocumentContainer::set_html(const String& html) {
	this->m_document = litehtml::document::createFromString(html.utf8().get_data(), this);
	this->m_document->render(this->get_size().width);
	this->queue_redraw();
}

void DocumentContainer::_draw() {
	if (this->m_document == nullptr) return;
	litehtml::position origin(
		0,
		0,
		this->get_size().width,
		this->get_size().height
	);
	this->m_document->draw(0, 0, 0, &origin);
}

void DocumentContainer::get_client_rect(litehtml::position& client) const {
	Rect2 rect = this->get_global_rect();
	client.x = rect.position.x;
	client.y = rect.position.y;
	client.width = rect.size.width;
	client.height = rect.size.height;
}

litehtml::uint_ptr DocumentContainer::create_font(const char* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics* fm) {
	FontHandle* handle = new FontHandle();
	handle->font = Control::get_theme_default_font();
	handle->ascent = handle->font->get_ascent(size);
	handle->descent = handle->font->get_descent(size);
	handle->height = handle->font->get_height(size);

	if (fm != nullptr) {
		fm->ascent = handle->ascent;
		fm->descent = handle->descent;
		fm->height = handle->height;
		fm->x_height = handle->height * 0.5f;
	}
	
	return reinterpret_cast<litehtml::uint_ptr>(handle);
}


void DocumentContainer::delete_font(litehtml::uint_ptr hFont) {
	FontHandle* handle = reinterpret_cast<FontHandle*>(hFont);
	delete handle;
}

int	DocumentContainer::text_width(const char* text, litehtml::uint_ptr hFont) {
	FontHandle* handle = reinterpret_cast<FontHandle*>(hFont);
	return handle->font->get_string_size(text).x;
}

int DocumentContainer::pt_to_px(int pt) const {
	constexpr float PT_TO_PX_FACTOR = (72.0f / 96.0f);
	return pt * PT_TO_PX_FACTOR; // Narrowing conversion, but that's fine
}

int DocumentContainer::get_default_font_size() const {
	return 16;
}

void DocumentContainer::draw_background(litehtml::uint_ptr hdc, const std::vector<litehtml::background_paint>& bg) {
	for (const litehtml::background_paint& backgroundItem : bg) {
		printf("BG image string: %s\n", backgroundItem.image.c_str());
		uint32_t hash = HashFnv1a(backgroundItem.image);
		if (this->m_imageCache.find(hash) == this->m_imageCache.end()) {
			// WARN_PRINT("Image not found cache!");
			continue;
		}
		const Ref<ImageTexture>& texture = this->m_imageCache.at(hash);
		
		Rect2 destinationRect(
			backgroundItem.position_x + backgroundItem.origin_box.x,
			backgroundItem.position_y + backgroundItem.origin_box.y,
			texture->get_width(),
			texture->get_height()
		);
		this->draw_texture_rect(texture, destinationRect, false);
	}
}

void DocumentContainer::load_image(const char* src, const char* baseurl, bool redraw_on_ready) {
	printf("Src: %s, Base url: %s\n", src, baseurl);
	std::string fetchUrl = src;
	if (this->m_imageCache.find(HashFnv1a(fetchUrl)) != this->m_imageCache.end()) {
		return;
	}
	this->m_httpClient.send_get(src, [fetchUrl, this](int32_t status, const PackedByteArray& body) {
        if (status >= HTTPClient::ResponseCode::RESPONSE_BAD_REQUEST) {
			std::string tmpStr(reinterpret_cast<const char*>(body.ptr()), body.size());
	        String bodyStr = tmpStr.c_str();
        	ERR_PRINT(String("Not able to load image from ") + String(fetchUrl.c_str()) + String(", response: ") + bodyStr);
        	return;
        }

		// All of the images we need to handle are most likely png
		Ref<Image> image = memnew(Image);
		Error err = image->load_png_from_buffer(body);
		
		if (err != Error::OK) {
			ERR_PRINT(String("Failed to parse image, possible bad format, source: ") + String(fetchUrl.c_str()));
			return;
		}
		
		// Create an ImageTexture based on the data
		Ref<ImageTexture> texture = ImageTexture::create_from_image(image);
		
        // We do copy the data, but that's fine
        this->m_imageCache.insert_or_assign(HashFnv1a(fetchUrl), texture);
	}, {});
}


void DocumentContainer::draw_text(litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) {
	// Let's try the stack alloc approach
	FontHandle* fontHandle = reinterpret_cast<FontHandle*>(hFont);
	// Ref<TextLine> textLine = memnew(TextLine);
	this->draw_string(fontHandle->font, Vector2(pos.x, pos.y + fontHandle->ascent), text);
}


void DocumentContainer::get_image_size(const char* src, const char* baseurl, litehtml::size& sz) {
	
}

void DocumentContainer::draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) {
	
}

void DocumentContainer::set_caption(const char* caption) {
	
}

void DocumentContainer::set_base_url(const char* base_url) {
	
}

void DocumentContainer::link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) {
	
}

void DocumentContainer::on_anchor_click(const char* url, const litehtml::element::ptr& el) {
	
}

void DocumentContainer::set_cursor(const char* cursor) {
	
}

void DocumentContainer::transform_text(litehtml::string& text, litehtml::text_transform tt) {
	
}

void DocumentContainer::import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) {
	
}

void DocumentContainer::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius) {
	
}

void DocumentContainer::del_clip() {
	
}

void DocumentContainer::get_media_features(litehtml::media_features& media) const {
	
}

void DocumentContainer::get_language(litehtml::string& language, litehtml::string& culture) const {
	
}

void DocumentContainer::split_text(const char* text, const std::function<void(const char*)>& on_word, const std::function<void(const char*)>& on_space) {
	
}


void DocumentContainer::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) {

}


litehtml::element::ptr	DocumentContainer::create_element( const char* tag_name,
													const litehtml::string_map& attributes,
													const std::shared_ptr<litehtml::document>& doc) {
	return nullptr;
}

void DocumentContainer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_html", "html"), &DocumentContainer::set_html);
}


