#pragma once
#include "../utils/Singleton.h"
#include "../utils/Vector2D.h"
#include "../ecs/ecs.h"
#include "../utils/EventsSystem.hpp"
#include <vector>

namespace ecs {
class Manager;
}
class Scene;
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
		NUM_SCENE,
	};
	friend Singleton<Game>;
	virtual ~Game();
	bool init();
	void initGame();
	void start();

	ecs::Manager* get_mngr();
	event_system::event_manager* get_event_mngr();
	
	Scene* get_currentScene();
	void change_Scene(State);
	std::pair<int,int> get_world_half_size() const;

	//bool para salir del bucle principal
	bool exit;
	inline void set_exit(bool b) { exit = b; }
	bool get_exit() { return exit; }


private:
	int _current_scene_index = -1;
	std::vector<Scene*> _scenes;
	std::vector<bool> _scene_inits;
	std::pair<int, int> _screen_size = std::make_pair(960,540);
	Game();
	ecs::Manager* _mngr;
	void set_volumes();
	void create_camera();
};
