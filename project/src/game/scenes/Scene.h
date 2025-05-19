#pragma once
#include <vector>
#include "../../ecs/Manager.h"
#include "../../ecs/Entity.h"
#include "../../ecs/Component.h"
#include "../GameStructs.h"
#include "../../ecs/ecs.h"
#include "../Game.h"

#include "../../network/network_utility.hpp"
#include "../../network/network_message.hpp"

class Transform;
class Texture;
class Scene
{
public:
	constexpr static const ::rect_f32 default_scene_bounds{
		.position = {0.0, 0.0},
		.size = {32.0, 18.0},
	};

	Scene(ecs::sceneId_t id);
	virtual ~Scene() {};
	// Each scene
	virtual void initScene() = 0;
	virtual void enterScene() = 0;
	virtual void exitScene() = 0;

	virtual void update(uint32_t delta_time);
	virtual void render();
	virtual ecs::entity_t create_proyectile(const GameStructs::BulletProperties &bp, ecs::grpId_t gid) { return nullptr; };

	template <typename... Cmps>
	static ecs::entity_t create_entity(ecs::grpId_t gid, ecs::sceneId_t sid, Cmps... components)
	{
		ecs::entity_t ent = Game::Instance()->get_mngr()->addEntity(sid, gid);
		Game::Instance()->get_mngr()->addExistingComponent(ent, components...);
		return ent;
	}

	struct rendering
	{
		enum camera_creation_descriptor_options
		{
			camera_creation_descriptor_options_none = 0,			 // 0
			camera_creation_descriptor_options_follow = 1 << 0,		 // 1
			camera_creation_descriptor_options_set_handler = 1 << 1, // 2
			camera_creation_descriptor_options_clamp = 1 << 2,		 // 4
		};
		using camera_creation_descriptor_flags = uint8_t;

#define CZPP_NULLABLE
		static ecs::entity_t create_camera(
			const ecs::sceneId_t scene,
			const camera_creation_descriptor_flags flags,
			CZPP_NULLABLE const Transform *optional_follow_target);
	};

protected:
	ecs::sceneId_t _scene_ID;

	ecs::entity_t create_button(const GameStructs::ButtonProperties &bp);
	ecs::entity_t create_decoration_image(const GameStructs::ButtonProperties &ip);
	void create_static_background(Texture *bg);

	bool checkAllPlayersReady();

	std::string message{};
	uint32_t message_time{5 * 1000};
	bool showing_message{false};

public:
	ecs::sceneId_t get_scene_id() const { return _scene_ID; }
	void show_message(const std::string &text, uint32_t time);
};