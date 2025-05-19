#pragma once

#include "../../ecs/Component.h"

class LifetimeTimer : public ecs::Component {
	float _lifetime;
public:
	__CMPID_DECL__(ecs::cmp::LIFETIMETIMER);
	LifetimeTimer(float lifetime);
	virtual ~LifetimeTimer();
	void update(uint32_t delta_time) override;
};
