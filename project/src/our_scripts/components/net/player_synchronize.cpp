#include "player_synchronize.h"
#include "../../../sdlutils/SDLUtils.h"
#include "../../../ecs/Manager.h"
#include "../../../game/Game.h"
#include <algorithm>
#include "../movement/Transform.h"
#include "../Health.h"
#include "GhostStateComponent.h"
#include "../rendering/dyn_image_with_frames.hpp"
#include "../../../network/network_message.hpp"

PlayerSynchronize::PlayerSynchronize()
    : _player_id(0), _tr(nullptr), _health(nullptr), _is_ghost(false) {};

PlayerSynchronize::~PlayerSynchronize() {};

void PlayerSynchronize::initComponent() {
    auto&& mngr = Game::Instance()->get_mngr();
    _tr = mngr->getComponent<Transform>(_ent);
    _health = mngr->getComponent<Health>(_ent);

    if (mngr->hasComponent<dyn_image_with_frames>(_ent)) {
        _tex_name = mngr->getComponent<dyn_image_with_frames>(_ent)->texture_name;
    }
    _is_ghost = mngr->hasComponent<GhostStateComponent>(_ent);

    _is_local_player = (_ent == Game::Instance()->get_mngr()->getHandler(ecs::hdlr::PLAYER));
}

void PlayerSynchronize::update(uint32_t delta_time) {
    (void)delta_time;
    if (_is_local_player) {
        sendPlayerUpdate();
    }
}

void PlayerSynchronize::sendPlayerUpdate()
{
    auto& network = Game::Instance()->get_network();

    GameStructs::NetPlayerData playerData{
        .id = _player_id,
        .pos = _tr->getPos(),
        .health = _health->getHealth(),
        .is_ghost = _is_ghost,
        .sprite_key = _tex_name
    };

    auto msg = create_player_update_message(playerData);
    
    network_message_pack_send(
       network.profile.client.socket_to_host,
       network_message_pack_create(network_message_type_player_update,msg)
    );
}

void PlayerSynchronize::updatePlayer(GameStructs::NetPlayerData& data) {
    if (_tr != nullptr) {

        _tr->setPos(data.pos);
    }
    if (_health != nullptr) {
        _health->setHeatlh(data.health);
    }
    _is_ghost = data.is_ghost;
}