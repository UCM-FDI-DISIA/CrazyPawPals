#ifndef NETWORK_CONTEXT_HPP
#define NETWORK_CONTEXT_HPP

#include <SDL_net.h>
#include "network_connection.hpp"

constexpr const network_connection_size network_context_host_maximum_connections =
    NETWORK_CONTEXT_HOST_MAXIMUM_CONNECTIONS;
struct network_context_host {
    network_context_connections<network_context_host_maximum_connections> sockets_to_clients;
    SDLNet_SocketSet clients_host_set;
    IPaddress ip_self;
};
struct network_context_client {
    TCPsocket socket_to_master;
    SDLNet_SocketSet client_set;
    IPaddress ip_host;
};

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

#endif