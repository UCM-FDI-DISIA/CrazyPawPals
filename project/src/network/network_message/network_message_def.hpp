#ifndef NETWORK_MESSAGE_DEF_HPP
#define NETWORK_MESSAGE_DEF_HPP

#include "SDL_net.h"
#include "../network_utility.hpp"
#include <cassert>
#include <type_traits>
#include <string_view>
#include "../../utils/Vector2D.h"
#include "../../game/GameStructs.h"
// #include "../src/our_scripts/components/WaveManager.h"
#include "../network_state.hpp"
#include "../../sdlutils/SDLUtils.h"

constexpr int fixed_point_fraction_bits = 12; // 12 bits para la fracción (parte decimal)
constexpr float fact_float_int = 1 << fixed_point_fraction_bits;
enum network_message_type
{
    network_message_type_none = 0,
    network_message_type_any,
    network_message_type_dbg_print,
    network_message_type_summon_true_bullet,
    network_message_type_summon_dummy_bullet,

    network_message_type_new_connection_sync_request,
    network_message_type_new_connection_sync_response,

    network_message_type_skin_selection_request,
    network_message_type_skin_selection_response,

    network_message_type_player_update,
    network_message_type_player_ready,
    network_message_type_start_game,

    network_message_type_enemy_update,
    network_message_type_create_enemy,
    network_message_type_start_wave,
    network_message_type_end_wave
};
using network_message_type_option = uint16_t;
using network_message_header_size = uint16_t;
struct network_message_header
{
    network_message_type_option connection_is_open : 1;
    network_message_type_option illegal : 1;
    network_message_type_option type_n : 14;
    network_message_header_size payload_size_n;
};
static_assert(
    sizeof(network_message_header) == 4,
    "static error: network_message_header size is not 4 bytes");

inline bool network_message_header_valid(const network_message_header header)
{
    return header.illegal == 1;
}
inline bool network_message_header_in_network_endian(const network_message_header header)
{
    return network_utility_in_network_endian_u16(header.payload_size_n) && network_utility_in_network_endian_u16(header.type_n);
}

#include <iostream>
#include <string_view>
inline network_message_header network_message_header_create(
    const network_message_type_option type,
    const network_message_header_size payload_size_h)
{
    network_message_header header;
    network_message_type_option type_n;

    SDLNet_Write16(type, &type_n);
    SDLNet_Write16(payload_size_h, &header.payload_size_n);
    header.connection_is_open = 1;
    header.illegal = 1;
    header.type_n = type_n;

    assert(
        network_message_header_valid(header) && "error: header must be valid after creation");
    assert(
        network_message_header_in_network_endian(header) && "error: header must be in network endian after creation");
    return header;
}
network_message_header network_message_header_receive(TCPsocket socket);
void network_message_header_send(TCPsocket socket, const network_message_header header);

template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
struct network_message_payload
{
    T content;
};

template <uint32_t ArgumentsSize>
struct network_message_payload_dbg_print
{
    uint32_t args_size_n;
    std::array<uint8_t, ArgumentsSize> args;
};

template <uint32_t ArgumentsSize>
network_message_payload_dbg_print<ArgumentsSize> network_message_payload_dbg_print_create(
    std::string_view str)
{
    network_message_payload_dbg_print<ArgumentsSize> msg;

    const size_t size = str.size();
    assert(
        size < std::numeric_limits<uint32_t>::max() && "error: string size exceeds uint32_t max");

    const uint32_t args_size_h = size;
    assert(
        args_size_h < ArgumentsSize && "error: string size exceeds buffer size");
    SDLNet_Write32(args_size_h, &msg.args_size_n);

    std::copy(str.begin(), str.end(), msg.args.begin());
    return msg;
}
struct NetworkNewWave
{
    uint16_t event_type;
};
NetworkNewWave network_message_wave_event_create(uint16_t event_type);

// Struct de BulletProperties que se envia por la red
struct NetworkBulletProperties
{
    int init_pos[2];
    int dir[2];
    int speed = 0.0f;
    int damage = 0;
    int pierce_number = 0;
    float life_time = 1.0f;
    float width = 40;
    float height = 40;
    GameStructs::WeaponType weapon_type = GameStructs::DEFAULT;
    GameStructs::collide_with collision_filter;
    uint8_t sprite_key_length;
    char sprite_key[32];
};

