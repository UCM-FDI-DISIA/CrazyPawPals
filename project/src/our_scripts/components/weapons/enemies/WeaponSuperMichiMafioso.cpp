
#include "WeaponSuperMichiMafioso.h"
#include "../../../../game/Game.h"
#include "../../../../game/scenes/GameScene.h"
#include "../../movement/Transform.h"
#include <our_scripts/components/WaveManager.h>


WeaponSuperMichiMafioso::WeaponSuperMichiMafioso()
	: Weapon(4, 200, 20.0f, 0.1f, "p_super_michi_mafioso", 1.0f, 1.0f),_warning(false){ }

WeaponSuperMichiMafioso::~WeaponSuperMichiMafioso() {}

void WeaponSuperMichiMafioso::attack1() {
	int scale = 2;
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.dir = Vector2D();
	bp.speed = 0;
	bp.width = _attack_width * scale;
	bp.height = _attack_height * scale;

	if (_warning) {
		bp.init_pos =_last_shootPos;
		bp.damage = 1.0f;
		bp.life_time = 1.0f;
		bp.sprite_key = _tex;
		bp.collision_filter = GameStructs::collide_with::player;
		_warning = false;
	}
	else {
		_last_shootPos = _player_pos;
		bp.init_pos = _last_shootPos;
		bp.damage = 0;
		bp.life_time = 0.4f;
		bp.sprite_key ="attack_warning";
		bp.collision_filter = GameStructs::collide_with::none;
		_warning = true;
	}

	Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);
}
void WeaponSuperMichiMafioso::attack2(Vector2D shootPos) {
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	Vector2D shootDir = (_player_pos - shootPos).normalize();
	bp.dir = shootDir;
	bp.init_pos = shootPos;
	bp.speed = _speed;
	bp.damage = _damage;
	bp.life_time = 2.0f;
	bp.width = _attack_width;
	bp.height = _attack_height;
	bp.sprite_key = "p_super_michi_mafioso_attack2";
	bp.collision_filter = GameStructs::collide_with::player;
	bp.weapon_type = GameStructs::SUPER_MICHI;

	Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);
}

void WeaponSuperMichiMafioso::attack3(Vector2D shootPos) {
	const int numAreas = 5;
	const float radius = 1.5f; 
	const float angle = 60.0f;

	Vector2D shootDir = (_player_pos - shootPos).normalize();

	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	Vector2D initialRot = bp.dir = shootDir;
	bp.speed = 0;
	bp.damage = 1.0f;
	bp.life_time = 3.0f;
	bp.width = radius;
	bp.height = radius;
	bp.sprite_key = _tex;
	bp.collision_filter = GameStructs::collide_with::player;
	bp.pierce_number = 5;

	for (int i = 0; i < numAreas; ++i) {
		float angleOffset = (angle * (i / (std::max(((float)numAreas - 1.0f), 1.0f))) - angle / 2.0f) * (M_PI / 180.0f);
		Vector2D rotatedDir(
			initialRot.getX() * cos(angleOffset) - initialRot.getY() * sin(angleOffset),
			initialRot.getX() * sin(angleOffset) + initialRot.getY() * cos(angleOffset)
		);
		bp.init_pos = shootPos + rotatedDir * 2;
		Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);
	}
}
void WeaponSuperMichiMafioso::generate_michi_mafioso() {
	Vector2D posRandom = _tr->getPos() + Vector2D{ (float)sdlutils().rand().nextInt(1, 5), (float)sdlutils().rand().nextInt(1, 5) };
	GameScene::spawn_michi_mafioso(posRandom);
	//Game::Instance()->get_mngr()->getComponent<WaveManager>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::WAVE))->newEnemy();
}

void WeaponSuperMichiMafioso::set_player_pos(Vector2D _pl) {
	_player_pos = _pl;
}
