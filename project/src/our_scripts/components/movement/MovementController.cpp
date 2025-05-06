
#include "MovementController.h"
#include "Transform.h"
#include "../../../ecs/Manager.h"
#include "../../../sdlutils/SDLUtils.h"
#include "../../../game/Game.h"
#include "../rigidbody_component.hpp"
MovementController::MovementController(float max_speed, float acceleration, float decceleration) 
	: _tr(nullptr), _max_speed(max_speed), _acceleration(acceleration), _decceleration(decceleration), 
	_dashing(false), _time_remaining(0),  _dash_duration(0), _start_pos(0,0), _frozen(false),_frozen_duration(0) {
	event_system::event_manager::Instance()->suscribe_to_event(event_system::change_deccel, this, &event_system::event_receiver::event_callback0);
}

MovementController::~MovementController() {
	event_system::event_manager::Instance()->unsuscribe_to_event(event_system::change_deccel,this, &event_system::event_receiver::event_callback0);
}

void
MovementController::initComponent() {
	_tr = Game::Instance()->get_mngr()->getComponent<Transform>(_ent);
	assert(_tr != nullptr);

	_coll = Game::Instance()->get_mngr()->getComponent<collisionable>(_ent);
	assert(_coll != nullptr);
}

void MovementController::set_input(Vector2D vec) {
	if (!_dashing) {
		if (vec.magnitude() > 1)
			_input = vec.normalize();
		else
			_input = vec;
	}
}

void MovementController::update(uint32_t delta_time)
{
	if (_frozen) {
		_frozen_duration = (_frozen_duration > delta_time) ? _frozen_duration - delta_time : 0;
		if (_frozen_duration == 0) _frozen = false;
		return;
	}

	if (_dashing) {
		float t = 1.0f - (static_cast<float>(_time_remaining) / static_cast<float>(_dash_duration));
        
		t = t * t * (3.0f - 2.0f * t);

        t = std::clamp(t, 0.0f, 1.0f);
        Vector2D newPos = Vector2D(
            _start_pos.getX() + (_dash_pos.getX() - _start_pos.getX()) * t,
            _start_pos.getY() + (_dash_pos.getY() -  _start_pos.getY()) * t
        );
       
		//_tr->setPos(_start_pos + (_dash_pos - _start_pos) * t);
		_tr->setPos(newPos);
        
		_time_remaining = (_time_remaining > delta_time) ? _time_remaining - delta_time : 0;

		float distanceToTarget = (_dash_pos - _tr->getPos()).magnitude();
		//std::cout << "distance: " << distanceToTarget << std::endl;

        if (_time_remaining <= 100 || distanceToTarget <= 0.1f) {
			//std::cout << "dashing" << std::endl;

            _dashing = false;
            _tr->setPos(_dash_pos);
            _tr->setDir(Vector2D(0, 0));
            _coll->options = collisionable_option_none;
        }
	}
	else {
		Vector2D expected_speed = _input * _max_speed;
		Vector2D speed_dif = expected_speed - _tr->getDir();
		//calculamos si usar aceleraci�n o deceleraci�n
		float accelRate = (expected_speed.magnitude() - _tr->getDir().magnitude() < 0 || abs(expected_speed.angle(_tr->getDir())) > 15) ? _decceleration : _acceleration;
		accelRate *= delta_time / 1000.0f;
		_tr->add_directional_speed(speed_dif * accelRate);
#ifdef GENERATE_LOG
		total_movement += _tr->getSpeed(); 
#endif

	}

	/*
	Vector2D my_new_speed = _tr->getDir() * (1 - _decceleration) + expected_speed * _decceleration;
	float accelerate = _acceleration * delta_time/1000;
	
	if (my_new_speed.magnitude() > _tr->getSpeed()) {
		//accelerate
		_tr->setDir(_tr->getDir() * (1 - accelerate) + expected_speed * accelerate);
	}
	else {

		float deccelerate = _decceleration * delta_time/1000;
		
		//deccelerate
		_tr->setDir(_tr->getDir() * (1 - deccelerate) + expected_speed * deccelerate);
	}
	*/
}


void MovementController::dash(Vector2D next_pos, uint32_t time) {
	if (!_dashing) {
		// Solo podemos iniciar un dash si no estamos ya en uno
		_dashing = true;
		_time_remaining = std::max(time, 100u); 
		_dash_pos = next_pos;
		_dash_duration = std::max(time, 100u); 
        _start_pos = _tr->getPos();
		_coll->options = collisionable_option_trigger;
	}
}

void MovementController::frozen(uint32_t time)
{
	_frozen = true;
	_frozen_duration = time;
	_input = { 0,0 };
	_tr->setDir({ 0,0 });
}

void MovementController::event_callback0(const event_system::event_receiver::Msg& m) {
	_decceleration *= m.float_value;
}