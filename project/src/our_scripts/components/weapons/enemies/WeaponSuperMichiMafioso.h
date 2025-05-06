#pragma once

#include "../Weapon.h"
#include "../../movement/Follow.h"
class Transform;
class WeaponSuperMichiMafioso : public Weapon {
protected:
	void callback(Vector2D shootPos, Vector2D shootDir) override {};

public:
	__CMPID_DECL__(ecs::cmp::WEAPON);
	WeaponSuperMichiMafioso();
	virtual ~WeaponSuperMichiMafioso();
	void set_player_pos(Vector2D _pl);
	void attack1();
	void attack2(Vector2D shootPos);
	void attack3(Vector2D shootPos);
	void generate_michi_mafioso();
private:
	Vector2D _last_shootPos;
	Vector2D _player_pos;
	bool _warning;
};