#include "player_synchronize.h"
#include "../../../sdlutils/SDLUtils.h"
#include "../../../ecs/Manager.h"
#include "../../../game/Game.h"
#include "../../../game/scenes/GameScene.h"
#include <algorithm>
#include "../rendering/dyn_image_with_frames.hpp"
#include "../../../network/network_message.hpp"
#include "../movement/Transform.h"
#include "../Health.h"
#include "../AnimationComponent.h"
#include "GhostStateComponent.h"
#include <cassert>
#include <cstdlib>

static uint32_t player_sync_player_id_expect(const uint32_t player_id) {
    if (player_id >= uint32_t(network_context_maximum_connections)) {
        assert(
            false
            && "error: player id must be less than the maximum number of connections"
        );
        std::exit(EXIT_FAILURE);
    }
    return player_id;
}

PlayerSynchronize::PlayerSynchronize(uint32_t _player_id)
    : _player_id(player_sync_player_id_expect(_player_id)), _tr(nullptr), _health(nullptr), _is_ghost(false) {};

PlayerSynchronize::~PlayerSynchronize() {};

void PlayerSynchronize::initComponent() {
    auto&& mngr = Game::Instance()->get_mngr();
    _tr = mngr->getComponent<Transform>(_ent);
    assert(_tr != nullptr);

    _health = mngr->getComponent<Health>(_ent);
    assert(_health != nullptr);

    _anim = mngr->getComponent<AnimationComponent>(_ent);
    assert(_anim != nullptr);

    if (mngr->hasComponent<dyn_image_with_frames>(_ent)) {
        _tex_name = mngr->getComponent<dyn_image_with_frames>(_ent)->texture_name;
    }
    _is_local_player = (_ent == Game::Instance()->get_mngr()->getHandler(ecs::hdlr::PLAYER));
}

void PlayerSynchronize::update(uint32_t delta_time) {
    (void)delta_time;
    if (_is_local_player) {
        _is_ghost = Game::Instance()->get_mngr()->hasComponent<GhostStateComponent>(_ent);
        sendPlayerUpdate();
    }
}

void PlayerSynchronize::sendPlayerUpdate()
{
    auto& network = Game::Instance()->get_network();

    GameStructs::NetPlayerData playerData;
    playerData.id = _player_id;
    playerData.pos = _tr->getPos();
    playerData.health = _is_ghost ? 0 : _health->getHealth();
    playerData.is_ghost = _is_ghost;
    playerData.sprite_key = _tex_name;
    playerData.current_anim = _anim->get_current_Anim();
    

    const auto payload = create_player_update_message(playerData);
    const auto message = network_message_pack_create(
        network_message_type_player_update,
        payload
    );
    switch (network.profile_status)
    {
    case network_context_profile_status_none: {
        assert(false && "unreachable: invalid network profile status");
        std::exit(EXIT_FAILURE);
    }
    case network_context_profile_status_host: {
        for (network_connection_size i = 0; i < network.profile.host.sockets_to_clients.connection_count; ++i) {
            TCPsocket& connection = network.profile.host.sockets_to_clients.connections[i];
            network_message_pack_send(connection, message);
        }
    }
    case network_context_profile_status_client: {
        network_message_pack_send(network.profile.client.socket_to_host, message);
        break;
    }
    default: {
        assert(false && "unreachable: invalid network profile status");
        std::exit(EXIT_FAILURE);
    }
    }
}

void PlayerSynchronize::updatePlayer(GameStructs::NetPlayerData& data) {
    if (data.sprite_key != _tex_name)
        GameScene::change_player_tex(_player_id, _tex_name = data.sprite_key);

    _tr->setPos(data.pos);
    _health->setHeatlh(data.health);

    if (data.current_anim != _anim->get_current_Anim())
        _anim->play_animation(data.current_anim);

    _is_ghost = data.is_ghost;
    GameScene::change_player_filter(data.id, _is_ghost
        ? filter{ 100, 180, 255, 160 } : filter{ 255, 255, 255, 255 });
    
}