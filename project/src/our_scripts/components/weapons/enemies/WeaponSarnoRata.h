#pragma once

#include "../Weapon.h"

class WeaponSarnoRata : public Weapon {
protected:
	uint32_t _time_to_shoot;
	void callback(Vector2D shootPos, Vector2D shootDir) override;
	void update(uint32_t delta_time);
	bool _shooting = false;
	GameStructs::BulletProperties _bp;
	void delayed_shoot();
public:
	__CMPID_DECL__(ecs::cmp::WEAPON);
	WeaponSarnoRata();
	virtual ~WeaponSarnoRata();
};