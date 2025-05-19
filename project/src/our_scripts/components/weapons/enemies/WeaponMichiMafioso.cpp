
#include "WeaponMichiMafioso.h"
#include "../../../../game/Game.h"
#include "../../../../game/scenes/GameScene.h"
#include "../../movement/Transform.h"

WeaponMichiMafioso::WeaponMichiMafioso() : Weapon(5, 3000, 20.0f, 0.13f, "p_michi_mafioso", 1.0f, 1.0f)
{
	_bp.speed = _speed;
	_bp.damage = _damage;
	_bp.life_time = 2;
	_bp.width = _attack_width;
	_bp.sprite_key = "p_michi_mafioso";
	_bp.height = _attack_height;
	_bp.collision_filter = GameStructs::collide_with::player;
}

void WeaponMichiMafioso::update(uint32_t delta_time)
{
	if (_shooting && _time_to_shoot < sdlutils().virtualTimer().currTime())
	{
		delayed_shoot();
		_shooting = false;
	}
}

void WeaponMichiMafioso::callback(Vector2D shootPos, Vector2D shootDir)
{
	_bp.dir = shootDir;
	//_bp.init_pos = shootPos;
	_shooting = true;
	_time_to_shoot = sdlutils().virtualTimer().currTime() + 1000;
}

void WeaponMichiMafioso::delayed_shoot()
{
	_bp.init_pos = _tr->getPos();
	Game::Instance()->get_currentScene()->create_proyectile(_bp, ecs::grp::BULLET);
}
