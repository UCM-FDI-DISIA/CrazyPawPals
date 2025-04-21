#ifndef NETWORK_CONNECTION_HPP
#define NETWORK_CONNECTION_HPP

#include <SDL_net.h>
#include <array>
#include <cstdint>

using network_connection_size = size_t;
template <network_connection_size MaximumConnections>
struct network_context_connections {
    std::array<TCPsocket, MaximumConnections> connections;
    network_connection_size connection_count;
};

#ifndef NETWORK_CONTEXT_HOST_MAXIMUM_CONNECTIONS
#define NETWORK_CONTEXT_HOST_MAXIMUM_CONNECTIONS_DEFAULT 8

#define NETWORK_CONTEXT_HOST_MAXIMUM_CONNECTIONS \
    NETWORK_CONTEXT_HOST_MAXIMUM_CONNECTIONS_DEFAULT
#endif


#endif