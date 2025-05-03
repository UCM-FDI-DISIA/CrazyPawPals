#include "player_synchronize.h"
#include "../../sdlutils/SDLUtils.h"
#include "../../ecs/Manager.h"
#include "../../game/Game.h"
#include <algorithm>
#include "movement/Transform.h"
#include "Health.h"
#include "GhostStateComponent.h"
#include "rendering/dyn_image_with_frames.hpp"


PlayerSynchronize::PlayerSynchronize()
	: _player_id(0), _tr(nullptr),_health(nullptr),_is_ghost(false){};


PlayerSynchronize::~PlayerSynchronize() {};

void PlayerSynchronize::initComponent()
{
	uint8_t _player_id;

	auto&& mngr = Game::Instance()->get_mngr();
	_tr = mngr->getComponent<Transform>(_ent);
	_health= mngr->getComponent<Health>(_ent);
	
	_tex_name = mngr->getComponent<dyn_image_with_frames>(_ent)->texture_name;
	_is_ghost = mngr->hasComponent<GhostStateComponent>(_ent);
}

void
PlayerSynchronize::update(uint32_t delta_time) {
	(void)delta_time;
	
}

