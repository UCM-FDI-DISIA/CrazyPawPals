#pragma once

#include "../../ecs/Component.h"

class BulletData :public ecs::Component
{
public:
	__CMPID_DECL__(ecs::cmp::BULLETDATA);

	BulletData(int damage, GameStructs::WeaponType weapon_type): _damage(damage), _weapon_type(weapon_type){};
	virtual ~BulletData() {};
	inline int damage() const { return _damage; }
	inline GameStructs::WeaponType weapon_type() const { return _weapon_type; }
protected:
	int _damage;
	GameStructs::WeaponType _weapon_type;
};
