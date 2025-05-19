#pragma region includes

#include "GameScene.h"

#include "../Game.h"
#include "../../ecs/Manager.h"
#include "../../sdlutils/InputHandler.h"
#include "../../sdlutils/SDLUtils.h"
#include "../../utils/Vector2D.h"

#include "../../our_scripts/components/rendering/Image.h"
#include "../../our_scripts/components/movement/Transform.h"
#include "../../our_scripts/components/movement/Follow.h"
#include "../../our_scripts/components/movement/MovementController.h"
#include "../../our_scripts/components/net/enemy_synchronize.h"
#include "../../our_scripts/components/LifetimeTimer.h"
#include "../../our_scripts/states/Conditions.h"
#include "../../our_scripts/states/WalkingState.h"
#include "../../our_scripts/states/AttackingState.h"
#include "../../our_scripts/states/WaitingState.h"
#include "../../our_scripts/states/RotatingState.h"
#include "../../our_scripts/states/DashingState.h"
#include "../../our_scripts/states/AnimationState.h"

#include "../../our_scripts/components/weapons/enemies/WeaponSuperMichiMafioso.h"

#include "../../our_scripts/components/KeyboardPlayerCtrl.h"
#include "../../our_scripts/components/cards/Mana.h"
#include "../../our_scripts/components/cards/Deck.hpp"
#include "../../our_scripts/components/rendering/dyn_image.hpp"
#include "../../our_scripts/components/rendering/dyn_image_with_frames.hpp"
#include "../../our_scripts/components/rendering/FlipXController.h"
#include "../../our_scripts/components/AnimationComponent.h"
#include "../../our_scripts/components/rendering/camera_component.hpp"
#include "../../our_scripts/components/rendering/RotationComponent.h"

#include "../../our_scripts/components/Health.h"
#include "../../our_scripts/components/weapons/BulletData.h"

#include "../../utils/Collisions.h"

#include "../../our_scripts/components/WaveManager.h"
#include "../../our_scripts/components/DumbWaveManager.h"
#include "../../our_scripts/components/Fog.h"
#include "../../our_scripts/components/weapons/enemies/WeaponMichiMafioso.h"
#include "../../our_scripts/components/weapons/enemies/WeaponPlimPlim.h"
#include "../../our_scripts/components/weapons/enemies/WeaponSarnoRata.h"
#include "../../our_scripts/components/weapons/enemies/WeaponBoom.h"
#include "../../our_scripts/components/weapons/enemies/WeaponCatKuza.h"
#include "../../our_scripts/components/weapons/enemies/WeaponRataBasurera.h"
#include "../../our_scripts/components/weapons/enemies/WeaponReyBasurero.h"
#include "../../our_scripts/components/Health.h"
#include "../../our_scripts/components/bullet_collision_component.hpp"
#include "../../our_scripts/components/fog_collision_component.hpp"

#include "../../our_scripts/components/StateMachine.h"
#include "../../our_scripts/components/rendering/dyn_image.hpp"
#include "../../our_scripts/components/rendering/transformless_dyn_image.h"
#include "../../our_scripts/components/rendering/render_ordering.hpp"
#include "../../our_scripts/components/rendering/rect_component.hpp"
#include "../../our_scripts/components/ui/PlayerHUD.h"
#include "../../our_scripts/components/ui/HUD.h"
#include "../../our_scripts/components/collision_triggerers.hpp"
#include "../../our_scripts/components/id_component.h"
#include "../../our_scripts/components/collision_registration_by_id.h"
#include "../../our_scripts/components/GamePadPlayerCtrl.hpp"
#include "../../our_scripts/components/rigidbody_component.hpp"

#include "../../our_scripts/card_system/PlayableCards.hpp"
#include "../../our_scripts/card_system/CardUpgrade.hpp"

#include "../../our_scripts/components/MythicComponent.h"
#include "../../our_scripts/components/net/GhostStateComponent.h"
#include "../../our_scripts/components/net/player_synchronize.h"
#include "../../our_scripts/components/ui/MultiplayerHUD.h"
#include "RewardScene.h"
#include "../../our_scripts/components/WaveManager.h"

#ifdef GENERATE_LOG
#include "../../our_scripts/log_writer_to_csv.hpp"
#endif

#include <iostream>
#include <string>

#pragma endregion

GameScene::GameScene() : Scene(ecs::scene::GAMESCENE)
{
	event_system::event_manager::Instance()->suscribe_to_event(event_system::change_deccel, this, &event_system::event_receiver::event_callback0);
	event_system::event_manager::Instance()->suscribe_to_event(event_system::player_dead, this, &event_system::event_receiver::event_callback1);
}
GameScene::~GameScene()
{
	event_system::event_manager::Instance()->unsuscribe_to_event(event_system::change_deccel, this, &event_system::event_receiver::event_callback0);
	event_system::event_manager::Instance()->unsuscribe_to_event(event_system::player_dead, this, &event_system::event_receiver::event_callback1);
}

float GameScene::deccel_spawned_creatures_multi = 1;

struct game_scene_map_walls
{
	std::array<ecs::entity_t, 4> wall_entities;
	std::array<collisionable *, 4> wall_collisionables;

	constexpr static size_t size()
	{
		return 4;
	}
};
static game_scene_map_walls game_scene_create_map_walls(ecs::Manager &manager, const ecs::sceneId_t scene_id, const rect_f32 scene_limits)
{
	const std::array<ecs::entity_t, game_scene_map_walls::size()> wall_entities{
		manager.addEntity(scene_id),
		manager.addEntity(scene_id),
		manager.addEntity(scene_id),
		manager.addEntity(scene_id)};

	constexpr static const float wall_thickness = 1.0f;
	const std::array<position2_f32, game_scene_map_walls::size()> wall_positions{
		position2_f32{scene_limits.position.x - scene_limits.size.x * 0.5f - 0.001f /*- wall_thickness * 0.25f-*/, scene_limits.position.y},
		position2_f32{scene_limits.position.x + scene_limits.size.x * 0.5f + 0.001f /*+ wall_thickness * 0.25f-*/, scene_limits.position.y},
		position2_f32{scene_limits.position.x, scene_limits.position.y - scene_limits.size.y * 0.5f - 0.001f /*- wall_thickness * 0.25f*/},
		position2_f32{scene_limits.position.x, scene_limits.position.y + scene_limits.size.y * 0.5f + 0.001f /*+ wall_thickness * 0.25f*/}};

	constexpr static const auto vec_from_position = [](const position2_f32 position) -> Vector2D
	{
		return Vector2D{position.x, position.y};
	};
	const std::array<Transform *, game_scene_map_walls::size()> wall_transforms{
		new Transform{vec_from_position(wall_positions[0]), Vector2D{0.0f, 0.0f}, 0.0f, 0.0f},
		new Transform{vec_from_position(wall_positions[1]), Vector2D{0.0f, 0.0f}, 0.0f, 0.0f},
		new Transform{vec_from_position(wall_positions[2]), Vector2D{0.0f, 0.0f}, 0.0f, 0.0f},
		new Transform{vec_from_position(wall_positions[3]), Vector2D{0.0f, 0.0f}, 0.0f, 0.0f}};

	const std::array<rect_component *, game_scene_map_walls::size()> wall_rects{
		new rect_component{0.0f, 0.0f, wall_thickness, scene_limits.size.y - wall_thickness},
		new rect_component{0.0f, 0.0f, wall_thickness, scene_limits.size.y - wall_thickness},
		new rect_component{0.0f, 0.0f, scene_limits.size.x - wall_thickness, wall_thickness},
		new rect_component{0.0f, 0.0f, scene_limits.size.x - wall_thickness, wall_thickness}};

	const std::array<rigidbody_component *, game_scene_map_walls::size()> wall_rigidbodies{
		new rigidbody_component{rect_f32_full_subrect, inverse_mass_f32{0.00000000f}, 1.0f},
		new rigidbody_component{rect_f32_full_subrect, inverse_mass_f32{0.00000000f}, 1.0f},
		new rigidbody_component{rect_f32_full_subrect, inverse_mass_f32{0.00000000f}, 1.0f},
		new rigidbody_component{rect_f32_full_subrect, inverse_mass_f32{0.00000000f}, 1.0f}};
	const std::array<collisionable *, game_scene_map_walls::size()> wall_collisionables{
		new collisionable{*wall_transforms[0], *wall_rigidbodies[0], *wall_rects[0], collisionable_option_none},
		new collisionable{*wall_transforms[1], *wall_rigidbodies[1], *wall_rects[1], collisionable_option_none},
		new collisionable{*wall_transforms[2], *wall_rigidbodies[2], *wall_rects[2], collisionable_option_none},
		new collisionable{*wall_transforms[3], *wall_rigidbodies[3], *wall_rects[3], collisionable_option_none}};

	for (size_t i = 0; i < wall_entities.size(); ++i)
	{
		manager.addExistingComponent(wall_entities[i], wall_transforms[i], wall_rects[i], wall_rigidbodies[i], wall_collisionables[i]);
		manager.addComponent<render_ordering>(wall_entities[i], 0);
	}

	return game_scene_map_walls{
		wall_entities,
		wall_collisionables};
}

