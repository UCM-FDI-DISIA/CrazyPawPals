
#include "DashingState.h"

#include "../../game/scenes/GameScene.h"
#include "../components/movement/Transform.h"
#include "../components/movement/Follow.h"
#include "../components/movement/MovementController.h"

DashingState::DashingState(Transform* tr, MovementController* movementController, Follow* follow, uint32_t time, float extra_space)
    : _tr(tr), _movementController(movementController), _fll(follow), _time(time), _extra_space(extra_space){}

void 
DashingState::enter() {
    assert(_tr != nullptr);
    assert(_movementController != nullptr);
    assert(_fll != nullptr);

    Vector2D direction = _fll->get_act_follow()->getPos() - _tr->getPos();
    direction = direction.normalize();
    Vector2D dash_target = _fll->get_act_follow()->getPos() + direction * _extra_space;
    _movementController->dash(dash_target, _time);
}

void 
DashingState::exit() {
   _movementController->set_input({0,0});
}