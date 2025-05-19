#pragma once

#include "Scene.h"
#include <vector>
#include <string>
#include <list>

class ImageForButton;
class Card;
using LastButton = ImageForButton*;
class SelectionMenuScene : public Scene
{
public:
	SelectionMenuScene();
	virtual ~SelectionMenuScene();
	void initScene() override;
	void enterScene() override;
	void exitScene() override;
	void render() override;
	void update(uint32_t delta_time) override;

private:
	void create_weapon_button(GameStructs::WeaponType wt, const GameStructs::ButtonProperties& bp);
	void create_deck_button(GameStructs::DeckType dt,const GameStructs::ButtonProperties& bp);
	void create_back_button(const GameStructs::ButtonProperties& bp);
	void create_weapon_buttons();
	void create_deck_buttons();
	void create_deck_info(const rect_f32& rect);
	void create_deck_infos();
	void set_concrete_deck_info(const std::list<Card*>& cl);
	void create_weapon_info();
	void create_enter_button();
	inline void weapon_selected() { _weapon_selected = true; };
	inline void deck_selected() { _deck_selected = true; };
	//flags to control if both has been selected
	bool _weapon_selected;
	bool _deck_selected;

	//last pressed button (visual)
	LastButton _last_weapon_button;
	LastButton _last_deck_button;

	int _num_cards_of_deck;

	bool _activate_play_button;

	void reset();
};