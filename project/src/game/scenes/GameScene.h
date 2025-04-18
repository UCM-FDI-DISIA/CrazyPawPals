#pragma once

#include "Scene.h"
#include <functional>
class Weapon;
class Transform;
class StateMachine;
struct EnemySpawnConfig;

class GameScene : public event_system::event_receiver, public Scene
{
	[[maybe_unused]]

	ecs::entity_t create_enemy(EnemySpawnConfig&& ec);

public:
	GameScene();
	~GameScene();
	void initScene() override;
	void enterScene() override;
	void exitScene() override;

	//Methods of entities to spawn
	ecs::entity_t create_player();
	void reset_player();

	void spawn_sarno_rata(Vector2D posVec);
	void spawn_michi_mafioso(Vector2D posVec);
	void spawn_plim_plim(Vector2D posVec);
	void spawn_boom(Vector2D posVec);
	void spawn_catkuza(Vector2D posVec);
	void spawn_ratatouille(Vector2D posVec);
	void spawn_rata_basurera(Vector2D posVec);
	void spawn_rey_basurero(Vector2D posVec);
	void spawn_wave_manager();
	void create_hud();
	void spawn_fog();
	void spawn_super_michi_mafioso(Vector2D posVec);
	void generate_proyectile(const GameStructs::BulletProperties& bp, ecs::grpId_t gid);
	void event_callback0(const event_system::event_receiver::Msg& m) override;
	void event_callback1(const event_system::event_receiver::Msg& m) override;
protected:
	float deccel_spawned_creatures_multi = 1;
};