
#include "FlipXController.h"
#include "../../../ecs/Manager.h"
#include "../../../game/Game.h"
#include "../movement/Transform.h"
#include "dyn_image.hpp"
#include "dyn_image_with_frames.hpp"

FlipXController::FlipXController() :_tr(nullptr), _dy(nullptr) {}

void FlipXController::initComponent() {
	_tr = Game::Instance()->get_mngr()->getComponent<Transform>(_ent);
	assert(_tr != nullptr);

	_dy = Game::Instance()->get_mngr()->getComponent<dyn_image>(_ent);
	if (!_dy) _dy = Game::Instance()->get_mngr()->getComponent<dyn_image_with_frames>(_ent);
	assert(_dy != nullptr);
}

void FlipXController::update(uint32_t delta_time) {
    (void)delta_time;
	float dirX = _tr->getDir().getX();
	_dy->flip = dirX > 0.001 ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
}