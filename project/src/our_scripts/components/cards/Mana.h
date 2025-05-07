#pragma once

#include "../../../sdlutils/Texture.h"
#include <algorithm>
#include "../../../ecs/Component.h" 

#include "SDL.h"
#include "SDL_image.h"
#include "../../../sdlutils/SDLUtils.h"
#ifdef GENERATE_LOG
#include "../../components/WaveManager.h"
#endif
class ManaComponent:public ecs::Component {
private:
	uint16_t _max_mana; // Maximum amount of mana that can be stored.
	int16_t _mana_regen; // Mana gained per second.
	uint16_t _mana_count; // Current amount of mana.
#ifdef GENERATE_LOG
	uint16_t _mana_mid = 0;
	uint16_t _mana_max = 0;
#endif
public:
	__CMPID_DECL__(ecs::cmp::MANA);
	ManaComponent(int max = 5000, int regen = 750) : _max_mana(max), _mana_regen(regen), _mana_count(0) {}
	void initComponent() override { _mana_count = 0; }
	void update(Uint32 delta_time) override {
		_mana_count = std::min(std::max(uint16_t(_mana_count + int(_mana_regen*delta_time)/1000),(uint16_t)0), _max_mana);
#ifdef GENERATE_LOG
		_mana_max = std::max(_mana_count, _mana_max);
		auto n_ticks = WaveManager::get_ticks_on_wave();
		_mana_mid = _mana_mid*((n_ticks-1)/(float)n_ticks) + _mana_count/n_ticks;
#endif
	}
#ifdef GENERATE_LOG
	inline void reset_mana_mid() {
		_mana_mid = 0;
		_mana_max = 0;
	}
	inline uint32_t get_mana_mid() {
		return _mana_mid;
	}
	inline uint32_t get_mana_max() {
		return _mana_max;
	}
#endif
	
	// Changes current mana by a given amount. Can be positive or negative, and cannot be lower than 0 or greater than _max_mana.
	void change_mana(float delta) { _mana_count = std::max(uint16_t(0), std::min(uint16_t(_mana_count + (delta*1000)), _max_mana)); }
	// Changes mana regeneration by a given amount. Can be positive or negative.
	void change_mana_regen(int delta) { _mana_regen += delta; }
	void set_mana_regen(int regen) { _mana_regen = regen; }
	// Changes maximum mana by a given amount. Can be positive or negative.
	void change_max_mana(int delta) { _max_mana += delta * 1000; }
	// Returns your current mana.
	int mana_count() const { return _mana_count/1000.0f; }
	int mana_regen() const { return  _mana_regen; }

	inline int raw_mana_count() const { return _mana_count; }
	inline int max_mana() const { return _max_mana; }

};
