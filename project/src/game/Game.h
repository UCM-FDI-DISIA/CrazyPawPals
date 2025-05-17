#pragma once

#include "../utils/Singleton.h"
#include "../utils/Vector2D.h"
#include "../ecs/ecs.h"
#include "../utils/EventsSystem.hpp"
#include <vector>
#include "../network/network_context.hpp"
#include "../network/network_state.hpp"

namespace ecs {
class Manager;
}
class Scene;
class WaveManagerFacade;
class Game: public Singleton<Game>  {
public:
	enum State {
		MAINMENU,
		SELECTIONMENU,
		GAMESCENE,
		REWARDSCENE,
		MYTHICSCENE,
		UPGRADESCENE,
		GAMEOVER,
		VICTORY,
		TUTORIAL,
		MULTIPLAYERMENU,
		NUM_SCENE,
	};
	using network_users_state = network_state<network_context_maximum_connections>;
	friend Singleton<Game>;
	virtual ~Game() override;
	bool init();
	void initGame();
	void start();

	ecs::Manager* get_mngr();
	WaveManagerFacade*& get_wave_manager();
	event_system::event_manager* get_event_mngr();
	
	Scene* get_currentScene();
	void update_scene();
	void queue_scene(State);
	std::pair<int,int> get_world_half_size() const;

	//bool para salir del bucle principal
	bool exit;
	inline void set_exit(bool b) { exit = b; }
	bool get_exit() { return exit; }


private:
	WaveManagerFacade* wave_manager;
	int _next_scene_index = -1;
	int _current_scene_index = -1;
	std::vector<Scene*> _scenes;
	std::vector<bool> _scene_inits;
	std::pair<int, int> _screen_size = std::make_pair(960, 540);
	Game();
	ecs::Manager* _mngr;
	network_context network;
	network_users_state network_state;
	void set_volumes();

public:
	constexpr static const uint16_t default_port = 52224; //49152-65535
	//Network
	inline network_context& get_network() {
		return network;
	}
	inline const network_context& get_network() const {
		return network;
	}

	inline network_context_profile_status_option network_profile_status() const {
		return network.profile_status;
	}
	inline bool is_network_none() const {
		return network_profile_status() == network_context_profile_status_none;
	}
	inline bool is_host() const {
		return network_profile_status() == network_context_profile_status_host;
	}
	inline bool is_client() const {
		return network_profile_status() == network_context_profile_status_client;
	}

	inline const network_users_state &get_network_state() const {
		assert(
			!is_network_none()
			&& "error: network state is not available when network context is none"
		);
		return network_state;
	}
	inline network_users_state &get_network_state() {
		assert(
			!is_network_none()
			&& "error: network state is not available when network context is none"
		);
		return network_state;
	}

	inline uint8_t client_id() const {
		return get_network_state().connections.local_user_index;
	}
};
