#include "network_utility.hpp"
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>

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

uint32_t network_utility_get_host_local_ip(const char *const host_name, const uint16_t port) {
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

std::string network_utility_get_host_canonical_local_ip(const uint8_t port) {
    const IPaddress ip{
        .host = INADDR_ANY,
        .port = port,
    };
    char ip_host[network_utility_write_canonical_ip_buffer_size] = {0};
    network_utility_write_canonical_ip(
        network_utility_get_host_local_ip(
            network_utility_get_host_name(ip),
            port
        ),
        ip_host
    );
    return std::string{ip_host};
}

[[maybe_unused]]
static std::string network_utility_get_host_canonical_public_ip_by_curl() {
    std::system("curl \"https://api.ipify.org\" >czpp_ip.txt");
    std::string canonical_public_ip; {
        std::ifstream ip_file{"czpp_ip.txt"};
        canonical_public_ip = std::string{
            std::istreambuf_iterator<char>(ip_file),
            std::istreambuf_iterator<char>()
        };
    }
    std::remove("czpp_ip.txt");
    return canonical_public_ip;
}

[[maybe_unused]]
static std::string network_utility_get_host_canonical_public_ip_by_http(const uint8_t port) {
    (void)port; // unused
    constexpr static const uint16_t http{80};
    constexpr static const uint16_t https{443};
    constexpr static const uint16_t alternative_http{8080};
    (void)https; // unused
    (void)alternative_http; // unused

    IPaddress ipify_ip;
    const int resolved = SDLNet_ResolveHost(&ipify_ip, "api.ipify.org", http);
    if (resolved == network_utility_sdl_net_failure) {
        assert(false && "fatal error: SDLNet_ResolveHost failed");
        std::exit(EXIT_FAILURE);
    } else if (resolved != network_utility_sdl_net_success) {
        assert(false && "fatal error: SDLNet_ResolveHost invalid");
        std::exit(EXIT_FAILURE);
    }

    TCPsocket ipify_socket = SDLNet_TCP_Open(&ipify_ip);
    if (ipify_socket == nullptr) {
        std::cerr << "error: SDLNet_TCP_Open failed: " << SDLNet_GetError() << std::endl;
        assert(false && "fatal error: SDLNet_TCP_Open failed");
        std::exit(EXIT_FAILURE);
    }

    const char request[] =
        "GET / HTTP/1.1\r\n"
        "Host: api.ipify.org\r\n"
        "Connection: close\r\n"
        "\r\n";
    constexpr const size_t request_size = sizeof(request) - 1;
    const int bytes_sent = SDLNet_TCP_Send(ipify_socket, request, request_size);

    if (bytes_sent == network_utility_sdl_net_failure) {
        std::cerr << "error: SDLNet_TCP_Send failed: " << SDLNet_GetError() << std::endl;
        assert(false && "fatal error: SDLNet_TCP_Send failed");
        std::exit(EXIT_FAILURE);
    } else if (bytes_sent != request_size) {
        assert(false && "fatal error: SDLNet_TCP_Send invalid number of bytes sent");
        std::exit(EXIT_FAILURE);
    }

    constexpr static const size_t response_buffer_size = 1024;
    static_assert(
        network_utility_write_canonical_ip_buffer_size < response_buffer_size,
        "static error: response buffer size must be greater than canonical ip buffer size"
    );
    char response_buffer[response_buffer_size];

    int bytes_received;
    std::string canonical_public_ip{};
    do {
        bytes_received = SDLNet_TCP_Recv(ipify_socket, response_buffer, response_buffer_size);
        if (bytes_received == network_utility_sdl_net_failure) {
            std::cerr << "error: SDLNet_TCP_Recv failed: " << SDLNet_GetError() << std::endl;
            assert(false && "fatal error: SDLNet_TCP_Recv failed");
            std::exit(EXIT_FAILURE);
        } else if (bytes_received <= 0) {
            assert(false && "fatal error: SDLNet_TCP_Recv invalid number of bytes received");
            std::exit(EXIT_FAILURE);
        }

        if (canonical_public_ip.empty()) {
            canonical_public_ip = std::string{response_buffer, size_t(bytes_received)};
            constexpr static const char header_termination[] = "\r\n\r\n";
            const size_t ip_start = canonical_public_ip.find(header_termination) + sizeof(header_termination) - 1;
            if (ip_start != std::string::npos) {
                canonical_public_ip = canonical_public_ip.substr(ip_start);
            }
        } else {
            canonical_public_ip += std::string{response_buffer, size_t(bytes_received)};
        }
    } while (canonical_public_ip.empty() || bytes_received == response_buffer_size);

    SDLNet_TCP_Close(ipify_socket);
    return canonical_public_ip;
}

std::string network_utility_get_host_canonical_public_ip(const uint8_t port) {
    (void)port; // unused
    return network_utility_get_host_canonical_public_ip_by_curl();
}
