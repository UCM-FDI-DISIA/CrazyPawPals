#include "network_message_def.h"
#include "network_message_def.hpp"

extern inline bool network_message_header_valid(const network_message_header header);
extern inline bool network_message_header_in_network_endian(const network_message_header header);

extern inline network_message_header network_message_header_create(
    const network_message_type_option type,
    const network_message_header_size size_h
);

network_message_header network_message_header_receive(TCPsocket socket) {
    network_message_header header;
    const int recv_result = SDLNet_TCP_Recv(
        socket,
        &header,
        int(sizeof(header))
    );
    if (recv_result == network_utility_sdl_net_failure) {
        assert(false && "fatal error: SDLNet_TCP_Recv failed");
        std::exit(EXIT_FAILURE);
    } else if (recv_result != sizeof(header)) {
        assert(false && "fatal error: SDLNet_TCP_Recv invalid number of bytes received");
        std::exit(EXIT_FAILURE);
    }
    if (!network_message_header_valid(header)) {
        assert(false && "fatal error: network_message_header invalid");
        std::exit(EXIT_FAILURE);
    } else if (!network_message_header_in_network_endian(header)) {
        assert(false && "fatal error: network_message_header not in network endian");
        std::exit(EXIT_FAILURE);
    }
    return header;
}
void network_message_header_send(TCPsocket socket, const network_message_header header) {
    assert(network_message_header_valid(header) && "error: header must be valid before sending");
    assert(network_message_header_in_network_endian(header) && "error: header must be in network endian before sending");
    const int send_result = SDLNet_TCP_Send(
        socket,
        &header,
        int(sizeof(header))
    );
    if (send_result == network_utility_sdl_net_failure) {
        assert(false && "fatal error: SDLNet_TCP_Send failed");
        std::exit(EXIT_FAILURE);
    } else if (send_result != sizeof(header)) {
        assert(false && "fatal error: SDLNet_TCP_Send invalid number of bytes sent");
        std::exit(EXIT_FAILURE);
    }
}
