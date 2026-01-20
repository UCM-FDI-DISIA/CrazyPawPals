// This file is part of the course TPV2@UCM - Samir Genaim
#include "../../ecs/ecs.h"

#pragma once

class State {
public:
	virtual ~State() {
	}
	virtual void enter() = 0;
	virtual void update(uint32_t delta_time) = 0;
	virtual void exit() = 0;

	virtual void setEntity(ecs::entity_t ent) {
		_ent = ent;
	}

protected:
	ecs::entity_t _ent; // Puntero a la entidad
};

