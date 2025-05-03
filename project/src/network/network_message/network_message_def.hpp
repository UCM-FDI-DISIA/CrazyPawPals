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

int const fact_float_int = 1024;
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
    struct deleter
    {
        void operator()(T *ptr) const
        {
            ptr->~T();
            delete[] reinterpret_cast<uint8_t *>(ptr);
        }
    };
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

//Struct de player cuando se conecta
struct network_message_player_connect {
    uint8_t player_id;
    int32_t pos[2];
    int16_t health;
    uint8_t sprite_key_length;
    char sprite_key[32];
    uint8_t is_ghost;
};

inline network_message_player_connect create_player_connect_message(const GameStructs::NetPlayerData& player) {
    network_message_player_connect player_connet;

    //uint8_t player id
    player_connet.player_id = player.id;

    //Vector2D pos
    SDLNet_Write32(player.pos.getX() * fact_float_int, &player_connet.pos[0]);
    SDLNet_Write32(player.pos.getY() * fact_float_int, &player_connet.pos[1]);

    //int health
    SDLNet_Write16(player.health, &player_connet.health);
    
    //bool is_ghost
    player_connet.is_ghost = static_cast<uint8_t>(player.is_ghost);

    //string
    const size_t key_size = player.sprite_key.size();
    assert(key_size < sizeof(player_connet.sprite_key) && "error: string size too long");
    player_connet.sprite_key_length = static_cast<uint8_t>(key_size);

    std::copy(player.sprite_key.begin(), player.sprite_key.end(), player_connet.sprite_key);

    return player_connet;
}


//Struct sincronizar player
struct network_message_player_update {
    uint8_t player_id;
    int16_t pos[2];
    int16_t health;
    uint8_t is_ghost;
};

// Struct de EnemyProperties
struct NetworkEnemyProperties
{
    int _pos[2];
};

// Constructor del struct de NetworkBulletProperties
inline NetworkEnemyProperties network_message_enemyProperties_create(GameStructs::DumbEnemyProperties ep)
{
    NetworkEnemyProperties n_ep;

    // Vector2D init_pos
    SDLNet_Write32(ep._pos.getX() * fact_float_int, &n_ep._pos[0]);
    SDLNet_Write32(ep._pos.getY() * fact_float_int, &n_ep._pos[1]);

    return n_ep;
}

inline network_message_player_update create_player_update_message(const GameStructs::NetPlayerData& player) {
    network_message_player_update player_connet;

    //uint8_t player id
    player_connet.player_id = player.id;

    //Vector2D pos
    SDLNet_Write32(player.pos.getX() * fact_float_int, &player_connet.pos[0]);
    SDLNet_Write32(player.pos.getY() * fact_float_int, &player_connet.pos[1]);

    //int health
    SDLNet_Write16(player.health, &player_connet.health);

    //bool is_ghost
    player_connet.is_ghost = static_cast<uint8_t>(player.is_ghost);

    return player_connet;
}



#endif
