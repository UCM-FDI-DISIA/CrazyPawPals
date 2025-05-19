#include "enemy_synchronize.h"
#include "../../../sdlutils/SDLUtils.h"
#include "../../../ecs/Manager.h"
#include "../../../game/Game.h"
#include <algorithm>
#include "../movement/Transform.h"
#include "../rendering/dyn_image.hpp"
#include "../Health.h"
#include "../id_component.h"
#include "../../../network/network_message.hpp"

EnemySynchronize::EnemySynchronize()
	: _enemy_id(-1), _ht(nullptr), _tr(nullptr), _last_update(0) {};

EnemySynchronize::~EnemySynchronize() {};

void EnemySynchronize::initComponent()
{
	auto &&mngr = Game::Instance()->get_mngr();

	_tr = mngr->getComponent<Transform>(_ent);
	assert(_tr != nullptr);
	_ht = mngr->getComponent<Health>(_ent);
	assert(_ht != nullptr);
	auto id_comp = mngr->getComponent<id_component>(_ent);
	_enemy_id = id_comp->getId();
	// std::cout << "Enemy ID en syncronize: " << (int)_enemy_id << std::endl;

	_dy = Game::Instance()->get_mngr()->getComponent<dyn_image>(_ent);
	assert(_dy != nullptr);

	assert(_enemy_id != -1);
}

void EnemySynchronize::update(uint32_t delta_time)
{
	_last_update += delta_time;

	// if (_last_update >= 500) {
	send_enemy_update();
	//_last_update = 0;
	//}
}

void EnemySynchronize::send_enemy_update()
{
	auto &network = Game::Instance()->get_network();

	GameStructs::DumbEnemyProperties enemyData;
	enemyData._id = _enemy_id;
	// std::cout << "Enemy ID en syncronize: " << (int)enemyData._id << std::endl;

	enemyData._pos = _tr->getPos();
	enemyData._health = _ht->getHealth();

	auto msg = create_update_enemy_message(enemyData);
	const auto message = network_message_pack_create(network_message_type_enemy_update, msg);
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

void EnemySynchronize::update_enemy(GameStructs::DumbEnemyProperties &data)
{
	auto &&mngr = Game::Instance()->get_mngr();

	auto id_comp = mngr->getComponent<id_component>(_ent);

	_enemy_id = id_comp->getId();
	// std::cout << "Enemy ID en syncronize: " << (int)_enemy_id << std::endl;

	Vector2D _last_pos = _tr->getPos();
	_tr->setPos(data._pos);
	Vector2D delta = _tr->getPos() - _last_pos;

	//_dy->flip = (delta.getX() >= 0) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;

	_ht->setHeatlh(data._health);

	if (_ht->getHealth() <= 0)
		Game::Instance()->get_mngr()->setAlive(_ent, false);
}
