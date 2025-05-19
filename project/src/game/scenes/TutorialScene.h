#pragma once

#include "Scene.h"
#include <vector>
#include <string>
#include <list>
#include <functional>
class Deck;
class Health;
class TutorialScene : public event_system::event_receiver,public Scene
{
	enum class TutorialState { //para pasar de un pop up a otro
		NONE,
		NEXT_POP_UP,
		WAIT_FOR_ACTION,
		WAIT_FOR_DURATION,
		FINISHED
	};

	struct pop_up {
		std::string image_key; //png con el texto de explicacion
		std::function<bool()> condition;
		uint32_t duration;
		rect_f32 rect;
		std::function<void()> on_enter = nullptr;
	};
public:
	TutorialScene();
	virtual ~TutorialScene();
	void initScene() override;
	void enterScene() override;
	void exitScene() override;
	void update(uint32_t delta_time) override;
	void render() override;

	ecs::entity_t create_proyectile(const GameStructs::BulletProperties& bp, ecs::grpId_t gid) override;
	void event_callback0(const event_system::event_receiver::Msg& m) override;
private:
	bool has_pass_input();
	std::vector<pop_up> _pop_ups;
	int _current_pop_up;
	ecs::entity_t _current_pop_up_entity;

	Health* _player_health;

	uint32_t _popup_timer;
	TutorialState _tutorial_state;
	float _tutorial_time_seconds;
	bool _show_tutorial_hud;
	bool _enemy_killed = false;

	void create_pop_up();
	ecs::entity_t create_change_scene_button(const GameStructs::ButtonProperties& bp, Game::State nextScene);
};