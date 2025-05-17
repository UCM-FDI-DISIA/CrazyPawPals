
#include "MythicItems.h"
#include "../../game/Game.h"
#include "../../game/scenes/GameScene.h"
#include <iostream>

#pragma region BloodClaw
BloodClaw::BloodClaw() :MythicItem("blood_claw", "mythic_blood_claw")
{
	auto manager = Game::Instance()->get_mngr();
	_health = manager->getComponent<Health>(_player);
	_weapon = manager->getComponent<Weapon>(_player);
}
void BloodClaw::apply_effects() {
	int currhealth = _health->getHealth() / 2;
	int damage = _weapon->damage() * 2;
	_health->takeDamage(currhealth);
	_weapon->set_damage(damage);
}
#pragma endregion


#pragma region DreamRecicler
DreamRecicler::DreamRecicler() :MythicItem("dream_recicler","mythic_dream_recicler")
{
	_mana = Game::Instance()->get_mngr()->getComponent<ManaComponent>(_player);
	event_system::event_manager::Instance()->suscribe_to_event(event_system::mill, this, &event_system::event_receiver::event_callback0);
}

DreamRecicler::~DreamRecicler()
{
	event_system::event_manager::Instance()->unsuscribe_to_event(event_system::mill, this, &event_system::event_receiver::event_callback0);
}

void 
DreamRecicler::event_callback0(const event_system::event_receiver::Msg& m) {
	int newManaCount = _mana->mana_count() + m.int_value;
	_mana->change_mana(newManaCount);
}

void 
DreamRecicler::apply_effects() {
	int m = _mana->mana_regen();
	_mana->change_mana_regen(-(m/2));
}

#pragma endregion

#pragma region CurtainReaper
CurtainReaper::CurtainReaper() :MythicItem("curtain_reaper", "mythic_curtain_reaper")
{
	_health = Game::Instance()->get_mngr()->getComponent<Health>(_player);
	event_system::event_manager::Instance()->suscribe_to_event(event_system::enemy_dead, this, &event_system::event_receiver::event_callback0);
}

CurtainReaper::~CurtainReaper()
{
	event_system::event_manager::Instance()->unsuscribe_to_event(event_system::enemy_dead, this, &event_system::event_receiver::event_callback0);
}

void 
CurtainReaper::event_callback0(const event_system::event_receiver::Msg& m) {
	int shield =  m.int_value;
	_health->takeShield(shield);
}

void 
CurtainReaper::apply_effects() {
	int currhealth = _health->getHealth() / 2;
	_health->takeDamage(currhealth);
}

#pragma endregion

#pragma region Incense
Incense::Incense() :MythicItem("incense", "mythic_incense")
{
	auto manager = Game::Instance()->get_mngr();
	_mana = manager->getComponent<ManaComponent>(_player);
	_weapon = manager->getComponent<Weapon>(_player);
}

void 
Incense::apply_effects() {
	_mana->change_mana_regen(_mana->mana_regen());
	int damage = _weapon->damage() / 2;
	_weapon->set_damage(damage);
}

#pragma endregion

#pragma region ArcaneSurge
QuickDrawDeck::QuickDrawDeck() :MythicItem("quick_draw_deck", "mythic_quick_draw_deck"), _set(false)
{
	auto manager = Game::Instance()->get_mngr();
	_mana = manager->getComponent<ManaComponent>(_player);
	_deck = manager->getComponent<Deck>(_player);
	_ini_mana = _mana->mana_regen();
}

void 
QuickDrawDeck::apply_effects() {
	_deck->set_reload_time(_deck->reload_time() - 0.5f * _deck->reload_time());
}

void QuickDrawDeck::update(uint32_t dt) {
	(void)dt;
	if (_deck->is_reloading()) {
		if (!_set) {
			
			_set = true;
			_mana->change_mana_regen(0.5f *_ini_mana);
			

		}
	}
	else if(_set){
		_mana->change_mana_regen(-(_ini_mana * 0.5f));
		_set = false;
		
	}
}

#pragma endregion

