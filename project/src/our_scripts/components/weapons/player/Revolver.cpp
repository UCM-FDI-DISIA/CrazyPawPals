
#include "Revolver.h"
#include "../../../../game/Game.h"
#include "../../../../game/scenes/GameScene.h"

Revolver::Revolver() : Weapon(2, 500.0f, 2.0f, 0.2f, "p_revolver", 1.0f, 1.0f) { }

Revolver::~Revolver() {}

void 
Revolver::callback(Vector2D shootPos, Vector2D shootDir) {
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.dir = shootDir;
	bp.init_pos = shootPos;
	bp.speed =_speed;
	bp.damage = _damage;
	bp.life_time = 2;
	bp.width = _attack_width;
	bp.height = _attack_height;
	//bp.rot = atan2(bp.dir.getY(), bp.dir.getX()) * 180.0f / M_PI;
	bp.sprite_key = _tex;
	bp.weapon_type = GameStructs::REVOLVER;
	bp.collision_filter = GameStructs::collide_with::enemy;

	Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);
}