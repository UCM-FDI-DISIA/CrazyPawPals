
#pragma once

#include "../../../ecs/Component.h"
#include "../cards/Mana.h"
#include "../Health.h"
#include "../cards/Deck.hpp"
#include "../movement/Transform.h"
#include "../rendering/camera_component.hpp"
#include "../WaveManagerFacade.h"

class HUD : public ecs::Component {
private:
	WaveManagerFacade* _wm;
	const camera_component* _camera;
	std::vector<Texture*> _event_textures;

	events _current_event;

	const int _wave_event_timeout = 3000;
	int _current_wave_event_time = 0;
	bool _displaying_wave_event = false;
public:
	__CMPID_DECL__(ecs::cmp::GENERAL_HUD);
	HUD();
	virtual ~HUD();
	void initComponent() override;
	void update(uint32_t delta_time) override;
	void render() override;
	void start_new_wave();
};