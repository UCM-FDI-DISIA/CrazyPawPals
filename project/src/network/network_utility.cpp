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
    return uint8_t(std::snprintf(
        ip_string,
        network_utility_write_canonical_ip_buffer_size,
        "%u.%u.%u.%u",
        ip3, ip2, ip1, ip0
    ));
}

// Rosetta Code contributors. Canonicalize CIDR [Internet]. Rosetta Code; 2024 Dec 19, 18:12 UTC [cited 2025 May 2].
// Available from: https://rosettacode.org/wiki/Canonicalize_CIDR?oldid=374560.
bool network_utility_canonicalize_ip(const char *const ip, char out_canonical_ip[network_utility_write_canonical_ip_buffer_size]) {
    out_canonical_ip[0] = '\0';

    int ip0;
    int ip1;
    int ip2;
    int ip3;
    if (std::sscanf(ip, "%d.%d.%d.%d", &ip0, &ip1, &ip2, &ip3) != 4) {
        return false;
    }

    if (
        ip0 < 0 || ip0 > UINT8_MAX
        || ip1 < 0 || ip1 > UINT8_MAX
        || ip2 < 0 || ip2 > UINT8_MAX
        || ip3 < 0 || ip3 > UINT8_MAX
    ) {
        return false;
    }
    const uint32_t address = (
        (static_cast<uint32_t>(ip3) << 24)
        | (static_cast<uint32_t>(ip2) << 16)
        | (static_cast<uint32_t>(ip1) << 8)
        | static_cast<uint32_t>(ip0)
    );
    network_utility_write_canonical_ip(address, out_canonical_ip);
    return true;
}
