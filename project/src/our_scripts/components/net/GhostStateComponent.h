#pragma once
#include "../../../ecs/Component.h"
#include "../../../game/GameStructs.h"

struct dyn_image_with_frames;
class MovementController;
class ManaComponent;
class Deck;
class Weapon;
class Health;

class GhostStateComponent :public ecs::Component {
public:
    __CMPID_DECL__(ecs::cmp::GHOSTSTATE);

    GhostStateComponent();

    virtual ~GhostStateComponent();

    void initComponent() override;

    void change_state();

    inline bool isGhost() { return _isGhost; }

private:
    bool _isGhost;

    void setState(GameStructs::PlayerData p);

    dyn_image_with_frames* _dy;
    MovementController* _mc;
    ManaComponent* _mana;
    Deck* _deck;
    Weapon* _weapon;
    Health* _health;

    GameStructs::PlayerData _normal_state;
    GameStructs::PlayerData _ghost_state;
};
