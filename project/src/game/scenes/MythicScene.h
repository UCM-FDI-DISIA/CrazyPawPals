#pragma once

#include "Scene.h"
#include <string>
#include <list>
#include <unordered_set>

#include <utility>

class Button;
class MythicItem;
class ImageForButton;
class MythicScene : public Scene
{
public:
	MythicScene();
	virtual ~MythicScene() {};
	void initScene() override;
	void enterScene() override;
	void exitScene() override;
	void render() override;
	void update(uint32_t delta_time) override;
private:
    ecs::entity_t create_mythic_button(const GameStructs::MythicButtonProperties& bp);
	//methods to show your actual deck info
	void create_my_mythics();
	void create_a_mythic(const GameStructs::MythicButtonProperties& bp);
	void refresh_my_mythics(const std::vector<MythicItem*>& cl);

	//my mythics part
	MythicItem* _selected_mythic;
	ImageForButton* _last_my_mythic_img;

	//reward part
	ImageForButton* _lm;

	bool _selected;
	bool _activate_confirm_button; //to add cards to deck
	ecs::entity_t _chosen_mythic;

	//method to select a mythic randomly
	std::string select_mythic(GameStructs::MythicType ct);
	std::pair<std::string, GameStructs::MythicType> get_unique_mythic(std::unordered_set<std::string>& appeared_mythics);

	//methods to create reward mythic buttons
	void create_reward_mythic_button(const GameStructs::ButtonProperties& bp);
	void create_reward_mythic_buttons();
	void refresh_mythics();

	//method to create button to select card
	void create_mythic_selected_button(const GameStructs::ButtonProperties& bp);

	void add_new_reward_mythic();

	void create_next_round_button(const GameStructs::ButtonProperties& bp);

	//method to create a reward info (shown when hover)
	void create_mythic_info();
};