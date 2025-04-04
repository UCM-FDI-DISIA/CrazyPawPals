#include "Card.hpp"
#include "../components/Deck.hpp"


//Decorator Patron --> https://refactoring.guru/design-patterns/decorator/cpp/example
class BaseCardUpgrade : public Card {
protected:
	//reference to the card it upgrades
	std::unique_ptr<Card> _card;
public:
	BaseCardUpgrade(std::unique_ptr<Card>&& my_card, Resources& res_mod = Resources(0,0))
		:_card(std::move(my_card)) {
		Resources& res = get_costs();
		res = res + res_mod;
		//my_card = this;
	}

	void 
	(Deck& d, Vector2D* player_position, Vector2D* target_position) override{
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

	virtual std::string get_name() override {
		return _card->get_name();
	}
};


//Upgrades-----------------------------------------------------------
class PlayItTwice : public BaseCardUpgrade {
public:
	PlayItTwice(std::unique_ptr<Card>&& c) :BaseCardUpgrade(c, Resources(1,0)) {}
	void on_play(Deck& d,Vector2D* player_position, Vector2D* target_position) override {
		BaseCardUpgrade::on_play(player_position, target_position);
		_card->on_play(player_position, target_position);
	}
};
class CheaperBy1 : public BaseCardUpgrade {
public:
	CheaperBy1(std::unique_ptr<Card>&& c) :BaseCardUpgrade(c, Resources(-1, 0)) {}
};