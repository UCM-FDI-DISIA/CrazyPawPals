
#pragma once

#include <array>
#include <bitset>
#include <cassert>
#include <vector>

#include "Component.h"
#include "ecs.h"

namespace ecs {

/*
 * A struct that represents a collection of components.
 *
 */

struct Entity {
public:
	Entity(grpId_t gId, Manager *mngr, sceneId_t sId) : //
			_cmps(), //
			_currCmps(), //
			_alive(),  //
			_gId(gId), //
			_sId(sId)
	{
		(void)mngr;
		// We reserve the a space for the maximum number of
		// components. This way we avoid resizing the vector.
		//
		_currCmps.reserve(ecs::maxComponentId);
	}

	// we delete the copy constructor/assignment because it is
	// not clear how to copy the components
	//
	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	// Exercise: define move constructor/assignment for class Entity

	// Destroys the entity
	//
	virtual ~Entity() {

		// we delete all available components
		//
		for (auto c : _currCmps)
			delete c;
	}
	/*
	// Returns the manager to which this entity belongs
	inline Manager* getMngr() {
		return _mngr;
	}
	*/
	inline int get_currCmps_size() { return int(_currCmps.size()); }
	inline int get_scene() { return _sId; }

private:

	// We could make the constructors private as well, so only
	// the manager can create instances (because it is a friend)

	friend Manager; // so we can update these fields directly from the manager

	//Manager *_mngr;
	std::array<Component*, maxComponentId> _cmps;
	std::vector<Component*> _currCmps;
	bool _alive;
	grpId_t _gId;
	sceneId_t _sId;
};

} // end of name space
