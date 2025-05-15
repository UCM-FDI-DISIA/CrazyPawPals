
#pragma once


#include <cassert>
#include <cmath>
#include <ostream>
#include <utility>

/*
 * A class implementing a 2-dimensional vector and corresponding
 * operations. All operations generate new Vector2D, they do not
 * modify the vectors on which they operate. The only methods that
 * modify the state are the different setters (and operator=).
 *
 */
class Vector2D {
public:

	// various constructors
	constexpr Vector2D() noexcept :
			_x(), _y() {
	}
	constexpr Vector2D(float x, float y) noexcept :
			_x(x), _y(y) {
	}

	// various getters
	inline float getX() const {
		return _x;
	}

	inline float getY() const {
		return _y;
	}

	// various setters
	inline void setX(float x) {
		_x = x;
	}

	inline void setY(float y) {
		_y = y;
	}

	inline void set(float x, float y) {
		_x = x;
		_y = y;
	}

	inline void set(const Vector2D &v) {
		_x = v._x;
		_y = v._y;
	}
	
	// v[0] is the first coordinate and v[1] is the second
	inline float& operator[](int i) noexcept {
		assert(i == 0 || i == 1);
		return i == 0 ? _x : _y;
	}

	// v[0] is the first coordinate and v[1] is the second
	inline const float& operator[](int i) const noexcept {
		assert(i == 0 || i == 1);
		return i == 0 ? _x : _y;
	}

	inline bool operator==(const Vector2D& v) const {
		return std::equal_to<float>()(_x, v._x)
			&& std::equal_to<float>()(_y, v._y);
	}

	inline bool operator!=(const Vector2D& v) const {
		return !(*this == v);
	}
	// ** various operations

	// length of the vector
	inline float magnitude() const {
		return sqrtf(powf(_x, 2) + powf(_y, 2));
	}

	// vector in the same direction of length 1
	inline Vector2D normalize() const {
		constexpr static const Vector2D zero = {0, 0};
		if (*this == zero)
			return *this;
		else
			return *this / magnitude();
	}

	// counter clockwise rotation in a normal coordinate system, and
	// it is clockwise rotation if we work with a coordinate system
	// in which the vertical axe is flipped (it is like a mirror over
	// the horizontal axe) -- which the case when working with SDL.
	//
	Vector2D rotate(float degrees) const;

	// Computes the angle between 'this' and 'v'. The result is
	// between -180 and 180, and is such that the following holds:
	//
	//   this->rotate(angle).normalize() == v.normalize()
	//
	float angle(const Vector2D &v) const;

	// vector subtraction
	inline Vector2D operator-(const Vector2D &v) const {
		return Vector2D(_x - v._x, _y - v._y);
	}

	// vector addition
	inline Vector2D operator+(const Vector2D &v) const {
		return Vector2D(_x + v._x, _y + v._y);
	}

	// vector +=
	inline void operator+=(const Vector2D& v){
		_x = _x + v._x;
		_y = _y + v._y;
	}

	// multiplication by constant (scaling)
	inline Vector2D operator*(float d) const {
		return Vector2D(_x * d, _y * d);
	}

	// division by constant (scaling)
	inline Vector2D operator/(float d) const {
		return Vector2D(_x / d, _y / d);
	}

	// scalar multiplication
	inline float operator *(const Vector2D &d) const {
		return d._x * _x + d._y * _y;
	}

	inline bool inRange(float x0, float x2, float y0, float y1) {
		return _x >= x0 && _x <= x2 && _y >= y0 && _y <= y1;
	}
private:
	float _x;  // first coordinate
	float _y;  // second coordinate
};

// needed for printing a value of type Vector2D with std::cout.
// The definition is in .cpp
std::ostream& operator<<(std::ostream &os, const Vector2D &v);
