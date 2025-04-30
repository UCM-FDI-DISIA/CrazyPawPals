#ifndef NETWORK_UTILITY_HPP
#define NETWORK_UTILITY_HPP

#include <SDL_net.h>
#include <cstdint>
#include <cassert>
#include <cstdlib>

#define NETWORK_UTILITY_SDL_NET_FAILURE (-1)
constexpr const int network_utility_sdl_net_failure = NETWORK_UTILITY_SDL_NET_FAILURE;

#define NETWORK_UTILITY_SDL_NET_SUCCESS (0)
constexpr const int network_utility_sdl_net_success = NETWORK_UTILITY_SDL_NET_SUCCESS;


#ifndef NETWORK_UTILITY_SDL_NET_DRAIN_AND_CLOSE_BUFFER_CHUNK_SIZE
#define NETWORK_UTILITY_SDL_NET_DRAIN_AND_CLOSE_BUFFER_CHUNK_SIZE_DEFAULT 1024

#define NETWORK_UTILITY_SDL_NET_DRAIN_AND_CLOSE_BUFFER_CHUNK_SIZE \
    NETWORK_UTILITY_SDL_NET_DRAIN_AND_CLOSE_BUFFER_CHUNK_SIZE_DEFAULT

#endif

constexpr const size_t network_utility_sdlnet_drain_and_close_buffer_chunk_size =
    NETWORK_UTILITY_SDL_NET_DRAIN_AND_CLOSE_BUFFER_CHUNK_SIZE;
template <size_t BufferChunkSize = network_utility_sdlnet_drain_and_close_buffer_chunk_size>
void network_utility_sdlnet_drain_and_close(TCPsocket socket) {
    assert(socket != nullptr && "error: socket must not be null before draining and closing");
    static_assert(
        BufferChunkSize > 0,
        "error: BufferChunkSize must be greater than 0"
    );
    int bytes_received = 0;
    do {
        char buffer[BufferChunkSize];
        bytes_received = SDLNet_TCP_Recv(socket, buffer, sizeof(buffer));
    } while (bytes_received > 0);

    if (bytes_received == network_utility_sdl_net_failure) {
        assert(false && "fatal error: SDLNet_TCP_Recv failed");
        std::exit(EXIT_FAILURE);
    } else if (bytes_received < 0) {
        assert(false && "error: SDLNet_TCP_Recv invalid number of bytes received");
        std::exit(EXIT_FAILURE);
    }
    SDLNet_TCP_Close(socket);
}

inline bool network_utility_in_network_endian_u16(const uint16_t value_n) {
    const uint16_t value_h = SDLNet_Read16(&value_n);
    const uint8_t value_h0 = value_h & 0xFF;
    const uint8_t value_h1 = (value_h >> 8) & 0xFF;
    return (value_h0 == *(reinterpret_cast<const uint8_t *>(&value_n) + sizeof(uint16_t) - 1))
           & (value_h1 == *(reinterpret_cast<const uint8_t *>(&value_n) + sizeof(uint16_t) - 2));
}
inline bool network_utility_in_network_endian_u32(const uint32_t value_n) {
    const uint32_t value_h = SDLNet_Read32(&value_n);
    const uint8_t value_h0 = value_h & 0xFF;
    const uint8_t value_h1 = (value_h >> 8) & 0xFF;
    const uint8_t value_h2 = (value_h >> 16) & 0xFF;
    const uint8_t value_h3 = (value_h >> 24) & 0xFF;
    return (value_h0 == *(reinterpret_cast<const uint8_t *>(&value_n) + sizeof(uint32_t) - 1))
           & (value_h1 == *(reinterpret_cast<const uint8_t *>(&value_n) + sizeof(uint32_t) - 2))
           & (value_h2 == *(reinterpret_cast<const uint8_t *>(&value_n) + sizeof(uint32_t) - 3))
           & (value_h3 == *(reinterpret_cast<const uint8_t *>(&value_n) + sizeof(uint32_t) - 4));
}

#endif
