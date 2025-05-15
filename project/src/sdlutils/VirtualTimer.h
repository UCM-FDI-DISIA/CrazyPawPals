
#pragma once


#include <SDL.h>
#include <cassert>


/*
 * This class implements a virtual timer, i.e., a timer that can be paused and
 * resumed.
 */

struct VirtualTimer {
	inline void resetTime() {
		current_time = 0;
	}

	inline Uint32 currRealTime() const {
		return uint32_t(current_time);
	}

	inline Uint32 currTime() const {
		return uint32_t(current_time);
	}

	uint64_t current_time;
};

