#pragma once

#include "Scene.h"
#include <vector>
#include <string>
#include <list>

class VictoryScene : public Scene
{
public:
	VictoryScene();
	virtual ~VictoryScene();
	void initScene() override;
	void enterScene() override;
	void exitScene() override;
	void render() override;

private:
	void create_enter_button();
	void create_exit_button();
};