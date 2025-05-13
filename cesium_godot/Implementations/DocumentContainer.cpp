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
#include "litehtml/os_types.h"
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
	// Ref<TextLine> textLine = memnew(TextLine);
	// Let's try the stack alloc approach
	FontHandle* fontHandle = reinterpret_cast<FontHandle*>(hFont);
	TextLine textLine;
	this->draw_string(fontHandle->font, Vector2(pos.x, pos.y + fontHandle->ascent), text);
}

