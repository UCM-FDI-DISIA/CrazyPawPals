
#pragma once


#include <iostream>
#include <SDL.h>
#include <array>
#include <cassert>
#include "../utils/Vector2D.h"
#include "../utils/Singleton.h"
#include <bitset>
#include <string>
#include <vector>

// Instead of a Singleton class, we could make it part of
// SDLUtils as well.
constexpr uint8_t button_list_size = 16;

class InputHandler: public Singleton<InputHandler> {

	friend Singleton<InputHandler> ;

public:
	enum MOUSEBUTTON : uint8_t {
		LEFT = 0, MIDDLE, RIGHT, _LAST_MOUSEBUTTON_VALUE
	};

	//Events in Callbacjs
	enum INPUT_EVENTES : uint8_t {
		MOUSE_LEFT_CLICK_DOWN,
		MOUSE_LEFT_CLICK_UP
	};
	enum CONTROLLER_BUTTONS : uint8_t {
		A = 0,
		B = 1,
		Y = 2,
		X = 3,
		LT = 4,
		RT = 5,
		_aux_LT = 6,
		_aux_RT = 7
	};
	enum LAST_DEVICE_ACTIVE : uint8_t {
		KEYBOARD,
		CONTROLLER
	};

	// clear the state
	inline void clearState() {
		_controller_buttons_pressed = std::bitset<button_list_size>(false);
		_isCloseWindoEvent = false;
		_isKeyDownEvent = false;
		_isKeyUpEvent = false;

		_isMouseButtonUpEvent = false;
		_isMouseButtonDownEvent = false;

		_isMouseMotionEvent = false;
		last_events.clear();
	}

	// update the state with a new event
	void update(const SDL_Event& event);

	// refresh
	inline void refresh() {
		SDL_Event event;

		clearState();
		while (SDL_PollEvent(&event)) {
			update(event);
			last_events.push_back(event);
		}
		
		/*
		for (uint8_t i = 0; i < _controller_buttons_pressed.size(); ++i) {
			std::cout << _controller_buttons_pressed[i] << " , ";
		}
		std::cout << std::endl;
		*/
		
		//std::cout << "L (" << _lStickPos.getX() << "," << _lStickPos.getY() << ")   -   R (" << _rStickPos.getX() << "," << _rStickPos.getY() << ")" << std::endl;
	}
	inline void consume(CONTROLLER_BUTTONS b) {
		_controller_buttons_pressed[b] = false;
	}

	// close window event
	inline bool closeWindowEvent() {
		return _isCloseWindoEvent;
	}

	// keyboard
	inline bool keyDownEvent() {
		return _isKeyDownEvent;
	}

	inline bool keyUpEvent() {
		return _isKeyUpEvent;
	}

	inline bool isKeyDown(SDL_Scancode key) {
		return _kbState[key] == 1;
	}

	inline bool isKeyDown(SDL_Keycode key) {
		return isKeyDown(SDL_GetScancodeFromKey(key));
	}

	inline bool isKeyUp(SDL_Scancode key) {
		return _kbState[key] == 0;
	}

	inline bool isKeyUp(SDL_Keycode key) {
		return isKeyUp(SDL_GetScancodeFromKey(key));
	}
	inline bool isControllerButtonDown(CONTROLLER_BUTTONS b) {
		return _controller_buttons_pressed[b];
	}

	// mouse
	inline bool mouseMotionEvent() {
		return _isMouseMotionEvent;
	}

	inline bool mouseButtonEvent() {
		return _isMouseButtonUpEvent || _isMouseButtonDownEvent;
	}

	inline bool mouseButtonUpEvent() {
		return _isMouseButtonUpEvent;
	}

	inline bool mouseButtonDownEvent() {
		return _isMouseButtonDownEvent;
	}

	inline const Vector2D& getMousePos() {
		return _mousePos;
	}