ecs::entity_t GameScene::create_environment(ecs::sceneId_t scene)
{
	auto &&manager = *Game::Instance()->get_mngr();
	auto environment = manager.addEntity(scene);
	auto &&tr = *manager.addComponent<Transform>(environment, Vector2D(-16.0, 9.0), Vector2D(0.0, 0.0), 0.0f, 0.05f);
	auto &&rect = *manager.addComponent<rect_component>(environment, -0.5f, 0.5f, 35.0f, 20.0f);
	(void)tr;
	manager.addComponent<offset_dyn_image>(environment, rect_f32{{0.0, 0.0}, {1.0, 1.0}}, position2_f32{0.0f, 0.0f}, rect, manager.getComponent<camera_component>(manager.getHandler(ecs::hdlr::CAMERA))->cam, sdlutils().images().at("floor"), tr);
	manager.addComponent<render_ordering>(environment, 0);
	game_scene_create_map_walls(manager, scene, GameScene::default_scene_bounds);
	return environment;
}

void GameScene::initScene()
{
	id_component::reset();
	auto &&manager = *Game::Instance()->get_mngr();
	auto player = manager.getHandler(ecs::hdlr::PLAYER);
	manager.addComponent<MythicComponent>(player);

	manager.refresh();
	create_environment();
	// spawn_sarno_rata(Vector2D{5.0f, 0.0f});
	spawn_fog();
	spawn_wave_manager();
	auto hud = create_hud();
	Game::Instance()->get_mngr()->setHandler(ecs::hdlr::HUD_ENTITY, hud);
}

void GameScene::enterScene()
{
	auto &&manager = *Game::Instance()->get_mngr();
	auto player = manager.getHandler(ecs::hdlr::PLAYER);

	auto camera = manager.getHandler(ecs::hdlr::CAMERA);
	Game::Instance()->get_mngr()->change_ent_scene(camera, ecs::scene::GAMESCENE);

	manager.addComponent<camera_follow>(camera, camera_follow_descriptor{.previous_position = manager.getComponent<camera_component>(camera)->cam.camera.position, .lookahead_time = 1.0f, .semi_reach_time = 2.5f}, *manager.getComponent<camera_component>(camera), *manager.getComponent<Transform>(player));

	manager.refresh();

	manager.addComponent<id_component>(player);

	auto d = manager.getComponent<Deck>(player);
	d->reload();

	manager.getComponent<fog_collision_component>(manager.getHandler(ecs::hdlr::FOGGROUP))->reset();
	manager.addComponent<KeyboardPlayerCtrl>(player);
	manager.addComponent<GamePadPlayerCtrl>(player);
	manager.addComponent<PlayerHUD>(player);
	auto wm = Game::Instance()->get_wave_manager();
	if (Game::Instance()->is_host() || Game::Instance()->is_network_none())
	{
		dynamic_cast<WaveManager *>(wm)->start_new_wave();
	}

	if (!Game::Instance()->is_network_none())
	{
		manager.addComponent<PlayerSynchronize>(player, uint32_t{Game::Instance()->client_id()});
		manager.addComponent<MultiplayerHUD>(player);
	}
	// get the current event
	// auto e = wm->get_current_event();
	// RewardScene::will_have_mythic(e != NONE || ((wm->get_current_wave() + 1) % 5 == 0));
	//}
	manager.getComponent<HUD>(manager.getHandler(ecs::hdlr::HUD_ENTITY))->start_new_wave();
	//spawn_catkuza(Vector2D{10.0f, 0.0f});
	//spawn_rata_basurera(Vector2D{5.0f, 0.0f});
	//spawn_rey_basurero(Vector2D{-5.0f, 0.0f});
	//spawn_super_michi_mafioso(Vector2D{5.0f, 0.0f});
#ifdef GENERATE_LOG
	log_writer_to_csv::Instance()->add_new_log();
	log_writer_to_csv::Instance()->add_new_log("ENTERED GAME SCENE");
#endif
}

void GameScene::exitScene()
{
	auto &&manager = *Game::Instance()->get_mngr();
	Game::Instance()->get_wave_manager()->reset_wave_time();

	if (!Game::Instance()->is_network_none())
	{
		auto player = manager.getHandler(ecs::hdlr::PLAYER);
		manager.removeComponent<PlayerSynchronize>(player);
		manager.removeComponent<MultiplayerHUD>(player);
	}

	auto wm = Game::Instance()->get_wave_manager();
	auto e = wm->get_current_event();
	RewardScene::will_have_mythic(e!=NONE || (wm->get_current_wave()) % 5 == 0);
	wm->reset_wave_time();
#ifdef GENERATE_LOG
	log_writer_to_csv::Instance()->add_new_log("EXIT GAME SCENE");
	log_writer_to_csv::Instance()->add_new_log();
#endif
}

void GameScene::update(uint32_t delta_time)
{
	Scene::update(delta_time);

	network_context &network = Game::Instance()->get_network();
	switch (network.profile_status)
	{
	case network_context_profile_status_none:
		break;
	case network_context_profile_status_host:
	{
		host_handle_menssage(network);
		break;
	}
	case network_context_profile_status_client:
	{
		client_handle_menssage(network);
		break;
	}
	default:
	{
		assert(false && "unreachable: invalid network profile status");
		std::exit(EXIT_FAILURE);
	}
	}
}

// metodos de create/spawn

