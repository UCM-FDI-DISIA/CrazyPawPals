#pragma once




class WaveManager;

class wave_event {
public:
	virtual void start_wave_callback();
	virtual void end_wave_callback();
	virtual void update(unsigned int delta_time);
};