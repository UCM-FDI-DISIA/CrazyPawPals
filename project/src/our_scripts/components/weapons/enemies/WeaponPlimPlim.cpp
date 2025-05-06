#include "WeaponPlimPlim.h"
#include "../../../../game/Game.h"
#include "../../../../game/scenes/GameScene.h"
#include "../../../card_system/ShootPatrons.hpp"

WeaponPlimPlim::WeaponPlimPlim() : Weapon(4, 5000, 20.0f, 0.1f, "p_plimplim", 1.0f, 1.0f) { }

WeaponPlimPlim::~WeaponPlimPlim() {}

void 
WeaponPlimPlim::callback(Vector2D shootPos, Vector2D shootDir) {
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.dir = shootDir;
	bp.init_pos = shootPos;
	bp.speed =_speed;
	bp.damage = _damage;
	bp.life_time = 2;
	bp.width = _attack_width* 0.5f;
	bp.height = _attack_height * 0.5f;
	bp.collision_filter = GameStructs::collide_with::player;

	bp.sprite_key = "p_plimplim";
	auto* scene = static_cast<GameScene*>(Game::Instance()->get_currentScene());

	float initialRot = atan2(bp.dir.getY(), bp.dir.getX()) * 180.0f / M_PI;

	float totalAngle = 120.0f;

	patrons::ShotgunPatron(bp, ecs::grp::BULLET, totalAngle, 3);

}