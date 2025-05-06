#pragma once

#include "ecs/Component.h"
#include "sdlutils/SDLUtils.h"
#include <SDL.h> // Add this include to resolve the incomplete type error
class Transform;
//class dyn_image;
class DamagePopup :public ecs::Component {
public:
    __CMPID_DECL__(ecs::cmp::DAMAGE_POPUP);

    DamagePopup();

    virtual ~DamagePopup() {};

    void initComponent() override;
    void update(uint32_t dt) override;
private:
    Transform* _tr;
    float _target_y;
    //dyn_image* _img;

    uint32_t _time_since_spawn;
    const uint32_t _total_time = 700; // time the popup is active for 
    const uint32_t _movement_time = 500; // time the popup is in motion for
};
