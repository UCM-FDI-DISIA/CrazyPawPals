#pragma once

#include <string>
#include "../components/cards/Mana.h"
#include "../components/Health.h"
#include "../components/movement/MovementController.h"
#include "../components/weapons/Weapon.h"
#include "../components/cards/Deck.hpp"
#include "game/Game.h"
#include "ecs/Manager.h"
class MythicItem {
protected:
	std::string _name;
	Texture* _texture;
	ecs::entity_t _player;
	
public:
	MythicItem(std::string name = "default",std::string texture_name = "mythic")
		:_name(name), _texture(&sdlutils().images().at(texture_name)) {
		_player = Game::Instance()->get_mngr()->getHandler(ecs::hdlr::PLAYER);
	};
	virtual ~MythicItem() = default;

	virtual void apply_effects() = 0;

	virtual void update(uint32_t dt) {(void)dt;};
	virtual std::string get_name() const { return _name; };
	virtual Texture* get_texture() const { return _texture; };
};
