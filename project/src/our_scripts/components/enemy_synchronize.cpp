#include "enemy_synchronize.h"
#include "../../sdlutils/SDLUtils.h"
#include "../../ecs/Manager.h"
#include "../../game/Game.h"
#include <algorithm>
#include "movement/Transform.h"


EnemySynchronize::EnemySynchronize() 
	: _tr(nullptr) {};


EnemySynchronize::~EnemySynchronize() {};

void EnemySynchronize::initComponent()
{
	_tr = Game::Instance()->get_mngr()->getComponent<Transform>(_ent);
	
}

void
EnemySynchronize::update(uint32_t delta_time) {
	(void)delta_time;
	
}

