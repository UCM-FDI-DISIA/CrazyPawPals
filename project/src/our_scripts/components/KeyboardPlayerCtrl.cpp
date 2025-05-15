
#include "KeyboardPlayerCtrl.h"
#include "../../sdlutils/InputHandler.h"
#include "../../game/Game.h"
#include "../../ecs/Manager.h"
#include "../components/rendering/camera_component.hpp"
#include "cards/Deck.hpp"
#include "movement/Transform.h"
#include "weapons/Weapon.h"
#include "movement/MovementController.h"
#include "Health.h"
#include "MythicComponent.h"
#include "../mythic/MythicItems.h"
#include "AnimationComponent.h"
#ifdef GENERATE_LOG
#include "../log_writer_to_csv.hpp"
#endif

KeyboardPlayerCtrl::KeyboardPlayerCtrl()
    : _left(SDL_SCANCODE_A), _right(SDL_SCANCODE_D), _up(SDL_SCANCODE_W), _down(SDL_SCANCODE_S), 
      _reload(SDL_SCANCODE_SPACE), _collect(SDL_SCANCODE_F), 
      _mc(nullptr), _w(nullptr), _dc(nullptr), _dy(nullptr), _mouse_pos(Vector2D(0, 0)) {}

KeyboardPlayerCtrl::~KeyboardPlayerCtrl() {}

void 
KeyboardPlayerCtrl::initComponent() {

    /*
    _tr = Game::Instance()->get_mngr()->getComponent<Transform>(_ent);
    assert(_tr != nullptr);
    */
    _mc = Game::Instance()->get_mngr()->getComponent<MovementController>(_ent);
    assert(_mc != nullptr);

    _w = Game::Instance()->get_mngr()->getComponent<Weapon>(_ent);
    assert(_w != nullptr);

    _dc = Game::Instance()->get_mngr()->getComponent<Deck>(_ent);
    assert(_dc != nullptr);

    _my = Game::Instance()->get_mngr()->getComponent<MythicComponent>(_ent);
    assert(_my != nullptr);

    _h = Game::Instance()->get_mngr()->getComponent<Health>(_ent);
    assert(_h != nullptr);

    _m = Game::Instance()->get_mngr()->getComponent<ManaComponent>(_ent);
    assert(_m != nullptr);

    _tr = Game::Instance()->get_mngr()->getComponent<Transform>(_ent);
    assert(_tr != nullptr);

    _dy = Game::Instance()->get_mngr()->getComponent<AnimationComponent>(_ent);
    assert(_dy != nullptr);

}

