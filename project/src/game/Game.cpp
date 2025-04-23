// This file is part of the course TPV2@UCM - Samir Genaim
#include "Game.h"

#include "../ecs/Manager.h"
#include "../sdlutils/InputHandler.h"
#include "../sdlutils/SDLUtils.h"
#include "../utils/Vector2D.h"
#include "../utils/Collisions.h"

#include "../our_scripts/components/rendering/Image.h"
#include "../our_scripts/components/movement/Transform.h"
#include "../our_scripts/components/KeyboardPlayerCtrl.h"
#include "../our_scripts/components/movement/MovementController.h"
#include "../our_scripts/components/cards/Mana.h"

#include "../our_scripts/components/cards/Deck.hpp"
#include "../our_scripts/components/rendering/dyn_image.hpp"
#include "../our_scripts/components/rendering/camera_component.hpp"
#include "../our_scripts/components/rendering/rect_component.hpp"
#include "../our_scripts/components/weapons/player/Revolver.h"
#include "../our_scripts/components/weapons/player/Rampage.h"
//Scenes for SceneManager
#include "scenes/Scene.h"
#include "scenes/MainMenuScene.h"
#include "scenes/ControlsScene.h"
#include "scenes/SelectionMenuScene.h"
#include "scenes/GameScene.h"
#include "scenes/GameOverScene.h"
#include "scenes/RewardScene.h"
#include "scenes/TutorialScene.h"
#include "scenes/MultiplayerMenu.h"
#include "scenes/VictoryScene.h"



#ifdef GENERATE_LOG
#include "../our_scripts/log_writer_to_csv.hpp"
#endif


using namespace std;

Game::Game() : _mngr(nullptr){}

Game::~Game() {

	for (auto scene : _scenes) {
		if (scene != nullptr) {
			delete scene;
		}
	}
	_scenes.clear();

	// release InputHandler if the instance was created correctly.
	if (InputHandler::HasInstance())
		InputHandler::Release();

	// release SLDUtil if the instance was created correctly.
	if (SDLUtils::HasInstance())
		SDLUtils::Release();
#ifdef GENERATE_LOG
	if (log_writer_to_csv::HasInstance())
		log_writer_to_csv::Release();
#endif

}

bool Game::init() {
	
	// initialize the SDL singleton
	if (!SDLUtils::Init("crazy paw pals", _screen_size.first, _screen_size.second,
		"resources/config/crazypawpals.resources.json")) {
		
		std::cerr << "Something went wrong while initializing SDLUtils"
		<< std::endl;
		return false;
	}

	// initialize the InputHandler singleton
	if (!InputHandler::Init()) {
		std::cerr << "Something went wrong while initializing SDLHandler"
		<< std::endl;
		return false;
	}

	if (!event_system::event_manager::Init()) {
		std::cerr << "Something went wrong while initializing event_system"
			<< std::endl;
		return false;
	}
#ifdef GENERATE_LOG
	if (!log_writer_to_csv::Init()) {
		std::cerr << "Something went wrong while initializing log_writer_to_csv"
			<< std::endl;
		return false;
	}
#endif
	
	// enable the cursor visibility
	SDL_ShowCursor(SDL_ENABLE);
	// disable mipmap
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	
	// fullscreen mode
	// HACK: uncomment this to fullscreen
	SDL_SetWindowFullscreen(sdlutils().window(), 0);//SDL_WINDOW_FULLSCREEN_DESKTOP);
	
	_mngr = new ecs::Manager();

	// Inicializar el vector de escenas
	_scenes.resize(NUM_SCENE);

	_scenes[GAMESCENE] = new GameScene();
	_scenes[GAMESCENE]->initScene();

	_scenes[TUTORIAL] = new TutorialScene();
	_scenes[TUTORIAL]->initScene();

	_scenes[MAINMENU] = new MainMenuScene();
	_scenes[MAINMENU]->initScene();

	_scenes[CONTROLSSCENE] = new ControlsScene();
	_scenes[CONTROLSSCENE]->initScene();

	_scenes[SELECTIONMENU] = new SelectionMenuScene();
	_scenes[SELECTIONMENU]->initScene();

	_scenes[REWARDSCENE] = new RewardScene();
	_scenes[REWARDSCENE]->initScene();

	_scenes[GAMEOVER] = new GameOverScene();
	_scenes[GAMEOVER]->initScene();

	_scenes[MULTIPLAYERMENU] = new MultiplayerMenu();
	_scenes[MULTIPLAYERMENU]->initScene();
	
	_scenes[VICTORY] = new VictoryScene(); 
	_scenes[VICTORY]->initScene();

	change_Scene(MAINMENU);
	return true;
}

void Game::start() {

	// a boolean to exit the loop
	exit = false;

	auto& ihdlr = ih();
	//delta time
	constexpr static const uint32_t target_delta_time_milliseconds = 10;

	uint64_t last_frame_start_tick = SDL_GetTicks64();
	SDL_Delay(target_delta_time_milliseconds);

	while (!exit) {
		const uint64_t frame_start_tick = SDL_GetTicks64();
		const uint32_t delta_time_milliseconds = frame_start_tick - last_frame_start_tick;
		assert(delta_time_milliseconds > 0 && "fatal error: delta time must be strictly positive");
		sdlutils().virtualTimer() = VirtualTimer{
			.current_time = frame_start_tick
		};

		last_frame_start_tick = frame_start_tick;
		ihdlr.refresh();

		if (ihdlr.isKeyDown(SDL_SCANCODE_ESCAPE) || ihdlr.closeWindowEvent()) {
			exit = true;
			continue;
		}
		_scenes[_current_scene_index]->update(delta_time_milliseconds);

		//Transform* tr = Game::Instance()->get_mngr()->getComponent<Transform>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::PLAYER));
		//std::cout << "PLAYER: " << tr->getPos().getX() << "," << tr->getPos().getY() << std::endl;

		_mngr->refresh();


		sdlutils().clearRenderer();
		_scenes[_current_scene_index]->render();
		sdlutils().presentRenderer();

		const uint64_t frame_end_tick = SDL_GetTicks64();
		const uint32_t frame_duration_milliseconds = frame_end_tick - frame_start_tick;
		if (frame_duration_milliseconds < target_delta_time_milliseconds) {
			SDL_Delay(target_delta_time_milliseconds - frame_duration_milliseconds);
		}
	}

}

ecs::Manager* Game::get_mngr() {
	return _mngr;
}

event_system::event_manager* Game::get_event_mngr()
{
	return event_system::event_manager::Instance();
}

Scene* Game::get_currentScene() {
	return _scenes[_current_scene_index];
}

std::pair<int, int> Game::get_world_half_size() const
{
	return std::pair<int, int>(15,8);
}

void Game::change_Scene(State nextScene){
	if (nextScene < 0 || nextScene >= NUM_SCENE) {
		std::cerr << "Error: Invalid scene index" << std::endl;
		return;
	}

	if (_current_scene_index != -1) {
		_scenes[_current_scene_index]->exitScene();
	}

	_current_scene_index = nextScene;
	_scenes[_current_scene_index]->enterScene();
}
