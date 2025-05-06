
#include "Image.h"

#include <cassert>

#include "../../../ecs/Manager.h"
#include "../../../sdlutils/macros.h"
#include "../../../sdlutils/Texture.h"
#include "../movement/Transform.h"
#include "../../../game/Game.h"


Image::Image() :
		_tr(), _tex() {
}

Image::Image(Texture *tex) :
		_tr(), _tex(tex) {
}

Image::~Image() {
}

void Image::initComponent() {
	_tr = Game::Instance()->get_mngr()->getComponent<Transform>(_ent);
	assert(_tr != nullptr);
}

void Image::render() {

	//assert(false && "deprecated: removed functionality, use dyn_image");
	 SDL_Rect dest = build_sdlrect(_tr->getPos(), _tr->getWidth(),
	 		_tr->getHeight());

	 assert(_tex != nullptr);
	 _tex->render(dest, _tr->getRot());

}
