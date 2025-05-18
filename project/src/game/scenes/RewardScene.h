#pragma once

#include "Scene.h"
#include <string>
#include <list>
#include <unordered_set>
#include <utility>


class Button;
class Card;
class ImageForButton;
class RewardScene : public Scene
{
public:
	RewardScene();
	virtual ~RewardScene();
	void initScene() override;
	void enterScene() override;
	void exitScene() override;
	void render() override;
	void update(uint32_t delta_time) override;
	static void will_have_mythic(bool state) { 
		_mythic = state; };
private:
    ecs::entity_t create_card_button(const GameStructs::CardButtonProperties& bp);
	//methods to show your actual deck info
	void create_my_deck_cards();
	void create_a_deck_card(const GameStructs::CardButtonProperties& bp);
	void refresh_my_deck_cards(const std::list<Card*>& cl);

	//deck cards part
	Card* _selected_card;
	ImageForButton* _last_deck_card_img;

	//reward part
	bool _heal;
	bool _activate_heal; //flag to save if we need to activate heal reward
	ImageForButton* _lr;

	bool _selected;
	bool _activate_confirm_button; //to add cards to deck
	bool _activate_exchange_button; //to exchange cards from deck
	bool _special_case; //when we only activate exchange
	ecs::entity_t _chosen_card;
	bool _exchange;

	static bool _mythic;
	//method to select a card randomly
	std::string select_card(GameStructs::CardType ct);
	std::pair<std::string, GameStructs::CardType> get_unique_card(std::unordered_set<std::string>& appeared_cards);

	//methods to create reward buttons
	void create_reward_card_button(const GameStructs::ButtonProperties& bp);
	void create_reward_health_button(const GameStructs::ButtonProperties& bp);
	void create_reward_buttons();
	void refresh_rewards();


	//method to change the position between health button and a card button
	void change_pos(bool enter);

	//method to create button to select card
	void create_reward_selected_button(const GameStructs::ButtonProperties& bp);

	//method to swap card if you got 8 or more in the deck
	void remove_deck_card();  
	void check_number();

	//method to create button to exchange card
	void create_reward_exchange_button(const GameStructs::ButtonProperties& bp);

	void add_new_reward_card();
	void create_next_round_button(const GameStructs::ButtonProperties& bp);

	//method to create a reward info (shown when hover)
	void create_reward_info();
};