#pragma once



#include "Scene.h"
#include <functional>
#include "../../network/network_message.hpp"
class Weapon;
class Transform;
class StateMachine;
struct EnemyProperties;
struct filter;

class GameScene : public event_system::event_receiver, public Scene
{
	[[maybe_unused]]

	static ecs::entity_t create_enemy(GameStructs::EnemyProperties& ec, ecs::sceneId_t scene, Weapon* weapon = nullptr);
	static void select_create_dumb_enemy(GameStructs::DumbEnemyProperties& ec);
	static ecs::entity_t dumb_enemy(GameStructs::EnemyProperties& ec, ecs::sceneId_t scene);
	static ecs::entity_t get_network_enemy(uint8_t _id);
	static void online_enemy(ecs::entity_t ec);


	void host_handle_menssage(network_context& ctx);
	void client_handle_menssage(network_context& ctx);

public:
	GameScene();
	~GameScene();
	void initScene() override;
	void enterScene() override;
	void exitScene() override;

	void update(uint32_t delta_time) override;
	//Methods of entities to spawn
	//Statics
	static ecs::entity_t create_player(ecs::sceneId_t scene = ecs::scene::GAMESCENE);
	static bool change_player_tex(const std::string& key_name);
	static bool change_player_tex(uint32_t playerId, const std::string& key_name);
	static bool change_player_filter(uint32_t playerId, filter filter);
	void reset_player();

	static uint8_t spawn_sarno_rata(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE,  uint8_t _id = -1);
	static uint8_t spawn_michi_mafioso(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE, uint8_t _id = -1);
	static uint8_t spawn_plim_plim(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE, uint8_t _id = -1);
	static uint8_t spawn_boom(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE, uint8_t _id = -1);
	static uint8_t spawn_ratatouille(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE, uint8_t _id = -1);
	static uint8_t spawn_rata_basurera(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE, uint8_t _id = -1);
	static uint8_t spawn_rey_basurero(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE, uint8_t _id = -1);

	static uint8_t spawn_super_michi_mafioso(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE, uint8_t _id = -1);
	static uint8_t spawn_catkuza(Vector2D posVec, ecs::sceneId_t scene = ecs::scene::GAMESCENE, uint8_t _id = -1);

	static ecs::entity_t create_hud(ecs::sceneId_t scene = ecs::scene::GAMESCENE);
	static ecs::entity_t create_environment(ecs::sceneId_t scene = ecs::scene::GAMESCENE);

	static ecs::entity_t generate_proyectile(const GameStructs::BulletProperties& bp, ecs::grpId_t gid, ecs::sceneId_t scene = ecs::scene::GAMESCENE);

	ecs::entity_t create_proyectile(const GameStructs::BulletProperties& bp, ecs::grpId_t gid) override; //para la escena de tutorial

	//
	void spawn_wave_manager();
	void spawn_fog();
	void event_callback0(const event_system::event_receiver::Msg& m) override;
	void event_callback1(const event_system::event_receiver::Msg& m) override;


	static ecs::entity_t create_dumb_player(ecs::sceneId_t scene, uint32_t playerId, std::string tex_name);
protected:

	static float deccel_spawned_creatures_multi;
};