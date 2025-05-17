#pragma once

#include "WaveManager.h"
#include "../../ecs/Component.h"
#include "../wave_events/wave_event.hpp"

class DumbWaveManager : /*public event_system::event_receiver,*/ public ecs::Component {
	uint32_t change_to_rewards_time;
	uint32_t _currentWaveTime = 0; //tiempo actual (post calculo, inicial en constructor)
	uint32_t _currentWaveInitTime; // cuándo empezó la oleada
	uint8_t _currentWave = 0;
	std::unique_ptr<wave_event> _current_wave_event;
	bool _wave_active;
private:
	void erase_all_bullets();
	void erase_all_enemies();
public:
	__CMPID_DECL__(ecs::cmp::DUMB_WAVE_MANAGER)
	//void initComponent() override;
	void start_wave();
	void end_wave();
	void update(uint32_t dt) override;
};