
#include "Deck.hpp"
#include "ecs/Manager.h"
#include <iostream>
#include "game/Game.h"
#include "../movement/MovementController.h"
#include "../movement/Transform.h"
#include "../../../rendering/card_rendering.hpp"


#include "../../card_system/PlayableCards.hpp"
#ifdef GENERATE_LOG
#include "../../log_writer_to_csv.hpp"
#endif
#include <algorithm>


void Deck::_put_new_card_on_hand()
{
	if (!_draw_pile.empty()) {
		_hand = _draw_pile.pop_first();
		_av._last_card_draw_time = sdlutils().virtualTimer().currTime();
	}
	else {
		_hand = nullptr;
		reload();
	}
}

Deck::Deck() 
	:_milled_card_to_erase()
{
	_discard_pile = CardList();
	_hand = nullptr;
	std::list<Card*> default_cardList = { new Lighting(), new Fireball(), new Minigun(), new Lighting() };
	_draw_pile = CardList(default_cardList);
	_draw_pile.shuffle();
	_put_new_card_on_hand();
};

Deck::Deck(std::list<Card*>& starterDeck) noexcept
{
	_discard_pile = CardList();
	_hand = nullptr;
	//_mana = new Mana(); // REMOVE AFTER IMPLEMENTING PLAYER
	_draw_pile = CardList(starterDeck);
	_draw_pile.shuffle();
	_put_new_card_on_hand();
}

Deck::Deck(CardList&& starterDeck) noexcept
{
	_discard_pile = CardList();
	_hand = nullptr;
	//_mana = new Mana(); // REMOVE AFTER IMPLEMENTING PLAYER
	_draw_pile = starterDeck;
	_draw_pile.shuffle();
	_put_new_card_on_hand();
}

Deck::~Deck()
{
	//Hand es un puntero a una carta
	if(_hand!=nullptr)
		delete _hand;
	//_draw_pile y _discard_pile llamar�n a su destructor cuando esto se destruya al salir de �mbito
	for (auto& c : _draw_pile.card_list())
		delete c;
	for (auto& c : _discard_pile.card_list())
		delete c;
	for (auto& c : _milled_card_to_erase)
		delete c;
}

bool Deck::use_card(const Vector2D* target_pos) noexcept
{
	if (_can_play_hand_card()) {
		//Se pudo usar la carta
		_mana->change_mana(-_hand->get_costs().get_mana());
		_health->payHealth(_hand->get_costs().get_health());
		_hand->on_play(*this, &_tr->getPos(), target_pos);
		switch (_hand->get_play_destination()) {
		case DISCARD_PILE:
			_discard_pile.add_card(std::move(_hand));
			break;
		case DRAW_PILE:
			_draw_pile.add_card(std::move(_hand));
			break;
		case DESTROY:
			delete _hand;
			_hand = nullptr;
			break;
		}
		_put_new_card_on_hand();

		sdlutils().soundEffects().at("card_play").play();

		return true;
	}
	else {
		//No se pudo usar la carta, dar indicaci�n visual y auditiva al player
		return false;
	}
}

bool Deck::discard_card() noexcept
{
	if (_hand != nullptr) {
		switch (_hand->get_discard_destination()) {
		case DISCARD_PILE:
			_discard_pile.add_card(std::move(_hand));
			break;
		case DRAW_PILE:
			_draw_pile.add_card(std::move(_hand));
			break;
		case DESTROY:
			delete _hand;
			_hand = nullptr;
			break;
		}
		_put_new_card_on_hand();
		return true;
	}
	else {
		return false;
	}
}

std::pair<bool, Card*> Deck::mill() noexcept
{		
	bool milled = false;
	if (!_draw_pile.empty()) {
		milled = true;
		_last_milled_card = _draw_pile.pop_first()->on_mill(*this, &_tr->getPos());
		
		event_system::event_receiver::Msg mill_msg;
		mill_msg.int_value = _last_milled_card->get_costs().get_mana();
		switch (_last_milled_card->get_mill_destination()) {
		case DISCARD_PILE:
			_discard_pile.add_card(_last_milled_card);
			break;
		case DRAW_PILE:
			_draw_pile.add_card(_last_milled_card);
			break;
		case DESTROY:
			_milled_card_to_erase.push_back(_last_milled_card);
			break;
		}
		_av._last_milled_card_time = sdlutils().virtualTimer().currTime();
		Game::Instance()->get_event_mngr()->fire_event(event_system::mill, mill_msg);

		sdlutils().soundEffects().at("card_mill").play();

	}
	return std::make_pair(milled, _last_milled_card);
}

