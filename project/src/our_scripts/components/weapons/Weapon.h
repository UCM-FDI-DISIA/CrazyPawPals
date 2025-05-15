#pragma once

//this is the abstract class of player's weapons
#include "../../../ecs/Component.h"
#include <vector>
#include <string>
#include <cmath>
#include "../../../utils/Vector2D.h"

class Bullet;
class Transform;
class Weapon : public ecs::Component {
protected:
	int _damage; //weapon's damage
	float _cooldown; //cooldown
	float _distance; //the distance to reach
	float _speed; //attack speed
	std::string _tex; //sprite
	float _attack_width; //sizes
	float _attack_height;
	float _lastShoot;
	Transform* _tr; //transform component

	virtual void callback(Vector2D shootPos, Vector2D shootDir) = 0; //when you attacks, this callback will be called
public:
	__CMPID_DECL__(ecs::cmp::WEAPON);
	Weapon(int d, float cd, float dis, float s, const std::string& t, float w, float h);
	virtual ~Weapon();
	virtual void initComponent() override;
	//NECCESARY, ALL TYPE OF WEAPON NEED TO CALL THIS METHOD TO INITIALIZE ATTRIBUTES
	//void init(int d, float cd, float dis, float s, const std::string& t);
	//not neccesary (50x50 as default value)
	void set_attack_size(float w, float h);
	bool shoot(const Vector2D& target);
	bool shoot(float shootDirectionX, float shootDirectionY);
	inline int damage() { return _damage; };
	inline void set_damage(int damage) { _damage = damage; };
	inline float cooldown() { return _cooldown; };
	inline void set_cooldown(float cooldown) { _cooldown = cooldown; };
	inline float distance() { return _distance; };
};