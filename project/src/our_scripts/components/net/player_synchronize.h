#pragma once
#include "../../../ecs/Component.h"

class Transform;
class Health;
class PlayerSynchronize :public ecs::Component
{
public:
	__CMPID_DECL__(ecs::cmp::SYNCHRONIZE);
	PlayerSynchronize();
	~PlayerSynchronize();
	void initComponent() override;
	void update(uint32_t delta_time) override;

protected:
	void sendPlayerUpdate();
	uint8_t _player_id;
	Transform* _tr;
	Health* _health;
	std::string _tex_name;
	bool _is_ghost;
};
