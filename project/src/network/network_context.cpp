#include "network_context.hpp"
#include "network_utility.hpp"
#include "network_message.hpp"

#include <cassert>
#include <cstdlib>

extern inline bool network_context_host_resolved(const network_context_host &host);
extern inline bool network_context_host_connected(const network_context_host &host);
network_context_host network_context_host_create(const char *const host_name, const uint16_t port) {
    IPaddress host_ip;
    const int resolved = SDLNet_ResolveHost(&host_ip, host_name, port);
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

void network_context_host_connect_alloc(network_context_host &host) {
    assert(network_context_host_resolved(host) && "error: host context must be resolved before connecting");
    assert(!network_context_host_connected(host) && "error: host context must not be connected before connecting");

    TCPsocket host_socket = SDLNet_TCP_Open(&host.ip_self);
    if (host_socket == nullptr) {
        assert(false && "fatal error: SDLNet_TCP_Open failed");
        std::exit(EXIT_FAILURE);
    }

    host.host_socket = host_socket;
    host.clients_host_set = SDLNet_AllocSocketSet(network_context_host_maximum_connections + 1);
    if (host.clients_host_set == nullptr) {
        assert(false && "fatal error: SDLNet_AllocSocketSet failed");
        std::exit(EXIT_FAILURE);
    }
    const int add_socket_result = SDLNet_TCP_AddSocket(host.clients_host_set, host.host_socket);
    if (add_socket_result == network_utility_sdl_net_failure) {
        assert(false && "fatal error: SDLNet_TCP_AddSocket failed");
        std::exit(EXIT_FAILURE);
    } else if (add_socket_result != 1) {
        assert(false && "fatal error: SDLNet_TCP_AddSocket invalid number of sockets added");
        std::exit(EXIT_FAILURE);
    }
    
    assert(network_context_host_connected(host) && "fatal error: network_context_host_connect_alloc invalid");
}

void network_context_host_destroy(network_context_host &host) {
    assert(network_context_host_connected(host) && "error: host context must be connected before destroying");
    
    SDLNet_CheckSockets(host.clients_host_set, 0);
    for (network_connection_size i = 0; i < host.sockets_to_clients.connection_count; ++i) {
        TCPsocket &connection = host.sockets_to_clients.connections[i];
        assert(connection != nullptr && "fatal error: connection must not be null before destroying");
        network_utility_sdlnet_drain_and_close(connection);
        connection = nullptr;
    }
    
    for (network_connection_size i = host.sockets_to_clients.connection_count; i < host.sockets_to_clients.connections.size(); ++i) {
        assert(
            host.sockets_to_clients.connections[i] == nullptr
            && "fatal error: connection must be null after destroying"
        );
    }
    host.sockets_to_clients.connection_count = 0;

    network_utility_sdlnet_drain_and_close(host.host_socket);
    SDLNet_FreeSocketSet(host.clients_host_set);
    host.host_socket = nullptr;
    host.clients_host_set = nullptr;
    host.ip_self.host = INADDR_NONE;
    assert(!network_context_host_connected(host) && "fatal error: network_context_host_destroy invalid");
    assert(!network_context_host_resolved(host) && "fatal error: network_context_host_destroy invalid");
}

network_context_host_accept_connection_status_flags network_context_host_accept_connection(
    network_context_host &host,
    network_connection_size &out_connection_index
) {
    assert(network_context_host_connected(host) && "error: host context must be connected before accepting connections");
    if (SDLNet_SocketReady(host.host_socket)) {
        const TCPsocket client = SDLNet_TCP_Accept(host.host_socket);
        if (host.sockets_to_clients.connection_count >= host.sockets_to_clients.connections.size()) {
            const auto rejection_message =
                network_message_connection_client_from_host_create_rejected();
            const int send_result = SDLNet_TCP_Send(
                client,
                &rejection_message,
                sizeof(rejection_message)
            );
            if (send_result == network_utility_sdl_net_failure) {
                assert(false && "fatal error: SDLNet_TCP_Send failed");
                std::exit(EXIT_FAILURE);
            } else if (send_result != int(sizeof(rejection_message))) {
                assert(false && "fatal error: SDLNet_TCP_Send invalid number of bytes sent");
                std::exit(EXIT_FAILURE);
            }
            
            network_utility_sdlnet_drain_and_close(client);
            return network_context_host_accept_connection_status_rejected
                | network_context_host_accept_connection_status_full;
        } else {
            const network_connection_size connection_count =
                host.sockets_to_clients.connection_count;
            TCPsocket &connection = host.sockets_to_clients.connections[connection_count];
            assert(connection == nullptr && "error: connection must be null before accepting connection");

            connection = client;
            ++host.sockets_to_clients.connection_count;
            const int add_socket_result = SDLNet_TCP_AddSocket(
                host.clients_host_set,
                connection
            );
            if (add_socket_result == network_utility_sdl_net_failure) {
                assert(false && "fatal error: SDLNet_TCP_AddSocket failed");
                std::exit(EXIT_FAILURE);
            } else if (add_socket_result != int(host.sockets_to_clients.connection_count + 1)) {
                assert(
                    false
                    && "fatal error: SDLNet_TCP_AddSocket invalid number of sockets added."
                    "number of sockets added must be equal to the number of connections (clients) + 1 (host socket)"
                );
                std::exit(EXIT_FAILURE);
            }
            const auto acceptance_message =
                network_message_connection_client_from_host_create_accepted();
            const int send_result = SDLNet_TCP_Send(
                connection,
                &acceptance_message,
                sizeof(acceptance_message)
            );
            if (send_result == network_utility_sdl_net_failure) {
                assert(false && "fatal error: SDLNet_TCP_Send failed");
                std::exit(EXIT_FAILURE);
            } else if (send_result != sizeof(acceptance_message)) {
                assert(false && "fatal error: SDLNet_TCP_Send invalid number of bytes sent");
                std::exit(EXIT_FAILURE);
            }
            out_connection_index = connection_count;
            return network_context_host_accept_connection_status_accepted;
        }
    } else {
        return network_context_host_accept_connection_status_none;
    }
}

network_context_host_accept_connection_status_flags network_context_host_reject_connection(network_context_host &host) {
    if (SDLNet_SocketReady(host.host_socket)) {
        const TCPsocket client = SDLNet_TCP_Accept(host.host_socket);
        const auto rejection_message =
            network_message_connection_client_from_host_create_rejected();
        const int send_result = SDLNet_TCP_Send(
            client,
            &rejection_message,
            sizeof(rejection_message)
        );
        if (send_result == network_utility_sdl_net_failure) {
            assert(false && "fatal error: SDLNet_TCP_Send failed");
            std::exit(EXIT_FAILURE);
        } else if (send_result != int(sizeof(rejection_message))) {
            assert(false && "fatal error: SDLNet_TCP_Send invalid number of bytes sent");
            std::exit(EXIT_FAILURE);
        }
        
        network_utility_sdlnet_drain_and_close(client);
        return network_context_host_accept_connection_status_rejected
            | network_context_host_accept_connection_status_full;
    } else {
        return network_context_host_accept_connection_status_none;
    }
}

extern inline bool network_context_client_resolved(const network_context_client &client);
extern inline bool network_context_client_connected(const network_context_client &client);

bool network_context_client_can_resolve(const char *const host, const uint16_t port) {
    IPaddress host_ip;
    const int resolved = SDLNet_ResolveHost(&host_ip, host, port);
    (void)host_ip;

    if (resolved == network_utility_sdl_net_failure) {
        return false;
    } else if (resolved != network_utility_sdl_net_success) {
        assert(false && "fatal error: SDLNet_ResolveHost returned invalid value");
        std::exit(EXIT_FAILURE);
    }
    return true;
}
network_context_client network_context_client_create(const char *host, const uint16_t port)
{
    IPaddress host_ip;
    const int resolved = SDLNet_ResolveHost(&host_ip, host, port);
    if (resolved == network_utility_sdl_net_failure) {
        assert(
            false
            && "fatal error: SDLNet_ResolveHost failed"
            "call network_context_client_can_resolve to check if the host is valid"
        );
        std::exit(EXIT_FAILURE);
    } else if (resolved != network_utility_sdl_net_success) {
        assert(false && "fatal error: SDLNet_ResolveHost invalid");
        std::exit(EXIT_FAILURE);
    }

    const network_context_client client = network_context_client{
        .socket_to_host = nullptr,
        .client_set = nullptr,
        .ip_host = host_ip,
    };
    assert(network_context_client_resolved(client) && "fatal error: network_context_client_create invalid");
    return client;
}

void network_context_client_destroy(network_context_client &client) {
    assert(network_context_client_connected(client) && "error: client context must be connected before destroying");
    
    SDLNet_CheckSockets(client.client_set, 0);
    network_utility_sdlnet_drain_and_close(client.socket_to_host);
    
    SDLNet_FreeSocketSet(client.client_set);
    client.socket_to_host = nullptr;
    client.client_set = nullptr;
    client.ip_host.host = INADDR_NONE;
    assert(!network_context_client_connected(client) && "fatal error: network_context_client_destroy invalid");
    assert(!network_context_client_resolved(client) && "fatal error: network_context_client_destroy invalid");
}

network_context_client_connect_status_flags network_context_client_connect_alloc(network_context_client &client) {
    assert(network_context_client_resolved(client) && "error: client context must be resolved before connecting");
    assert(!network_context_client_connected(client) && "error: client context must not be connected before connecting");

    const TCPsocket socket_to_master = SDLNet_TCP_Open(&client.ip_host);
    if (socket_to_master == nullptr) {
        // either the host did not try to open a connection for clients
        // or the host ip address is invalid
        // which, is the same because it just means it is a machine not executing that part of the host programme,
        // either not executing our application or not executing the host part of our application

        // this does not constitute a fatal error
        return network_context_client_connect_status_error
            | network_context_client_connect_status_invalid;
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
        client.socket_to_host = socket_to_master;
        client.client_set = SDLNet_AllocSocketSet(1);
        if (client.client_set == nullptr) {
            assert(false && "fatal error: SDLNet_AllocSocketSet failed");
            std::exit(EXIT_FAILURE);
        }
        const int add_socket_result = SDLNet_TCP_AddSocket(client.client_set, client.socket_to_host);
        if (add_socket_result == network_utility_sdl_net_failure) {
            assert(false && "fatal error: SDLNet_TCP_AddSocket failed");
            std::exit(EXIT_FAILURE);
        } else if (add_socket_result != 1) {
            assert(false && "fatal error: SDLNet_TCP_AddSocket invalid number of sockets added");
            std::exit(EXIT_FAILURE);
        }

        assert(network_context_client_connected(client) && "fatal error: network_context_client_connect_alloc invalid");
        return network_context_client_connect_status_connected;
    } else {
        SDLNet_TCP_Close(socket_to_master);
        return network_context_client_connect_status_error
            | network_context_client_connect_status_rejected;
    }
}


extern inline network_context network_context_create_host(const char *const host_name, const uint16_t port);
extern inline network_context network_context_create_client(const char *host, const uint16_t port);

