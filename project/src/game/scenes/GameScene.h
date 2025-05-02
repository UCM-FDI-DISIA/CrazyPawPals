#pragma once

#include "Scene.h"
#include <functional>
class Weapon;
class Transform;
class StateMachine;
struct EnemyProperties;

class GameScene : public event_system::event_receiver, public Scene
{
	[[maybe_unused]]

	static ecs::entity_t create_enemy(GameStructs::EnemyProperties& ec, ecs::sceneId_t scene, Weapon* weapon = nullptr);
	static ecs::entity_t dumb_enemy(GameStructs::EnemyProperties& ec, ecs::sceneId_t scene);
	static void online_enemy(ecs::entity_t ec);

public:
	GameScene();
	~GameScene();
	void initScene() override;
	void enterScene() override;
	void exitScene() override;

	//Methods of entities to spawn
	//Statics
	static ecs::entity_t create_player(ecs::sceneId_t scene = ecs::scene::GAMESCENE);
	static bool change_player_tex(const std::string& key_name);
	void reset_player();

	static void spawn_sarno_rata(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE);
	static void spawn_michi_mafioso(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE);
	static void spawn_plim_plim(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE);
	static void spawn_boom(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE);
	static void spawn_ratatouille(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE);
	static void spawn_rata_basurera(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE);
	static void spawn_rey_basurero(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE);

	static void spawn_super_michi_mafioso(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE);
	static void spawn_catkuza(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE);

	static ecs::entity_t create_hud(ecs::sceneId_t scene = ecs::scene::GAMESCENE);
	static ecs::entity_t create_environment(ecs::sceneId_t scene = ecs::scene::GAMESCENE);

	static void generate_proyectile(const GameStructs::BulletProperties& bp, ecs::grpId_t gid, ecs::sceneId_t scene = ecs::scene::GAMESCENE);

	void create_proyectile(const GameStructs::BulletProperties& bp, ecs::grpId_t gid) override; //para la escena de tutorial

	//
	void spawn_wave_manager();
	void spawn_fog();
	void event_callback0(const event_system::event_receiver::Msg& m) override;
	void event_callback1(const event_system::event_receiver::Msg& m) override;
protected:
	static float deccel_spawned_creatures_multi;
};