#include "PumpShotgun.h"
#include "../../game/Game.h"
#include "../../game/GameScene.h"


PumpShotgun::PumpShotgun() : Weapon(1, 0.9f, 1.5f, 0.6f, "p_pump_shotgun",1.0f,1.0f), _has_mill(false) {
	event_system::event_manager::Instance()->suscribe_to_event(event_system::mill, this, &event_system::event_receiver::event_callback0);
}

PumpShotgun::~PumpShotgun() {}

void PumpShotgun::event_callback0(const event_system::event_receiver::Msg& m)
{
	_has_mill = true;
}

void
PumpShotgun::callback(Vector2D shootPos, Vector2D shootDir) {
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();

	bp.dir = shootDir;
	bp.init_pos = shootPos;
	bp.speed = _speed;
	bp.damage = _damage;
	bp.life_time = 0.2f;//_lifeTime;
	bp.width = _attack_width * 0.6f;
	bp.height = _attack_height * 0.6f;
	bp.sprite_key = _tex;

	float totalAngle = 60.0f;

	patrons::ShotgunPatron(bp, ecs::grp::BULLET, totalAngle, _has_mill ? 8 : 4);

	_has_mill = false;
}