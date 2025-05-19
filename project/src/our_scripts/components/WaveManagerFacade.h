#pragma once

#include <cstdint>
enum events : int16_t;

class WaveManagerFacade {
public:
	virtual bool wave_completed() = 0;
	virtual events get_current_event() = 0;
	virtual int get_current_wave() = 0;
	virtual uint32_t get_wave_time() = 0;
	virtual void reset_wave_time() = 0;
};