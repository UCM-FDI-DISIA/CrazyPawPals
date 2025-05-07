
#include "WeaponSarnoRata.h"
#include "../../../../game/Game.h"
#include "../../../../game/scenes/GameScene.h"
#include "../../movement/Transform.h"

WeaponSarnoRata::WeaponSarnoRata() : Weapon(6, 2500, 20.0f, 0.0f, "p_sarno_rata", 5.0f, 5.0f){
	//_bp.dir = shootDir;
	//_bp.init_pos = shootPos + shootDir.normalize() * 0.5f;
	_bp.speed = _speed;
	_bp.damage = _damage;
	_bp.life_time = 2;
	_bp.width = float(_attack_width) * 0.8f;
	_bp.height = float(_attack_height) * 0.8f;
	_bp.sprite_key = "p_sarno_rata";
	_bp.collision_filter = GameStructs::collide_with::player;
}

WeaponSarnoRata::~WeaponSarnoRata() {}

void 
WeaponSarnoRata::callback(Vector2D shootPos, Vector2D shootDir) {
	_bp.dir = shootDir;
	_bp.init_pos = shootPos;
	_shooting = true;
	_time_to_shoot = sdlutils().virtualTimer().currTime() + 400;
}
void WeaponSarnoRata::update(uint32_t delta_time)
{
	if (_shooting && _time_to_shoot < sdlutils().virtualTimer().currTime()) {
		delayed_shoot();
		_shooting = false;
	}
}

void WeaponSarnoRata::delayed_shoot()
{
	_bp.init_pos = _tr->getPos();
	Game::Instance()->get_currentScene()->create_proyectile(_bp, ecs::grp::BULLET);
}