#pragma region Player
ecs::entity_t GameScene::create_player(ecs::sceneId_t scene)
{
	auto &&manager = *Game::Instance()->get_mngr();
	auto &&camera = manager.getComponent<camera_component>(manager.getHandler(ecs::hdlr::CAMERA))->cam;

	auto &&player_transform = *new Transform({0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, 2.0f);
	auto &&player_rect = *new rect_component{0, 0, 1.125f, 1.5f};
	auto &&player_rigidbody = *new rigidbody_component{rect_f32{{0.15f, -0.125}, {0.5f, 0.75f}}, mass_f32{7.0f}, 1.0f};
	auto &&player_collisionable = *new collisionable{player_transform, player_rigidbody, player_rect, collisionable_option_none};
	ecs::entity_t player = create_entity(
		ecs::grp::PLAYER,
		scene,
		&player_transform,
		&player_rect,
		new dyn_image_with_frames(
			rect_f32{{0, 0}, {0.2, 1}},
			player_rect,
			camera,
			sdlutils().images().at("piu"),
			player_transform, "piu"),
		new render_ordering{5},
		new Health(100, true),
		new ManaComponent(),
		&player_rigidbody,
		&player_collisionable,
		new MovementController(0.1f, 5.0f, 20.0f * deccel_spawned_creatures_multi),
		new player_collision_triggerer(),
		new id_component());

	// si tiene mas de una animacion
	auto *anim = manager.addComponent<AnimationComponent>(player);
	anim->add_animation("andar", 1, 5, 100);
	anim->add_animation("idle", 0, 0, 100);

	return player;
}

bool GameScene::change_player_tex(const std::string &text_name)
{
	auto &&manager = *Game::Instance()->get_mngr();
	auto player = manager.getHandler(ecs::hdlr::PLAYER);
	if (auto &&dy = manager.getComponent<dyn_image_with_frames>(player))
	{
		dy->texture = &sdlutils().images().at(text_name);
		dy->texture_name = text_name;
		return true;
	}
	return false;
}

void GameScene::reset_player()
{
	auto &&mngr = *Game::Instance()->get_mngr();
	auto player = mngr.getHandler(ecs::hdlr::PLAYER);

	mngr.removeComponent<Weapon>(player);
	mngr.removeComponent<Deck>(player);
	mngr.removeComponent<KeyboardPlayerCtrl>(player);
	mngr.removeComponent<GamePadPlayerCtrl>(player);
	mngr.removeComponent<PlayerHUD>(player);
	mngr.removeComponent<GhostStateComponent>(player);

	mngr.getComponent<MythicComponent>(player)->reset();
	mngr.getComponent<dyn_image_with_frames>(player)->isDamaged = false;
	auto tr = mngr.getComponent<Transform>(player);
	tr->setPos({0.0f, 0.0f});
	tr->setDir({0.0f, 0.0f});

	mngr.getComponent<AnimationComponent>(player)->play_animation("idle");
	mngr.getComponent<Health>(player)->resetCurrentHeatlh();
	mngr.addComponent<ManaComponent>(player);
	mngr.addComponent<MovementController>(player, 0.1f, 5.0f, 20.0f * deccel_spawned_creatures_multi);
}

#pragma endregion

#pragma region Enemy
ecs::entity_t GameScene::create_enemy(GameStructs::EnemyProperties &ec, ecs::sceneId_t scene, Weapon *weapon)
{
	auto &&manager = *Game::Instance()->get_mngr();

	float randSize = float(sdlutils().rand().nextInt(6, 10)) / 10.0f;
	auto &&rect = *new rect_component{0, 0, ec.width * randSize, ec.height * randSize};
	auto &&rigidbody = *new rigidbody_component{rect_f32{{0.0f, -0.15f}, {0.5f, 0.6f}}, mass_f32{3.0f}, 0.05f};
	auto &&tr = *new Transform(ec.start_pos, ec.dir, ec.r, ec.s * randSize);
	auto &&fll = *new Follow(ec.follow);
	auto &&col = *new collisionable{tr, rigidbody, rect, collisionable_option_none};
	auto &&mc = *new MovementController(ec.max_speed, ec.acceleration, ec.decceleration * deccel_spawned_creatures_multi);
	auto &&state = *new StateMachine();

	auto e = create_entity(
		ecs::grp::ENEMY,
		scene,
		&tr,
		&rect,
		new dyn_image(
			rect_f32{{0, 0}, {1, 1}},
			rect,
			manager.getComponent<camera_component>(manager.getHandler(ecs::hdlr::CAMERA))->cam,
			sdlutils().images().at(ec.sprite_key),
			tr),
		new Health(ec.health),
		new FlipXController(),
		new enemy_collision_triggerer(),
		new id_component(),
		weapon,
		&rigidbody,
		&col,
		&mc,
		&state,
		&fll);
	online_enemy(e);

	if (scene == ecs::scene::GAMESCENE)
		Game::Instance()->get_mngr()->getComponent<WaveManager>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::WAVE))->newEnemy();
	// Si es online, agregar el componente de sincronizacion

	return e;
}
void GameScene::select_create_dumb_enemy(GameStructs::DumbEnemyProperties &ec)
{
	switch (ec._type)
	{
	case 0:
		std::cout << "spawning sarno rata" << std::endl;
		spawn_sarno_rata(ec._pos, ecs::scene::GAMESCENE, (uint8_t)ec._id);
		break;
	case 1:
		std::cout << "spawning michi mafioso" << std::endl;
		spawn_michi_mafioso(ec._pos, ecs::scene::GAMESCENE, (uint8_t)ec._id);
		break;
	case 2:
		std::cout << "spawning plim plim" << std::endl;
		spawn_plim_plim(ec._pos, ecs::scene::GAMESCENE, (uint8_t)ec._id);
		break;
	case 3:
		std::cout << "spawning boom" << std::endl;
		spawn_boom(ec._pos, ecs::scene::GAMESCENE, (uint8_t)ec._id);
		break;
	case 4:
		std::cout << "spawning ratatouille" << std::endl;
		spawn_ratatouille(ec._pos, ecs::scene::GAMESCENE, (uint8_t)ec._id);
		break;
	case 5:
		std::cout << "spawning rata basurera" << std::endl;
		spawn_rata_basurera(ec._pos, ecs::scene::GAMESCENE, (uint8_t)ec._id);
		break;
	case 6:
		std::cout << "spawning rey basurero" << std::endl;
		spawn_rey_basurero(ec._pos, ecs::scene::GAMESCENE, (uint8_t)ec._id);
		break;
	case 7:
		std::cout << "spawning super michi mafioso" << std::endl;
		spawn_super_michi_mafioso(ec._pos, ecs::scene::GAMESCENE, (uint8_t)ec._id);
		break;
	case 8:
		std::cout << "spawning cat kuza" << std::endl;
		spawn_catkuza(ec._pos, ecs::scene::GAMESCENE, (uint8_t)ec._id);
		break;
	default:
		break;
	}
}
ecs::entity_t GameScene::dumb_enemy(GameStructs::EnemyProperties &ec, ecs::sceneId_t scene)
{
	auto &&manager = *Game::Instance()->get_mngr();

	float randSize = float(sdlutils().rand().nextInt(6, 10)) / 10.0f;
	auto &&tr = *new Transform(ec.start_pos, ec.dir, ec.r, ec.s * randSize);
	auto &&rect = *new rect_component{0, 0, ec.width * randSize, ec.height * randSize};
	auto &&rigidbody = *new rigidbody_component{rect_f32{{0.0f, -0.15f}, {0.5f, 0.6f}}, mass_f32{3.0f}, 0.05f};
	auto &&col = *new collisionable{tr, rigidbody, rect, collisionable_option_none};

	auto e = create_entity(
		ecs::grp::ENEMY,
		scene,
		&tr,
		&rect,
		new dyn_image(
			rect_f32{{0, 0}, {1, 1}},
			rect,
			manager.getComponent<camera_component>(manager.getHandler(ecs::hdlr::CAMERA))->cam,
			sdlutils().images().at(ec.sprite_key),
			tr),
		new Health(ec.health),
		new id_component(ec._id),
		&rigidbody,
		new FlipXController(),
		&col);

	// Agregar el componente de sincronizacion
	online_enemy(e);
	return e;
}
ecs::entity_t GameScene::get_network_enemy(uint8_t _id)
{
	auto &&manager = *Game::Instance()->get_mngr();
	auto e = nullptr;
	auto &group = manager.getEntities(ecs::grp::ENEMY);
	for (const auto &ent : group)
	{
		auto n = group.size();
		auto _id_c = manager.getComponent<id_component>(ent);
		if (_id_c != nullptr && _id_c->getId() == _id)
		{
			return ent;
		}
	}

	return e;
}
void GameScene::online_enemy(ecs::entity_t ec)
{
	if (Game::Instance()->is_host() || Game::Instance()->is_client())
	{
		auto &&manager = *Game::Instance()->get_mngr();
		manager.addComponent<EnemySynchronize>(ec);
	}
}
#pragma endregion

#pragma region Super Michi Mafioso
uint8_t GameScene::spawn_super_michi_mafioso(Vector2D posVec, ecs::sceneId_t scene, uint8_t _id)
{
	auto &&manager = *Game::Instance()->get_mngr();
	GameStructs::EnemyProperties ec = GameStructs::EnemyProperties{
		"super_michi_mafioso", // sprite_key
		_id,
		posVec,					   // start_pos
		GameStructs::DEFAULT,	   // enemy_type
		25,						   // health
		1.75f,					   // width
		2.25f,					   // height
		GameStructs::HIGHEST_LIFE, // target_strategy
		{0.0f, 0.0f},			   // velocity
		0.0f,					   // rotation
		2.0f					   // scale
	};

	if (Game::Instance()->is_host() || Game::Instance()->is_network_none())
	{
		auto &&weapon = *new WeaponSuperMichiMafioso();

		auto e = create_enemy(
			ec,
			scene,
			static_cast<Weapon *>(&weapon));
		Transform *tr = manager.getComponent<Transform>(e);
		MovementController *mc = manager.getComponent<MovementController>(e);
		mc->set_max_speed(0.06);
		StateMachine *state = manager.getComponent<StateMachine>(e);

		Follow *fll = manager.getComponent<Follow>(e);
		auto _p_tr = fll->get_act_follow();

		auto state_cm = state->getConditionManager();

		state_cm->set_cooldown("area_attack_duration", 400);
		state_cm->set_cooldown("large_area_attack_duration", 10000);
		state_cm->set_cooldown("shot_attack", 2000);
		state_cm->set_cooldown("spawn_michi", 8000);

		// Crear estados
		auto walkingState = std::make_shared<WalkingState>(tr, mc, fll);
		auto waitingState = std::make_shared<WaitingState>();

		auto areaAttackState = std::make_shared<AttackingState>(
			tr, fll, &weapon, false,
			std::function<bool()>([&weapon, fll]() -> bool
								  {
				weapon.set_player_pos(fll->get_act_follow()->getPos());
				weapon.attack1();
				return false; }));

		auto shotAttackState = std::make_shared<AttackingState>(
			tr, fll, &weapon, false,
			std::function<bool()>([&weapon, tr]() -> bool
								  {
				Vector2D shootPos = tr->getPos(); // Posición del enemigo
				weapon.attack2(shootPos);
				return false; }));

		auto largeAreaAttackState = std::make_shared<AttackingState>(
			tr, fll, &weapon, false,
			std::function<bool()>([&weapon, tr]() -> bool
								  {
				Vector2D shootPos = tr->getPos(); // Posición del enemigo
				weapon.attack2(shootPos);
				return false; }));

		auto spawnMichiState = std::make_shared<AttackingState>(
			tr, fll, &weapon, false,
			std::function<bool()>([&weapon]() -> bool
								  { weapon.generate_michi_mafioso(); 
				return false; }));

		// poner los estado a la state
		state->add_state("Walking", walkingState);
		state->add_state("AreaAttack", areaAttackState);
		state->add_state("ShotAttack", shotAttackState);
		state->add_state("LargeAreaAttack", largeAreaAttackState);
		state->add_state("SpawnMichi", spawnMichiState);
		state->add_state("Waiting", waitingState);

		float dist_to_attack = 4.0f;

		// Condiciones de cada estado
		// Patron1: cuando se acerca al player empieza el p1
		state->add_transition("Walking", "AreaAttack", [state_cm, _p_tr, tr, dist_to_attack]()
							  { return state_cm->can_use("area_attack_duration", sdlutils().virtualTimer().currTime()) && state_cm->is_player_near(_p_tr, tr, dist_to_attack); });

		state->add_transition("AreaAttack", "Waiting", [state_cm, dist_to_attack]()
							  {
			uint32_t currentTime = sdlutils().virtualTimer().currTime();
			bool trans = state_cm->can_use("area_attack_duration", currentTime);
			if (trans) { state_cm->reset_cooldown("area_attack_duration", currentTime); };
			return trans; });

		state->add_transition("Waiting", "AreaAttack", [state_cm, _p_tr, tr, dist_to_attack]()
							  { return state_cm->can_use("area_attack_duration", sdlutils().virtualTimer().currTime()) && state_cm->is_player_near(_p_tr, tr, dist_to_attack); });

		// patron2: cuando ataque 3 veces p1, pasa a 2 si el player no se aleja mucho
		state->add_transition("Waiting", "ShotAttack", [state_cm, _p_tr, tr, dist_to_attack]()
							  {
			uint32_t currentTime = sdlutils().virtualTimer().currTime();

			bool trans = state_cm->can_use("shot_attack", currentTime) && state_cm->is_player_near(_p_tr, tr, dist_to_attack);
			if (trans) { state_cm->reset_cooldown("shot_attack", currentTime); };
			return trans; });

		state->add_transition("ShotAttack", "Waiting", []()
							  { return true; });

		// spawn
		state->add_transition("Waiting", "SpawnMichi", [state_cm]()
							  {
			uint32_t currentTime = sdlutils().virtualTimer().currTime();
			bool trans = state_cm->can_use("spawn_michi", currentTime);
			if (trans) {
				state_cm->reset_cooldown("spawn_michi", currentTime);
			}
			return trans; });
		state->add_transition("SpawnMichi", "Waiting", []()
							  { return true; });

		// patron 3
		state->add_transition("Waiting", "LargeAreaAttack", [state_cm, _p_tr, tr]()
							  {
			uint32_t currentTime = sdlutils().virtualTimer().currTime();
			bool trans = state_cm->can_use("large_area_attack_duration", currentTime) && state_cm->is_player_near(_p_tr, tr, 2.0f);;
			if (trans) {
				state_cm->reset_cooldown("large_area_attack_duration", currentTime);
			}
			return trans; });

		state->add_transition("LargeAreaAttack", "Waiting", []()
							  { return true; });

		// A walking si el player se aleja
		state->add_transition("Waiting", "Walking", [state_cm, _p_tr, tr, dist_to_attack]()
							  { return !state_cm->is_player_near(_p_tr, tr, dist_to_attack * 1.2); });

		state->add_transition("SpawnMichi", "Walking", [state_cm, _p_tr, tr, dist_to_attack]()
							  { return !state_cm->is_player_near(_p_tr, tr, dist_to_attack * 1.2); });

		state->add_transition("LargeAreaAttack", "Walking", [state_cm, _p_tr, tr, dist_to_attack]()
							  { return !state_cm->is_player_near(_p_tr, tr, dist_to_attack * 1.2); });

		state->add_transition("ShotAttack", "Walking", [state_cm, _p_tr, tr, dist_to_attack]()
							  { return !state_cm->is_player_near(_p_tr, tr, dist_to_attack * 1.2); });

		state->add_transition("AreaAttack", "Walking", [state_cm, _p_tr, tr, dist_to_attack]()
							  { return !state_cm->is_player_near(_p_tr, tr, dist_to_attack * 1.2); });

		// Estado inicial
		state->set_initial_state("Walking");
		return Game::Instance()->get_mngr()->getComponent<id_component>(e)->getId();
	}
	else
	{
		dumb_enemy(ec, scene);
	}
	return 0;
}
#pragma endregion

