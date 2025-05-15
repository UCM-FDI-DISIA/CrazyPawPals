#pragma once


#include <unordered_map>
#include <forward_list>

#include "../utils/Singleton.h"


//CASE OF USE:
// 

// uncommment to see better
/*
	"Game::Instance()::get_event_mngr" is the same as "event_system::event_manager::Instance()" and should work the same (not tested, I recommend sticking with the first)

	//FIRE EVENT => for a real example of use see Deck.cpp line 83
	//sustitute event_system::mill with the name that the event to be fired was given in the "event_name" enum
	Game::Instance()::get_event_mngr()->fire_event(event_system::mill);

	//RECEIVE EVENT => For a real example see the commented lines on PlayableCards.hpp/.cpp of card Fireball
	//inherit from event_system::event_receiver and override event_callback0
	//On the init / constructor or similar of the object call:
	Game::Instance()::get_event_mngr()->suscribe_to_event(event_system_mill, this,  &event_system::event_receiver::event_callback0);
	//Where it reads "this" its literally was it has to be written, dont substitute it

	//If an object will need several events to call differents methods from it create new ones in the event_receiver public part of the class
*/

namespace event_system {

	enum event_name {
		mill = 0,				//bin 0
		shuffle = 1 << 0,		//bin 1
		change_deccel = 1 << 1,	//bin 2
		enemy_dead = 1 << 2,	//bin 4
		player_dead = 1 << 3
		//4?evento = 1 << 2,	
	};
	
	class event_receiver {
	public:
		//this struct will need a rework if intensive use of it its needed
		struct Msg {
			float float_value = 0;
			int int_value = 0;
		};
		//add more in case its needed, 
		virtual void event_callback0(const Msg& m) = 0;
		//recommended to only declare the first abstract, in order for the rest of the program to not explode when adding new ones
		virtual void event_callback1(const Msg& m) {
			(void)m;
		}
	protected:
		~event_receiver() {}
	};

	typedef void(event_receiver::* ev_rec_func)(const event_receiver::Msg&);

	class event_triggerer {
	private:
		std::forward_list<std::pair<event_receiver*, ev_rec_func>> suscribers_vec;
	public:
		event_triggerer() 
			: suscribers_vec() {}
		//not tested
		~event_triggerer() {
			//suscribers_vec->clear();
			//delete suscribers_vec;
		}
		inline void suscribe(event_receiver* e, ev_rec_func event_receiver_function) {
			suscribers_vec.push_front(std::make_pair(e,event_receiver_function));
		}
		//not tested
		inline void unsuscribe(event_receiver* e, ev_rec_func event_receiver_function) {
			//std::find(suscribers_vec->begin(), suscribers_vec->end(), std::make_pair(e, event_receiver_function));
			suscribers_vec.remove(std::make_pair(e, event_receiver_function));
		}
		inline void unsuscribe_all() {
			suscribers_vec.clear();
		}
		inline void trigger_event(const event_receiver::Msg& m) {
			for (std::pair<event_receiver*, ev_rec_func>& e : suscribers_vec) {
				(e.first->*(e.second))(m);
			}
		}
	};

	class event_manager : public Singleton<event_manager>{
		friend Singleton<event_manager>;
	private:
		std::unordered_map<event_name, event_triggerer>* event_calling_map;
		event_manager()
			:event_calling_map(nullptr){}
		bool init() {
			event_calling_map = new std::unordered_map<event_name, event_triggerer>();
			return true;
		}
		~event_manager(){
			for (auto ecm : *event_calling_map) {
				ecm.second.unsuscribe_all();
			}
			event_calling_map->clear();
			delete event_calling_map;
		}
	public:
		//Call this when an object that receives events is created
		inline void suscribe_to_event(event_name ev_name, event_receiver* ev_rec, ev_rec_func rec_func) {
			(*event_calling_map)[ev_name].suscribe(ev_rec,rec_func);
		}
		//Call this when an object that received events is destroyed
		inline void unsuscribe_to_event(event_name ev_name, event_receiver* ev_rec, ev_rec_func rec_func) {
			(*event_calling_map)[ev_name].unsuscribe(ev_rec,rec_func);
		}
		inline void unsuscribe_all_to_event(event_name ev_name) {
			(*event_calling_map)[ev_name].unsuscribe_all();
		}
		inline void fire_event(event_name ev_name, const event_receiver::Msg& m) {
			(*event_calling_map)[ev_name].trigger_event(m);
		}
	};
}
