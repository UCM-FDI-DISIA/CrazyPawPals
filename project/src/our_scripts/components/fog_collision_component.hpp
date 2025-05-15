#pragma once

#include "rigidbody_component.hpp"
#include "game/GameStructs.h"

class player_collision_triggerer;
class enemy_collision_triggerer;

constexpr int max_player_number = 4;

struct fog_collision_component : public on_trigger<fog_collision_component> {
    __CMPID_DECL__(ecs::cmp::FOG_COLLISION_COMPONENT);

    int fogDamage = 4;
    std::unordered_map<uint8_t,std::pair<ecs::entity_t,Uint32>> last_fog_time_collided_by_player;
    
    inline fog_collision_component() : on_trigger<fog_collision_component>() {};
    void update(uint32_t delta_time) override;
    void on_contact(const collision_manifold&);
    void reset();
};