#pragma region Catkuza
uint8_t GameScene::spawn_catkuza(Vector2D posVec, ecs::sceneId_t scene, uint8_t _id)
{
	auto &&manager = *Game::Instance()->get_mngr();
	auto &&weapon = *new WeaponCatKuza();

	auto ec = GameStructs::EnemyProperties{
		"catkuza",
		_id,
		posVec,
		GameStructs::DEFAULT,
		30,
		1.8f,
		2.5f,
		GameStructs::LOWREST_LIFE,
		{0.0f, 0.0f},
		0.0f,
		2.0f};

	if (Game::Instance()->is_host() || Game::Instance()->is_network_none())
	{

		auto e = create_enemy(
			ec,
			scene,
			static_cast<Weapon *>(&weapon));

		Transform *tr = manager.getComponent<Transform>(e);
		MovementController *mc = manager.getComponent<MovementController>(e);
		mc->set_max_speed(0.04);
		StateMachine *state = manager.getComponent<StateMachine>(e);

		Follow *fll = manager.getComponent<Follow>(e);
		fll->act_follow();
		auto state_cm = state->getConditionManager();

		state_cm->set_cooldown("wind_attack_duration", 1000);
		state_cm->set_cooldown("charging_duration", 500);
		state_cm->set_cooldown("dash_attack_duration", 1000);
		state_cm->set_cooldown("explosion_attack_duration", 800);
		state_cm->set_cooldown("delayed_slash_duration", 1200);

		state_cm->add_pattern("PATTERN_1", 1);
		state_cm->add_pattern("PATTERN_2", 1);

		// Crear estados
		auto walkingState = std::make_shared<WalkingState>(tr, mc, fll);
		auto dashState = std::make_shared<DashingState>(tr, mc, fll);
		auto chargingState = std::make_shared<WaitingState>();

		auto windAttackState = std::make_shared<AttackingState>(
			tr, fll, &weapon, false,
			std::function<bool()>([&weapon, tr]() -> bool
								  {
			weapon.wind_attack(tr->getPos());
			return false; }));

		auto areaAttackState = std::make_shared<AttackingState>(
			tr, fll, &weapon, false,
			std::function<bool()>([&weapon, tr]() -> bool
								  {
			weapon.area_attack(tr->getPos());
			return false; }));

		auto dashAttackState = std::make_shared<AttackingState>(
			tr, fll, &weapon, false,
			std::function<bool()>([&weapon, tr, fll, mc]() -> bool
								  {
			Vector2D shootPos = tr->getPos();
			Vector2D shootDir = (fll->get_act_follow()->getPos() - shootPos).normalize();
			Vector2D dash_target = fll->get_act_follow()->getPos() + shootDir * 1.8f;
			weapon.dash_attack(shootPos, dash_target);

			return false; }));

		auto waitingState = std::make_shared<WaitingState>();

		// poner los estado a la state
		state->add_state("Walking", walkingState);
		state->add_state("Charging", chargingState);
		state->add_state("Dash", dashState);
		state->add_state("Dash2", dashState);
		state->add_state("Dash3", dashState);
		state->add_state("WindAttack", windAttackState);
		state->add_state("WindAttack2", windAttackState);
		state->add_state("DashAttack", dashAttackState);
		state->add_state("AreaAttack", areaAttackState);
		state->add_state("Waiting", waitingState);

		// Transiciones Patrón 1
		state->add_transition("Walking", "Charging",
							  [state_cm, fll, tr, &weapon]()
							  {
								  bool trans = state_cm->is_player_near(fll->get_act_follow(), tr, 5.0f) && state_cm->get_current_pattern() == "PATTERN_1";
								  if (trans)
								  {
									  state_cm->reset_cooldown("charging_duration", sdlutils().currRealTime());
									  weapon.set_player_pos(fll->get_act_follow()->getPos());
								  }
								  return trans;
							  });

		state->add_transition("Charging", "WindAttack",
							  [state_cm, &weapon, fll]()
							  {
								  bool trans = state_cm->can_use("charging_duration", sdlutils().currRealTime());
								  if (trans)
								  {
									  state_cm->reset_cooldown("wind_attack_duration", sdlutils().currRealTime());
									  weapon.set_player_pos(fll->get_act_follow()->getPos());
								  }
								  return trans;
							  });

		state->add_transition("WindAttack", "Dash",
							  [state_cm, &weapon, fll]()
							  {
								  bool trans = state_cm->can_use("wind_attack_duration", sdlutils().currRealTime());
								  if (trans)
								  {
									  state_cm->reset_cooldown("dash_attack_duration", sdlutils().currRealTime());

									  weapon.set_player_pos(fll->get_act_follow()->getPos());
								  }
								  return trans;
							  });

		state->add_transition("Dash", "WindAttack2",
							  [state_cm]()
							  {
								  bool trans = state_cm->can_use("dash_attack_duration", sdlutils().currRealTime());
								  if (trans)
								  {
									  state_cm->reset_cooldown("wind_attack_duration", sdlutils().currRealTime());
								  }
								  return trans;
							  });

		state->add_transition("WindAttack2", "Walking",
							  [state_cm]()
							  {
								  bool trans = state_cm->can_use("wind_attack_duration", sdlutils().currRealTime());
								  if (trans)
								  {
									  state_cm->reset_cooldown("wind_attack_duration", sdlutils().currRealTime());
									  state_cm->switch_pattern();
								  }
								  return trans;
							  });

		// Transiciones Patrón 2
		state->add_transition("Walking", "Dash2",
							  [state_cm, fll, &weapon]()
							  {
								  bool trans = state_cm->get_current_pattern() == "PATTERN_2" && state_cm->can_use("dash_attack_duration", sdlutils().currRealTime());
								  if (trans)
								  {
									  state_cm->reset_cooldown("dash_attack_duration", sdlutils().currRealTime());
									  weapon.set_player_pos(fll->get_act_follow()->getPos());
								  }
								  return trans;
							  });

		state->add_transition("Dash2", "AreaAttack",
							  [state_cm]()
							  {
								  bool trans = state_cm->can_use("dash_attack_duration", sdlutils().currRealTime());
								  if (trans)
								  {
									  state_cm->reset_cooldown("explosion_attack_duration", sdlutils().currRealTime());
								  }
								  return trans;
							  });

		state->add_transition("AreaAttack", "Dash3",
							  [state_cm, fll, &weapon]()
							  {
								  bool trans = state_cm->can_use("explosion_attack_duration", sdlutils().currRealTime());
								  if (trans)
								  {
									  state_cm->reset_cooldown("dash_attack_duration", sdlutils().currRealTime());
									  weapon.set_player_pos(fll->get_act_follow()->getPos());
								  }
								  return trans;
							  });

		state->add_transition("Dash3", "DashAttack",
							  [state_cm]()
							  {
								  state_cm->reset_cooldown("dash_attack_duration", sdlutils().currRealTime());
								  return true;
							  });

		state->add_transition("DashAttack", "Walking",
							  [state_cm]()
							  {
								  bool trans = state_cm->can_use("dash_attack_duration", sdlutils().currRealTime());
								  if (trans)
								  {
									  state_cm->reset_cooldown("dash_attack_duration", sdlutils().currRealTime());
									  state_cm->switch_pattern();
								  }
								  return trans;
							  });

		// Estado inicial
		state->set_initial_state("Walking");
		return Game::Instance()->get_mngr()->getComponent<id_component>(e)->getId();
	}
	else
	{
		dumb_enemy(ec, scene);
	}
	return 0;
}
#pragma endregion

