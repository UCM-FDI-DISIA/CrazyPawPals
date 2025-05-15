#pragma once

#include <string>
#include <ostream>
#include <iostream>
#include <algorithm>
#include "../../utils/Vector2D.h"
#include "../components/cards/Deck.hpp"
enum Destination {
	DRAW_PILE,
	DISCARD_PILE,
	DESTROY
};
class Deck;

class Resources {
private:
	int _mana = 0;
	int _health = 0;
public:
	Resources() = default;
	Resources(int m, int h = 0) : _mana(m), _health(h) {}
	void modify_mana(int mana_mod) { set_mana(_mana + mana_mod); }
	void set_mana(int mana) { _mana = std::max(mana, 0); }
	int get_mana() const { return _mana; }
	void modify_health(int health_mod) { set_health(_health + health_mod); }
	void set_health(int health) { _health = std::max(health, 0); }
	int get_health() const { return _health; }
	Resources operator+(const Resources& res) const {
		return Resources(_mana+res._mana, _health+res._health);
	}

};

class Card {
protected:
	std::string _name;
	Resources _my_costs;
	Destination _play_destination;
	Destination _discard_destination;
	Destination _mill_destination;
	bool _can_be_replaced;
	// OPTIONAL ATTRIBUTES
	// These can be declared, modified and utilized within specific subclasses if necessary.
	// Optional attributes are custom-made and can be implemented in a multitude of ways.
	// Don't hesitate to make your own on the spot if needed.
	// 
	// int _boost = 0; // Unique counter for specific cards that are enhanced under certain conditions.
	// int _boost_threshold = 0; //Represents the value at which _boost enhances the card. 
public:
	//Card();
	Card(std::string = "default", Resources = Resources(0,0), Destination play = DISCARD_PILE, Destination discard = DISCARD_PILE, Destination mill = DISCARD_PILE);
	virtual ~Card(); 

	virtual Resources& get_costs();

	//bool can_play();
	virtual void on_play( Deck& d, const Vector2D* player_position, const Vector2D* target_position) = 0;
	virtual Card* on_mill(Deck& d, const Vector2D* player_position);

	//friend std::ostream& operator << (std::ostream& os, const Card& card);
	//Debug purposes
	virtual std::string get_written_info();
	inline Destination get_play_destination() { return _play_destination; }
	inline Destination get_discard_destination() { return _discard_destination; }
	inline Destination get_mill_destination() { return _mill_destination; }
	virtual void update(uint32_t dt) {
		(void)dt;
	};

	virtual std::string& get_name() {
		return _name;
	}
	inline bool can_be_replaced() const { return _can_be_replaced; };
};
