
#include "Health.h"
#include "../../sdlutils/SDLUtils.h"
#include "../../ecs/Manager.h"
#include "../../game/Game.h"
#include "EraseTextureComponent.h"
#include "rendering/dyn_image.hpp"
#include "rendering/dyn_image_with_frames.hpp"
#include <algorithm>
#include "ui/DamagePopup.h"
#include "movement/Transform.h"
#include "rendering/camera_component.hpp"
#include "net/GhostStateComponent.h"
#ifdef GENERATE_LOG
#include "../../our_scripts/log_writer_to_csv.hpp"
#endif

Health::Health(int maxHealth, bool isPlayer)
	: _is_player(isPlayer), _currentHealth(maxHealth), _maxHealth(maxHealth), _shield(0), _shieldTime(0), _dy(nullptr), _tr(nullptr) {};
Health::~Health() {};

void Health::initComponent()
{
	_tr = Game::Instance()->get_mngr()->getComponent<Transform>(_ent);
	_dy = Game::Instance()->get_mngr()->getComponent<dyn_image>(_ent);
	if (!_dy)
		_dy = Game::Instance()->get_mngr()->getComponent<dyn_image_with_frames>(_ent);

	assert(_dy != nullptr);
}
void Health::heal(int health)
{
	if (_currentHealth + health < _maxHealth)
		_currentHealth += health;
	else
		_currentHealth = _maxHealth;
}
int Health::getMaxHealth() const { return _maxHealth; }

void Health::takeDamage(int damage)
{
	// si ya es fantasma no recibe damage
	if (_is_player)
	{
		auto *manager = Game::Instance()->get_mngr();
		if (manager->hasComponent<GhostStateComponent>(_ent))
		{
			return;
		}
	}

	// damage popup
	auto tr = new Transform(_tr->getPos(), {0.0f, 0.0f}, 0.0f, 0.0f);
	auto rect = new rect_component(0, 0, damage >= 10 ? 1.0 : 0.5, 0.9);
	auto text = new Texture(
		sdlutils().renderer(),
		std::to_string(damage),
		sdlutils().fonts().at("PROTEST_GUERRILLA100"),
		SDL_Color(255, 50, 20, 255));
	// auto text =
	auto img = new dyn_image(
		rect_f32{{0, 0}, {1, 1}},
		*rect,
		Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam,
		*text,
		*tr);
	auto popup = new DamagePopup();
	auto ent = Game::Instance()->get_mngr()->addEntity(ecs::scene::GAMESCENE, ecs::grp::DAMAGE_POPUPS);
	Game::Instance()->get_mngr()->addExistingComponent(ent, tr, img, popup, rect, new EraseTextureComponent(text));

	if (_shield <= damage)
	{
		damage -= _shield;
		_shield = 0;

		_currentHealth -= damage;
		_dy->isDamaged = true;
		if (_currentHealth <= 0)
		{
			event_system::event_receiver::Msg msg;
			msg.int_value = _maxHealth * 0.2; // magic number random?

			if (_is_player)
				Game::Instance()->get_event_mngr()->fire_event(event_system::player_dead, msg);
			else
			{
				Game::Instance()->get_event_mngr()->fire_event(event_system::enemy_dead, msg);
#ifdef GENERATE_LOG
				log_writer_to_csv::Instance()->add_new_log("ENEMY KILLED");
#endif
			}
			if (!_is_player)
				Game::Instance()->get_mngr()->setAlive(_ent, false);
		}
	}
	else
		_shield -= damage;
}

void Health::setMaxHeatlh(int maxHeatlh)
{
	if (maxHeatlh > 0)
		_maxHealth = maxHeatlh;
}
void Health::setHeatlh(int h)
{
	if (h > 0 && h < _maxHealth)
		_currentHealth = h;
}
void Health::resetCurrentHeatlh()
{
	_currentHealth = _maxHealth;
}
int Health::getHealth() const { return _currentHealth; }
int Health::getShield() const
{
	return _shield;
}
void Health::takeShield(int shield)
{
	_shield = shield;
}
void Health::payHealth(int cost)
{
	_currentHealth = std::max(1, _currentHealth - cost);
}
void Health::update(uint32_t delta_time)
{
	(void)delta_time;
	if (_shield > 0 && sdlutils().virtualTimer().currRealTime() >= _shieldTime)
	{
		_shieldTime = sdlutils().virtualTimer().currRealTime() + FRAME_DURATION;
		_shield--;
	}
}
