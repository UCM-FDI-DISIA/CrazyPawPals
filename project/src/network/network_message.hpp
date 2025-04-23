#ifndef NETWORK_MESSAGE_HPP
#define NETWORK_MESSAGE_HPP

#include "network_message/network_message_def.hpp"
#include "network_utility.hpp"

#include <cstdint>
#include <cassert>
#include <memory>

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

using network_message_connection = network_message_connection_client_from_host;
union network_message_header_union {
    network_message_connection connection_header_message;
    network_message_header header;
};


template <typename T>
struct network_message_pack {
    network_message_header header;
    network_message_payload<T> payload;
};

template <typename T>
network_message_pack<T> network_message_pack_create(
    const network_message_type_option header_type,
    const network_message_payload_type_option content_type,
    const T &content
) {
    const network_message_pack<T> message{
        .header = network_message_header_create(
            header_type,
            sizeof(network_message_payload<T>)
        ),
        .payload = network_message_payload<T>{
            .type = content_type,
            .content = content,
        },
    };
    assert(
        network_message_header_valid(message.header)
        && "error: message header must be valid after creation"
    );
    assert(
        network_message_header_in_network_endian(message.header)
        && "error: message header must be in network endian after creation"
    );
    return message;
}

template <typename T>
network_message_pack<T> network_message_pack_receive_static(
    TCPsocket socket
) {
    const network_message_header header = network_message_header_receive(socket);
    const size_t payload_size = size_t(SDLNet_Read16(&header.payload_size_n));

    if (sizeof(network_message_pack<T>::payload) < payload_size) {
        assert(false && "fatal error: payload size must be less than or equal to the size of the payload");
        std::exit(EXIT_FAILURE);
    }
    
    network_message_pack<T> message;
    const int recv_result = SDLNet_TCP_Recv(
        socket,
        &message,
        int(sizeof(message))
    );
    if (recv_result == network_utility_sdl_net_failure) {
        assert(false && "fatal error: SDLNet_TCP_Recv failed");
        std::exit(EXIT_FAILURE);
    } else if (recv_result != sizeof(message)) {
        assert(false && "fatal error: SDLNet_TCP_Recv invalid number of bytes received");
        std::exit(EXIT_FAILURE);
    }
    return message;
}

template <typename T>
std::unique_ptr<network_message_pack<T>> network_message_pack_from_dynamic(
    const network_message_pack<uint8_t *> &message
) {
    assert(
        network_message_header_valid(message.header)
        && "error: message header must be valid before conversion"
    );
    assert(
        network_message_header_in_network_endian(message.header)
        && "error: message header must be in network endian before conversion"
    );
    return std::make_unique<network_message_pack<T>>(
        network_message_pack<T>{
            .header = message.header,
            .content = *reinterpret_cast<const network_message_payload<T> *>(&message.payload),
        }
    );
}

std::unique_ptr<network_message_pack<uint8_t *>> network_message_pack_receive_dynamic(
    TCPsocket socket
);

template <typename T>
void network_message_pack_send(
    TCPsocket socket,
    const network_message_pack<T> &message
) {
    assert(
        network_message_header_valid(message.header)
        && "error: message header must be valid before sending"
    );
    assert(
        network_message_header_in_network_endian(message.header)
        && "error: message header must be in network endian before sending"
    );
    const int header_send_result = SDLNet_TCP_Send(
        socket,
        &message.header,
        int(sizeof(message.header))
    );
    if (header_send_result == network_utility_sdl_net_failure) {
        assert(false && "fatal error: SDLNet_TCP_Send failed");
        std::exit(EXIT_FAILURE);
    } else if (header_send_result != sizeof(message.header)) {
        assert(false && "fatal error: SDLNet_TCP_Send invalid number of bytes sent");
        std::exit(EXIT_FAILURE);
    }

    const int payload_send_result = SDLNet_TCP_Send(
        socket,
        &message.payload,
        int(sizeof(message.payload))
    );
    if (payload_send_result == network_utility_sdl_net_failure) {
        assert(false && "fatal error: SDLNet_TCP_Send failed");
        std::exit(EXIT_FAILURE);
    } else if (payload_send_result != sizeof(message.payload)) {
        assert(false && "fatal error: SDLNet_TCP_Send invalid number of bytes sent");
        std::exit(EXIT_FAILURE);
    }
}

#endif
