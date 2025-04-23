#include "network_message.hpp"

extern inline bool network_message_connection_client_from_host_valid(const network_message_connection_client_from_host message);
extern inline bool network_message_connection_client_from_host_accepted(const network_message_connection_client_from_host message);

extern inline network_message_connection_client_from_host network_message_connection_client_from_host_create_accepted();
extern inline network_message_connection_client_from_host network_message_connection_client_from_host_create_rejected();

std::unique_ptr<network_message_pack<uint8_t *>> network_message_pack_receive_dynamic(TCPsocket socket) {
    network_message_header header = network_message_header_receive(socket);
    const size_t payload_size = size_t(SDLNet_Read16(&header.payload_size_n));
    
    auto buffer = std::make_unique<uint8_t[]>(
        offsetof(network_message_pack<uint8_t *>, payload) + payload_size
    );
    std::unique_ptr<network_message_pack<uint8_t *>> message{
        reinterpret_cast<network_message_pack<uint8_t *> *>(buffer.get())
    };
    message->header = header;

    const int recv_result = SDLNet_TCP_Recv(
        socket,
        buffer.get(),
        int(payload_size)
    );
    if (recv_result == network_utility_sdl_net_failure) {
        assert(false && "fatal error: SDLNet_TCP_Recv failed");
        std::exit(EXIT_FAILURE);
    } else if (recv_result != int(payload_size)) {
        assert(false && "fatal error: SDLNet_TCP_Recv invalid number of bytes received");
        std::exit(EXIT_FAILURE);
    }
    return message;
}
