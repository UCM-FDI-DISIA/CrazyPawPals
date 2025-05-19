#include "network_message_def.hpp"
#include "../network_utility.hpp"

#include <limits>

extern inline bool network_message_header_valid(const network_message_header header);
extern inline bool network_message_header_in_network_endian(const network_message_header header);

extern inline network_message_header network_message_header_create(
    const network_message_type_option type,
    const network_message_header_size size_h);

network_message_header network_message_header_receive(TCPsocket socket)
{
    network_message_header header;
    const int recv_result = SDLNet_TCP_Recv(
        socket,
        &header,
        int(sizeof(header)));

    if (recv_result == network_utility_sdl_net_failure)
    {
        std::cerr << "error: SDLNet_TCP_Recv failed: " << SDLNet_GetError() << std::endl;
        assert(false && "fatal error: SDLNet_TCP_Recv failed");
        std::exit(EXIT_FAILURE);
    }
    else if (recv_result == network_utility_sdl_net_connection_closed_bytes)
    {
        assert(
            network_utility_is_same_in_network_endian<network_message_type_option>(0) && "fatal error: 0 must be represented the same way in network endian");
        assert(
            network_utility_is_same_in_network_endian<network_message_type_option>(network_message_type_none) && "fatal error: network_message_type_none must be represented the same way in network endian");
        header = network_message_header{
            .connection_is_open = 0,
            .illegal = 0,
            .type_n = network_message_type_none,
            .payload_size_n = 0};
    }
    else if (recv_result != sizeof(header))
    {
        assert(false && "fatal error: SDLNet_TCP_Recv invalid number of bytes received");
        std::exit(EXIT_FAILURE);
    }
    if (!network_message_header_valid(header))
    {
        assert(false && "fatal error: network_message_header invalid");
        std::exit(EXIT_FAILURE);
    }
    else if (!network_message_header_in_network_endian(header))
    {
        assert(false && "fatal error: network_message_header not in network endian");
        std::exit(EXIT_FAILURE);
    }
    return header;
}
void network_message_header_send(TCPsocket socket, const network_message_header header)
{
    assert(network_message_header_valid(header) && "error: header must be valid before sending");
    assert(network_message_header_in_network_endian(header) && "error: header must be in network endian before sending");
    const int send_result = SDLNet_TCP_Send(
        socket,
        &header,
        int(sizeof(header)));
    if (send_result == network_utility_sdl_net_failure)
    {
        assert(false && "fatal error: SDLNet_TCP_Send failed");
        std::exit(EXIT_FAILURE);
    }
    else if (send_result != sizeof(header))
    {
        assert(false && "fatal error: SDLNet_TCP_Send invalid number of bytes sent");
        std::exit(EXIT_FAILURE);
    }
}

NetworkNewWave network_message_wave_event_create(uint16_t event_type)
{
    NetworkNewWave nne;

    SDLNet_Write32(event_type, &nne.event_type);
    return nne;
}

network_message_payload_new_connection_sync_request network_message_payload_new_connection_sync_create(const std::string_view sprite_key)
{
    static_assert(
        network_user_sprite_key_maximum_key_length <= std::numeric_limits<uint8_t>::max(),
        "static error: sprite key length exceeds uint8_t max");
    assert(
        sprite_key.size() <= network_user_sprite_key_maximum_key_length && "error: sprite key size exceeds capacity");
    return network_message_payload_new_connection_sync_request{
        network_user_sprite_key_create<network_user_sprite_key_maximum_buffer_size>(sprite_key)};
}

network_payload_skin_selection_request network_payload_skin_selection_create(
    const uint8_t requester_id,
    const std::string_view sprite_key)
{
    static_assert(
        network_user_sprite_key_maximum_key_length <= std::numeric_limits<uint8_t>::max(),
        "static error: sprite key length exceeds uint8_t max");
    assert(
        sprite_key.size() <= network_user_sprite_key_maximum_key_length && "error: sprite key size exceeds capacity");
    assert(
        requester_id < network_context_maximum_connections && "error: requester id out of range, it exceeds maximum connections");
    return network_payload_skin_selection_request{
        requester_id,
        network_user_sprite_key_create<network_user_sprite_key_maximum_buffer_size>(sprite_key)};
}