#pragma region BloodPact
BloodPact::BloodPact() :MythicItem("blood_pact", "mythic_blood_pact")
{
	auto manager = Game::Instance()->get_mngr();
	_mana = manager->getComponent<ManaComponent>(_player);
	_health = manager->getComponent<Health>(_player);
}

void 
BloodPact::apply_effects() {
	int currhealth = _health->getHealth() / 2;
	_health->takeDamage(currhealth);
	_mana->change_mana_regen(_mana->mana_regen());
	
}
#pragma endregion

#pragma region PreternaturalForce
DemonicScratcher::DemonicScratcher() :MythicItem("demonic_scratcher", "mythic_demonic_scratcher")
{
	auto manager = Game::Instance()->get_mngr();
	_mana = manager->getComponent<ManaComponent>(_player);
	_weapon = manager->getComponent<Weapon>(_player);
}

void 
DemonicScratcher::apply_effects() {
	_mana->change_mana_regen(- (_mana->mana_regen() * 0.5));
	int damage = _weapon->damage() * 2;
	_weapon->set_damage(damage);
}
#pragma endregion


#pragma region ClawFile
ClawFile::ClawFile() :MythicItem("claw_file","mythic_claw_file"), _set(false)
{
	auto manager = Game::Instance()->get_mngr();
	_deck = manager->getComponent<Deck>(_player);
	_mc = manager->getComponent<MovementController>(_player);
	_ini_mc = _mc->get_max_speed();
}

void 
ClawFile::apply_effects() {
	_deck->set_reload_time(_deck->reload_time() - 0.5f * _deck->reload_time());
}

void ClawFile::update(uint32_t dt) {
	(void)dt;
	if (_deck->is_reloading()) {
		if (!_set) {
			_set = true;
			_mc->set_max_speed(0.5f *_ini_mc);
		}
	}
	else if(_set){
		_mc->set_max_speed((_ini_mc));
		_set = false;
	}
}
#pragma endregion

#pragma region MeowOrNever
UraniumSocks::UraniumSocks():MythicItem("uranium_socks", "mythic_uranium_socks")
{
	auto manager = Game::Instance()->get_mngr();
	_health = manager->getComponent<Health>(_player);
	_mc = manager->getComponent<MovementController>(_player);
}

void 
UraniumSocks::apply_effects() {
	int currhealth = _health->getHealth() / 2;
	_health->takeDamage(currhealth);
	_mc->set_max_speed(2.0f *_mc->get_max_speed());
}
#pragma endregion

#pragma region ZoomiesInducer
ZoomiesInducer::ZoomiesInducer() :MythicItem("zoomies_inducer", "mythic_zoomies_inducer"), 
	_timer(10000), _last_time(0), _distance(3.0f), _duration(1000) 
{
	auto manager = Game::Instance()->get_mngr();
	_tr = manager->getComponent<Transform>(_player);
	_mc = manager->getComponent<MovementController>(_player);

}
ZoomiesInducer::ZoomiesInducer(MovementController* mc, Transform* tr, uint32_t time, uint32_t duration, float distance)
	:MythicItem("ZoomiesInducer"), _mc(mc), _tr(tr), _timer(time), _last_time(0), _distance(distance), _duration(duration){
}

void
ZoomiesInducer::apply_effects() {
	//std::cout << "vel: " << _mc->get_max_speed() << std::endl;
	_mc->set_max_speed(2.0f * _mc->get_max_speed());
	//std::cout << "velini: " << _mc->get_max_speed() << std::endl;
}

void ZoomiesInducer::update(uint32_t dt) {
	(void)dt;
	if (_timer + _last_time <= sdlutils().virtualTimer().currTime()) {
		//std::cout << "ZoomiesInducer" << std::endl;
		_last_time = sdlutils().virtualTimer().currTime();
		if (_tr->getDir().magnitude() > 0.05f) {
			Vector2D nextPos = _tr->getPos() + _tr->getDir().normalize() * _distance;
			_mc->dash(nextPos, _duration);
		}
	}
}
#pragma endregion