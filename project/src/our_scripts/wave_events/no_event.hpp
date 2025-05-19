
#include "wave_event.hpp"

class no_event : public wave_event {
public:
	void start_wave_callback() override {};
	void update(unsigned int) override {};
	void end_wave_callback() override {};
};