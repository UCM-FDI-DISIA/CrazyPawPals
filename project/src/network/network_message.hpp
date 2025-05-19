#ifndef NETWORK_MESSAGE_HPP
#define NETWORK_MESSAGE_HPP

#include "network_message/network_message_def.hpp"
#include "network_utility.hpp"

#include <cstdint>
#include <cassert>
#include <memory>
#include <cstddef>

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

struct network_message {
    network_message_header header;

    struct deleter {
        void operator()(network_message *message) const {
            message->~network_message();
            delete[] reinterpret_cast<uint8_t *>(message);
        }
    };
};
template <typename T>
struct network_message_pack : public network_message {
    uint32_t padding;
    network_message_payload<T> payload;
    struct deleter {
        void operator()(network_message_pack<T> *ptr) const {
            ptr->payload.content.~T();
            delete[] reinterpret_cast<uint8_t *>(ptr);
        }
    };
};


template <typename T>
network_message_pack<T> network_message_pack_create(
    const network_message_type_option type,
    const T &content
) {
    static_assert(
        offsetof(network_message_pack<T>, payload) == offsetof(network_message_pack<max_align_t>, payload),
        "static error: network_message_pack<T> payload offset must be equal to network_message_pack<max_align_t> payload offset"
    );
    network_message_pack<T> message;
    message.header = network_message_header_create(
        type,
        sizeof(network_message_payload<T>)
    );
    message.payload = network_message_payload<T>{
        .content = content,
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
network_message_pack<T> network_message_pack_receive(
    TCPsocket socket
) {
    static_assert(
        offsetof(network_message_pack<T>, payload) == offsetof(network_message_pack<max_align_t>, payload),
        "static error: network_message_pack<T> payload offset must be equal to network_message_pack<max_align_t> payload offset"
    );
    const network_message_header header = network_message_header_receive(socket);
    assert(
        network_message_header_valid(header)
        && "fatal error: message header must be valid before receiving"
    );
    assert(
        network_message_header_in_network_endian(header)
        && "fatal error: message header must be in network endian before receiving"
    );
    
    const size_t payload_size = size_t(SDLNet_Read16(&header.payload_size_n));

    if (sizeof(network_message_pack<T>::payload) < payload_size) {
        assert(false && "fatal error: payload size must be less than or equal to the size of the payload");
        std::exit(EXIT_FAILURE);
    }
    
    network_message_pack<T> message;
    message.header = header;
    if (payload_size != 0) {
        const int recv_result = SDLNet_TCP_Recv(
            socket,
            &message.payload,
            int(payload_size)
        );
        if (recv_result == network_utility_sdl_net_failure) {
            assert(false && "fatal error: SDLNet_TCP_Recv failed");
            std::exit(EXIT_FAILURE);
        } else if (recv_result != int(payload_size)) {
            assert(false && "fatal error: SDLNet_TCP_Recv invalid number of bytes received");
            std::exit(EXIT_FAILURE);
        }
    }
    return message;
}

using network_message_dynamic_pack = std::unique_ptr<network_message, network_message::deleter>;
network_message_dynamic_pack network_message_dynamic_pack_receive(
    TCPsocket socket
);

template <typename T>
void network_message_pack_send(
    TCPsocket socket,
    const network_message_pack<T> &message
) {
    static_assert(
        offsetof(network_message_pack<T>, payload) == offsetof(network_message_pack<max_align_t>, payload),
        "static error: network_message_pack<T> payload offset must be equal to network_message_pack<max_align_t> payload offset"
    );
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


template <typename From, typename To>
network_message_pack<To> network_message_pack_into(network_message_pack<From> &&from) {
    static_assert(
        offsetof(network_message_pack<To>, payload) == offsetof(network_message_pack<max_align_t>, payload),
        "static error: network_message_pack<To> payload offset must be equal to network_message_pack<max_align_t> payload offset"
    );
    static_assert(
        sizeof(network_message_pack<To>) <= sizeof(network_message_pack<From>),
        "static error: network_message_pack<To> must be less than or equal to network_message_pack<From>"
    );
    return reinterpret_cast<network_message_pack<To> &>(std::move(from));
}

template <typename T>
using network_message_resolved_dynamic_pack = std::unique_ptr<network_message_pack<T>, typename network_message_pack<T>::deleter>;
template <typename To>
network_message_resolved_dynamic_pack<To> network_message_dynamic_pack_into(network_message_dynamic_pack &&from) {
    static_assert(
        offsetof(network_message_pack<To>, payload) == offsetof(network_message_pack<max_align_t>, payload),
        "static error: network_message_pack<To> payload offset must be equal to network_message_pack<max_align_t> payload offset"
    );
    const size_t payload_size = size_t(SDLNet_Read16(&from->header.payload_size_n));
    if (sizeof(network_message_pack<To>) < payload_size) {
        assert(false && "fatal error: payload size must be less than or equal to the size of the payload");
        std::exit(EXIT_FAILURE);
    }

    network_message *ptr = std::move(from).release();
    if (ptr == nullptr) {
        assert(false && "fatal error: pointer must not be null before resolving dynamic pack");
        std::exit(EXIT_FAILURE);
    }

    return network_message_resolved_dynamic_pack<To>{
        static_cast<network_message_pack<To> *>(ptr),
        typename network_message_pack<To>::deleter()
    };
}

#endif
