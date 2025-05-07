#pragma once

#include "../../ecs/Component.h" 
#include <SDL.h>
class AnimationComponent;
class MovementController;
class Deck;
class Health;
class ManaComponent;
class Transform;
class Weapon;


class GamePadPlayerCtrl :public ecs::Component
{
private:
    MovementController* _mc;
    Weapon* _w;
    Deck* _dc;
    Health* _h;
    ManaComponent* _m;
    Transform* _tr;
    Vector2D _reticle_position;
    AnimationComponent* _dy;
public:
    inline Vector2D get_reticle_position(){
        return _reticle_position;
    }
	__CMPID_DECL__(ecs::cmp::GAMEPAD_PLAYER_CTRL);
    void initComponent() override;

    void update(Uint32 delta_time) override;

};