// Constructor del struct de NetworkBulletProperties
inline NetworkBulletProperties network_message_bulletProperties_create(GameStructs::BulletProperties bp)
{
    NetworkBulletProperties n_bp;

    // Vector2D init_pos
    SDLNet_Write32(bp.init_pos.getX() * fact_float_int, &n_bp.init_pos[0]);
    SDLNet_Write32(bp.init_pos.getY() * fact_float_int, &n_bp.init_pos[1]);

    // Vector2D dir
    SDLNet_Write32(bp.dir.getX() * fact_float_int, &n_bp.dir[0]);
    SDLNet_Write32(bp.dir.getY() * fact_float_int, &n_bp.dir[1]);

    // ints
    SDLNet_Write32(bp.speed * fact_float_int, &n_bp.speed);
    SDLNet_Write32(bp.damage, &n_bp.damage);
    SDLNet_Write32(bp.pierce_number, &n_bp.pierce_number);

    // floats
    SDLNet_Write32(bp.life_time * fact_float_int, &n_bp.life_time);
    SDLNet_Write32(bp.width * fact_float_int, &n_bp.width);
    SDLNet_Write32(bp.height * fact_float_int, &n_bp.height);

    // ints
    SDLNet_Write32(bp.weapon_type, &n_bp.weapon_type);
    SDLNet_Write32(bp.collision_filter, &n_bp.collision_filter);

    // strings
    const size_t size = bp.sprite_key.size();
    assert(
        size < 32 && "error: string size exceeds 32");
    SDLNet_Write32(32, &n_bp.sprite_key_length);

    std::copy(bp.sprite_key.begin(), bp.sprite_key.end(), n_bp.sprite_key);

    return n_bp;
}

// si esta preparado para empezar el juego (si ha eleigo mazo y arma)
struct network_message_player_id
{
    uint32_t id_n;
};

inline network_message_player_id create_player_id_message(uint32_t id)
{
    network_message_player_id msg;
    SDLNet_Write32(id, &msg.id_n);
    return msg;
};

// mensaje sin contenido
struct network_message_payload_empty
{
    constexpr static const network_message_type type{
        network_message_type::network_message_type_none};
};
inline network_message_payload_empty create_payload_empty_message()
{
    return network_message_payload_empty{};
}

constexpr static const uint8_t network_user_sprite_key_maximum_buffer_size{32};
constexpr static const uint8_t network_user_sprite_key_maximum_key_length{
    network_user_sprite_key_maximum_buffer_size - 1};
template <uint8_t MaxKeyBufferSize>
struct network_user_sprite_key
{
    std::array<char, MaxKeyBufferSize> sprite_key;
    uint8_t sprite_key_length;
};
template <uint8_t MaxKeyBufferSize>
network_user_sprite_key<MaxKeyBufferSize> network_user_sprite_key_create(
    const std::string_view sprite_key)
{
    static_assert(
        MaxKeyBufferSize <= std::numeric_limits<uint8_t>::max(),
        "static error: sprite key length exceeds uint8_t max");
    assert(
        sprite_key.size() <= MaxKeyBufferSize && "error: sprite key size exceeds capacity");
    network_user_sprite_key<MaxKeyBufferSize> payload;
    payload.sprite_key_length = uint8_t(sprite_key.size());
    std::copy_n(sprite_key.begin(), sprite_key.size(), payload.sprite_key.begin());
    return payload;
}

struct network_message_payload_new_connection_sync_request
{
    network_user_sprite_key<network_user_sprite_key_maximum_buffer_size> sprite_key;
};
network_message_payload_new_connection_sync_request network_message_payload_new_connection_sync_create(
    const std::string_view sprite_key);

template <size_t MaximumConnections>
struct network_message_payload_new_connection_sync_response
{
    std::array<network_user_sprite_key<network_user_sprite_key_maximum_buffer_size>, MaximumConnections> sprite_keys;
    network_connections connections;
};
template <size_t MaximumConnections>
network_message_payload_new_connection_sync_response<MaximumConnections> network_message_payload_new_connection_sync_response_create(
    const network_connections connections,
    const std::vector<std::string_view> &sprite_keys)
{
    network_message_payload_new_connection_sync_response<MaximumConnections> payload;
    payload.connections = connections;
    assert(
        sprite_keys.size() <= MaximumConnections && "error: sprite keys size exceeds maximum connections");
    for (size_t i = 0; i < sprite_keys.size(); ++i)
    {
        payload.sprite_keys[i] = network_user_sprite_key_create<network_user_sprite_key_maximum_buffer_size>(sprite_keys[i]);
    }
    return payload;
}

