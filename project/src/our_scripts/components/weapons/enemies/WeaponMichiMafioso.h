#pragma once

#include "../Weapon.h"

class WeaponMichiMafioso : public Weapon
{
private:
	uint32_t _time_to_shoot;
	bool _shooting = false;
	GameStructs::BulletProperties _bp;

protected:
	void callback(Vector2D shootPos, Vector2D shootDir) override;
	void delayed_shoot();

public:
	__CMPID_DECL__(ecs::cmp::WEAPON);
	WeaponMichiMafioso();
	void update(uint32_t delta_time) override;
};