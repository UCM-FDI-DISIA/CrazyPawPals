#pragma once

#include "../../../ecs/Component.h"
#include "../cards/Mana.h"
#include "../Health.h"
#include "../cards/Deck.hpp"
#include "../movement/Transform.h"
#include "../rendering/camera_component.hpp"
#include "../MythicComponent.h"
class KeyboardPlayerCtrl;
class GamePadPlayerCtrl;
class PlayerHUD : public ecs::Component {
private:
	ManaComponent* _mana;
	Health* _health;
	Deck* _deck;
	Transform* _tr;
	MythicComponent* _mythics;
	const camera_component* _camera;
	KeyboardPlayerCtrl* _kpc;
	GamePadPlayerCtrl* _gpc;

	Texture* _tex_orb;
	Texture* _tex_orb_empty;
	Texture* _tex_prime;
	Texture* _tex_reticle;
public:
	__CMPID_DECL__(ecs::cmp::PLAYER_HUD);
	PlayerHUD();
	virtual ~PlayerHUD();
	void initComponent() override;
	void update(uint32_t delta_time) override;
	void render() override;
};