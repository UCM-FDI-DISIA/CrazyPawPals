#include "network_message_def.hpp"
#include "../network_utility.hpp"

#include <limits>

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
    // FIXME: allow for connection closed message
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

NetworkWaveEvent network_message_wave_event_create(events event_type) {
	NetworkWaveEvent n_we;

	//n_we.event_type = event_type;
	SDLNet_Write32(event_type, &n_we.event_type);
    return n_we;
}

network_message_payload_new_connection_sync_request network_message_payload_new_connection_sync_create(const std::string_view sprite_key) {
    static_assert(
        network_user_sprite_key_maximum_key_length <= std::numeric_limits<uint8_t>::max(),
        "static error: sprite key length exceeds uint8_t max"
    );
    assert(
        sprite_key.size() <= network_user_sprite_key_maximum_key_length
        && "error: sprite key size exceeds capacity"
    );
    return network_message_payload_new_connection_sync_request{
        network_user_sprite_key_create<network_user_sprite_key_maximum_buffer_size>(sprite_key)
    };
}
