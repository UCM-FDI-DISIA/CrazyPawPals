

#include "InputHandler.h"
#include "SDLUtils.h"


void InputHandler::update(const SDL_Event& event) {

	switch (event.type) {
	case SDL_JOYAXISMOTION: {
		//DEAD ZONE
		auto stickValue = std::abs(event.jaxis.value) > _joystick_dead_zone ? event.jaxis.value : 0;
		//X axis
		switch (event.jaxis.axis) {
		case 0: {
			_lStickPos.setX(stickValue / 32767.0);
			break;
		}
		case 1: {
			_lStickPos.setY(-stickValue / 32767.0);
			break;
		}
		case 2: {
			_rStickPos.setX(stickValue / 32767.0);
			break;
		}
		case 3: {
			_rStickPos.setY(-stickValue / 32767.0);
			break;
		}
			  //LT
		case 4: {
			if (_next_LT_click < sdlutils().virtualTimer().currTime()) {
				_controller_buttons_pressed[LT] = event.jaxis.value > 0;
				if (event.jaxis.value > 0)
					_next_LT_click = sdlutils().virtualTimer().currTime() + _time_till_next_click_ms;
			}
			break;
		}
			  //RT
		case 5: {
			if (_next_RT_click < sdlutils().virtualTimer().currTime()) {
				_controller_buttons_pressed[RT] = event.jaxis.value > 0;
				if (event.jaxis.value > 0)
					_next_RT_click = sdlutils().virtualTimer().currTime() + _time_till_next_click_ms;
			}
			break;
		}
		}
		if (abs(event.jaxis.value) > 3000) _last_active_device = CONTROLLER;
		break;
	}
	case SDL_JOYBUTTONDOWN: {
		//uint8_t i = event.jbutton.button;
		_controller_buttons_pressed[event.jbutton.button] = true;
		_last_active_device = CONTROLLER;
		break;
	}
	case SDL_JOYBUTTONUP: {
		//uint8_t i = event.jbutton.button;
		_controller_buttons_pressed[event.jbutton.button] = false;
		_last_active_device = CONTROLLER;
		break;
	}
	case SDL_KEYDOWN:
		onKeyDown(event);
		_last_active_device = KEYBOARD;
		break;
	case SDL_KEYUP:
		onKeyUp(event);
		_last_active_device = KEYBOARD;
		break;
	case SDL_MOUSEMOTION:
		onMouseMotion(event);
		_last_active_device = KEYBOARD;
		break;
	case SDL_MOUSEBUTTONDOWN:
		onMouseButtonDown(event);
		_last_active_device = KEYBOARD;
		break;
	case SDL_MOUSEBUTTONUP:
		onMouseButtonUp(event);
		_last_active_device = KEYBOARD;
		break;
	case SDL_WINDOWEVENT:
		handleWindowEvent(event);
		break;
	case SDL_JOYDEVICEADDED:
		SDL_JoystickEventState(SDL_ENABLE);
		assert(SDL_NumJoysticks() < 4);
		for (int i = 0; i < SDL_NumJoysticks(); ++i)
			_joystick[i] = SDL_JoystickOpen(i);
		break;
	default:
		break;
	}
}