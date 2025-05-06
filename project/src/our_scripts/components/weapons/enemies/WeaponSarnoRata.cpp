#include "WeaponSarnoRata.h"
#include "../../../../game/Game.h"
#include "../../../../game/scenes/GameScene.h"

WeaponSarnoRata::WeaponSarnoRata() : Weapon(6, 2500, 20.0f, 0.0f, "p_sarno_rata", 3.0f, 3.0f) { }

WeaponSarnoRata::~WeaponSarnoRata() {}

void 
WeaponSarnoRata::callback(Vector2D shootPos, Vector2D shootDir) {
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.dir = shootDir;
	bp.init_pos = shootPos + shootDir.normalize() * 0.5f;
	bp.speed =_speed;
	bp.damage = _damage;
	bp.life_time = 2;
	bp.width = float(_attack_width) * 0.8f;
	bp.height = float(_attack_height)* 0.8f;
	bp.sprite_key = "p_sarno_rata";
	bp.collision_filter = GameStructs::collide_with::player;

	Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);
}