#pragma region Sarno Rata
uint8_t GameScene::spawn_sarno_rata(Vector2D posVec, ecs::sceneId_t scene, uint8_t _id)
{
	auto &&manager = *Game::Instance()->get_mngr();

	int random_strategy = sdlutils().rand().nextInt(0, 3);
	GameStructs::EnemyProperties ec =
		GameStructs::EnemyProperties{
			"sarno_rata", // sprite_key
			_id,
			posVec,													// start_pos
			GameStructs::DEFAULT,									// enemy_type
			7,														// health
			1.125f,													// width
			1.5f,													// height
			static_cast<GameStructs::EnemyFollow>(random_strategy), // target_strategy
			{0.0f, 0.0f},											// velocity
			0.0f,													// rotation
			1.0f													// scale
		};

	if (Game::Instance()->is_host() || Game::Instance()->is_network_none())
	{
		auto &&weapon = *new WeaponSarnoRata();

		auto e = create_enemy(
			ec,
			scene,
			static_cast<Weapon *>(&weapon));

		Transform *tr = manager.getComponent<Transform>(e);
		MovementController *mc = manager.getComponent<MovementController>(e);
		mc->set_max_speed(0.08);
		StateMachine *state = manager.getComponent<StateMachine>(e);

		Follow *fll = manager.getComponent<Follow>(e);
		fll->act_follow();

		auto state_cm = state->getConditionManager();

		auto walkingState = std::make_shared<WalkingState>(tr, mc, fll);
		auto attackingState = std::make_shared<AttackingState>(tr, fll, &weapon);

		state->add_state("Walking", walkingState);
		state->add_state("Attacking", attackingState);

		state->add_transition("Walking", "Attacking", [state_cm, fll, tr]()
							  { return state_cm->is_player_near(fll->get_act_follow(), tr, 1.5f); });

		state->add_transition("Attacking", "Walking", [state_cm, fll, tr]()
							  { return !state_cm->is_player_near(fll->get_act_follow(), tr, 1.3f); });

		state->set_initial_state("Walking");

		online_enemy(e);
		return Game::Instance()->get_mngr()->getComponent<id_component>(e)->getId();
	}
	else
	{
		dumb_enemy(ec, scene);
	}
	return 0;
}
#pragma endregion

#pragma region Michi Mafioso
uint8_t GameScene::spawn_michi_mafioso(Vector2D posVec, ecs::sceneId_t scene, uint8_t _id)
{
	auto &&manager = *Game::Instance()->get_mngr();
	GameStructs::EnemyProperties ec =
		GameStructs::EnemyProperties{
			"michi_mafioso", // sprite_key
			_id,
			posVec,				   // start_pos
			GameStructs::DEFAULT,  // enemy_type
			5,					   // health
			1.0f,				   // width
			1.125f,				   // height
			GameStructs::FURTHEST, // target_strategy
			{0.0f, 0.0f},		   // velocity
			0.0f,				   // rotation
			2.0f				   // scale
		};

	if (Game::Instance()->is_host() || Game::Instance()->is_network_none())
	{
		auto &&weapon = *new WeaponMichiMafioso();
		auto e = create_enemy(
			ec,
			scene,
			static_cast<Weapon *>(&weapon));

		Transform *tr = manager.getComponent<Transform>(e);
		MovementController *mc = manager.getComponent<MovementController>(e);
		mc->set_max_speed(0.05);
		StateMachine *state = manager.getComponent<StateMachine>(e);
		auto state_cm = state->getConditionManager();

		Follow *fll = manager.getComponent<Follow>(e);
		fll->act_follow();

		auto walkingState = std::make_shared<WalkingState>(tr, mc, fll);
		auto backingState = std::make_shared<WalkingState>(tr, mc, fll, true);
		auto attackingState = std::make_shared<AttackingState>(tr, fll, &weapon);

		state->add_state("Walking", walkingState);
		state->add_state("Attacking", attackingState);
		state->add_state("Backing", backingState);

		float dist_to_attack = 5.0f;
		float dist_to_fallback = 4.5f;

		state->add_transition("Walking", "Attacking", [state_cm, fll, tr, dist_to_attack]()
							  { return state_cm->is_player_near(fll->get_act_follow(), tr, dist_to_attack); });

		state->add_transition("Attacking", "Walking", [state_cm, fll, tr, dist_to_attack]()
							  { return !state_cm->is_player_near(fll->get_act_follow(), tr, dist_to_attack); });

		state->add_transition("Walking", "Backing", [state_cm, fll, tr, dist_to_fallback]()
							  { return state_cm->is_player_near(fll->get_act_follow(), tr, dist_to_fallback); });

		state->add_transition("Backing", "Walking", [state_cm, fll, tr, dist_to_fallback, dist_to_attack]()
							  { return !state_cm->is_player_near(fll->get_act_follow(), tr, dist_to_fallback) &&
									   !state_cm->is_player_near(fll->get_act_follow(), tr, dist_to_attack); });

		state->add_transition("Attacking", "Backing", [state_cm, fll, tr, dist_to_fallback]()
							  { return state_cm->is_player_near(fll->get_act_follow(), tr, dist_to_fallback); });

		state->add_transition("Backing", "Attacking", [state_cm, fll, tr, dist_to_fallback]()
							  { return !state_cm->is_player_near(fll->get_act_follow(), tr, dist_to_fallback); });

		state->set_initial_state("Walking");
		return Game::Instance()->get_mngr()->getComponent<id_component>(e)->getId();
	}
	else
	{
		dumb_enemy(ec, scene);
	}
	return 0;
}
#pragma endregion

#pragma region Plim Plim
uint8_t GameScene::spawn_plim_plim(Vector2D posVec, ecs::sceneId_t scene, uint8_t _id)
{
	auto &&manager = *Game::Instance()->get_mngr();

	GameStructs::EnemyProperties ec =
		GameStructs::EnemyProperties{
			"plim_plim", // sprite_key
			_id,
			posVec,				  // start_pos
			GameStructs::DEFAULT, // enemy_type
			4,					  // health
			1.0f,				  // width
			1.0f,				  // height
			GameStructs::CLOSEST, // target_strategy
			{0.0f, 0.0f},		  // velocity
			0.0f,				  // rotation
			2.0f				  // scale
		};

	if (Game::Instance()->is_host() || Game::Instance()->is_network_none())
	{
		auto &&weapon = *new WeaponPlimPlim();
		auto e = create_enemy(
			ec,
			scene,
			static_cast<Weapon *>(&weapon));

		Transform *tr = manager.getComponent<Transform>(e);
		MovementController *mc = manager.getComponent<MovementController>(e);
		mc->set_max_speed(0.06);
		StateMachine *state = manager.getComponent<StateMachine>(e);
		auto state_cm = state->getConditionManager();

		Follow *fll = manager.getComponent<Follow>(e);
		fll->act_follow();

		auto walkingState = std::make_shared<WalkingState>(tr, mc, fll);
		auto attackingState = std::make_shared<AttackingState>(tr, fll, &weapon);

		state->add_state("Walking", walkingState);
		state->add_state("Attacking", attackingState);

		state->add_transition("Walking", "Attacking", [state_cm, fll, tr]()
							  { return state_cm->is_player_near(fll->get_act_follow(), tr, 4.0f); });

		state->add_transition("Attacking", "Walking", [state_cm, fll, tr]()
							  { return !state_cm->is_player_near(fll->get_act_follow(), tr, 6.0f); });

		state->set_initial_state("Walking");
		return Game::Instance()->get_mngr()->getComponent<id_component>(e)->getId();
	}
	else
	{
		dumb_enemy(ec, scene);
	}
	return 0;
}
#pragma endregion

