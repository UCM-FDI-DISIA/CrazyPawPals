
#include "../../ecs/Component.h"
#include "../../ecs/ecs.h"

struct player_collision_triggerer : public ecs::Component {
	__CMPID_DECL__(ecs::cmp::PLAYER_TRIGGERER);
	player_collision_triggerer()
		:Component() {}
};

struct enemy_collision_triggerer : public ecs::Component {
	__CMPID_DECL__(ecs::cmp::ENEMY_TRIGGERER);
};