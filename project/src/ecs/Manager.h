
// This file is part of the course TPV2@UCM - Samir Genaim
#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <cassert>

#include "Component.h"
#include "ecs.h"
#include "Entity.h"
#include "sdlutils/SDLUtils.h"




namespace ecs {

/*
 * A class for managing the list of entities, groups, etc.
 *
 */
class Manager {

public:
	Manager();
	virtual ~Manager();

	// Adding an entity simply creates an instance of Entity, adds
	// it to the list of entities and returns it to the caller.
	//
	inline entity_t addEntity(ecs::sceneId_t sId = ecs::scene::GAMESCENE,ecs::grpId_t gId = ecs::grp::DEFAULT) {

		// create and initialise the entity
		auto e = new Entity(gId, this, sId);
		e->_alive = true;

		// add the entity 'e' to list of entities of the given group
		//
		// IMPORTANT NOTE:
		//
		// Currently we immediately add the entity to the list of entities,
		// so we will actually see them in this 'frame' if we traverse the list of
		// entities afterwards!
		//
		// A better solution would be to add them to an auxiliary list, and
		// define a method 'flush()' that moves them from the auxiliary list
		// to the corresponding list of entities.
		//
		// We will have to call 'flush()' in each iteration of the
		// main loop. This way we guarantee that entities that are added in one
		// 'frame' they will appear only in the next 'frame' -- I leave it as an
		// exercise for you ... it could be incorporated in 'refresh' as well.
		//

		_pendingEntities.push_back(e);
		// return it to the caller
		//
		return e;
	}

	// Setting the state of the entity (alive or dead)
	//
	inline void setAlive(entity_t e, bool alive) {
		e->_alive = alive;
	}

	// Returns the state of the entity (alive o dead)
	//
	inline bool isAlive(entity_t e) {
		return e->_alive;
	}

	// Adds a component to an entity. It receives the type T (to be created),
	// and the list of arguments (if any) to be passed to the constructor.
	//
	template<typename T, typename ...Ts>
	inline T* addComponent(entity_t e, Ts &&... args) {

		// the component id
		constexpr cmpId_t cId = cmpId<T>;
		static_assert(cId < ecs::maxComponentId);

		// delete the current component, if any
		//
		removeComponent<T>(e);

		// create, initialise and install the new component
		//
		Component *c = new T(std::forward<Ts>(args)...);
		c->setContext(e);
		c->initComponent();
		e->_cmps[cId] = c;
		e->_currCmps.push_back(c);

		// return it to the user so i can be initialised if needed
		return static_cast<T*>(c);
	}

	//For one component only
	template<typename T>
	inline T* addExistingComponent(entity_t e, T* c) {
		constexpr cmpId_t cId = cmpId<T>;
		static_assert(cId < ecs::maxComponentId);
		removeComponent<T>(e);

		c->setContext(e);
		c->initComponent();
		e->_cmps[cId] = c;
		e->_currCmps.push_back(c);

		return static_cast<T*>(c);
	}
	//recursive for more than one argument
	template<typename T, typename... Args>
	inline void addExistingComponent(entity_t e, T* c, Args*... args) {
		constexpr cmpId_t cId = cmpId<T>;
		static_assert(cId < ecs::maxComponentId);
		removeComponent<T>(e);

		c->setContext(e);
		c->initComponent();
		e->_cmps[cId] = c;
		e->_currCmps.push_back(c);

		addExistingComponent(e,args...);
	}

	// Removes the component T, from the entity.
	//
	template<typename T>
	inline void removeComponent(entity_t e) {

		// the component id
		constexpr cmpId_t cId = cmpId<T>;
		static_assert(cId < ecs::maxComponentId);

		if (e->_cmps[cId] != nullptr) {

			// find the element that is equal tocmps_[cId] (returns an iterator)
			//
			auto iter = std::find(e->_currCmps.begin(), e->_currCmps.end(),
					e->_cmps[cId]);

			// must have such a component
			assert(iter != e->_currCmps.end());

			// and then remove it
			e->_currCmps.erase(iter);

			// destroy it
			//
			delete e->_cmps[cId];

			// remove the pointer
			//
			e->_cmps[cId] = nullptr;
		}
	}

	// Returns the component, of the entity, that corresponds to position T,
	// casting it to T*. The casting is done just for ease of use, to avoid casting
	// outside.
	//
	template<typename T>
	inline T* getComponent(entity_t e) {

		// the component id
		constexpr cmpId_t cId = cmpId<T>;
		static_assert(cId < ecs::maxComponentId);

		return static_cast<T*>(e->_cmps[cId]);
	}

	// return true if there is a component with identifier T::id in the entity
	//
	template<typename T>
	inline bool hasComponent(entity_t e) {

		constexpr cmpId_t cId = T::id;
		assert(cId < ecs::maxComponentId);

		return e->_cmps[cId] != nullptr;
	}

	// returns the entity's group 'gId'
	//
	inline ecs::grpId_t groupId(entity_t e) {
		return e->_gId;
	}

	// returns the vector of all entities
	//
	inline const auto& getEntities(grpId_t gId = ecs::grp::DEFAULT) {
		return _entsByGroup[gId];
	}

	inline const auto& getEntitiesByScene(sceneId_t sId = ecs::scene::GAMESCENE) {
		return _entsByScene[sId];
	}

	// associates the entity 'e' to the handler 'hId'
	//
	inline void setHandler(hdlrId_t hId, entity_t e) {
		assert(hId < ecs::maxHandlerId);
		_hdlrs[hId] = e;
	}


	// returns the entity associated to the handler 'hId'
	//
	inline entity_t getHandler(hdlrId_t hId) {
		assert(hId < ecs::maxHandlerId);
		if (_hdlrs[hId] == nullptr) {
			return nullptr;
		}
		return _hdlrs[hId];
	}

	// Updating  an entity simply calls the update of all
	// components
	//
	inline void update(entity_t e, Uint32 dt) {
		auto n = e->_currCmps.size();
		for (auto i = 0u; i < n; i++)
			e->_currCmps[i]->update(dt);
	}

	// Rendering an entity simply calls the render of all
	// components
	//
	inline void render(entity_t e) {
		auto n = e->_currCmps.size();
		for (auto i = 0u; i < n; i++)
			e->_currCmps[i]->render();
	}

	// update all entities in a certain Scene (Group)
	//
	void update(sceneId_t sId,Uint32 dt);
	// render all entities in a certain Scene (Group)
	//
	void render(sceneId_t sId);

	// render all entities
	//
	void render() {
	
		for (auto &ents : _entsByGroup) {
			auto n = ents.size();
			for (auto i = 0u; i < n; i++)
				render(ents[i]);
		}
	}
	// update all entities
	//
	void update(Uint32 dt) {
		for (auto& ents : _entsByGroup) {
			auto n = ents.size();
			for (auto i = 0u; i < n; i++)
				update(ents[i], dt);
		}
	}

	inline void change_ent_scene(entity_t e, ecs::sceneId_t new_scene_id) {
		//Quitar de escena actual
		
		//anyadir a la cola de otra escena
		e->_sId = new_scene_id;
		_pendingEntities.push_back(e);
	}

	// eliminate dead entities (the implementation of this method
	// is in Manager.cpp, but we could also defined it here).
	//
	void refresh();


private:

	std::array<entity_t, maxHandlerId> _hdlrs;
	std::array<std::vector<entity_t>, maxSceneId> _entsByScene;
	std::array<std::vector<entity_t>, maxGroupId> _entsByGroup;

	std::vector<entity_t> _pendingEntities;
	Uint32 _last_frame = 0;
};

} // end of namespace
