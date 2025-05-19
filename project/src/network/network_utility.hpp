#ifndef NETWORK_UTILITY_HPP
#define NETWORK_UTILITY_HPP

#include <SDL_net.h>
#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <array>
#include <string>
#include <type_traits>

#define NETWORK_UTILITY_SDL_NET_FAILURE (-1)
constexpr const int network_utility_sdl_net_failure = NETWORK_UTILITY_SDL_NET_FAILURE;

#define NETWORK_UTILITY_SDL_NET_SUCCESS (0)
constexpr const int network_utility_sdl_net_success = NETWORK_UTILITY_SDL_NET_SUCCESS;

#define NETWORK_UTILITY_SDL_NET_CONNECTION_CLOSED_BYTES (0)
constexpr const int network_utility_sdl_net_connection_closed_bytes =
    NETWORK_UTILITY_SDL_NET_CONNECTION_CLOSED_BYTES;


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
    if (SDLNet_SocketReady(socket)) {
        static_assert(
            BufferChunkSize > 0,
            "error: BufferChunkSize must be greater than 0"
        );
        int bytes_received;
        do {
            char buffer[BufferChunkSize];
            bytes_received = SDLNet_TCP_Recv(socket, buffer, sizeof(buffer));
        } while (bytes_received > 0);
    
        if ((bytes_received != network_utility_sdl_net_failure) && (bytes_received < 0)) {
            assert(false && "error: SDLNet_TCP_Recv invalid number of bytes received");
            std::exit(EXIT_FAILURE);
        }
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

template <
    typename T,
    typename = std::enable_if_t<
        std::conjunction_v<
            std::is_integral<T>,
            std::disjunction<
                std::bool_constant<sizeof(T) == sizeof(uint8_t)>,
                std::bool_constant<sizeof(T) == sizeof(uint16_t)>,
                std::bool_constant<sizeof(T) == sizeof(uint32_t)>
            >
        >
    >
>
inline bool network_utility_is_same_in_network_endian(auto value) {
    if constexpr (sizeof(value) == sizeof(uint8_t)) {
        return true;
    } else if constexpr (sizeof(value) == sizeof(uint16_t)) {
        return network_utility_in_network_endian_u16(value);
    } else if constexpr (sizeof(value) == sizeof(uint32_t)) {
        return network_utility_in_network_endian_u32(value);
    } else {
        assert(false && "unreachable: value is not an 8, 16, or 32 bit integral type");
        std::exit(EXIT_FAILURE);
        return false;
    }
}

constexpr static const size_t network_utility_write_canonical_ip_buffer_size = 3 * 4 + 3 + 1;
const char *network_utility_get_host_name(const IPaddress &ip);
uint32_t network_utility_get_host_local_ip(const char *const host_name, const uint16_t port);
uint8_t network_utility_write_canonical_ip(const uint32_t ip, char ip_string[network_utility_write_canonical_ip_buffer_size]);

bool network_utility_canonicalize_ip(const char *const ip, char out_canonical_ip[network_utility_write_canonical_ip_buffer_size]);

std::string network_utility_get_host_canonical_local_ip(const uint8_t port);
std::string network_utility_get_host_canonical_public_ip(const uint8_t port);

#endif
