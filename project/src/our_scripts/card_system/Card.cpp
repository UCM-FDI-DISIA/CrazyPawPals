
#include "Card.hpp"
#include "../components/cards/Deck.hpp"



Card::Card(std::string card_name, Resources r,Destination pd, Destination dd, Destination md)
	:_name(card_name), _my_costs(r),_play_destination(pd), _discard_destination(dd), _mill_destination(md), _can_be_replaced(true)
{
}


Card::~Card() { }

Resources& Card::get_costs()
{
	return _my_costs;
}


Card* Card::on_mill(Deck& d, const Vector2D* player_position)
{
	(void)d;
	(void)player_position;
	// Mill effect
	return this;
}

void Card::on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) {
	(void)d;
	(void)player_position;
	(void)target_position;
	// Deploy effect
	
}

std::string Card::get_written_info()
{
	return std::to_string(_my_costs.get_mana()) + '-' + _name;
}