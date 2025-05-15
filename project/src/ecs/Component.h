
#pragma once


#include "ecs.h"
#include "../game/GameStructs.h"

namespace ecs {

/*
 * Classes that inherit from Component must define a field
 *
 *   	constexpr static ecs::cmpId_type id = value;
 *
 * where value is from the enum ecs::cmpId (see ecs.h). This
 * how we assign numeric identifiers to components (so we can
 * easily put them in an array). The list of possible identifiers
 * is defined as an enum in ecs.h
 *
 */
class Component {
public:
	Component() :
			_ent() //
	{
	}

	// Destroys the component.
	// Careful! ent_ and mngr_ should not be destroyed
	//
	virtual ~Component() {
	}

	// This method is used to set the context, which is a
	// reference to the entity in which the component is
	// installed. It will be called by Manager when adding
	// a component.
	//
	inline void setContext(Entity *ent) {
		_ent = ent;
	}

	// We assume that initComponent will be called when adding a
	// component to an entity, immediately after setContext.
	//
	virtual void initComponent() {
	}

	// A component has an update method, and by default it
	// does nothing -- some components will not override this
	// method.
	// delta_time represents the milliseconds elapsed since
	// the last frame. To be used in time-sensitive calculations.
	// 
	virtual void update(uint32_t delta_time) {
		(void)delta_time;
	}

	// A component has a render method, and by default it
	// does nothing -- some components will not override this
	// method.
	//
	virtual void render() {
	}

protected: // we allow direct use these fields from subclasses

	Entity *_ent; // a pointer to the entity, should not be deleted on destruction
};

} // end of namespace
