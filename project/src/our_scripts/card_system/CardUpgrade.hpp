
#include "Card.hpp"
#include "../components/cards/Deck.hpp"


//Decorator Patron --> https://refactoring.guru/design-patterns/decorator/cpp/example
class BaseCardUpgrade : public Card {
protected:
	//reference to the card it upgrades
	Card* _card;
public:
	BaseCardUpgrade(Card* my_card, const Resources res_mod = Resources(0, 0))
		: _card(my_card), Card(my_card->get_name(), 
			my_card->get_costs() + res_mod, 
			my_card->get_play_destination(),
			my_card->get_mill_destination()) {
		//Resources& res = get_costs();
		//res = res + res_mod;
		//my_card = this;
	}
	virtual ~BaseCardUpgrade() {
		if (_card != nullptr)
			delete _card;
	}

	void on_play (Deck& d, const Vector2D* player_position, const Vector2D* target_position) override {
		_card->on_play(d,player_position, target_position);
		//call CardUpgrade::on_play and then 
		//add to the function whatever you need
	}
	Card* on_mill(Deck& d, const Vector2D* player_position) override {
		_card->on_mill(d,player_position);
		return this;
		//add to the function whatever you need and then call CardUpgrade::on_mill
	}
	virtual Resources& get_costs() {
		return _card->get_costs();
	}
	virtual std::string get_written_info() override {
		return _card->get_written_info();
	}

	virtual std::string& get_name() override {
		return _card->get_name();
	}
};


//Upgrades-----------------------------------------------------------
class PlayItTwice : public BaseCardUpgrade {
public:
	//Necesita poner un timer antes de hacer otra vez el efecto de la carta pq si no parece que no pasa
	PlayItTwice(Card* c) :BaseCardUpgrade(c, Resources(1,0)) {}
	void on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) override {
		BaseCardUpgrade::on_play(d, player_position, target_position);
		_card->on_play(d, player_position, target_position);
	}
};
class CheaperBy1 : public BaseCardUpgrade {
public:
	CheaperBy1(Card* c) :BaseCardUpgrade(c, Resources(-1, 0)) {}
};
class Ephemeral : public BaseCardUpgrade {
public:
	Ephemeral(Card* c) : BaseCardUpgrade(c){
		_play_destination = DESTROY;
		_discard_destination = DESTROY;
		_mill_destination = DESTROY;
	}
};