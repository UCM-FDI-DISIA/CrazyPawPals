#pragma once

#include <list>
#include <vector>
#include <algorithm>
#include <random>
#include <ostream>
class Card;

class CardList {
protected:
	std::list<Card*> _my_card_list;
public:
	CardList();
	~CardList();
	CardList(std::list<Card*>&);
	void move_from_this_to(CardList&);
	Card* add_card(Card*);
	void erase_card(std::list<Card*>::iterator);
	Card* pop_first();
	int number_of_cards();
	bool empty();
	void shuffle();
	void update(uint32_t);
	inline std::list<Card*>& card_list() { return _my_card_list; };
	friend std::ostream& operator << (std::ostream& os, const CardList& card_list);

};