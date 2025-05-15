#pragma once

#include "../Weapon.h"

class WeaponCatKuza : public Weapon {
protected:
	void callback(Vector2D shootPos, Vector2D shootDir) override;
public:
	__CMPID_DECL__(ecs::cmp::WEAPON);
	WeaponCatKuza();
	virtual ~WeaponCatKuza();
	void wind_attack(Vector2D shootPos);
	void dash_attack(Vector2D shootPos, Vector2D shootDir);
	void set_player_pos(Vector2D);

	void area_attack(Vector2D shootPos);
private:
	Vector2D _player_pos;
	int _wind_p, _dash_p, _area_p;
};