#pragma region Boom
uint8_t GameScene::spawn_boom(Vector2D posVec, ecs::sceneId_t scene, uint8_t _id)
{
	auto &&manager = *Game::Instance()->get_mngr();
	GameStructs::EnemyProperties ec =
		GameStructs::EnemyProperties{
			"boom", // sprite_key
			_id,
			posVec,					   // start_pos
			GameStructs::DEFAULT,	   // enemy_type
			28,						   // health
			1.8f,					   // width
			1.8f,					   // height
			GameStructs::LOWREST_LIFE, // target_strategy
			{0.0f, 0.0f},			   // velocity
			0.0f,					   // rotation
			2.0f					   // scale
		};

	if (Game::Instance()->is_host() || Game::Instance()->is_network_none())
	{
		auto &&weapon = *new WeaponBoom();

		auto e = create_enemy(
			ec,
			scene,
			static_cast<Weapon *>(&weapon));

		Transform *tr = manager.getComponent<Transform>(e);
		MovementController *mc = manager.getComponent<MovementController>(e);
		mc->set_max_speed(0.03f);
		StateMachine *state = manager.getComponent<StateMachine>(e);
		auto state_cm = state->getConditionManager();
		Health *health = manager.getComponent<Health>(e);

		Follow *fll = manager.getComponent<Follow>(e);

		auto walkingState = std::make_shared<WalkingState>(tr, mc, fll);
		auto attackingState = std::make_shared<AttackingState>(tr, fll, &weapon, false,
															   std::function<bool()>([health]() -> bool
																					 { health->takeDamage(health->getMaxHealth());
		return true; }));

		state->add_state("Walking", walkingState);
		state->add_state("Attacking", attackingState);

		state->add_transition("Walking", "Attacking", [state_cm, fll, tr]()
							  { return state_cm->is_player_near(fll->get_act_follow(), tr, 1.0f); });

		state->set_initial_state("Walking");
		return Game::Instance()->get_mngr()->getComponent<id_component>(e)->getId();
	}
	else
	{
		dumb_enemy(ec, scene);
	}
	return 0;
}
#pragma endregion

#pragma region Ratatouille
uint8_t GameScene::spawn_ratatouille(Vector2D posVec, ecs::sceneId_t scene, uint8_t _id)
{
	auto &&manager = *Game::Instance()->get_mngr();
	int random_strategy = sdlutils().rand().nextInt(0, 3);

	GameStructs::EnemyProperties ec = GameStructs::EnemyProperties{
		"ratatouille", // sprite_key
		_id,
		posVec,													// start_pos
		GameStructs::DEFAULT,									// enemy_type (especial, no usa weapon)
		2,														// health
		0.8f,													// width
		0.8f,													// height
		static_cast<GameStructs::EnemyFollow>(random_strategy), // target_strategy
		{0.0f, 0.0f},											// velocity
		0.0f,													// rotation
		2.0f													// scale
	};

	if (Game::Instance()->is_host() || Game::Instance()->is_network_none())
	{
		int damage = 4;

		float randSize = float(sdlutils().rand().nextInt(6, 10)) / 10.0f;
		auto &&rect = *new rect_component{0, 0, ec.width * randSize, ec.height * randSize};
		auto &&rigidbody = *new rigidbody_component{rect_f32{{0.0f, -0.15f}, {0.5f, 0.6f}}, mass_f32{3.0f}, 0.05f};
		auto &&tr_a = *new Transform(ec.start_pos, ec.dir, ec.r, ec.s * randSize);
		auto &&fll = *new Follow(ec.follow);
		auto &&col = *new collisionable{tr_a, rigidbody, rect, collisionable_option_trigger};
		auto &&mc = *new MovementController(ec.max_speed * 1.4, ec.acceleration, ec.decceleration * deccel_spawned_creatures_multi);
		auto &&state = *new StateMachine();

		auto e = create_entity(
			ecs::grp::ENEMY,
			scene,
			&tr_a,
			&rect,
			new dyn_image(
				rect_f32{{0, 0}, {1, 1}},
				rect,
				manager.getComponent<camera_component>(manager.getHandler(ecs::hdlr::CAMERA))->cam,
				sdlutils().images().at(ec.sprite_key),
				tr_a),
			new Health(ec.health),
			new FlipXController(),
			new enemy_collision_triggerer(),
			new id_component(),
			&rigidbody,
			&col,
			&mc,
			&state,
			&fll);

		manager.addComponent<ratatouille_collision_component>(e, damage, 2);
		online_enemy(e);

		if (scene == ecs::scene::GAMESCENE)
			Game::Instance()->get_mngr()->getComponent<WaveManager>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::WAVE))->newEnemy();

		auto state_cm = state.getConditionManager();
		Transform *tr = manager.getComponent<Transform>(e);

		fll.act_follow();

		auto walkingState = std::make_shared<WalkingState>(tr, &mc, &fll);
		auto rotatingState = std::make_shared<RotatingState>(tr, &fll, &mc);

		state.add_state("Walking", walkingState);
		state.add_state("Rotating", rotatingState);

		float dist_to_rotate = 3.5f;

		// Condiciones de cada estado
		// De: Walking a: Rotating, Condición: Jugador cerca
		state.add_transition("Walking", "Rotating", [state_cm, fll, tr, dist_to_rotate]()
							 { return state_cm->is_player_near(fll.get_act_follow(), tr, dist_to_rotate); });

		// De: Rotating a: Walking, Condición: Jugador lejos
		state.add_transition("Rotating", "Walking", [state_cm, fll, tr, dist_to_rotate]()
							 { return !state_cm->is_player_near(fll.get_act_follow(), tr, dist_to_rotate * 1.8f); });

		state.set_initial_state("Walking");
		return Game::Instance()->get_mngr()->getComponent<id_component>(e)->getId();
	}
	else
	{
		dumb_enemy(ec, scene);
	}

	Game::Instance()->get_mngr()->getComponent<WaveManager>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::WAVE))->newEnemy();
	return 0;
}
#pragma endregion

#pragma region Rata Basurera
uint8_t GameScene::spawn_rata_basurera(Vector2D posVec, ecs::sceneId_t scene, uint8_t _id)
{
	auto &&manager = *Game::Instance()->get_mngr();

	GameStructs::EnemyProperties ec =
		GameStructs::EnemyProperties{
			"basurero", // sprite_key
			_id,
			posVec,				  // start_pos
			GameStructs::DEFAULT, // enemy_type
			16,					  // health
			2.0f,				  // width
			2.2f,				  // height
			GameStructs::CLOSEST, // target_strategy
			{0.0f, 0.0f},		  // velocity
			0.0f,				  // rotation
			2.0f				  // scale
		};

	if (Game::Instance()->is_host() || Game::Instance()->is_network_none())
	{
		auto &&weapon = *new WeaponRataBasurera();

		auto e = create_enemy(
			ec,
			scene,
			static_cast<Weapon *>(&weapon));

		Transform *tr = manager.getComponent<Transform>(e);
		MovementController *mc = manager.getComponent<MovementController>(e);
		StateMachine *state = manager.getComponent<StateMachine>(e);
		Health *ht = manager.getComponent<Health>(e);

		Follow *fll = manager.getComponent<Follow>(e);
		fll->act_follow();

		auto state_cm = state->getConditionManager();

		// Configuración especial para Rata Basurera
		weapon.sendHealthComponent(manager.getComponent<Health>(e));

		auto walkingState = std::make_shared<WalkingState>(tr, mc, fll);
		auto attackingState = std::make_shared<AttackingState>(tr, fll, &weapon, true, std::function<bool()>([ht, tr]() -> bool
																											 {
		if(ht->getHealth() <= ht->getMaxHealth()/2){
			spawn_rey_basurero(tr->getPos() + Vector2D{0.5f, 0.5f});
			return true;
		}
		return false; }));

		state->add_state("Walking", walkingState);
		state->add_state("Attacking", attackingState);

		// Condiciones de cada estado
		// De: Walking a: Attacking, Condición: Jugador a distancia correcta
		state->add_transition("Walking", "Attacking", [state_cm, fll, tr]()
							  { return state_cm->is_player_near(fll->get_act_follow(), tr, 50.0f); });

		state->set_initial_state("Walking");
		return Game::Instance()->get_mngr()->getComponent<id_component>(e)->getId();
	}
	else
	{
		dumb_enemy(ec, scene);
		return 0;
	}
}
#pragma endregion

