#pragma once
#include "../../../ecs/Component.h"

class Transform;
class Health;
class AnimationComponent;
class PlayerSynchronize :public ecs::Component
{
public:
	__CMPID_DECL__(ecs::cmp::SYNCHRONIZE);
	PlayerSynchronize(uint32_t _player_id);
	~PlayerSynchronize();
	void initComponent() override;
	void update(uint32_t delta_time) override;
	void updatePlayer(GameStructs::NetPlayerData& data);

protected:
	void sendPlayerUpdate();
	uint32_t _player_id;
	Transform* _tr;
	Health* _health;
	AnimationComponent* _anim;
	std::string _tex_name;
	bool _is_ghost;

	bool _is_local_player = false;
};
