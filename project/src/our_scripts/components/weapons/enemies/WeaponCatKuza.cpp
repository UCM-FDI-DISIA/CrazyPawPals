
#include "WeaponCatKuza.h"
#include "../../../../game/Game.h"
#include "../../../../game/scenes/GameScene.h"
#include "../../../card_system/ShootPatrons.hpp"
#include "../../rendering/RotationComponent.h"

WeaponCatKuza::WeaponCatKuza() : Weapon(4, 5000, 20.0f, 0.1f, "p_wind_catkuza", 0.70f, 1.25f), _player_pos(), _wind_p(3), _dash_p(6), _area_p(5) {}

WeaponCatKuza::~WeaponCatKuza() {}

void WeaponCatKuza::callback(Vector2D shootPos, Vector2D shootDir)
{
	(void)shootDir;
	(void)shootPos;
}

void WeaponCatKuza::set_player_pos(Vector2D _pl)
{
	_player_pos = _pl;
}

void WeaponCatKuza::wind_attack(Vector2D shootPos)
{
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	Vector2D shootDir = (_player_pos - shootPos).normalize();
	bp.dir = shootDir;
	bp.init_pos = shootPos;

	bp.speed = _speed;
	bp.damage = _damage;
	bp.life_time = 2;
	bp.width = _attack_width * 0.75f;
	bp.height = _attack_height * 0.75f;

	bp.sprite_key = "p_wind_catkuza";
	bp.collision_filter = GameStructs::collide_with::player;
	bp.weapon_type = GameStructs::CATKUZA_WEAPON;

	float totalAngle = 60.0f;

	auto *mngr = Game::Instance()->get_mngr();
	std::vector<ecs::entity_t> es = patrons::ShotgunPatron(bp, ecs::grp::BULLET, totalAngle, _wind_p);
	for (auto e : es)
	{
		mngr->addComponent<RotationComponent>(e, RotationComponent::Mode::NONE, 0.0f, 0.0f, 500, 0, 0, 0, true);
		RotationComponent *rot = mngr->getComponent<RotationComponent>(e);
		rot->setInitialRotation(0.0f);
	}
}

void WeaponCatKuza::dash_attack(Vector2D shootPos, Vector2D shoot_end_pos)
{
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();

	bp.dir = Vector2D(0, 0);
	bp.init_pos = shootPos;
	bp.speed = 0;
	bp.damage = _damage;
	bp.life_time = 2;
	bp.width = _attack_width * 2.2f;
	bp.height = _attack_height * 1.2f;
	bp.sprite_key = "p_dash_catkuza";
	bp.collision_filter = GameStructs::collide_with::player;

	Vector2D espacio = (shoot_end_pos - shootPos) / (_dash_p - 1);

	for (int i = 0; i < _dash_p; ++i)
	{
		bp.init_pos = shootPos + espacio * i;

		Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);
	}
}

void WeaponCatKuza::area_attack(Vector2D shootPos)
{
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.init_pos = shootPos;
	bp.speed = _speed;
	bp.damage = _damage;
	bp.life_time = 1;
	bp.width = _attack_width * 2.75f;
	bp.height = _attack_height * 2.5f;
	bp.sprite_key = "p_area_catkuza";
	bp.collision_filter = GameStructs::collide_with::player;

	auto *mngr = Game::Instance()->get_mngr();
	for (int i = 0; i < _area_p; ++i)
	{
		auto e = Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);
		mngr->addComponent<RotationComponent>(e, RotationComponent::Mode::CONTINUOUS, 0.5f, 360.0f);
	}
}