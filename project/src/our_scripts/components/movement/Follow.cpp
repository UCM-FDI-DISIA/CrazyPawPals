#pragma once

#include "Transform.h"
#include "Follow.h"
#include "../Health.h"
#include "../../../ecs/Manager.h"
#include "../../../game/Game.h"

Follow::Follow(GameStructs::EnemyFollow type) : _my_follow_type(type), _my_tr(nullptr), _act_follow(nullptr), last_time_act(0)
{
}

void Follow::initComponent()
{
	_my_tr = Game::Instance()->get_mngr()->getComponent<Transform>(_ent);
	assert(_my_tr != nullptr);
	act_follow();
}

void Follow::update(uint32_t delta_time)
{
	last_time_act += delta_time;
	if (last_time_act > 2000)
	{
		//std::cout << "Follow::update" << std::endl;
		act_follow();
		last_time_act = 0;
	}
}

void Follow::act_follow()
{
	switch (_my_follow_type)
	{
	case GameStructs::EnemyFollow::CLOSEST:
		search_closest_player();
		break;
	case GameStructs::EnemyFollow::FURTHEST:
		search_furthest_player();
		break;
	case GameStructs::EnemyFollow::LOWREST_LIFE:
		search_lowest_life_player();
		break;
	case GameStructs::EnemyFollow::HIGHEST_LIFE:
		search_highest_life_player();
		break;
	default:
		search_closest_player();
		break;
	}
}

void Follow::search_closest_player()
{
	Transform *player = nullptr;
	float min_distance = 1000.0f;

	auto &&manager = *Game::Instance()->get_mngr();
	auto playerEntities = manager.getEntities(ecs::grp::PLAYER); // Entidad de players

	for (auto playerEntity : playerEntities)
	{
		auto tr = manager.getComponent<Transform>(playerEntity);
		float distance = (_my_tr->getPos() - tr->getPos()).magnitude();
		if (distance < min_distance && tr != _act_follow)
		{
			//std::cout << "Encontre a un player mas cerca" << std::endl;
			min_distance = distance;
			player = tr;
		}
	}

	if(player)_act_follow = player;
}

void Follow::search_furthest_player()
{
	Transform *player = nullptr;
	float min_distance = 0.0f;

	auto &&manager = *Game::Instance()->get_mngr();
	auto playerEntities = manager.getEntities(ecs::grp::PLAYER); // Entidad de players

	for (auto playerEntity : playerEntities)
	{
		auto tr = manager.getComponent<Transform>(playerEntity);
		float distance = (_my_tr->getPos() - tr->getPos()).magnitude();
		if (distance > min_distance)
		{
			min_distance = distance;
			player = tr;
		}
	}
	_act_follow = player;
}

void Follow::search_lowest_life_player()
{
	Transform *player = nullptr;
	float min_life = 1000.0f;

	auto &&manager = *Game::Instance()->get_mngr();
	auto playerEntities = manager.getEntities(ecs::grp::PLAYER); // Entidad de players

	for (auto playerEntity : playerEntities)
	{
		auto hc = manager.getComponent<Health>(playerEntity);
		float act_health = hc->getHealth();
		if (act_health < min_life)
		{
			min_life = act_health;
			auto tr = manager.getComponent<Transform>(playerEntity);
			player = tr;
		}
	}
	_act_follow = player;
}

void Follow::search_highest_life_player()
{
	Transform *player = nullptr;
	float min_life = 0.0f;

	auto &&manager = *Game::Instance()->get_mngr();
	auto playerEntities = manager.getEntities(ecs::grp::PLAYER); // Entidad de players

	for (auto playerEntity : playerEntities)
	{
		auto hc = manager.getComponent<Health>(playerEntity);
		float act_health = hc->getHealth();
		if (act_health > min_life)
		{
			min_life = act_health;
			auto tr = manager.getComponent<Transform>(playerEntity);
			player = tr;
		}
	}
	_act_follow = player;
}
