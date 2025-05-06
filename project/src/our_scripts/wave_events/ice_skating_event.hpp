#pragma once

#include "../components/WaveManager.h"
#include "../../utils/EventsSystem.hpp"
class ice_skating_event : public wave_event {
private:
	float deccel_multiplier = 0.1f;
public:
	inline ice_skating_event(WaveManager* wav)
		:wave_event(wav)
	{
	}

	inline void start_wave_callback() override {
		wave_event::start_wave_callback();
		event_system::event_manager::Instance()->fire_event(event_system::change_deccel, (event_system::event_receiver::Msg{deccel_multiplier}));
	}
	inline void end_wave_callback() override {
		//TODO: send event to revert
		event_system::event_manager::Instance()->fire_event(event_system::change_deccel, (event_system::event_receiver::Msg{ 1/deccel_multiplier }));
	}
};