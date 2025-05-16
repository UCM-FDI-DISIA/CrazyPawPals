
#include "GhostStateComponent.h"

#include "../../../game/Game.h"
#include "../../../ecs/Manager.h"

#include "../rendering/dyn_image_with_frames.hpp"
#include "../Health.h"
#include "../movement/MovementController.h"
#include "../weapons/Weapon.h"
#include "../cards/Mana.h"
#include "../cards/Deck.hpp"

GhostStateComponent::GhostStateComponent()
    :_dy(nullptr), _mc(nullptr), _mana(nullptr), _deck(nullptr), _weapon(nullptr),
    _normal_state(),_ghost_state(), _isGhost(false) {}

GhostStateComponent::~GhostStateComponent()
{
    _dy->_current_filter = filter{ 255, 255, 255, 255 };
    setState(_normal_state);
}

void GhostStateComponent::initComponent()
{
    auto* manager = Game::Instance()->get_mngr();

    _dy = manager->getComponent<dyn_image_with_frames>(_ent);
    assert(_dy != nullptr);

    _mc = manager->getComponent<MovementController>(_ent);
    assert(_mc != nullptr);

    _mana = manager->getComponent<ManaComponent>(_ent);
    assert(_mana != nullptr);

    _deck = manager->getComponent<Deck>(_ent);
    assert(_deck != nullptr);

    _weapon = manager->getComponent<Weapon>(_ent);
    assert(_weapon != nullptr);

    _health = manager->getComponent<Health>(_ent);
    assert(_health != nullptr);

    _normal_state = {
      .weapon_damage = _weapon->damage(),
      .max_speed = _mc->get_max_speed(),
      .cooldown = _weapon->cooldown(),
      .reload = _deck->reload_time(),
      .mana_regen = _mana->mana_regen(),
      .health = int(_health->getMaxHealth() * 0.2)
    };

    _ghost_state = {
        .weapon_damage = _normal_state.weapon_damage / 2,
        .max_speed = _normal_state.max_speed / 2.0f,
        .cooldown = _normal_state.cooldown * 2,
        .reload = _normal_state.reload * 2,
        .mana_regen = _normal_state.mana_regen / 2,
        .health = 0
    };

    change_state();
}

void GhostStateComponent::change_state()
{
    _isGhost = !_isGhost;
    _dy->_current_filter = _isGhost ? filter{ 100, 180, 255, 160 } : filter{ 255, 255, 255, 255 };
    setState(_isGhost ? _ghost_state : _normal_state);
}

void GhostStateComponent::setState(GameStructs::PlayerData p)
{
    _mc->set_max_speed(p.max_speed);
    _weapon->set_damage(p.weapon_damage);
    _weapon->set_cooldown(p.cooldown);
    _deck->set_reload_time(p.reload);
    _mana->set_mana_regen(p.mana_regen);
    _health->setHeatlh(p.health);

}
