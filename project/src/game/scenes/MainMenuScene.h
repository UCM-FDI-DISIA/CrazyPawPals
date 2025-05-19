#pragma once


#include "Scene.h"

class Texture;
class MainMenuScene : public Scene
{
public:
	MainMenuScene();
	~MainMenuScene();
	void initScene() override;
	void enterScene() override;
	void exitScene() override;
private:
	//Methods of entities to spawn
    void create_start_button(const GameStructs::ButtonProperties& bp);
    void create_decoration_images(const GameStructs::ButtonProperties& ip);
	void create_multiplayer_button(const GameStructs::ButtonProperties& bp);
	void create_controls_button(const GameStructs::ButtonProperties& bp);
	void create_exit_button(const GameStructs::ButtonProperties& bp);
};