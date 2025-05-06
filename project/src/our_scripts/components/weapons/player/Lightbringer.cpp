
#include "Lightbringer.h"
#include "game/Game.h"
#include "../../cards/Deck.hpp"
#include "ecs/Manager.h"
#include "../../../../game/scenes/GameScene.h"
#include "../../../../game/GameStructs.h"

Lightbringer::Lightbringer() : Weapon(2, 400.0f, 2.0f, 0.3f, "p_lightbringer", 1.0f, 1.0f), _base_damage(2), _amp_damage(4), _base_cooldown(400.0f), _amp_cooldown(200.0f), _base_tex("p_lightbringer"), _amp_tex("p_lightbringer_amp"), _offset(0.0f)
{
}

Lightbringer::~Lightbringer()
{
}

void Lightbringer::initComponent()
{
	Weapon::initComponent();
	_deck = Game::Instance()->get_mngr()->getComponent<Deck>(_ent);
}

void Lightbringer::update(uint32_t dt)
{
	bool is_primed = _deck->get_primed();
	_damage = is_primed ? _amp_damage : _base_damage;
	_cooldown = is_primed ? _amp_cooldown : _base_cooldown;
	_speed = is_primed ? 0 : 0.3f;
	_attack_height = is_primed ? 6.0f : 1.0f;
	_tex = is_primed ? _amp_tex : _base_tex;
	_offset = is_primed ? 3.0f : 0.0f;
}

void Lightbringer::callback(Vector2D shootPos, Vector2D shootDir)
{
	//if (_deck->get_primed()) std::cout << "PRIMED" << std::endl;
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.dir = shootDir;
	bp.init_pos = shootPos + (shootDir*_offset);
	bp.speed = _speed;
	bp.damage = _damage;
	bp.life_time = 0.2;
	bp.width = _attack_width;
	bp.height = _attack_height;
	//bp.rot = atan2(bp.dir.getY(), bp.dir.getX()) * 180.0f / M_PI;
	bp.sprite_key = _tex;
	bp.collision_filter = GameStructs::collide_with::enemy;
	bp.weapon_type = GameStructs::LIGHTBRINGER;

	Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);
}
