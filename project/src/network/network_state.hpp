#ifndef NETWORK_STATE_HPP
#define NETWORK_STATE_HPP

#include "network_state/network_state_def.hpp"
#include "network_context.hpp"

#include <cstdint>
#include <cassert>
#include <cstdlib>

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

template <size_t MaximumConnections>
inline size_t network_state_next_user_index(
    const network_state<MaximumConnections> &state
) {
    if (state.connections.connected_users >= MaximumConnections) {
        assert(false && "fatal error: too many users, should not request next user index");
        std::exit(EXIT_FAILURE);
    }
    return state.connections.connected_users;
}

// TODO: remove user
template <size_t MaximumConnections>
size_t network_state_add_user(
    network_state<MaximumConnections> &state,
    const ecs::entity_t player_entity,
    std::string &&sprite_key
) {
    if (state.connections.connected_users >= MaximumConnections) {
        assert(false && "fatal error: too many users");
        std::exit(EXIT_FAILURE);
    }

    size_t user_index = network_state_next_user_index(state);
    ecs::entity_t &player_slot = state.game_state.user_players.at(user_index);
    if (player_slot != ecs::entity_t{}) {
        assert(false && "fatal error: user slot is not empty");
        std::exit(EXIT_FAILURE);
    }
    std::string &sprite_key_slot = state.game_state.users_sprite_keys.at(user_index);
    if (!sprite_key_slot.empty()) {
        assert(false && "fatal error: sprite key slot is not empty");
        std::exit(EXIT_FAILURE);
    }

    player_slot = player_entity;
    sprite_key_slot = std::move(sprite_key);

    ++state.connections.connected_users;
    return user_index;
}

template <size_t MaximumConnections>
size_t network_state_add_host_user_as_self(
    const network_context &ctx,
    network_state<MaximumConnections> &state,
    const ecs::entity_t player_entity,
    std::string &&sprite_key
) {
    if (ctx.profile_status != network_context_profile_status_host) {
        assert(false && "fatal error: network context must be a host to add a host user");
        std::exit(EXIT_FAILURE);
    }

    if (state.connections.connected_users >= 1) {
        assert(false && "fatal error: too many users, host user must be the first");
        std::exit(EXIT_FAILURE);
    }

    if (state.connections.local_user_index != network_context_maximum_connections) {
        assert(false && "fatal error: local user index must be uninitialized");
        std::exit(EXIT_FAILURE);
    }

    size_t host = network_state_add_user(state, player_entity, std::move(sprite_key));
    if (host != 0) {
        assert(false && "fatal error: host user index must be 0");
        std::exit(EXIT_FAILURE);
    }
    state.connections.local_user_index = host;
    return host;
}

template <size_t MaximumConnections>
size_t network_state_add_client_user(
    const network_context &ctx,
    network_state<MaximumConnections> &state,
    const ecs::entity_t player_entity,
    std::string &&sprite_key
) {
    if (ctx.profile_status != network_context_profile_status_host) {
        assert(false && "fatal error: network context must be a host to add a client user");
        std::exit(EXIT_FAILURE);
    }

    const size_t client = network_state_add_user(state, player_entity, std::move(sprite_key));
    if (client < state.connections.oldest_non_host_index) {
        state.connections.oldest_non_host_index = client;
    }
    return client;
}


template <size_t MaximumConnections>
inline bool network_state_is_user_ready(
    const network_state<MaximumConnections> &state,
    const size_t user_index
) {
    if (user_index >= state.connections.connected_users) {
        assert(false && "fatal error: user index out of bounds");
        std::exit(EXIT_FAILURE);
    }
    return state.game_state.ready_users.test(user_index);
}

template <size_t MaximumConnections>
size_t network_state_set_user_ready(
    network_state<MaximumConnections> &state,
    size_t user_index
) {
    if (user_index >= state.connections.connected_users) {
        assert(false && "fatal error: user index out of bounds");
        std::exit(EXIT_FAILURE);
    } else if (network_state_is_user_ready(state, user_index)) {
        assert(false && "fatal error: user is already ready");
        std::exit(EXIT_FAILURE);
    }

    state.game_state.ready_users.set(user_index);
    ++state.game_state.ready_user_count;
    return state.game_state.ready_user_count;
}

template <size_t MaximumConnections>
size_t network_state_set_user_not_ready(
    network_state<MaximumConnections> &state,
    size_t user_index
) {
    if (user_index >= state.connections.connected_users) {
        assert(false && "fatal error: user index out of bounds");
        std::exit(EXIT_FAILURE);
    } else if (!network_state_is_user_ready(state, user_index)) {
        assert(false && "fatal error: user is not ready");
        std::exit(EXIT_FAILURE);
    }

    state.game_state.ready_users.reset(user_index);
    --state.game_state.ready_user_count;
    return state.game_state.ready_user_count;
}

#endif