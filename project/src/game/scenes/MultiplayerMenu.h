#pragma once

#include "Scene.h"

class Texture;
class MultiplayerMenu : public Scene
{
public:
	MultiplayerMenu();
	~MultiplayerMenu();
	void initScene() override;
	void enterScene() override;
	void exitScene() override;
private:
	//Methods of entities to spawn
	void create_play_button(const GameStructs::ButtonProperties& bp);
	void create_host_button(const GameStructs::ButtonProperties& bp);
	void create_copy_ip_button(const GameStructs::ButtonProperties& bp);
	void create_client_button(const GameStructs::ButtonProperties& bp);
	void create_back_button(const GameStructs::ButtonProperties& bp);
	void create_skin_button(const GameStructs::ButtonProperties& bp);

};