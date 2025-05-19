#pragma once

#include "Scene.h"

class Texture;
class ImageForButton;
class MultiplayerMenu : public Scene
{
public:
	MultiplayerMenu();
	~MultiplayerMenu();
	void initScene() override;
	void enterScene() override;
	void exitScene() override;
	void update(uint32_t delta_time);
	void render() override;
private:
	//Methods of entities to spawn
	ecs::entity_t create_edit_ip_button(const GameStructs::ButtonProperties& bp);
	void create_play_button(const GameStructs::ButtonProperties& bp);
	void create_host_button(const GameStructs::ButtonProperties& bp);
	void create_copy_ip_button(const GameStructs::ButtonProperties& bp);
	void create_client_button(const GameStructs::ButtonProperties& bp);
	void create_back_button(const GameStructs::ButtonProperties& bp);
	void create_skin_button(const GameStructs::ButtonProperties& bp, const std::string& tex_name);

	Texture ip_input;
	std::string _ipHost;
	bool input_field_has_focus;

	ImageForButton* _lastChosenSkin;
	ImageForButton* _playButton;
};