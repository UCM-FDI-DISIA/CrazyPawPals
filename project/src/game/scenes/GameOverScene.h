#pragma once

#include "Scene.h"
#include <vector>
#include <string>
#include <list>

class GameOverScene : public Scene
{
public:
	GameOverScene();
	virtual ~GameOverScene();
	void initScene() override;
	void enterScene() override;
	void exitScene() override;
	void render() override;

private:
	void create_enter_button();
	void create_exit_button();
};