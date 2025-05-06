

#include "Texture.h"

Texture& Texture::operator=(Texture &&other) noexcept {
	this->~Texture();
	_texture = other._texture;
	other._texture = nullptr;
	_renderer = other._renderer;
	other._renderer = nullptr;
	_width = other._width;
	_height = other._height;

	return *this;
}

Texture::Texture(Texture &&other) noexcept {
	_texture = other._texture;
	other._texture = nullptr;
	_renderer = other._renderer;
	other._renderer = nullptr;
	_width = other._width;
	_height = other._height;
}

Texture::Texture(SDL_Renderer *renderer, const std::string &fileName) {
	assert(renderer != nullptr);
	_renderer = renderer;

	SDL_Surface *surface = IMG_Load(fileName.c_str());
#ifdef _DEBUG
	if (surface == nullptr)
		std::cerr << "Couldn't load image: " << fileName;
#endif
	assert(surface != nullptr);

	_width = surface->w;
	_height = surface->h;

	_texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	assert(_texture != nullptr);
	SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
}

Texture::Texture(SDL_Renderer *renderer, const std::string &text,
		const Font &font, const SDL_Color &fgColor) {
	constructFromText(renderer, text, font, &fgColor);
}

Texture::Texture(SDL_Renderer *renderer, const std::string &text,
		const Font &font, const SDL_Color &fgColor, const SDL_Color &bgColor) {
	constructFromText(renderer, text, font, &fgColor, &bgColor);
}

void Texture::constructFromText(SDL_Renderer *renderer, const std::string &text,
		const Font &font, const SDL_Color *fgColor, const SDL_Color *bgColor) {

	assert(renderer != nullptr);
	_renderer = renderer;

	SDL_Surface *textSurface =
			bgColor == nullptr ?
					font.renderText(text, *fgColor) :
					font.renderText(text, *fgColor, *bgColor);
	assert(textSurface != nullptr);

	_width = textSurface->w;
	_height = textSurface->h;

	_texture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_FreeSurface(textSurface);
	assert(_texture != nullptr);
}
