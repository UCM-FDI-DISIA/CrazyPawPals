#ifndef NETWORK_MESSAGE_DEF_HPP
#define NETWORK_MESSAGE_DEF_HPP

#include "SDL_net.h"
#include "../network_utility.hpp"
#include <cassert>
#include <type_traits>
#include <string_view>

enum network_message_type {
    network_message_type_none = 0,
    network_message_type_any,
    network_message_type_dbg_print,
    network_message_type_dbg_print_two_byte_test = 0x0102,
};
using network_message_type_option = uint16_t;
using network_message_header_size = uint16_t;
struct network_message_header {
    network_message_type_option unused : 1;
    network_message_type_option illegal : 1;
    network_message_type_option type_n : 14;
    network_message_header_size payload_size_n;
};
static_assert(
    sizeof(network_message_header) == 4,
    "static error: network_message_header size is not 4 bytes"
);

inline bool network_message_header_valid(const network_message_header header) {
    return header.unused == 0 && header.illegal == 1;
}
inline bool network_message_header_in_network_endian(const network_message_header header) {
    return network_utility_in_network_endian_u16(header.payload_size_n)
        && network_utility_in_network_endian_u16(header.type_n);
}

#include <iostream>
inline network_message_header network_message_header_create(
    const network_message_type_option type,
    const network_message_header_size payload_size_h
) {
    network_message_header header;
    network_message_type_option type_n;
    
    SDLNet_Write16(type, &type_n);
    SDLNet_Write16(payload_size_h, &header.payload_size_n);
    header.unused = 0;
    header.illegal = 1;
    header.type_n = type_n;

    assert(
        network_message_header_valid(header) && "error: header must be valid after creation"
    );
    assert(
        network_message_header_in_network_endian(header) && "error: header must be in network endian after creation"
    );
    return header;
}
network_message_header network_message_header_receive(TCPsocket socket);
void network_message_header_send(TCPsocket socket, const network_message_header header);


template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
struct network_message_payload {
    T content;
    struct deleter {
        void operator()(T *ptr) const {
            ptr->~T();
            delete[] reinterpret_cast<uint8_t *>(ptr);
        }
    };
};

template <uint32_t ArgumentsSize>
struct network_message_payload_dbg_print {
    uint32_t args_size_n;
    std::array<uint8_t, ArgumentsSize> args;
};

template <uint32_t ArgumentsSize>
network_message_payload_dbg_print<ArgumentsSize> network_message_payload_dbg_print_create(
    std::string_view str
) {
    network_message_payload_dbg_print<ArgumentsSize> msg;

    const size_t size = str.size();
    assert(
        size < std::numeric_limits<uint32_t>::max() && "error: string size exceeds uint32_t max"
    );

    const uint32_t args_size_h = size;
    assert(
        args_size_h < ArgumentsSize && "error: string size exceeds buffer size"
    );
    SDLNet_Write32(args_size_h, &msg.args_size_n);
    
    std::copy(str.begin(), str.end(), msg.args.begin());
    return msg;
}


#endif
