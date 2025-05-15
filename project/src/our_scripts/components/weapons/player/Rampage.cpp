
#include "Rampage.h"
#include "../../../../game/Game.h"
#include "../../../../game/scenes/GameScene.h"
#include "our_scripts/card_system/ShootPatrons.hpp"


Rampage::Rampage() : Weapon(1, 550.0f, 0.4f, 0.15f, "p_rampage", 1.0f, 1.0f) { }

Rampage::~Rampage() {}

void 
Rampage::callback(Vector2D shootPos, Vector2D shootDir) {
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.dir = shootDir;
	bp.init_pos = shootPos;
	bp.speed = _speed;
	bp.damage = _damage;
	bp.life_time = _distance;
	bp.width = _attack_width * 0.6f;
	bp.height = _attack_height * 0.6f;
	bp.sprite_key = _tex;
	bp.weapon_type = GameStructs::RAMPAGE;
	bp.collision_filter = GameStructs::collide_with::enemy;
	patrons::ShotgunPatron(bp, ecs::grp::BULLET, 120, 5);
}