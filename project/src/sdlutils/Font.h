
#pragma once



#include <SDL_ttf.h>
#include <string>
#include <cassert>
#include <iostream>

#include "macros.h"

class Font {
public:

	// cannot copy objects of this type!
	Font(const Font &other) = delete;
	Font& operator=(const Font &other) = delete;

	// can be moved
	Font& operator=(Font &&other) noexcept {
		this->~Font();
		_font = other._font;
		other._font = nullptr;
		return *this;
	}

	Font(Font &&other) noexcept {
		_font = other._font;
		other._font = nullptr;
	}

	Font(const std::string &fileName, int size) {
		_font = TTF_OpenFont(fileName.c_str(), size);
		assert(_font != nullptr);
	}

	virtual ~Font() {
		if (_font != nullptr)
			TTF_CloseFont(_font);
	}

	inline SDL_Surface* renderText(const std::string &text,
			SDL_Color fgColor) const {
		assert(_font != nullptr);
		return TTF_RenderUTF8_Solid(_font, text.c_str(), fgColor);
	}

	inline SDL_Surface* renderText(const std::string &text, SDL_Color fgColor,
			SDL_Color bgColor) const {
		assert(_font != nullptr);
		return TTF_RenderUTF8_Shaded(_font, text.c_str(), fgColor, bgColor);
	}

private:
	TTF_Font *_font;
}
;
