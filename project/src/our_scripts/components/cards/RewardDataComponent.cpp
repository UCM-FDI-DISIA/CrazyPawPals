
#include "RewardDataComponent.h"

RewardDataComponent::RewardDataComponent() : _ct(GameStructs::LAST_CARD), _data("") {}

RewardDataComponent::RewardDataComponent(const std::string& s) : _data(s), _ct(GameStructs::LAST_CARD) {}

RewardDataComponent::~RewardDataComponent() {}

void 
RewardDataComponent::set_data(const std::string& s, GameStructs::CardType ct) {
	_data = s;
	_ct = ct;
}