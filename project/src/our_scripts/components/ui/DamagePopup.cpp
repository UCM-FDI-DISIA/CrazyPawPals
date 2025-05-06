
#include "DamagePopup.h"
#include "../movement/Transform.h"
//#include "../rendering/dyn_image.hpp"
#include "../../../ecs/Manager.h"
#include "../../../game/Game.h"
#include <cmath>

DamagePopup::DamagePopup():
	_tr(nullptr),
	_time_since_spawn(0U)
{
}

void DamagePopup::initComponent()
{
	_tr = Game::Instance()->get_mngr()->getComponent<Transform>(_ent);
	_target_y = _tr->getPos().getY() + 1.0f;
	//_img = Game::Instance()->get_mngr()->getComponent<dyn_image>(_ent);
	_time_since_spawn = 0U;
}

void DamagePopup::update(uint32_t dt)
{
	_time_since_spawn += dt;
	if (_time_since_spawn < _movement_time) {
		_tr->getPos() = Vector2D(_tr->getPos().getX(), std::lerp(_tr->getPos().getY(), _target_y, 0.1f));
		//std::cout << (_movement_time - _time_since_spawn) * 0.001 <<std::endl;
	}
	else if (_time_since_spawn > _total_time) {
		Game::Instance()->get_mngr()->setAlive(_ent, false);
	}
}
