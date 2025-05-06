/*
* ELEMENTS:
* -Position
* -Direction
* -Width
* -Height
* -Rotation
* -Speed (float)
*/
#pragma once

#include "../../../ecs/Component.h"
#include <cassert>
#include <iostream>

class Transform: public ecs::Component {
public:

	__CMPID_DECL__(ecs::cmp::TRANSFORM)

	Transform() :
			_pos(), previous_position(), _dir(), _rot(), _width(), _height() {
	}

	Transform(Vector2D pos, Vector2D dir, float r, float s) :
			_pos(pos), previous_position(pos), _dir(dir), _rot(r), _width(), _height() {
		setSpeed(s);
	}
	Transform(Vector2D pos, Vector2D dir, float r, float s, int w, int h) :
		_pos(pos), previous_position(pos), _dir(dir), _rot(r), _width(w), _height(h) {
		setSpeed(s);
	}

	virtual ~Transform() override {
	}

	Vector2D& getPos() {
		return _pos;
	}
	Vector2D get_previous_position() {
		return previous_position;
	}
	Vector2D& getDir() {
		return _dir;
	}
	
	const Vector2D& getPos() const {
		return _pos;
	}
	const Vector2D& getDir() const {
		return _dir;
	}


	void add_directional_speed(Vector2D extra_speed) {
		_dir = _dir + extra_speed;
	}

	void setPos(Vector2D& p) {
		previous_position = _pos;
		_pos=p;
	}
	void setDir(Vector2D& d) {
		_dir = d;
	}
	void setPos(Vector2D&& p) {
		previous_position = _pos;
		_pos = p;
	}
	void setDir(Vector2D&& d) {
		_dir = d;
	}


	float getRot() const {
		return _rot;
	}


	void setRot(float r) {
		_rot = r;
	}
	
	float getSpeed() {
		return _dir.magnitude();
	}

	void setSpeed(float s) {
		_dir = _dir.normalize() * s;
	}

	inline int getWidth() const { return _width; }
	inline int getHeight() const { return _height; }
	inline void setWidth(int w) { _width = w; }
	inline void setHeight(int h) { _height = h; }

	void update(uint32_t delta_time) override {
		(void)delta_time;
		previous_position = _pos;
		// const float delta_time_seconds = (delta_time / 1000.0f);
		// _pos = _pos + _dir * delta_time_seconds;
		_pos = _pos + _dir;
	}

private:
	Vector2D _pos;
	Vector2D previous_position;
	Vector2D _dir;
	float _rot;
	int _width;
	int _height;
};

