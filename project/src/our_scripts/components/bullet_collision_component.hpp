#pragma once

#include "rigidbody_component.hpp"
#include "game/GameStructs.h"

class player_collision_triggerer;
class enemy_collision_triggerer;

struct bullet_collision_component : public on_trigger<bullet_collision_component> {
    GameStructs::collide_with collision_filter;
    GameStructs::WeaponType type;
    int my_damage;
    int pierce_number;
    __CMPID_DECL__(ecs::cmp::BULLET_COLLISION_COMPONENT);
    inline bullet_collision_component(const GameStructs::BulletProperties& bp)
        : collision_filter(bp.collision_filter), my_damage(bp.damage), pierce_number(bp.pierce_number), type(bp.weapon_type) {}
    //void initComponent() override;
    void on_contact(const collision_manifold&); //override;
    bool check_if_valid_collision(ecs::entity_t);

    void apply_weapon_effect(GameStructs::WeaponType type, ecs::entity_t target);
};

struct ratatouille_collision_component : public on_trigger<ratatouille_collision_component> {
    int my_damage;
    uint32_t last_damage_time;
    uint32_t damage_interval;
    __CMPID_DECL__(ecs::cmp::RATATOUILLE_COLLISION_COMPONENT);
    inline ratatouille_collision_component(int damage, uint32_t interval)
        : my_damage(damage), damage_interval(interval*1000),last_damage_time(0){}
    void on_contact(const collision_manifold&); //override;
};