#pragma once

#include "Scene.h"

class Texture;
class ControlsScene : public Scene
{
public:
	ControlsScene();
	~ControlsScene();
	void initScene() override;
	void enterScene() override;
	void exitScene() override;
	void render() override;
private:
	Texture* _background;
	//Methods of entities to spawn
	void create_mainmenu_button(const GameStructs::ButtonProperties& bp);

};