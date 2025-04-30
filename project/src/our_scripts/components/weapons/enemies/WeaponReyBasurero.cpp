#include "WeaponReyBasurero.h"
#include "../../../../game/Game.h"
#include "../../../../game/scenes/GameScene.h"

WeaponReyBasurero::WeaponReyBasurero() : Weapon(6, 8000, 25.0f, 0.02f, "p_michi_mafioso", 1.5f, 1.5f) {}

WeaponReyBasurero::~WeaponReyBasurero() {}

void
WeaponReyBasurero::callback(Vector2D shootPos, Vector2D shootDir) {
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