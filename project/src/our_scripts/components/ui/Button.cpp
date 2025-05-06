
#include "Button.h"
#include "../../../sdlutils/InputHandler.h"
#include "../../../ecs/Manager.h"
#include "../../../game/Game.h"
#include "../movement/Transform.h"
#include "../rendering/transformless_dyn_image.h"
#include <iostream>

using namespace ecs;

Button::Button(): _current_state(EMPTY),_previous_state() {
}

Button::~Button() {}

void Button::initComponent() {
    update_collider();
}
void Button::update_collider() {
    auto* mngr = Game::Instance()->get_mngr();
    auto img = mngr->getComponent<transformless_dyn_image>(_ent);
    assert(img != nullptr);

    _button_collider = img->get_destination_rect();
}
void Button::update(uint32_t delta_time) {
    (void)delta_time;
    _previous_state = _current_state;  // Guardamos el estado anterior

    update_collider();

    if (mouseOver()) {
        if (_current_state != HOVER) {
            _current_state = HOVER;
            emitHover();
        }
    }
    else {
        if (_previous_state == HOVER) {  // Detectamos la salida del puntero
            emitExit();
        }
        _current_state = EMPTY;
    }

    if (ih().mouseButtonDownEvent() && ih().getMouseButtonState(InputHandler::LEFT)) {
        leftClickDown();
    }
}

void Button::leftClickUp() {
    _current_state = EMPTY;
}

void Button::leftClickDown() {
    if (mouseOver() && _current_state == HOVER) {
        _current_state = CLICK;
        emitClick(); 
    }
}

bool Button::mouseOver() {
    Vector2D mousePos(ih().getMousePos().getX(), ih().getMousePos().getY());
    SDL_Rect mouseRect = build_sdlrect(mousePos, 1, 1);
    return SDL_HasIntersection(&_button_collider, &mouseRect);
}


void Button::connectClick(SDLEventCallback callback) {
    _click_callbacks.push_back(callback);
}


void Button::connectHover(SDLEventCallback callback) {
    _hover_callbacks.push_back(callback);
}


void Button::emitClick() const {
    for (const auto& callback : _click_callbacks) {
        callback();
    }
}


void Button::emitHover() const {
    for (const auto& callback : _hover_callbacks) {
        callback();
    }
}
void Button::connectExit(SDLEventCallback callback) {
    _pointer_exit_callbacks.push_back(callback);
}

void Button::emitExit() const {
    for (const auto& callback : _pointer_exit_callbacks) {
        callback();
    }
}

CardButton::CardButton() : Button(), it(nullptr), name() {

}

MythicButton::MythicButton() : Button(), it(nullptr), name() {

}