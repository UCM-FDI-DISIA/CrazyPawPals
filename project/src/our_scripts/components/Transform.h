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
#include "../../ecs/Component.h"
#include <cassert>
#include <iostream>

class Transform: public ecs::Component {
public:

	__CMPID_DECL__(ecs::cmp::TRANSFORM)

	Transform() :
			_pos(), _dir(), _rot(), _width(), _height() {
	}

	Transform(Vector2D pos, Vector2D dir, float r, float s) :
			_pos(pos), _dir(dir), _rot(r), _width(), _height() {
		setSpeed(s);
	}
	Transform(Vector2D pos, Vector2D dir, float r, float s, int w, int h) :
		_pos(pos), _dir(dir), _rot(r), _width(w), _height(h) {
		setSpeed(s);
	}

	virtual ~Transform() {
	}

	Vector2D& getPos() {
		return _pos;
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
		_pos=p;
	}
	void setDir(Vector2D& d) {
		_dir = d;
	}
	void setPos(Vector2D&& p) {
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
		//Movement
		//std::cout << _pos << " + " << _dir << std::endl;
		_pos = _pos + _dir;
		//std::cout << "=>" << _pos << std::endl;
	}

private:
	Vector2D _pos;
	Vector2D _dir;
	float _rot;
	int _width;
	int _height;
};

