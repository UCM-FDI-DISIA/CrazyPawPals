#pragma once
#include "../../../ecs/Component.h"

class Transform;
class EnemySynchronize :public ecs::Component
{
public:
	__CMPID_DECL__(ecs::cmp::SYNCHRONIZE);
	EnemySynchronize();
	~EnemySynchronize();
	void initComponent() override;
	void update(uint32_t delta_time) override;
protected:
	Transform* _tr;
};
