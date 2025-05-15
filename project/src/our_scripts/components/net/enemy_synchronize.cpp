#include "enemy_synchronize.h"
#include "../../../sdlutils/SDLUtils.h"
#include "../../../ecs/Manager.h"
#include "../../../game/Game.h"
#include <algorithm>
#include "../movement/Transform.h"
#include "../Health.h"
#include "../id_component.h"
#include "../../../network/network_message.hpp"

EnemySynchronize::EnemySynchronize()
	: _enemy_id(-1), _ht(nullptr), _tr(nullptr) {};


EnemySynchronize::~EnemySynchronize() {};

void EnemySynchronize::initComponent()
{
    auto&& mngr = Game::Instance()->get_mngr();

	_tr = mngr->getComponent<Transform>(_ent);
    assert(_tr != nullptr);
	 _ht = mngr->getComponent<Health>(_ent);
    assert(_ht != nullptr);
	auto id_comp = mngr->getComponent<id_component>(_ent);
	_enemy_id = id_comp->getId();
	std::cout << "Enemy ID: " << _enemy_id << std::endl;
	//assert(_enemy_id != -1);
}

void
EnemySynchronize::update(uint32_t delta_time) {
	(void)delta_time;
	send_enemy_update();
}

void EnemySynchronize::send_enemy_update()
{
	auto& network = Game::Instance()->get_network();

    GameStructs::DumbEnemyProperties enemyData;
    enemyData._id = _enemy_id;
    enemyData._pos = _tr->getPos();
    enemyData._health = _ht->getHealth();
    
    auto msg = create_update_enemy_message(enemyData);
    network_message_pack_send(
       network.profile.client.socket_to_host,
       network_message_pack_create(network_message_type_enemy_update,msg)
    );
}

void EnemySynchronize::update_enemy(GameStructs::DumbEnemyProperties &data)
{
    _tr->setPos(data._pos);
    _ht->setHeatlh(data._health);
}