void Deck::reload() noexcept
{
	if (!_is_reloading) {
#ifdef GENERATE_LOG
		if (empty_hand())
			log_writer_to_csv::Instance()->add_new_log("RELOAD STARTED", "AUTOMATIC RELOAD");
		else {
			log_writer_to_csv::Instance()->add_new_log("RELOAD STARTED", "MANUAL RELOAD", "CARDS LEFT", _draw_pile.card_list().size()+1, "CARD ON HAND", _hand->get_name());
		}
		times_reloaded++;
#endif
		//TODO -> block player action
		_is_reloading = true;
		_time_till_reload_finishes = _reload_time;
		event_system::event_manager::Instance()->fire_event(event_system::shuffle, event_system::event_receiver::Msg());
		//Puts all cards on discard
		if (_hand != nullptr) {
			if (_hand->get_discard_destination() == DESTROY) {
				delete _hand;
			}else{
				_discard_pile.add_card(std::move(_hand));
			}
			_hand = nullptr;
		}
		//_draw_pile.move_from_this_to(_discard_pile);
		while (!_draw_pile.empty()) {
			Card* c = _draw_pile.pop_first();
			if (c->get_discard_destination() == DESTROY) {
				delete c;
			}else{
				_discard_pile.add_card(c);
			}
		}

		sdlutils().soundEffects().at("shuffle").play();

		_primed = false;
	}
}
void Deck::_finish_realoading()
{
	//this is called after waiting
	//this moves everything to draw_pile, shuffles and return control to player
	_is_reloading = false;
	_discard_pile.move_from_this_to(_draw_pile);
	_draw_pile.shuffle();
	_put_new_card_on_hand();
	
}
bool Deck::_can_finish_reloading()
{
	return _is_reloading && _time_till_reload_finishes <= 0;
}

bool Deck::_can_play_hand_card()
{
	return (!_is_reloading && _mana->mana_count() >= _hand->get_costs().get_mana());
	//TODO: card checks mana and life costs
	
}

void Deck::update(Uint32 deltaTime) noexcept
{
	//TODO
	//Counts time down for reload time and do the rest of things needed for finishing reload

	_draw_pile.update(deltaTime);
	if(_hand != nullptr)
		_hand->update(deltaTime);
	_discard_pile.update(deltaTime);

	_time_till_reload_finishes -= deltaTime;
	//if(_is_reloading)
		
	
	if (_can_finish_reloading()) {
		_finish_realoading();
	}
}

void Deck::add_card_to_deck(Card* c)
{
	assert(c != nullptr);
	_draw_pile.add_card(std::move(c));
}

void Deck::add_card_to_discard_pile(Card* c)
{
	assert(c != nullptr);
	_discard_pile.add_card(std::move(c));
}

void Deck::remove_card(Card* c)
{
	auto& cl = _draw_pile.card_list();
	cl.remove(c);
	std::cout << "draw pile size: " + _draw_pile.card_list().size();
	std::cout << "discard pile size: " + _discard_pile.card_list().size();
}

MovementController* Deck::get_movement_controller()
{
	return Game::Instance()->get_mngr()->getComponent<MovementController>(_ent); 
}

void Deck::set_primed(bool prime)
{
	_primed = prime;
}

CardList& Deck::move_discard_to_draw(bool menu) {
	_discard_pile.move_from_this_to(_draw_pile);
	if (menu) {
		std::erase_if(_draw_pile.card_list(), [](Card* c) { 
			if (c->get_discard_destination() == DESTROY) { 
				delete c; return true;
			} 
			return false; 
		});
		if (_hand != nullptr) {
			if (_hand->get_discard_destination() == DESTROY)
				delete _hand;
			else
				_discard_pile.add_card(std::move(_hand));
			_hand = nullptr;
		}
		//_draw_pile.move_from_this_to(_discard_pile);
		while (!_discard_pile.empty()) {
			Card* c = _discard_pile.pop_first();
			if (c->get_discard_destination() == DESTROY) {
				delete c;
			}
			else {
				_draw_pile.add_card(c);
			}
		}
	}
	return _draw_pile;
}
void Deck::initComponent()
{
	_mana = Game::Instance()->get_mngr()->getComponent<ManaComponent>(_ent);
	assert(_mana!=nullptr);
	_health = Game::Instance()->get_mngr()->getComponent<Health>(_ent);
	assert(_health != nullptr);
	_tr = Game::Instance()->get_mngr()->getComponent<Transform>(_ent);
	assert(_tr!=nullptr);
	_camera = Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA));
	assert(_camera != nullptr);
}



std::ostream& operator<<(std::ostream& os, const Deck& deck)
{
	os << "Draw Pile: " << std::endl;
	os << deck._draw_pile;
	os << std::endl;

	if(deck._hand!=nullptr)
		os << "Hand: "  << std::endl << deck._hand->get_written_info() << std::endl;

	os << "DiscardPile: " << std::endl;
	os << deck._discard_pile;
	os<<std::endl;
	
	return os;
}
int
Deck::get_total_cards_num() {
	return (_draw_pile.card_list().size() + _discard_pile.card_list().size() + !empty_hand()); 
}