
#include "WeaponPlimPlim.h"
#include "../../../../game/Game.h"
#include "../../../../game/scenes/GameScene.h"
#include "../../../card_system/ShootPatrons.hpp"
#include "../../rendering/RotationComponent.h"

WeaponPlimPlim::WeaponPlimPlim() : Weapon(4, 5000, 20.0f, 0.1f, "p_plimplim", 1.0f, 1.0f) {}

WeaponPlimPlim::~WeaponPlimPlim() {}

void WeaponPlimPlim::callback(Vector2D shootPos, Vector2D shootDir)
{
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.dir = shootDir;
	bp.init_pos = shootPos;
	bp.speed = _speed;
	bp.damage = _damage;
	bp.life_time = 2;
	bp.width = _attack_width * 0.5f;
	bp.height = _attack_height * 0.5f;
	bp.collision_filter = GameStructs::collide_with::player;
	bp.sprite_key = "p_plimplim";

	float totalAngle = 120.0f;
	auto *mngr = Game::Instance()->get_mngr();
	std::vector<ecs::entity_t> es = patrons::ShotgunPatron(bp, ecs::grp::BULLET, totalAngle, 3);
	for (auto e : es)
	{
		mngr->addComponent<RotationComponent>(e, RotationComponent::Mode::CONTINUOUS, 0.9f, 360.0f);
	}
}