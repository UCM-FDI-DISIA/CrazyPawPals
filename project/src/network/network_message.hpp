#ifndef NETWORK_MESSAGE_HPP
#define NETWORK_MESSAGE_HPP

#include <cstdint>
#include <cassert>

struct network_message_connection_client_from_host {
    bool accepted : 1;
    uint8_t unused : 7;
};
inline bool network_message_connection_client_from_host_valid(const network_message_connection_client_from_host message) {
    return message.unused == 0;
}
inline bool network_message_connection_client_from_host_accepted(const network_message_connection_client_from_host message) {
    assert(
        network_message_connection_client_from_host_valid(message)
        && "error: message must be valid before checking accepted status"
    );
    return message.accepted;
}

inline network_message_connection_client_from_host network_message_connection_client_from_host_create_accepted() {
    return network_message_connection_client_from_host{
        .accepted = true,
        .unused = 0,
    };
}
inline network_message_connection_client_from_host network_message_connection_client_from_host_create_rejected() {
    return network_message_connection_client_from_host{
        .accepted = false,
        .unused = 0,
    };
}

#endif