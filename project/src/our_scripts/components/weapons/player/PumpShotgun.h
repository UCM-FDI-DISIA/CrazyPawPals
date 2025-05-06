#pragma once

#include "../Weapon.h"
#include "../../../../utils/EventsSystem.hpp"
#include "../../../card_system/ShootPatrons.hpp"
class PumpShotgun : public event_system::event_receiver, public Weapon {
protected:
	void callback(Vector2D shootPos, Vector2D shootDir);// override;
	bool _has_mill;
public:
	__CMPID_DECL__(ecs::cmp::WEAPON)
	PumpShotgun();
	void event_callback0(const event_system::event_receiver::Msg& m) override;
	~PumpShotgun() override;
};