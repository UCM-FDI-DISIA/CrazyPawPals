#ifndef NETWORK_MESSAGE_DEF_HPP
#define NETWORK_MESSAGE_DEF_HPP

#include "SDL_net.h"
#include "../network_utility.hpp"
#include <cassert>
#include <type_traits>
#include <string_view>
#include "../../utils/Vector2D.h"
#include "../../game/GameStructs.h"
#include "../src/our_scripts/components/WaveManager.h"

static int const fact_float_int = 1024;
enum network_message_type
{
    network_message_type_none = 0,
    network_message_type_any,
    network_message_type_dbg_print,
    network_message_type_dbg_print_two_byte_test = 0x0102,
    network_message_type_summon_true_bullet,
    network_message_type_summon_dummy_bullet,
    network_message_type_player_connect,
    network_message_type_player_update,
    network_message_type_client_id,
    network_message_type_new_player,
    network_message_type_create_enemy,
    network_message_type_host_has_pressed_play,
    network_message_type_player_ready,
    network_message_type_start_game,
};
using network_message_type_option = uint16_t;
using network_message_header_size = uint16_t;
struct network_message_header
{
    network_message_type_option unused : 1;
    network_message_type_option illegal : 1;
    network_message_type_option type_n : 14;
    network_message_header_size payload_size_n;
};
static_assert(
    sizeof(network_message_header) == 4,
    "static error: network_message_header size is not 4 bytes");

inline bool network_message_header_valid(const network_message_header header)
{
    return header.unused == 0 && header.illegal == 1;
}
inline bool network_message_header_in_network_endian(const network_message_header header)
{
    return network_utility_in_network_endian_u16(header.payload_size_n) && network_utility_in_network_endian_u16(header.type_n);
}

#include <iostream>
inline network_message_header network_message_header_create(
    const network_message_type_option type,
    const network_message_header_size payload_size_h)
{
    network_message_header header;
    network_message_type_option type_n;

    SDLNet_Write16(type, &type_n);
    SDLNet_Write16(payload_size_h, &header.payload_size_n);
    header.unused = 0;
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

// Struct de eventos de oleada
struct NetworkWaveEvent
{
    events event_type;
};

NetworkWaveEvent network_message_wave_event_create(events event_type);

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
struct network_message_player_ready
{
    uint32_t id_n;
    bool is_ready;
};

inline network_message_player_ready create_player_ready_message(uint32_t id, bool is_ready)
{
    network_message_player_ready msg;
    SDLNet_Write32(id, &msg.id_n);
    msg.is_ready = is_ready;
    return msg;
};

// mensaje sin contenido
struct network_message_payload_empty
{
    // vac��a
};

inline network_message_payload_empty create_payload_empty_message()
{
    return network_message_payload_empty{};
}

// mandar al cliente su id
struct network_message_client_id_from_host
{
    uint32_t client_id;
};

inline network_message_client_id_from_host create_client_id_message(uint32_t client_id)
{

    network_message_client_id_from_host id_from_host;
    SDLNet_Write32(client_id, &id_from_host.client_id);
    return id_from_host;
};

inline uint32_t network_message_client_id_from_host_get_id(const network_message_client_id_from_host message)
{
    return message.client_id;
};

// Struct de player cuando se conecta
struct network_message_player_connect
{
    uint32_t player_id;
    uint32_t sprite_key_length;
    char sprite_key[32];
};

inline network_message_player_connect create_player_connect_message(uint32_t id, std::string texture)
{
    network_message_player_connect player_connet;

    SDLNet_Write32(id, &player_connet.player_id);

    const size_t size = texture.size();
    assert(size < sizeof(player_connet.sprite_key) && "error: string size exceeds sprite_key capacity");

    SDLNet_Write32(static_cast<uint32_t>(size), &player_connet.sprite_key_length);

    std::copy_n(texture.begin(), size, player_connet.sprite_key);
    player_connet.sprite_key[size] = '\0';

    return player_connet;
}

// Struct sincronizar player
struct network_message_player_update
{
    uint32_t player_id_n;
    uint32_t sprite_key_length;
    char sprite_key[32];
    uint32_t anim_key_length;
    char anim_key[32];
    int16_t pos_n[2];
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
    SDLNet_Write16(player.pos.getX() * fact_float_int, &player_connet.pos_n[0]);
    SDLNet_Write16(player.pos.getY() * fact_float_int, &player_connet.pos_n[1]);

    // int health
    SDLNet_Write16(player.health, &player_connet.health_n);

    // bool is_ghost
    SDLNet_Write16(player.is_ghost, &player_connet.is_ghost_n);

    return player_connet;
};

// Struct de EnemyProperties
struct network_message_enemy_create
{
    int16_t _pos[2];
    uint16_t _type;
    uint32_t _enemy_id;
};

inline network_message_enemy_create create_enemy(GameStructs::DumbEnemyProperties &ep)
{
    network_message_enemy_create n_ep;

    SDLNet_Write32(ep._id, &n_ep._enemy_id);
    SDLNet_Write16(ep._type, &n_ep._type);

    // Vector2D init_pos
    SDLNet_Write16(ep._pos.getX() * fact_float_int, &n_ep._pos[0]);
    SDLNet_Write16(ep._pos.getY() * fact_float_int, &n_ep._pos[1]);

    return n_ep;
};

// Struct de EnemyProperties
struct network_message_enemy_update
{
    int16_t _pos[2];
    uint16_t _health_n;
    uint32_t _enemy_id;
};

inline network_message_enemy_update update_enemy(GameStructs::DumbEnemyProperties &ep)
{
    network_message_enemy_update n_ep;

    SDLNet_Write32(ep._id, &n_ep._enemy_id);

    // Vector2D init_pos
    SDLNet_Write16(ep._pos.getX() * fact_float_int, &n_ep._pos[0]);
    SDLNet_Write16(ep._pos.getY() * fact_float_int, &n_ep._pos[1]);

    // int health
    SDLNet_Write16(ep._health, &n_ep._health_n);

    return n_ep;
};

#endif
