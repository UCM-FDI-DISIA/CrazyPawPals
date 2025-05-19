
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
#include "scenes/SelectionMenuScene.h"
#include "scenes/GameScene.h"
#include "scenes/GameOverScene.h"
#include "scenes/RewardScene.h"
#include "scenes/TutorialScene.h"
#include "scenes/MultiplayerMenu.h"
#include "scenes/VictoryScene.h"
#include "scenes/MythicScene.h"
#include <cassert>
#include <cstdlib>
#include "our_scripts/components/WaveManagerFacade.h"

#include "../network/network_message.hpp"

#ifdef GENERATE_LOG
#include "../our_scripts/log_writer_to_csv.hpp"
#endif


using namespace std;

Game::Game() : _mngr(nullptr), network{
	.profile = {},
	.profile_status = network_context_profile_status_none,
}, network_state{
	.game_state = {},
	.connections = {
		.connected_users = 0,
		.local_user_index = network_context_maximum_connections,
		.oldest_non_host_index = network_context_maximum_connections
	}
} {

}

static void game_destroy_network_context(network_context &ctx) {
	switch (ctx.profile_status) {
	case network_context_profile_status_none:
		break;
	case network_context_profile_status_host: {
		if (network_context_host_connected(ctx.profile.host)) {
			network_context_host_destroy(ctx.profile.host);
		} else {
			ctx.profile.host.ip_self.host = INADDR_NONE;
		}
		break;
	}
	case network_context_profile_status_client: {
		if (network_context_client_connected(ctx.profile.client)) {
			network_context_client_destroy(ctx.profile.client);
		} else {
			ctx.profile.client.ip_host.host = INADDR_NONE;
		}
		break;
	}
	default: {
		assert(false && "fatal error: invalid network context profile status");
		std::exit(EXIT_FAILURE);
	}
	}
	ctx.profile_status = network_context_profile_status_none;
}
Game::~Game() {
	for (auto scene : _scenes) {
		if (scene != nullptr) {
			delete scene;
		}
	}
	_scenes.clear();
	game_destroy_network_context(network);

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

static void game_init_network_context(network_context &ctx) {
#if DBG_NETWORK
#if !defined(DBG_NETWORK_HOST) || !defined(DBG_NETWORK_CLIENT) || !defined(DBG_NETWORK_HOST_IP) && !defined(DBG_NETWORK_HOST_PORT)
	static_assert(
		false,
		"static error: DBG_NETWORK_HOST and DBG_NETWORK_CLIENT must be defined at the same time"
		"as well as DBG_NETWORK_HOST_IP and DBG_NETWORK_HOST_PORT"
	);
#endif

#if DBG_NETWORK_HOST && DBG_NETWORK_CLIENT
	static_assert(
		false,
		"static error: DBG_NETWORK_HOST and DBG_NETWORK_CLIENT cannot be defined at the same time"
	);
#elif DBG_NETWORK_HOST
	ctx = network_context_create_host(nullptr, DBG_NETWORK_HOST_PORT);
#elif DBG_NETWORK_CLIENT
	ctx = network_context_create_client(DBG_NETWORK_HOST_IP, DBG_NETWORK_HOST_PORT);
#else
	static_assert(
		false,
		"static error: DBG_NETWORK_HOST or DBG_NETWORK_CLIENT must be defined at the same time"
	);
#endif
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
	SDLNet_Init();

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
	game_init_network_context(network);

	// enable the cursor visibility
	SDL_ShowCursor(SDL_ENABLE);
	// disable mipmap
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	
	// fullscreen mode
	// HACK: uncomment this to fullscreen
	SDL_SetWindowFullscreen(sdlutils().window(),SDL_WINDOW_FULLSCREEN_DESKTOP); //0 for non-full screen mode SDL_WINDOW_FULLSCREEN_DESKTOP
	
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
	_scenes[MULTIPLAYERMENU] = new MultiplayerMenu();
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

	GameScene::create_environment();
	//crear player
	ecs::entity_t player = GameScene::create_player();
	_mngr->setHandler(ecs::hdlr::PLAYER, player);

	//iniciar el juego en el mainmenu
	queue_scene(MAINMENU);
	update_scene();
	set_volumes();
}

static void game_start_network_dbg(network_context &ctx) {
	switch (ctx.profile_status) {
	case network_context_profile_status_none: 
		break;
	case network_context_profile_status_host: {
		network_context_host_connect_alloc(ctx.profile.host);

		while (true) {
			if (SDLNet_CheckSockets(ctx.profile.host.clients_host_set, 60 * 1000) > 0) {
				if (SDLNet_SocketReady(ctx.profile.host.host_socket)) {
					network_connection_size connection_index;
					network_context_host_accept_connection_status_flags status = network_context_host_accept_connection(
						ctx.profile.host,
						connection_index
					);
					if (status & network_context_host_accept_connection_status_accepted) {
						std::cout << "Accepted connection. Connection index: " << connection_index << std::endl;
					}
					else if (status & network_context_host_accept_connection_status_rejected) {
						std::cout << "Rejected connection" << std::endl;
					}
					
					if (status & network_context_host_accept_connection_status_full) {
						std::cout << "Connection full" << std::endl;
					}
					else if (status & network_context_host_accept_connection_status_error) {
						std::cerr << "Error accepting connection" << std::endl;
					}
				}
				for (network_connection_size i = 0; i < ctx.profile.host.sockets_to_clients.connection_count; ++i) {
					TCPsocket &connection = ctx.profile.host.sockets_to_clients.connections[i];
					if (SDLNet_SocketReady(connection)) {
						network_message_pack<network_message_payload_dbg_print<256>> message =
							network_message_pack_receive<network_message_payload_dbg_print<256>>(
							connection
						);

						const network_message_type_option type_n = message.header.type_n;
						const network_message_type_option type_h = SDLNet_Read16(&type_n);
						if (
							type_h == network_message_type_dbg_print
						) {
							const size_t length = SDLNet_Read32(&message.payload.content.args_size_n);
							assert(length < message.payload.content.args.size() && "fatal error: message length is too long");

							message.payload.content.args[length] = '\0';
							std::cout << "Received message: " << message.payload.content.args.data() << "\t with length: " << length << std::endl;
						}
					}
				}
			}
		}
		break;
	}
	case network_context_profile_status_client: {
		const network_context_client_connect_status_flags connected =
			network_context_client_connect_alloc(ctx.profile.client);
		
		if (connected & network_context_client_connect_status_error) {
			std::cerr << "Error connecting to host" << std::endl;
			break;
		}

		network_message_pack_send(
			ctx.profile.client.socket_to_host,
			network_message_pack_create(
				network_message_type_dbg_print,
				network_message_payload_dbg_print_create<256>(
					"Hello from client!"
				)
			)
		);
		// while (true) {
		// 	if (SDLNet_CheckSockets(ctx.profile.client.client_set,))
		// }
		break;
	}
	default: {
		assert(false && "fatal error: invalid network context profile status");
		std::exit(EXIT_FAILURE);
	}
	}
}
void Game::start() {

	// a boolean to exit the loop
	exit = false;

	auto& ihdlr = ih();
	//delta time
	constexpr static const uint32_t target_delta_time_milliseconds = 10;

	// game_start_network_dbg(network);

	uint64_t last_frame_start_tick = SDL_GetTicks64();
	SDL_Delay(target_delta_time_milliseconds);
	sdlutils().virtualTimer().resetTime();
	while (!exit) {
		const uint64_t frame_start_tick = SDL_GetTicks64();
		const uint32_t delta_time_milliseconds = uint32_t(frame_start_tick - last_frame_start_tick);
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

		if ( _next_scene_index != -1) update_scene();
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

WaveManagerFacade*& Game::get_wave_manager()
{
	assert(wave_manager);
	return wave_manager;
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

void Game::update_scene(){

	//Inicializa cuando entra por primera vez a una escena
	if (!_scene_inits[_next_scene_index] && _scenes[_next_scene_index] != nullptr) {
		_scenes[_next_scene_index]->initScene();
		_scene_inits[_next_scene_index] = true;
		_mngr->refresh();
	}

	if (_current_scene_index != -1 && _scenes[_current_scene_index] != nullptr) {
		_scenes[_current_scene_index]->exitScene();
	}

	_current_scene_index = _next_scene_index;
	_scenes[_current_scene_index]->enterScene();
	_next_scene_index = -1;
}
void Game::queue_scene(State nextScene)
{
	if (nextScene < 0 || nextScene >= NUM_SCENE) {
		std::cerr << "Error: Invalid scene index" << std::endl;
		return;
	}

	_next_scene_index = nextScene;
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
