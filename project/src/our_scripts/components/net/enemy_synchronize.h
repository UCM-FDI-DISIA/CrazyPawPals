#pragma once
#include "../../../ecs/Component.h"

class Transform;
class Health;
class EnemySynchronize :public ecs::Component
{
public:
	__CMPID_DECL__(ecs::cmp::SYNCHRONIZE);
	EnemySynchronize();
	~EnemySynchronize();
	void initComponent() override;
	void update(uint32_t delta_time) override;

	void update_enemy(GameStructs::DumbEnemyProperties& data);

protected:
	Transform* _tr;
	Health* _ht;
	uint8_t _enemy_id;

	void send_enemy_update();
};
