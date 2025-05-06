
#include "GamePadPlayerCtrl.hpp"
#include "game/Game.h"
#include "ecs/Manager.h"
#include <sdlutils/InputHandler.h>
#include "our_scripts/components/movement/Transform.h"
#include "our_scripts/components/movement/MovementController.h"
#include "our_scripts/components/AnimationComponent.h"
#include "our_scripts/components/cards/Deck.hpp"
#include "our_scripts/components/weapons/Weapon.h"

void GamePadPlayerCtrl::initComponent()
{
    _mc = Game::Instance()->get_mngr()->getComponent<MovementController>(_ent);
    assert(_mc != nullptr);

    _w = Game::Instance()->get_mngr()->getComponent<Weapon>(_ent);
    assert(_w != nullptr);

    _dc = Game::Instance()->get_mngr()->getComponent<Deck>(_ent);
    assert(_dc != nullptr);

    _h = Game::Instance()->get_mngr()->getComponent<Health>(_ent);
    assert(_h != nullptr);

    _m = Game::Instance()->get_mngr()->getComponent<ManaComponent>(_ent);
    assert(_m != nullptr);

    _tr = Game::Instance()->get_mngr()->getComponent<Transform>(_ent);
    assert(_tr != nullptr);

    _dy = Game::Instance()->get_mngr()->getComponent<AnimationComponent>(_ent);
    assert(_dy != nullptr);
}

void GamePadPlayerCtrl::update(Uint32 delta_time)
{
    if (ih().getLastDevice() != InputHandler::LAST_DEVICE_ACTIVE::CONTROLLER) return;
    Vector2D aux_stick = ih().getRStick();
    aux_stick.setX(aux_stick.getX() * 8);
    aux_stick.setY(aux_stick.getY() * 4);
    _reticle_position = _tr->getPos() + aux_stick;
    _mc->set_input(ih().getLStick());
    if (ih().getLStick().magnitude() > 0.2) _dy->play_animation("andar");
    else _dy->play_animation("idle");
    if (ih().isControllerButtonDown(InputHandler::CONTROLLER_BUTTONS::LT)) {
        _dc->use_card(&_reticle_position);
        ih().consume(InputHandler::CONTROLLER_BUTTONS::LT);
    }
    if (ih().isControllerButtonDown(InputHandler::CONTROLLER_BUTTONS::RT)) {
        auto dir = ih().getRStick();
        if (!_dc->empty_hand() && _w->shoot(dir.getX(), dir.getY())) {
            //position2_f32 mouse_pos = Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->mouse_world_position; 
            _dc->discard_card();
            ih().consume(InputHandler::CONTROLLER_BUTTONS::RT);
        }
    }
    if (ih().isControllerButtonDown(InputHandler::CONTROLLER_BUTTONS::B)) {
        _dc->reload();
    }
}
