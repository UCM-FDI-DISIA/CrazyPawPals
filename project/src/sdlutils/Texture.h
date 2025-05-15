
#pragma once


#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <cassert>
#include <iostream>
#include <cstdint>

#include "Font.h"

class Texture {
public:

	// cannot copy textures
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	// can be moved
	Texture& operator=(Texture &&other) noexcept;
	Texture(Texture &&other) noexcept;

	// Construct from image
	Texture(SDL_Renderer *renderer, const std::string &fileName);

	// Construct from text
	Texture(SDL_Renderer *renderer, const std::string &text, const Font &font,
			const SDL_Color &fgColor);

	// Construct from text with background
	Texture(SDL_Renderer *renderer, const std::string &text, const Font &font,
			const SDL_Color &fgColor, const SDL_Color &bgColor);


	virtual ~Texture() {
		if (_texture != nullptr)
			SDL_DestroyTexture(_texture); // delete the SDL texture
	}

	inline int width() const {
		return _width;
	}

	inline int height() const {
		return _height;
	}

	// This rendering method corresponds to method SDL_RenderCopyEx.
	//
	// Renders part of the texture (src) to a destination rectangle (dest)
	// with a rotation (angle) around point p (of dest), and flips
	// according to the value of flip. If 'p'is nullptr, the rotation is done
	// wrt. the center
	//
	inline void render(const SDL_Rect &src, const SDL_Rect &dest, double angle,
			const SDL_Point *p = nullptr,
			SDL_RendererFlip flip = SDL_FLIP_NONE) {
		assert(_texture != nullptr);
		SDL_RenderCopyEx(_renderer, _texture, &src, &dest, angle, p, flip);
	}

	// This rendering method corresponds to method SDL_RenderCopy.
	//
	// Renders part of the texture (src) to a destination rectangle (dest).
	// It can be implemented by calling the previous render method as well,
	// but we use SDL_RenderCopy directly since it does less checks so it
	// saves some checks ...
	inline void render(const SDL_Rect &src, const SDL_Rect &dest) {
		assert(_texture != nullptr);
		SDL_RenderCopy(_renderer, _texture, &src, &dest);
	}

	// render the complete texture at position (x,y).
	inline void render(int x, int y) {
		SDL_Rect dest = { x, y, _width, _height };
		render(dest);
	}

	// renders the complete texture at a destination rectangle (dest)
	inline void render(const SDL_Rect &dest) {
		SDL_Rect src = { 0, 0, _width, _height };
		render(src, dest);
	}

	// renders the complete texture at a destination rectangle (dest),
	// with rotation
	inline void render(const SDL_Rect &dest, float rotation) {
		SDL_Rect src = { 0, 0, _width, _height };
		render(src, dest, double(rotation));
	}

	inline auto &&get_texture() {
		return *_texture;
	}

	inline void apply_filter(int r, int g, int b, int a = 255) {
		SDL_SetTextureColorMod(_texture, r, g, b);
		SDL_SetTextureAlphaMod(_texture, a);
	}

private:

	// Construct from text
	void constructFromText(SDL_Renderer *renderer, const std::string &text,
			const Font &font, const SDL_Color *fgColor,
			const SDL_Color *bgColor = nullptr);

	SDL_Texture *_texture;
	SDL_Renderer *_renderer;
	int _width;
	int _height;
};
