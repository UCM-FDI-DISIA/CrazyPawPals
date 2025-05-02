#include "RotationComponent.h"
#include "../../../ecs/Manager.h"
#include "../../../game/Game.h"
#include "../movement/Transform.h"
#include "dyn_image.hpp"
#include "../../../sdlutils/SDLUtils.h"
#include <iostream>

RotationComponent::RotationComponent(
	Mode mode,
	float speed,
	float max_rotation,
	uint32_t flip_time,
	uint32_t toggle_interval,
	uint32_t active_time,
	uint32_t pause_time,
	bool can_flip)
	: _tr(nullptr),
	  _mode(mode),
	  _speed(speed),
	  _max_rotation(max_rotation),
	  _toggle_interval(toggle_interval),
	  _active_time(active_time),
	  _pause_time(pause_time),
	  _flip_time(flip_time),
	  _rotating(true),
	  _reverse(false),
	  _accumulated_rotation(0.0f),
	  _last_toggle_time(0),
	  _last_cycle_time(0),
	  _last_flip_time(0),
	  _can_flip(can_flip)
{
}

void RotationComponent::initComponent()
{
	_tr = Game::Instance()->get_mngr()->getComponent<Transform>(_ent);
	assert(_tr != nullptr);
	if (_can_flip)
	{
		_dy = Game::Instance()->get_mngr()->getComponent<dyn_image>(_ent);
		assert(_dy != nullptr);
	}

	// Initialize the rotation state
	_last_toggle_time = sdlutils().currRealTime();
	_last_cycle_time = _last_toggle_time;
	_last_flip_time = _last_toggle_time;
}

void RotationComponent::update(uint32_t delta_time)
{
	(void)delta_time; // Unused parameter
	uint32_t now = sdlutils().currRealTime();

	switch (_mode)
	{
	case Mode::CONTINUOUS:
		if (_rotating)
		{
			float delta_angle = (_reverse ? -_speed : _speed);
			_tr->setRot(_tr->getRot() + delta_angle);
		}
		break;

	case Mode::LIMITED_ANGLE:
		if (_rotating)
		{
			float delta_angle = (_reverse ? -_speed : _speed);
			_tr->setRot(_tr->getRot() + delta_angle);
			_accumulated_rotation += std::abs(delta_angle);
			if (_accumulated_rotation >= _max_rotation)
			{
				_rotating = false;
			}
		}
		break;

	case Mode::TOGGLE:
		if (now - _last_toggle_time >= _toggle_interval)
		{
			_reverse = !_reverse;
			_last_toggle_time = now;
		}
		_tr->setRot(_tr->getRot() + (_reverse ? -_speed : _speed));
		break;

	case Mode::CYCLE:
	{
		uint32_t elapsed = now - _last_cycle_time;
		if (_rotating && elapsed >= _active_time)
		{
			_rotating = false;
			_last_cycle_time = now;
		}
		else if (!_rotating && elapsed >= _pause_time)
		{
			_rotating = true;
			_last_cycle_time = now;
		}

		if (_rotating)
		{
			_tr->setRot(_tr->getRot() + (_reverse ? -_speed : _speed));
		}
	}
	break;
	}

	if (_can_flip && _dy && now - _last_flip_time >= _flip_time)
	{
		_dy->flip = (SDL_FLIP_HORIZONTAL == _dy->flip) ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL;
		_last_flip_time = now;
	}
}

void RotationComponent::setInitialRotation(float angle) { _tr->setRot(angle); }
