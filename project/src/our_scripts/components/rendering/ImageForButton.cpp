
#include "ImageForButton.h"

#include <cassert>

#include "../../../ecs/Manager.h"
#include "../../../sdlutils/macros.h"
#include "../../../sdlutils/Texture.h"
#include "../movement/Transform.h"
#include "../../../game/Game.h"

ImageForButton::ImageForButton(Texture* tex, Texture* stex, const rect_f32& viewportRect, float rotation, const camera_screen& cam)
	: transformless_dyn_image(viewportRect, rotation, cam, tex), _selected_tex(stex) {
	assert(stex != nullptr);
}
void
ImageForButton::swap_textures() {
	Texture* aux = texture;
	texture = _selected_tex; 
	_selected_tex = aux;
}
