#include "network_utility.hpp"
#include <cstdio>

extern inline bool network_utility_in_network_endian_u16(const uint16_t value_n);
extern inline bool network_utility_in_network_endian_u32(const uint32_t value_n);

const char *network_utility_get_host_name(const IPaddress &ip) {
    const char *const host_name = SDLNet_ResolveIP(&ip);
    if (host_name == nullptr) {
        assert(false && "fatal error: SDLNet_ResolveIP failed");
        std::exit(EXIT_FAILURE);
    }
    return host_name;
}

uint32_t network_utility_get_host_ip(const char *const host_name, const uint16_t port) {
    IPaddress ip;
    const int resolved = SDLNet_ResolveHost(&ip, host_name, port);
    if (resolved == network_utility_sdl_net_failure) {
        assert(false && "fatal error: SDLNet_ResolveHost failed");
        std::exit(EXIT_FAILURE);
    } else if (resolved != network_utility_sdl_net_success) {
        assert(false && "fatal error: SDLNet_ResolveHost invalid");
        std::exit(EXIT_FAILURE);
    }
    return ip.host;
}

uint8_t network_utility_write_canonical_ip(const uint32_t ip, char ip_string[network_utility_write_canonical_ip_buffer_size]) {
    const uint8_t ip0 = ip >> 24;
    const uint8_t ip1 = (ip >> 16) & 0xFF;
    const uint8_t ip2 = (ip >> 8) & 0xFF;
    const uint8_t ip3 = ip & 0xFF;
    return std::snprintf(ip_string, network_utility_write_canonical_ip_buffer_size, "%u.%u.%u.%u", ip3, ip2, ip1, ip0);
}