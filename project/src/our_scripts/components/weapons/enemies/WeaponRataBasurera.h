#pragma once

#include "../Weapon.h"
#include "../../Health.h"
#include "../../WaveManager.h"

class WeaponRataBasurera : public Weapon
{
protected:
	Health *_health;
	void callback(Vector2D shootPos, Vector2D shootDir) override;

public:
	void sendHealthComponent(Health *health);
	__CMPID_DECL__(ecs::cmp::WEAPON);
	WeaponRataBasurera();
	virtual ~WeaponRataBasurera();
};