#include "../../ecs/Component.h"
#include "../../ecs/ecs.h"
#include "../../utils/checkML.h"
#include <bitset>;

struct collision_registration_by_id : public ecs::Component {
	__CMPID_DECL__(ecs::cmp::COLLISION_REGISTRATION_BY_ID);
	std::bitset<256> already_collided_with = std::bitset<256>();
};