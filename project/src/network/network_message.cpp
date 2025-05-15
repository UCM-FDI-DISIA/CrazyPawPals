#include "network_message.hpp"
#include <SDL_net.h>
#include <cstddef>

extern inline bool network_message_connection_client_from_host_valid(const network_message_connection_client_from_host message);
extern inline bool network_message_connection_client_from_host_accepted(const network_message_connection_client_from_host message);

extern inline network_message_connection_client_from_host network_message_connection_client_from_host_create_accepted();
extern inline network_message_connection_client_from_host network_message_connection_client_from_host_create_rejected();

network_message_dynamic_pack network_message_dynamic_pack_receive(TCPsocket socket) {
    network_message_header header = network_message_header_receive(socket);
    assert(
        network_message_header_valid(header)
        && "fatal error: header must be valid after receiving"
    );
    assert(
        network_message_header_in_network_endian(header)
        && "error: header must be in network endian after receiving"
    );
    
    const size_t payload_size = size_t(SDLNet_Read16(&header.payload_size_n));
    constexpr static const size_t offset = offsetof(network_message_pack<max_align_t>, payload);
    
    auto buffer = std::make_unique<uint8_t []>(
        offset + payload_size
    );

    network_message_dynamic_pack message{
        reinterpret_cast<network_message *>(buffer.get()),
        network_message::deleter{}
    };
    message->header = header;
    if (payload_size != 0) {
        const int recv_result = SDLNet_TCP_Recv(
            socket,
            buffer.get() + offset,
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
    buffer.release();
    return message;
}
