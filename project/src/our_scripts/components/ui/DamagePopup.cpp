#include "DamagePopup.h"
#include "../movement/Transform.h"
//#include "../rendering/dyn_image.hpp"
#include "../../../ecs/Manager.h"
#include "../../../game/Game.h"

DamagePopup::DamagePopup():
	_tr(nullptr),
	_time_since_spawn(0U)
{
}

void DamagePopup::initComponent()
{
	_tr = Game::Instance()->get_mngr()->getComponent<Transform>(_ent);
	//_img = Game::Instance()->get_mngr()->getComponent<dyn_image>(_ent);
	_time_since_spawn = 0;
}

void DamagePopup::update(uint32_t dt)
{
	_time_since_spawn += dt;
	if (_time_since_spawn < _movement_time) {
		_tr->getPos() += Vector2D(0, (_movement_time - _time_since_spawn) / 10);
	}
	else if (_time_since_spawn > _total_time) {
		Game::Instance()->get_mngr()->setAlive(_ent, false);
	}
}
