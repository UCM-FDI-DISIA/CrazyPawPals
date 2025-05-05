#ifndef NETWORK_STATE_HPP
#define NETWORK_STATE_HPP

#include "network_state/network_state_def.hpp"
#include <cstdint>

struct network_connections {
    uint8_t connected_users;
    uint8_t local_user_index;
    uint8_t oldest_non_host_index;
};

template <size_t MaximumConnections>
struct network_state {
    network_game_state<MaximumConnections> game_state;
    network_connections connections;
};

#endif