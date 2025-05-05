#include "../utils/checkML.h"
#include "Game.h"

#include "../ecs/Manager.h"
#include "../sdlutils/InputHandler.h"
#include "../sdlutils/SDLUtils.h"
#include "../utils/Vector2D.h"
#include "../utils/Collisions.h"

#include "../our_scripts/components/rendering/camera_component.hpp"



//Scenes for SceneManager
#include "scenes/Scene.h"
#include "scenes/MainMenuScene.h"
#include "scenes/ControlsScene.h"
#include "scenes/SelectionMenuScene.h"
#include "scenes/GameScene.h"
#include "scenes/GameOverScene.h"
#include "scenes/RewardScene.h"
#include "scenes/TutorialScene.h"
#include "scenes/VictoryScene.h"
#include "scenes/MythicScene.h"


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

	// release SDLUtils if the instance was created correctly.
	if (SDLUtils::HasInstance())
		SDLUtils::Release();

	if (_mngr != nullptr) delete _mngr;

	// release event_manager if the instance was created correctly.
	if (event_system::event_manager::HasInstance())
		event_system::event_manager::Release();
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
	SDL_SetWindowFullscreen(sdlutils().window(), SDL_WINDOW_FULLSCREEN_DESKTOP); //0 for non-full screen mode
	
	initGame();

	return true;
}

void Game::initGame()
{
	_mngr = new ecs::Manager();

	// Inicializar el vector de escenas
	_scenes.resize(NUM_SCENE, nullptr);
	_scene_inits.resize(NUM_SCENE, false);

	_scenes[GAMESCENE] = new GameScene();
	_scenes[TUTORIAL] = new TutorialScene();
	_scenes[MAINMENU] = new MainMenuScene();
	_scenes[SELECTIONMENU] = new SelectionMenuScene();
	_scenes[REWARDSCENE] = new RewardScene();
	_scenes[MYTHICSCENE] = new MythicScene();
	_scenes[GAMEOVER] = new GameOverScene();
	_scenes[VICTORY] = new VictoryScene();

	//crear la camara
	const Scene::rendering::camera_creation_descriptor_flags flags =
		Scene::rendering::camera_creation_descriptor_options::camera_creation_descriptor_options_set_handler
		| Scene::rendering::camera_creation_descriptor_options::camera_creation_descriptor_options_clamp;
	Scene::rendering::create_camera(ecs::scene::MAINMENUSCENE, flags, nullptr);

	//crear player
	ecs::entity_t player = GameScene::create_player();
	_mngr->setHandler(ecs::hdlr::PLAYER, player);

	//iniciar el juego en el mainmenu
	change_Scene(MAINMENU);
	set_volumes();
}

void Game::start() {

	// a boolean to exit the loop
	exit = false;

	auto& ihdlr = ih();
	//delta time
	constexpr static const uint32_t target_delta_time_milliseconds = 10;

	uint64_t last_frame_start_tick = SDL_GetTicks64();
	SDL_Delay(target_delta_time_milliseconds);
	sdlutils().virtualTimer().resetTime();
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

	//Inicializa cuando entra por primera vez a una escena
	if (!_scene_inits[nextScene] && _scenes[nextScene] != nullptr) {
		_scenes[nextScene]->initScene();
		_scene_inits[nextScene] = true;
		_mngr->refresh();
	}

	if (_current_scene_index != -1 && _scenes[_current_scene_index] != nullptr) {
		_scenes[_current_scene_index]->exitScene();
	}

	_current_scene_index = nextScene;
	_scenes[_current_scene_index]->enterScene();
}
void Game::set_volumes() {
	sdlutils().soundEffects().at("button_hover").setVolume(20);
	sdlutils().soundEffects().at("enemy_shot").setVolume(35);
	sdlutils().soundEffects().at("player_shot").setVolume(35);
	sdlutils().soundEffects().at("card_mill").setVolume(25);
	sdlutils().soundEffects().at("card_play").setVolume(35);
	sdlutils().soundEffects().at("shuffle").setVolume(25);
	sdlutils().soundEffects().at("round_start").setVolume(45);
	sdlutils().soundEffects().at("round_start_event").setVolume(45);
	sdlutils().musics().at("main_menu_bgm").setMusicVolume(30);
	sdlutils().soundEffects().at("reward").setVolume(40);
	sdlutils().soundEffects().at("game_over").setVolume(40);
}
