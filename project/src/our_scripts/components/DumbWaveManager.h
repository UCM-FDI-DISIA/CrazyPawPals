#pragma once

#include "WaveManager.h"
#include "../../ecs/Component.h"
#include "../wave_events/wave_event.hpp"
#include "WaveManagerFacade.h"

class DumbWaveManager : /*public event_system::event_receiver,*/ public ecs::Component, public WaveManagerFacade {
	uint32_t change_to_rewards_time= 0;
	uint32_t _currentWaveTime = 0; //tiempo actual (post calculo, inicial en constructor)
	uint32_t _currentWaveInitTime; // cuándo empez?la oleada
	uint8_t _currentWave = 0;
	std::unique_ptr<wave_event> _current_wave_event;
	bool _wave_active = true;
	bool _has_changed_to_rewards = false;
	events _current_event = NONE;
private:
	void erase_all_bullets();
	void erase_all_enemies();
public:
	DumbWaveManager();
	__CMPID_DECL__(ecs::cmp::DUMB_WAVE_MANAGER)
	//void initComponent() override;
	void start_wave(uint16_t);
	void end_wave();
	void update(uint32_t dt) override;
	events get_current_event() override { return _current_event; }
	int get_current_wave() override { return _currentWave; }
	bool wave_completed() override { return !_wave_active; }
	uint32_t get_wave_time() override { return _currentWaveTime; }
	void reset_wave_time() override { _currentWaveTime = 0; }
	void choose_new_event(events);
};