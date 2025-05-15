
#include "MythicDataComponent.h"

MythicDataComponent::MythicDataComponent() : _mt(GameStructs::LAST_MYTHIC), _data("") {}

MythicDataComponent::MythicDataComponent(const std::string& s) : _data(s), _mt(GameStructs::LAST_MYTHIC) {}

MythicDataComponent::~MythicDataComponent() {}

void
MythicDataComponent::set_data(const std::string& s, GameStructs::MythicType mt) {
	_data = s;
	_mt = mt;
}