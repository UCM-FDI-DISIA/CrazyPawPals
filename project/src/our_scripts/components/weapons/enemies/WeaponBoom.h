#pragma once

#include "../Weapon.h"

class WeaponBoom : public Weapon
{
protected:
	void callback(Vector2D shootPos, Vector2D shootDir) override;

public:
	__CMPID_DECL__(ecs::cmp::WEAPON);
	WeaponBoom();
	virtual ~WeaponBoom();
};