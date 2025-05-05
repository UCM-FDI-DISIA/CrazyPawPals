#ifndef NETWORK_STATE_DEF_HPP
#define NETWORK_STATE_DEF_HPP

#include <bitset>
#include "../../ecs/Entity.h"

template <size_t MaximumConnections>
struct network_game_state {
    size_t ready_user_count;
    std::bitset<MaximumConnections> ready_users;
    std::array<std::string, MaximumConnections> users_sprite_keys;
    std::array<ecs::entity_t, MaximumConnections> user_players;
};

#endif