#pragma once
#include "../../ecs/Component.h"

class Transform;
class EnemySynchronize :public ecs::Component
{
public:
	__CMPID_DECL__(ecs::cmp::ENEMY_SYNCHRONIZE);
	EnemySynchronize();
	
protected:
	Transform* _tr;
};
