#ifndef NETWORK_MESSAGE_DEF_HPP
#define NETWORK_MESSAGE_DEF_HPP

#include "SDL_net.h"
#include <cassert>
#include <type_traits>
#include <string_view>

enum network_message_type {
    network_message_type_none = 0,
    network_message_type_any,
    network_message_type_dbg_print,
};
using network_message_type_option = uint16_t;
using network_message_header_size = uint16_t;
struct network_message_header {
    network_message_type_option unused : 1;
    network_message_type_option illegal : 1;
    network_message_type_option type : 14;
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
    return SDLNet_Read16(&header.payload_size_n) == header.payload_size_n;
}

inline network_message_header network_message_header_create(
    const network_message_type_option type,
    const network_message_header_size payload_size_h
) {
    network_message_header header{
        .unused = 0,
        .illegal = 1,
        .type = type,
        .payload_size_n = 0,
    };
    SDLNet_Write16(payload_size_h, &header.payload_size_n);
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

template <size_t ArgumentsSize>
struct network_message_payload_dbg_print {
    size_t args_size_n;
    std::array<uint8_t, ArgumentsSize> args;
};

template <size_t ArgumentsSize>
network_message_payload_dbg_print<ArgumentsSize> network_message_payload_dbg_print_create(
    std::string_view str
) {
    network_message_payload_dbg_print<ArgumentsSize> msg;
    msg.args_size_n = str.size();
    assert(
        str.size() <= ArgumentsSize && "error: string size exceeds buffer size"
    );
    std::copy(str.begin(), str.end(), msg.args.begin());
    return msg;
}


#endif
