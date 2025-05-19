#pragma once
#include "../../../ecs/Component.h"

class Transform;
class dyn_image;
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
	dyn_image *_dy;

	Health* _ht;
	uint8_t _enemy_id;
	uint32_t _last_update;
	void send_enemy_update();
};
