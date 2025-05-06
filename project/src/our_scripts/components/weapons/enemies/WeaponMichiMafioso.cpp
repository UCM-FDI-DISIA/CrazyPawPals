#include "WeaponMichiMafioso.h"
#include "../../../../game/Game.h"
#include "../../../../game/scenes/GameScene.h"

WeaponMichiMafioso::WeaponMichiMafioso() : Weapon(5, 3000, 20.0f, 0.2f, "p_michi_mafioso", 1.0f, 1.0f) { }

WeaponMichiMafioso::~WeaponMichiMafioso() {}

void 
WeaponMichiMafioso::callback(Vector2D shootPos, Vector2D shootDir) {
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.dir = shootDir;
	bp.init_pos = shootPos;
	bp.speed =_speed;
	bp.damage = _damage;
	bp.life_time = 2;
	bp.width = _attack_width;
	bp.sprite_key = "p_michi_mafioso";
	bp.height = _attack_height;
	bp.collision_filter = GameStructs::collide_with::player;
	//bp.rot = atan2(bp.dir.getY(), bp.dir.getX()) * 180.0f / M_PI;

	Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);
}