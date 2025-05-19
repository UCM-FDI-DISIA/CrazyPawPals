
#include "bullet_collision_component.hpp"
#include "rigidbody_component.hpp"
#include "collision_triggerers.hpp"
#include "../../game/Game.h"
#include "../../ecs/Manager.h"
#include "../../game/GameStructs.h"
#include "Health.h"
#include "movement/MovementController.h"
#include "collision_registration_by_id.h"
#include "id_component.h"
#include "cards/Mana.h"
#include "cards/Deck.hpp"
#include "../card_system/PlayableCards.hpp"
#include <our_scripts/card_system/ShootPatrons.hpp>

void bullet_collision_component::on_contact(const collision_manifold& tm)
{
    //takes pointer to the other body
    ecs::entity_t entity_collided_with = _ent==tm.body0 ? tm.body1 : tm.body0;

    if (check_if_valid_collision(entity_collided_with)) {
        auto &&manager = *Game::Instance()->get_mngr();
        if (manager.hasComponent<Health>(entity_collided_with)) {
            apply_weapon_effect(type, entity_collided_with);
            auto health = manager.getComponent<Health>(entity_collided_with);
            health->takeDamage(my_damage);

            Game::Instance()->get_mngr()->setAlive(_ent, pierce_number-- > 0);
        }
    }
}
bool bullet_collision_component::check_if_valid_collision(ecs::entity_t ent_col)
{
    switch (collision_filter)
    {
    case GameStructs::collide_with::player:
        return Game::Instance()->get_mngr()->hasComponent<player_collision_triggerer>(ent_col);
        break;
    case GameStructs::collide_with::enemy: {
        if (Game::Instance()->get_mngr()->hasComponent<enemy_collision_triggerer>(ent_col)) {
            std::bitset<256>& col_reg = Game::Instance()->get_mngr()->getComponent<collision_registration_by_id>(_ent)->already_collided_with;
            uint8_t id = Game::Instance()->get_mngr()->getComponent<id_component>(ent_col)->getId();
            bool aux_bool = col_reg.test(id);
            col_reg.set(id);
            return !aux_bool;
        }else return false;
        break;
    }
    case GameStructs::collide_with::all: {
        auto col_reg = Game::Instance()->get_mngr()->getComponent<collision_registration_by_id>(_ent)->already_collided_with;
        uint8_t id = Game::Instance()->get_mngr()->getComponent<id_component>(ent_col)->getId();
        bool aux_bool = col_reg[id];
        col_reg[id] = true;
        return aux_bool;
        break;
    }
    case GameStructs::collide_with::none:
        return false;
        break;
    default:
        return false;
    }
}

void bullet_collision_component::apply_weapon_effect(GameStructs::WeaponType type, ecs::entity_t target)
{
    auto& manager = *Game::Instance()->get_mngr();

    switch (type) {
    case GameStructs::WeaponType::RAMP_CANON: {
        auto player = manager.getHandler(ecs::hdlr::PLAYER);
        manager.getComponent<ManaComponent>(player)->change_mana(1.5);
        break;
    }
    case GameStructs::WeaponType::CATKUZA_WEAPON: {
        auto player = manager.getHandler(ecs::hdlr::PLAYER);
        auto d = manager.getComponent<Deck>(player);
        auto sum = d->get_total_cards_num();
        if (sum < 10) manager.getComponent<Deck>(player)->add_card_to_deck(new CatKuzaCard());
        break;
    }
    case GameStructs::WeaponType::SUPER_MICHI: {
        auto player = manager.getHandler(ecs::hdlr::PLAYER);
        auto d = manager.getComponent<Deck>(player);
        //manager.getComponent<Deck>(player)->add_card_to_deck(new SuperMichiCard());
        manager.getComponent<MovementController>(player)->frozen(1000);
        break;
    }
    case GameStructs::WeaponType::FIREBALL_EFFECT: {
        if (my_damage <= 1) return;

        auto tr = Game::Instance()->get_mngr()->getComponent<Transform>(_ent);

        GameStructs::BulletProperties bp;
        bp.collision_filter = GameStructs::collide_with::enemy;
        bp.damage = my_damage/2.0f;
        bp.dir = tr->getDir();
        bp.height = 1.25 + bp.damage * 0.25; //tr->getWidth() / 2.0f;
        bp.width = 0.75 + bp.damage * 0.25;//bp.height;
        bp.init_pos = tr->getPos();
        bp.life_time = 0.5 + bp.damage * 0.125;
        bp.pierce_number = 0;
        bp.speed = tr->getSpeed() * 2;
        bp.sprite_key = "p_fireball";
        bp.weapon_type = GameStructs::WeaponType::FIREBALL_EFFECT;
        bp.bitset = Game::Instance()->get_mngr()->getComponent<collision_registration_by_id>(_ent)->already_collided_with;
        
        patrons::ShotgunPatron(bp, ecs::grp::BULLET, 40, 2);
    }
    default:
        break;
    }
}

void ratatouille_collision_component::on_contact(const collision_manifold& tm) {
    ecs::entity_t entity_collided_with = (_ent == tm.body0) ? tm.body1 : tm.body0;
    uint32_t current_time = sdlutils().virtualTimer().currTime();
    auto&& manager = *Game::Instance()->get_mngr();
    if (manager.hasComponent<player_collision_triggerer>(entity_collided_with)) {
        if (current_time - last_damage_time >= damage_interval) {
            auto health = manager.getComponent<Health>(entity_collided_with);
            health->takeDamage(my_damage);
            last_damage_time = current_time;  
        }
    }
}