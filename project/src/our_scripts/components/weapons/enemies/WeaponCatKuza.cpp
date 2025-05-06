#include "WeaponCatKuza.h"
#include "../../../../game/Game.h"
#include "../../../../game/scenes/GameScene.h"
#include "../../../card_system/ShootPatrons.hpp"

WeaponCatKuza::WeaponCatKuza() : Weapon(4, 5000, 20.0f, 0.1f, "p_plimplim", 1.0f, 1.0f), _player_pos(), _wind_p(3), _dash_p(6), _area_p(5){ }

WeaponCatKuza::~WeaponCatKuza() {}

void 
WeaponCatKuza::callback(Vector2D shootPos, Vector2D shootDir) {
	// GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	// bp.dir = shootDir;
	// bp.init_pos = shootPos;

	// callback_3_wind(bp);

	// // switch (expression)
	// // {
	// // case constant expression:
	
	// // 	break;
	
	// // default:
	// // 	break;
	// // }


	// float totalAngle = 60.0f;

	 //patrons::ShotgunPatron(bp, ecs::grp::BULLET, totalAngle, _state);

}

void WeaponCatKuza::set_player_pos(Vector2D _pl) {
	_player_pos = _pl;
}

void WeaponCatKuza::wind_attack(Vector2D shootPos) {
    GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	Vector2D shootDir = (_player_pos - shootPos).normalize();
	bp.dir = shootDir;
	bp.init_pos = shootPos;

	bp.speed = _speed;
	bp.damage = _damage;
	bp.life_time = 2;
	bp.width = _attack_width * 0.5f;
	bp.height = _attack_height * 0.5f;

	bp.sprite_key = "p_wind_catkuza";
	bp.collision_filter = GameStructs::collide_with::player;
	bp.weapon_type = GameStructs::CATKUZA_WEAPON;

	float totalAngle = 60.0f;

	patrons::ShotgunPatron(bp, ecs::grp::BULLET, totalAngle, _wind_p);
	//std::cout << "aaaaaaaaa" << std::endl;
}

void WeaponCatKuza::dash_attack(Vector2D shootPos, Vector2D shoot_end_pos) {
    GameStructs::BulletProperties bp = GameStructs::BulletProperties();

    bp.dir = Vector2D(0,0);
	bp.init_pos = shootPos;
	bp.speed = 0;
	bp.damage = _damage;
	bp.life_time = 2;
	bp.width = _attack_width * 0.5f;
	bp.height = _attack_height * 0.5f;
	bp.sprite_key = "p_plimplim";
	bp.collision_filter = GameStructs::collide_with::player;
	
	Vector2D espacio = (shoot_end_pos - shootPos) / (_dash_p - 1);

	for (int i = 0; i < _dash_p; ++i) {
		bp.init_pos = shootPos + espacio * i;

		Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);
	}
}

void
WeaponCatKuza::area_attack(Vector2D shootPos) {
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.init_pos = shootPos;
	bp.speed = _speed;
	bp.damage = _damage;
	bp.life_time = 1;
	bp.width = _attack_width;
	bp.height = _attack_height;
	bp.sprite_key = "p_sarno_rata";
	bp.collision_filter = GameStructs::collide_with::player;

	for (int i = 0; i < _area_p; ++i) {
		static_cast<GameScene*>(Game::Instance()->get_currentScene())->generate_proyectile(bp, ecs::grp::BULLET);
	}
}