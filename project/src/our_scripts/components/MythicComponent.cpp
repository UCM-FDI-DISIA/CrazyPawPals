
#include "MythicComponent.h"
#include "../../game/Game.h"
#include "../../ecs/Manager.h"
#include "../mythic/MythicItem.h"
#include "../mythic/MythicItems.h"

MythicComponent::MythicComponent() {
}


MythicComponent::~MythicComponent(){
	reset();
}

void 
MythicComponent::initComponent() {
}

void 
MythicComponent::add_mythic(MythicItem* mythic) {
	 if (mythic == nullptr) return;
    _obtained_mythics.push_back(mythic);
    mythic->apply_effects();
}

void MythicComponent::reset()
{
	for (auto mythic : _obtained_mythics) {
		delete mythic;
		mythic = nullptr;
	}
	_obtained_mythics.clear();
}

void 
MythicComponent::update(uint32_t dt) {
	for (auto m : _obtained_mythics) {
		m->update(dt);
	}
}