	inline int getMouseButtonState(uint8_t b) {
		assert(b < _LAST_MOUSEBUTTON_VALUE);
		return _mbState[b];
	}

	inline void setWindowState(bool b) {
		_isCloseWindoEvent = b;
	}

	inline Vector2D& getLStick() {
		return _lStickPos;
	}
	inline Vector2D& getRStick() {
		return _rStickPos;
	}
	inline LAST_DEVICE_ACTIVE getLastDevice() {
		return _last_active_device;
	}

	// TODO add support for Joystick, see Chapter 4 of
	// the book 'SDL Game Development'

private:
	// Set all the mouse button state to false (each button turns true when it is pressed)
	InputHandler() : _mbState{} {
		_kbState = nullptr;
		clearState();
	}

	virtual ~InputHandler() {
	}

	inline bool init() {
		_kbState = SDL_GetKeyboardState(0);
		assert(_kbState != nullptr);


		//SDL_JoystickEventState(SDL_ENABLE);
		//_joystick = SDL_JoystickOpen(0);
		//assert(_joystick != nullptr);

		return true;
	}

	inline void onKeyDown(const SDL_Event&) {
		_isKeyDownEvent = true;
	}

	inline void onKeyUp(const SDL_Event&) {
		_isKeyUpEvent = true;
	}

	inline void onMouseMotion(const SDL_Event &event) {
		_isMouseMotionEvent = true;
		_mousePos.set( event.motion.x, event.motion.y);
	}

	inline void onMouseButtonDown(const SDL_Event &event) {
		_isMouseButtonDownEvent = true;
		switch (event.button.button) {
		case SDL_BUTTON_LEFT:
			_mbState[LEFT] = true;
			break;
		case SDL_BUTTON_MIDDLE:
			_mbState[MIDDLE] = true;
			break;
		case SDL_BUTTON_RIGHT:
			_mbState[RIGHT] = true;
			break;
		default:
			break;
		}
	}

	inline void onMouseButtonUp(const SDL_Event &event) {
		_isMouseButtonUpEvent = true;
		switch (event.button.button) {
		case SDL_BUTTON_LEFT:
			_mbState[LEFT] = false;
			break;
		case SDL_BUTTON_MIDDLE:
			_mbState[MIDDLE] = false;
			break;
		case SDL_BUTTON_RIGHT:
			_mbState[RIGHT] = false;
			break;
		default:
			break;
		}
	}

	inline void handleWindowEvent(const SDL_Event &event) {
		switch (event.window.event) {
		case SDL_WINDOWEVENT_CLOSE:
			_isCloseWindoEvent = true;
			break;
		default:
			break;
		}
	}

	bool _isCloseWindoEvent;
	bool _isKeyUpEvent;
	bool _isKeyDownEvent;
	bool _isMouseMotionEvent;
	bool _isMouseButtonUpEvent;
	bool _isMouseButtonDownEvent;
	Vector2D _mousePos;
	std::array<bool, 3> _mbState;
	const Uint8 *_kbState;
	//12, 13 are for triggers	(its press down)
	//14, 15 are aux for triggers (its real state)
	std::bitset<button_list_size> _controller_buttons_pressed;

	//Gamepad
	SDL_Joystick* _joystick[4];
	Vector2D _lStickPos;
	Vector2D _rStickPos;
	LAST_DEVICE_ACTIVE _last_active_device = KEYBOARD;

	int _joystick_dead_zone = 3000;

	int16_t _time_till_next_click_ms = 500;
	uint16_t _next_RT_click = 0;
	uint16_t _next_LT_click = 0;
	std::vector<SDL_Event> last_events;

public:
	inline const std::vector<SDL_Event>& get_last_events() const {
		return last_events;
	}
}
;

// This macro defines a compact way for using the singleton InputHandler, instead of
// writing InputHandler::instance()->method() we write ih().method()
//
inline InputHandler& ih() {
	return *InputHandler::Instance();
}