struct network_payload_skin_selection_request
{
    uint8_t requester_id;
    network_user_sprite_key<network_user_sprite_key_maximum_buffer_size> sprite_key;
};
using network_payload_skin_selection_response = network_payload_skin_selection_request;
network_payload_skin_selection_request network_payload_skin_selection_create(
    const uint8_t requester_id,
    const std::string_view sprite_key);

// Struct sincronizar player
struct network_message_player_update
{
    uint32_t player_id_n;
    uint32_t sprite_key_length;
    char sprite_key[32];
    uint32_t anim_key_length;
    char anim_key[32];
    int32_t pos_n[2];
    uint16_t health_n;
    uint16_t is_ghost_n;
};

inline network_message_player_update create_player_update_message(const GameStructs::NetPlayerData &player)
{
    network_message_player_update player_connet;

    // uint32_t player id
    SDLNet_Write32(player.id, &player_connet.player_id_n);

    // textura
    const size_t sizeTex = player.sprite_key.size();
    assert(sizeTex < sizeof(player_connet.sprite_key) && "error: string size exceeds sprite_key capacity");

    SDLNet_Write32(static_cast<uint32_t>(sizeTex), &player_connet.sprite_key_length);

    std::copy_n(player.sprite_key.begin(), sizeTex, player_connet.sprite_key);
    player_connet.sprite_key[sizeTex] = '\0';

    // animacion
    const size_t sizeAnim = player.current_anim.size();
    assert(sizeAnim < sizeof(player_connet.anim_key) && "error: string size exceeds sprite_key capacity");

    SDLNet_Write32(static_cast<uint32_t>(sizeAnim), &player_connet.anim_key_length);

    std::copy_n(player.current_anim.begin(), sizeAnim, player_connet.anim_key);
    player_connet.anim_key[sizeAnim] = '\0';

    // Vector2D pos
    SDLNet_Write32(player.pos.getX() * fact_float_int, &player_connet.pos_n[0]);
    SDLNet_Write32(player.pos.getY() * fact_float_int, &player_connet.pos_n[1]);

    // int health
    SDLNet_Write16(player.health, &player_connet.health_n);

    // bool is_ghost
    SDLNet_Write16(player.is_ghost, &player_connet.is_ghost_n);

    return player_connet;
};

// Struct de EnemyProperties
struct network_message_enemy_create
{
    int32_t _pos[2];
    uint16_t _type;
    uint16_t _enemy_id;
};

inline network_message_enemy_create create_enemy(GameStructs::DumbEnemyProperties &ep)
{
    network_message_enemy_create n_ep;

    SDLNet_Write16(ep._id, &n_ep._enemy_id);
    SDLNet_Write16(ep._type, &n_ep._type);

    // Vector2D init_pos
    SDLNet_Write32(ep._pos.getX() * fact_float_int, &n_ep._pos[0]);
    SDLNet_Write32(ep._pos.getY() * fact_float_int, &n_ep._pos[1]);

    return n_ep;
};

// Struct de EnemyProperties
struct network_message_enemy_update
{
    int32_t _pos[2];
    uint16_t _health_n;
    uint16_t _enemy_id;
};

inline network_message_enemy_update create_update_enemy_message(GameStructs::DumbEnemyProperties &ep)
{
    network_message_enemy_update n_ep;
    SDLNet_Write32(ep._pos.getX() * fact_float_int, &n_ep._pos[0]);
    SDLNet_Write32(ep._pos.getY() * fact_float_int, &n_ep._pos[1]);
    SDLNet_Write16(ep._health, &n_ep._health_n);
    SDLNet_Write16(ep._id, &n_ep._enemy_id);
    return n_ep;
};

struct network_message_start_wave
{
    uint16_t wave_event;
};
inline network_message_start_wave create_start_wave_message(uint16_t wave_event)
{
    network_message_start_wave n_sw;
    SDLNet_Write16(wave_event, &n_sw.wave_event);
    return n_sw;
};

struct network_message_end_wave
{
    bool mythics_reward;
};
inline network_message_end_wave create_end_wave_message()
{
    network_message_end_wave ew;
    return ew;
};

#endif