#pragma region Rey Basurero
uint8_t GameScene::spawn_rey_basurero(Vector2D posVec, ecs::sceneId_t scene, uint8_t _id)
{
	auto &&manager = *Game::Instance()->get_mngr();
	int random_strategy = sdlutils().rand().nextInt(0, 3);
	GameStructs::EnemyProperties ec =
		GameStructs::EnemyProperties{
			"rey_basurero", // sprite_key
			_id,
			posVec,													// start_pos
			GameStructs::DEFAULT,									// enemy_type
			14,														// health
			2.0f,													// width
			2.0f,													// height
			static_cast<GameStructs::EnemyFollow>(random_strategy), // target_strategy
			{0.0f, 0.0f},											// velocity
			0.0f,													// rotation
			1.0f													// scale
		};

	if (Game::Instance()->is_host() || Game::Instance()->is_network_none())
	{
		auto &&weapon = *new WeaponReyBasurero();

		auto e = create_enemy(
			ec,
			scene,
			static_cast<Weapon *>(&weapon));

		Transform *tr = manager.getComponent<Transform>(e);
		MovementController *mc = manager.getComponent<MovementController>(e);
		StateMachine *state = manager.getComponent<StateMachine>(e);
		auto state_cm = state->getConditionManager();

		Follow *fll = manager.getComponent<Follow>(e);
		fll->act_follow();

		auto walkingState = std::make_shared<WalkingState>(tr, mc, fll);
		auto backingState = std::make_shared<WalkingState>(tr, mc, fll, true);
		auto attackingState = std::make_shared<AttackingState>(tr, fll, &weapon);

		state->add_state("Walking", walkingState);
		state->add_state("Attacking", attackingState);
		state->add_state("Backing", backingState);

		float dist_to_attack = 3.0f;
		float dist_to_fallback = 2.5f;

		state->add_transition("Walking", "Attacking", [state_cm, fll, tr, dist_to_attack]()
							  { return state_cm->is_player_near(fll->get_act_follow(), tr, dist_to_attack); });

		state->add_transition("Attacking", "Walking", [state_cm, fll, tr, dist_to_attack]()
							  { return !state_cm->is_player_near(fll->get_act_follow(), tr, dist_to_attack); });

		state->add_transition("Walking", "Backing", [state_cm, fll, tr, dist_to_fallback]()
							  { return state_cm->is_player_near(fll->get_act_follow(), tr, dist_to_fallback); });

		state->add_transition("Backing", "Walking", [state_cm, fll, tr, dist_to_fallback, dist_to_attack]()
							  { return !state_cm->is_player_near(fll->get_act_follow(), tr, dist_to_fallback) &&
									   !state_cm->is_player_near(fll->get_act_follow(), tr, dist_to_attack); });

		state->add_transition("Attacking", "Backing", [state_cm, fll, tr, dist_to_fallback]()
							  { return state_cm->is_player_near(fll->get_act_follow(), tr, dist_to_fallback); });

		state->add_transition("Backing", "Attacking", [state_cm, fll, tr, dist_to_fallback]()
							  { return !state_cm->is_player_near(fll->get_act_follow(), tr, dist_to_fallback); });

		state->set_initial_state("Walking");
		return Game::Instance()->get_mngr()->getComponent<id_component>(e)->getId();
	}
	else
	{
		dumb_enemy(ec, scene);
	}
	return 0;
}
#pragma endregion

#pragma region Hud
ecs::entity_t GameScene::create_hud(ecs::sceneId_t scene)
{
	auto ent = create_entity(
		ecs::grp::DEFAULT,
		scene,
		new HUD());
	return ent;
}
#pragma endregion

