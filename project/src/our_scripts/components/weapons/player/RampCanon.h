#pragma once

#include "../Weapon.h"
class ManaComponent;
class RampCanon : public Weapon {
private:
	ManaComponent* _mana;
protected:
	void callback(Vector2D shootPos, Vector2D shootDir) override;
public:
	__CMPID_DECL__(ecs::cmp::WEAPON);
	RampCanon();
	virtual ~RampCanon();

};