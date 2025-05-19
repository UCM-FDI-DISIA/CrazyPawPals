
#include "WeaponBoom.h"
#include "../../../../game/Game.h"
#include "../../../../game/scenes/GameScene.h"
#include "our_scripts/card_system/ShootPatrons.hpp"

WeaponBoom::WeaponBoom() : Weapon(15, 5000, 20.0f, 0.0f, "p_boom", 2.0f, 2.0f) {}

WeaponBoom::~WeaponBoom() {}

void WeaponBoom::callback(Vector2D shootPos, Vector2D shootDir)
{
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	// float dist = 0.1;
	bp.dir = shootDir;
	bp.init_pos = shootPos + shootDir.normalize() * 1.5;
	bp.speed = _speed;
	bp.damage = _damage;
	bp.life_time = 2;
	bp.width = _attack_width;
	bp.height = _attack_height;
	bp.sprite_key = "p_boom";
	bp.collision_filter = GameStructs::collide_with::player;

	Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);
}