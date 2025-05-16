#pragma once

#include "../../../ecs/Component.h"
#include "../Health.h"
#include "../rendering/camera_component.hpp"

class MultiplayerHUD : public ecs::Component {
private:
	const camera_component* _camera;
	uint32_t _player_id;
	int _num_player_connected;
	void drawIcon(const std::string& texture, float x, float y);
	void drawHealthBar(float x, float y, int health, int maxHealth, int shield);
	void drawText(const std::string& text, float x, float y);
public:
	__CMPID_DECL__(ecs::cmp::MULTIPLAYER_HUD)
	inline MultiplayerHUD() = default;
	virtual inline ~MultiplayerHUD() = default;
	void initComponent() override;
	void render() override;
};