#pragma once

#include "../../ecs/Component.h" 
#include <SDL.h>
#include <unordered_map>

class Transform;
class Health;
class Weapon;
class Deck;
class ManaComponent;
class MovementController;
class MythicComponent;
class AnimationComponent;
class GhostStateComponent;

class KeyboardPlayerCtrl :public ecs::Component
{
public:
	__CMPID_DECL__(ecs::cmp::KEYBOARDPLAYERCTRL);
	KeyboardPlayerCtrl(); 
	virtual ~KeyboardPlayerCtrl(); 
	void initComponent() override;
	void update(Uint32 delta_time) override;

	bool is_moving_input() const;
	inline Vector2D get_reticle_position() {
		return _mouse_pos;
	}
#ifdef GENERATE_LOG
	int times_m2_used_cards = 0;
	int times_m2_failed_to_use_cards = 0;
	int times_m1_used = 0;
	std::unordered_map<std::string, uint8_t> cards_used_this_round;
	std::unordered_map<std::string, uint8_t> cards_discarded_this_round;
#endif
private:
	SDL_Scancode _left;
	SDL_Scancode _right;
	SDL_Scancode _up;
	SDL_Scancode _down;
	SDL_Scancode _reload;
	SDL_Scancode _collect;
	Vector2D _mouse_pos;

	AnimationComponent* _dy;
	MovementController* _mc;
	Weapon* _w;
	Deck* _dc;
	Health* _h;
	ManaComponent* _m;
	Transform* _tr;

	GhostStateComponent* _g;
	MythicComponent* _my;
};