#pragma region Waves
void GameScene::spawn_wave_manager()
{
	ecs::entity_t ent;
	if (Game::Instance()->is_host() || Game::Instance()->is_network_none())
	{
		auto wmf = new WaveManager();
		ent = create_entity(
			ecs::grp::DEFAULT,
			ecs::scene::GAMESCENE,
			wmf);
		Game::Instance()->get_wave_manager() = wmf;
	}
	else
	{
		auto wmf = new DumbWaveManager();
		ent = create_entity(
			ecs::grp::DEFAULT,
			ecs::scene::GAMESCENE,
			wmf);
		Game::Instance()->get_wave_manager() = wmf;
	}
	Game::Instance()->get_mngr()->setHandler(ecs::hdlr::WAVE, ent);
}
void GameScene::spawn_fog()
{
	auto &&transform = *new Transform({0.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, 1.0f);
	auto &&rect = *new rect_component{0.0f, 0.0f, 35.0f, 20.0f};
	dyn_image *this_fog_image = new dyn_image(
		rect_f32{{0, 0}, {1, 1}},
		rect,
		Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam,
		sdlutils().images().at("fog"),
		transform);

	auto &&fog_rigidbody = *new rigidbody_component{rect_f32{{0.0f, 0.0f}, {1.0f, 1.0f}}, mass_f32{1.0f}, 0.0f};
	auto &&fog_collisionable = *new collisionable{transform, fog_rigidbody, rect, collisionable_option_trigger};

	Fog *this_fog = new Fog();
	// La entidad tiene un grupo, una escena, un Transform, rect_component, un Fog y un dyn_image
	auto ent = create_entity(
		ecs::grp::DEFAULT,
		ecs::scene::GAMESCENE,
		&transform,
		&rect,
		this_fog,
		this_fog_image,
		&fog_rigidbody,
		&fog_collisionable,
		new fog_collision_component()

	);
	Game::Instance()->get_mngr()->setHandler(ecs::hdlr::FOGGROUP, ent);
}
#pragma endregion

#pragma region Proyectile
ecs::entity_t GameScene::generate_proyectile(const GameStructs::BulletProperties &bp, ecs::grpId_t gid, ecs::sceneId_t scene)
{
	auto manager = Game::Instance()->get_mngr();
	(void)gid;

	auto &&transform = *new Transform(bp.init_pos, bp.dir, (atan2(-bp.dir.getY(), bp.dir.getX()) + M_PI / 2) * 180.0f / M_PI, bp.speed);
	auto &&rect = *new rect_component{0, 0, bp.width, bp.height};
	auto &&player_rigidbody = *new rigidbody_component{rect_f32{{0.15f, -0.125}, {0.5f, 0.75f}}, mass_f32{7.0f}, 1.0f};
	auto &&player_collisionable = *new collisionable{transform, player_rigidbody, rect, collisionable_option_trigger};
	
	auto e = create_entity(
		gid,
		scene,
		&transform,
		&rect,
		new dyn_image(
			rect_f32{{0, 0}, {1, 1}},
			rect,
			manager->getComponent<camera_component>(manager->getHandler(ecs::hdlr::CAMERA))->cam,
			sdlutils().images().at(bp.sprite_key),
			transform),
		new LifetimeTimer(bp.life_time),
		new BulletData(bp.damage, bp.weapon_type),
		&player_rigidbody,
		&player_collisionable,
		new bullet_collision_component(bp));

	// if (bp.bitset != nullptr)
	// manager->addComponent<collision_registration_by_id>(e, bp.bitset);
	// else
	if (bp.collision_filter == GameStructs::collide_with::enemy || bp.collision_filter == GameStructs::collide_with::all)
		manager->addComponent<collision_registration_by_id>(e, bp.bitset);
	return e;
}
ecs::entity_t GameScene::create_proyectile(const GameStructs::BulletProperties &bp, ecs::grpId_t gid)
{
	return generate_proyectile(bp, gid);
}
#pragma endregion

void GameScene::event_callback0(const event_system::event_receiver::Msg &m)
{
	deccel_spawned_creatures_multi *= m.float_value;
}
void GameScene::event_callback1(const event_system::event_receiver::Msg &m)
{
	auto &&mngr = *Game::Instance()->get_mngr();
	deccel_spawned_creatures_multi = 1;
	mngr.getComponent<WaveManager>(mngr.getHandler(ecs::hdlr::WAVE))->reset_wave_manager();

	// si es multiplayer hay que hacer add del componente fantastma
	if (Game::Instance()->is_host() || Game::Instance()->is_client())
	{
		auto player = mngr.getHandler(ecs::hdlr::PLAYER);
		if (!mngr.hasComponent<GhostStateComponent>(player))
			mngr.addComponent<GhostStateComponent>(player);
	}
	else
	{
		// sino se resetea al jugador y pasa al gameOver
		reset_player();
		Game::Instance()->queue_scene(Game::GAMEOVER);
	}
}

void GameScene::host_handle_menssage(network_context &ctx)
{
	size_t count{0};
	while (SDLNet_CheckSockets(ctx.profile.host.clients_host_set, 0) > 0)
	{
		++count;
		if (SDLNet_SocketReady(ctx.profile.host.host_socket))
		{
			network_message_dynamic_pack dyn_message = network_message_dynamic_pack_receive(ctx.profile.host.host_socket);
			const uint16_t type_n{dyn_message->header.type_n};
			const uint16_t type_h{SDLNet_Read16(&type_n)};
			std::cout << "message: received message to host socket of type: " << type_h << std::endl;
			assert(false && "error: host socket should not receive messages in this scene");
			std::exit(EXIT_FAILURE);
		}

		for (network_connection_size i = 0; i < ctx.profile.host.sockets_to_clients.connection_count; ++i)
		{
			TCPsocket &connection = ctx.profile.host.sockets_to_clients.connections[i];
			if (SDLNet_SocketReady(connection))
			{
				network_message_dynamic_pack dyn_message = network_message_dynamic_pack_receive(connection);
				const uint16_t type_n{dyn_message->header.type_n};
				const uint16_t type_h{SDLNet_Read16(&type_n)};
				switch (type_h)
				{
				case network_message_type_player_update:
				{
					auto message = network_message_dynamic_pack_into<network_message_player_update>(std::move(dyn_message));
					auto &&payload = message->payload.content;

					GameStructs::NetPlayerData playerData;
					playerData.id = SDLNet_Read32(&payload.player_id_n);

					uint32_t key_length = SDLNet_Read32(&payload.sprite_key_length);
					playerData.sprite_key.assign(payload.sprite_key, key_length);

					key_length = SDLNet_Read32(&payload.anim_key_length);
					playerData.current_anim.assign(payload.anim_key, key_length);

					playerData.health = SDLNet_Read16(&payload.health_n);
					playerData.is_ghost = SDLNet_Read16(&payload.is_ghost_n);
					playerData.pos.setX(static_cast<int32_t>(SDLNet_Read32(&payload.pos_n[0])) / static_cast<float>(fact_float_int));
					playerData.pos.setY(static_cast<int32_t>(SDLNet_Read32(&payload.pos_n[1])) / static_cast<float>(fact_float_int));

					auto &&game = *Game::Instance();
					const Game::network_users_state &state = game.get_network_state();
					auto player = state.game_state.user_players.at(playerData.id);
					game.get_mngr()->getComponent<PlayerSynchronize>(player)->updatePlayer(playerData);

					auto player_update_msg = create_player_update_message(playerData);

					for (network_connection_size j = 0; j < ctx.profile.host.sockets_to_clients.connection_count; ++j)
					{
						TCPsocket &client = ctx.profile.host.sockets_to_clients.connections[j];

						if (client != connection)
						{
							network_message_pack_send(
								client,
								network_message_pack_create(network_message_type_player_update, player_update_msg));
						}
					}
					break;
				}
				default:
				{
					break;
				}
				}
				// TODO: listen to custom messages
			}
		}
	}
	(void)count;
	// std::cout << "message: host received " << count << " messages" << std::endl;
}

void GameScene::client_handle_menssage(network_context &ctx)
{
	// std::unordered_map<uint8_t, GameStructs::DumbEnemyProperties> latest_enemy_updates;

	size_t count{0};
	while (
		(SDLNet_CheckSockets(ctx.profile.client.client_set, 0) > 0) && SDLNet_SocketReady(ctx.profile.client.socket_to_host))
	{
		++count;
		auto dyn_message = network_message_dynamic_pack_receive(ctx.profile.client.socket_to_host);
		const uint16_t type_n{dyn_message->header.type_n};
		const uint16_t type_h{SDLNet_Read16(&type_n)};
		switch (type_h)
		{
		case network_message_type_player_update:
		{
			auto message = network_message_dynamic_pack_into<network_message_player_update>(std::move(dyn_message));
			auto &&payload = message->payload.content;

			GameStructs::NetPlayerData playerData;
			playerData.id = SDLNet_Read32(&payload.player_id_n);

			uint32_t key_length = SDLNet_Read32(&payload.sprite_key_length);
			playerData.sprite_key.assign(payload.sprite_key, key_length);

			key_length = SDLNet_Read32(&payload.anim_key_length);
			playerData.current_anim.assign(payload.anim_key, key_length);

			playerData.health = SDLNet_Read16(&payload.health_n);
			playerData.is_ghost = SDLNet_Read16(&payload.is_ghost_n);
			playerData.pos.setX(static_cast<int32_t>(SDLNet_Read32(&payload.pos_n[0])) / static_cast<float>(fact_float_int));
			playerData.pos.setY(static_cast<int32_t>(SDLNet_Read32(&payload.pos_n[1])) / static_cast<float>(fact_float_int));

			auto player = Game::Instance()->get_network_state().game_state.user_players.at(playerData.id);
			Game::Instance()->get_mngr()->getComponent<PlayerSynchronize>(player)->updatePlayer(playerData);

			break;
		}
		case network_message_type_create_enemy:
		{
			auto message = network_message_dynamic_pack_into<network_message_enemy_create>(std::move(dyn_message));
			auto &&payload = message->payload.content;

			GameStructs::DumbEnemyProperties _enemy_properties;
			_enemy_properties._id = SDLNet_Read16(&payload._enemy_id);
			_enemy_properties._type = SDLNet_Read16(&payload._type);

			_enemy_properties._pos.setX(static_cast<int32_t>(SDLNet_Read32(&payload._pos[0])) / static_cast<float>(fact_float_int));
			_enemy_properties._pos.setY(static_cast<int32_t>(SDLNet_Read32(&payload._pos[1])) / static_cast<float>(fact_float_int));
			std::cout << "Creando client enemigo con ID: " << (int)_enemy_properties._id << std::endl;

			select_create_dumb_enemy(_enemy_properties);
			break;
		}
		case network_message_type_enemy_update:
		{
			auto message = network_message_dynamic_pack_into<network_message_enemy_update>(std::move(dyn_message));
			auto &&payload = message->payload.content;

			uint8_t id = SDLNet_Read16(&payload._enemy_id);

			GameStructs::DumbEnemyProperties _enemy_properties;
			_enemy_properties._id = id;
			_enemy_properties._health = SDLNet_Read16(&payload._health_n);
			_enemy_properties._pos.setX(static_cast<int32_t>(SDLNet_Read32(&payload._pos[0])) / static_cast<float>(fact_float_int));
			_enemy_properties._pos.setY(static_cast<int32_t>(SDLNet_Read32(&payload._pos[1])) / static_cast<float>(fact_float_int));
			auto enemy = get_network_enemy(_enemy_properties._id);
			if (enemy != nullptr)
			{
				// std::cout << "Enemy ID en synchronize: " << (int)_enemy_properties._id << std::endl;
				Game::Instance()->get_mngr()->getComponent<EnemySynchronize>(enemy)->update_enemy(_enemy_properties);
			}
			else
				std::cout << "No se encontro el enemigo ID: " << (int)_enemy_properties._id << std::endl;
			break;
		}
		case network_message_type_start_wave:
		{
			auto message = network_message_dynamic_pack_into<network_message_start_wave>(std::move(dyn_message));
			auto &&payload = message->payload.content;
			dynamic_cast<DumbWaveManager *>(Game::Instance()->get_wave_manager())->start_wave(SDLNet_Read16(&payload.wave_event));

			auto mngr = Game::Instance()->get_mngr();
			auto player = mngr->getHandler(ecs::hdlr::PLAYER);
			if (mngr->hasComponent<GhostStateComponent>(player))
				mngr->removeComponent<GhostStateComponent>(player);
			break;
		}
		case network_message_type_end_wave:
		{
			auto message = network_message_dynamic_pack_into<network_message_end_wave>(std::move(dyn_message));
			auto &&payload = message->payload.content;
			dynamic_cast<DumbWaveManager *>(Game::Instance()->get_wave_manager())->end_wave();
			break;
		}
		default:
			break;
		}
	}
	(void)count;
	// std::cout << "message: client received " << count << " messages" << std::endl;

	// for (auto &[id, data] : latest_enemy_updates)
	//{
	//	auto enemy = get_network_enemy(id);
	//	if (enemy != nullptr)
	//	{
	//		//std::cout << "Enemy ID en synchronize: " << (int)id << std::endl;
	//		Game::Instance()->get_mngr()->getComponent<EnemySynchronize>(enemy)->update_enemy(data);
	//	}else
	//	{
	//		std::cout << "No se encontro el enemigo ID: " << (int)id << std::endl;
	//	}
	// }
}

bool GameScene::change_player_tex(uint32_t playerId, const std::string &key_name)
{
	auto player = Game::Instance()->get_network_state().game_state.user_players.at(playerId);
	auto &&manager = *Game::Instance()->get_mngr();
	if (auto &&dy = manager.getComponent<dyn_image_with_frames>(player))
	{
		dy->texture = &sdlutils().images().at(key_name);
		dy->texture_name = key_name;
		return true;
	}
	return false;
}

bool GameScene::change_player_filter(uint32_t playerId, filter filter)
{
	auto player = Game::Instance()->get_network_state().game_state.user_players.at(playerId);
	auto &&manager = *Game::Instance()->get_mngr();
	if (auto &&dy = manager.getComponent<dyn_image_with_frames>(player))
	{
		dy->_current_filter = filter;
		return true;
	}
	return false;
}

ecs::entity_t GameScene::create_dumb_player(ecs::sceneId_t scene, uint32_t playerId, std::string tex_name)
{
	auto &&manager = *Game::Instance()->get_mngr();
	auto &&camera = manager.getComponent<camera_component>(manager.getHandler(ecs::hdlr::CAMERA))->cam;

	auto &&player_transform = *new Transform({1.0f, 2.0f}, {0.0f, 0.0f}, 0.0f, 2.0f);
	auto &&player_rect = *new rect_component{0, 0, 1.125f, 1.5f};
	ecs::entity_t player = create_entity(
		ecs::grp::PLAYER,
		scene,
		&player_transform,
		new dyn_image_with_frames(
			rect_f32{{0, 0}, {0.2, 1}},
			player_rect,
			camera,
			sdlutils().images().at(tex_name),
			player_transform, tex_name),
		new render_ordering{1},
		new Health(100, true));

	auto *anim = manager.addComponent<AnimationComponent>(player);
	anim->add_animation("andar", 1, 5, 100);
	anim->add_animation("idle", 0, 0, 100);

	manager.addComponent<PlayerSynchronize>(player, playerId);

	// we already do it externally
	// Game::network_users_state &state = Game::Instance()->get_network_state();
	// network_state_add_user(state, player, std::move(tex_name));
	return player;
}
