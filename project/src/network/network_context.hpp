#ifndef NETWORK_CONTEXT_HPP
#define NETWORK_CONTEXT_HPP

#include <SDL_net.h>
#include "network_connection.hpp"

constexpr const network_connection_size network_context_host_maximum_connections =
    NETWORK_CONTEXT_HOST_MAXIMUM_CONNECTIONS;
constexpr const network_connection_size network_context_maximum_connections{
    network_context_host_maximum_connections + 1
};
struct network_context_host {
    network_context_connections<network_context_host_maximum_connections> sockets_to_clients;
    TCPsocket host_socket;
    SDLNet_SocketSet clients_host_set;
    IPaddress ip_self;
};
inline bool network_context_host_resolved(const network_context_host &host) {
    return host.ip_self.host != INADDR_NONE;
}
inline bool network_context_host_connected(const network_context_host &host) {
    return host.host_socket != nullptr;
}
network_context_host network_context_host_create(const char *const host_name, const uint16_t port);
void network_context_host_connect_alloc(network_context_host &host);
void network_context_host_destroy(network_context_host &host);

enum network_context_host_accept_connection_status {
    network_context_host_accept_connection_status_none = 0,
    network_context_host_accept_connection_status_accepted = 1 << 0,
    network_context_host_accept_connection_status_rejected = 1 << 1,
    network_context_host_accept_connection_status_full = 1 << 2,
    network_context_host_accept_connection_status_error = 1 << 3,
    network_context_host_accept_connection_status_invalid = 1 << 4,
};
using network_context_host_accept_connection_status_flags = uint8_t;
network_context_host_accept_connection_status_flags network_context_host_accept_connection(
    network_context_host &host,
    network_connection_size &out_connection_index 
);
network_context_host_accept_connection_status_flags network_context_host_reject_connection(
    network_context_host &host
);

struct network_context_client {
    TCPsocket socket_to_host;
    SDLNet_SocketSet client_set;
    IPaddress ip_host;
};
inline bool network_context_client_resolved(const network_context_client &client) {
    return client.ip_host.host != INADDR_NONE;
}
inline bool network_context_client_connected(const network_context_client &client) {
    return client.socket_to_host != nullptr;
}

bool network_context_client_can_resolve(const char *const host, const uint16_t port);
network_context_client network_context_client_create(const char *host, const uint16_t port);
void network_context_client_destroy(network_context_client &client);


enum network_context_client_connect_status {
    network_context_client_connect_status_none = 0,
    network_context_client_connect_status_connected = 1 << 0,
    network_context_client_connect_status_error = 1 << 1,
    network_context_client_connect_status_rejected = 1 << 2,
    network_context_client_connect_status_invalid = 1 << 3,
};
using network_context_client_connect_status_flags = uint8_t;
network_context_client_connect_status_flags network_context_client_connect_alloc(network_context_client &client);


union network_context_profile {
    network_context_host host;
    network_context_client client;
};
enum network_context_profile_status {
    network_context_profile_status_none = 0,
    network_context_profile_status_host,
    network_context_profile_status_client,
};
using network_context_profile_status_option = uint8_t;

struct network_context {
    network_context_profile profile;
    network_context_profile_status_option profile_status;
};
inline network_context network_context_create_host(const char *const host_name, const uint16_t port) {
    return network_context{
        .profile = {.host = network_context_host_create(host_name, port)},
        .profile_status = network_context_profile_status_host,
    };
}
inline network_context network_context_create_client(const char *host, const uint16_t port) {
    return network_context{
        .profile = {.client = network_context_client_create(host, port)},
        .profile_status = network_context_profile_status_client,
    };
}

#endif
