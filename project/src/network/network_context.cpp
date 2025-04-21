#include "network_context.hpp"
#include "network_utility.hpp"
#include "network_message.hpp"

#include <cassert>
#include <cstdlib>

extern inline bool network_context_host_resolved(const network_context_host &host);
extern inline bool network_context_host_connected(const network_context_host &host);
network_context_host network_context_host_create(const uint16_t port) {
    IPaddress host_ip;
    const int resolved = SDLNet_ResolveHost(&host_ip, nullptr, port);
    if (resolved == network_utility_sdl_net_failure) {
        assert(false && "fatal error: SDLNet_ResolveHost failed");
        std::exit(EXIT_FAILURE);
    } else if (resolved != network_utility_sdl_net_success) {
        assert(false && "fatal error: SDLNet_ResolveHost invalid");
        std::exit(EXIT_FAILURE);
    }

    const network_context_host host = network_context_host{
        .sockets_to_clients = network_context_connections_create_empty<network_context_host_maximum_connections>(),
        .host_socket = nullptr,
        .clients_host_set = nullptr,
        .ip_self = host_ip,
    };
    assert(network_context_host_resolved(host) && "fatal error: network_context_host_create invalid");
    return host;
}

extern inline bool network_context_client_resolved(const network_context_client &client);
extern inline bool network_context_client_connected(const network_context_client &client);
network_context_client network_context_client_create(const char *host, const uint16_t port) {
    IPaddress host_ip;
    const int resolved = SDLNet_ResolveHost(&host_ip, host, port);
    if (resolved == network_utility_sdl_net_failure) {
        assert(false && "fatal error: SDLNet_ResolveHost failed");
        std::exit(EXIT_FAILURE);
    } else if (resolved != network_utility_sdl_net_success) {
        assert(false && "fatal error: SDLNet_ResolveHost invalid");
        std::exit(EXIT_FAILURE);
    }

    const network_context_client client = network_context_client{
        .socket_to_master = nullptr,
        .client_set = nullptr,
        .ip_host = host_ip,
    };
    assert(network_context_client_resolved(client) && "fatal error: network_context_client_create invalid");
    return client;
}

bool network_context_client_connect_alloc(network_context_client &client) {
    assert(network_context_client_resolved(client) && "error: client context must be resolved before connecting");
    assert(!network_context_client_connected(client) && "error: client context must not be connected before connecting");

    const TCPsocket socket_to_master = SDLNet_TCP_Open(&client.ip_host);
    if (socket_to_master == nullptr) {
        assert(false && "fatal error: SDLNet_TCP_Open failed");
        std::exit(EXIT_FAILURE);
    }

    uint8_t message_buffer[sizeof(network_message_connection_client_from_host)];
    const int connection_result = SDLNet_TCP_Recv(
        socket_to_master,
        message_buffer,
        sizeof(network_message_connection_client_from_host)
    );
    if (connection_result == network_utility_sdl_net_failure) {
        assert(false && "fatal error: SDLNet_TCP_Recv failed");
        std::exit(EXIT_FAILURE);
    } else if (connection_result != sizeof(network_message_connection_client_from_host)) {
        assert(false && "fatal error: SDLNet_TCP_Recv invalid number of bytes received");
        std::exit(EXIT_FAILURE);
    }

    const network_message_connection_client_from_host message =
        *reinterpret_cast<network_message_connection_client_from_host *>(message_buffer);
    if (!network_message_connection_client_from_host_valid(message)) {
        assert(false && "fatal error: network_message_connection_client_from_host invalid");
        std::exit(EXIT_FAILURE);
    } else if (network_message_connection_client_from_host_accepted(message)) {
        client.socket_to_master = socket_to_master;
        client.client_set = SDLNet_AllocSocketSet(1);
        if (client.client_set == nullptr) {
            assert(false && "fatal error: SDLNet_AllocSocketSet failed");
            std::exit(EXIT_FAILURE);
        }
        const int add_socket_result = SDLNet_TCP_AddSocket(client.client_set, client.socket_to_master);
        if (add_socket_result == network_utility_sdl_net_failure) {
            assert(false && "fatal error: SDLNet_TCP_AddSocket failed");
            std::exit(EXIT_FAILURE);
        } else if (add_socket_result != 1) {
            assert(false && "fatal error: SDLNet_TCP_AddSocket invalid number of sockets added");
            std::exit(EXIT_FAILURE);
        }

        assert(network_context_client_connected(client) && "fatal error: network_context_client_connect_alloc invalid");
        return true;
    } else {
        SDLNet_TCP_Close(socket_to_master);
        return false;
    }
}