void KeyboardPlayerCtrl::update(Uint32 delta_time) {
    if (ih().getLastDevice() != InputHandler::LAST_DEVICE_ACTIVE::KEYBOARD) return;

    (void)delta_time;
    auto& ihdlr = ih();
    //auto& dir = _tr->getDir();
    //Horizontal axis

    //TODO CHOOSE IF THIS USES KEYBOARD AND MOUSE OR GAMEPAD
    _mc->set_input(Vector2D(
        (ihdlr.isKeyDown(_left) ? -1 : 0) + (ihdlr.isKeyDown(_right) ? 1 : 0),
        (ihdlr.isKeyDown(_up) ? 1 : 0) + (ihdlr.isKeyDown(_down) ? -1 : 0)
    ));


    if (is_moving_input()) _dy->play_animation("andar");
    else _dy->play_animation("idle");
    //Vertical axis
    //dir.setY((ihdlr.isKeyDown(_up) ? -1 : 0) + (ihdlr.isKeyDown(_down) ? 1 : 0));

    //reload
    if (ihdlr.isKeyDown(_reload)) {
        _dc->reload();
    }

    //collect
    /*
    if (ihdlr.isKeyDown(_collect)) {
        //if we are not close enought to a reward, do nothing

    }
    */

    ///inputs para probar cosas
    if (ihdlr.keyDownEvent() && ihdlr.isKeyDown(SDL_SCANCODE_Y)) {
        _my->add_mythic(new ZoomiesInducer());
    }
    if (ihdlr.keyDownEvent() && ihdlr.isKeyDown(SDL_SCANCODE_G)) {
        _my->add_mythic(new BloodClaw());
    }
    if (ihdlr.keyDownEvent() && ihdlr.isKeyDown(SDL_SCANCODE_Z)) {
        _my->add_mythic(new Incense());
    }
    ////

    auto _mouse_rect = rect_f32_global_from_screen_rect_flipped_y(
        {
            {
                ih().getMousePos().getX(),
                ih().getMousePos().getY()
            },
            {
                1,1
            }
        },
        Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam
    );

#ifdef GENERATE_LOG
    /*
    log_writer_to_csv::Instance()->add_new_log("Mouse Current Position",
        "x: " + std::to_string(_mouse_rect.position.x),
        "y: " + std::to_string(_mouse_rect.position.y),
        "Mouse Position Diff",
        "x: " + std::to_string(_mouse_rect.position.x - _mouse_pos.getX()),
        "y: " + std::to_string(_mouse_rect.position.y - _mouse_pos.getY()));
    */
#endif
    _mouse_pos = Vector2D(_mouse_rect.position.x, _mouse_rect.position.y);

    if (ihdlr.mouseButtonDownEvent()) {

        //Vector2D mousePos = { (float)ihdlr.getMousePos().getX(), (float)ihdlr.getMousePos().getY()};
        //shoot
        if (ihdlr.getMouseButtonState(InputHandler::LEFT)) {
            //send message to shootih().getMousePos();
            //_w->shoot(mousePos);
            if (!_dc->empty_hand() && _w->shoot(_mouse_pos)) {
                sdlutils().soundEffects().at("player_shot").play();
#ifdef GENERATE_LOG
                log_writer_to_csv::Instance()->add_new_log("M1", "CARD DISCARDED:", _dc->hand()->get_name(), "CURRENT MANA:", _m->mana_count(), "MOUSE POS", "X", _mouse_pos.getX(), "Y", _mouse_pos.getY());
                times_m1_used++;
                auto aux = cards_discarded_this_round.find(_dc->hand()->get_name());
                if (aux == cards_discarded_this_round.end())
                    cards_discarded_this_round[_dc->hand()->get_name()] = 1;
                else
                    (*aux).second++;
#endif
                //position2_f32 mouse_pos = Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->mouse_world_position; 
                _dc->discard_card();
            }
#ifdef GENERATE_LOG
            else {
                if (_dc->empty_hand())
                    log_writer_to_csv::Instance()->add_new_log("M1", "FAILED", "NO CARD IN HAND");
                else
                    log_writer_to_csv::Instance()->add_new_log("M1", "FAILED", "WEAPON IN CD", "CARD TRYING TO DISCARD:", _dc->hand()->get_name(), "CURRENT MANA:", _m->mana_count());
            }
#endif
        }
        //use card
        else if (ihdlr.getMouseButtonState(InputHandler::RIGHT)) {
            //send message to use a card
            //Vector2D* vec = new Vector2D{ converted_mouse_pos.position.x,  converted_mouse_pos.position.y };
#ifdef GENERATE_LOG
            Card* c = _dc->hand();
            if (
#endif
                _dc->use_card(&_mouse_pos)
#ifndef GENERATE_LOG
                ;
#endif
#ifdef GENERATE_LOG
            ) {
                log_writer_to_csv::Instance()->add_new_log("M2", "CARD USED:", c->get_name(), "MANA LEFT", "MOUSE POS", "X", _mouse_pos.getX(), "Y", _mouse_pos.getY());
                times_m2_used_cards++;
                auto aux = cards_used_this_round.find(c->get_name());
                if (aux == cards_used_this_round.end())
                    cards_used_this_round[c->get_name()] = 1;
                else
                    (*aux).second++;
            }
            else {
                if (_dc->empty_hand())
                    log_writer_to_csv::Instance()->add_new_log("M2", "FAILED", "NO CARD IN HAND", "MANA LEFT", _m->mana_count());
                else {
                    log_writer_to_csv::Instance()->add_new_log("M2", "FAILED", "CARD TRYING TO USE", c->get_name(), "MANA LEFT", _m->mana_count());
                    times_m2_failed_to_use_cards++;
                }
            }
#endif
        }

    }
}

bool KeyboardPlayerCtrl::is_moving_input() const
{
    auto& ihdlr = ih();
    return ihdlr.isKeyDown(_left) || ihdlr.isKeyDown(_right) || ihdlr.isKeyDown(_up) || ihdlr.isKeyDown(_down);
}
