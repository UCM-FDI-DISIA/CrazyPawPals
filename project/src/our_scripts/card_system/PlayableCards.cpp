
#include "PlayableCards.hpp"
#include "CardUpgrade.hpp"
#include "../components/movement/MovementController.h"
#include "../../game/Game.h"
#include "../../game/scenes/GameScene.h"
#include "ShootPatrons.hpp"
#include "../components/Health.h"
#define PI 3.14159265358979323846

#pragma region fireball
Fireball::Fireball() :Card("card_fireball", Resources(2))
{
	//Game::Instance()->get_event_mngr()->suscribe_to_event(event_system::mill,this, &event_system::event_receiver::event_callback0);
}
void Fireball::on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) {
	Card::on_play(d, player_position, target_position);
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.dir = ((*target_position) - (*player_position)).normalize();
	bp.init_pos = *player_position;
	bp.speed = 0.08f;
	bp.height = 3;
	bp.width = 2.3;
	bp.life_time = 2;
	bp.sprite_key = "p_fireball";
	bp.damage = 8;
	bp.collision_filter = GameStructs::collide_with::enemy;
	bp.weapon_type = GameStructs::WeaponType::FIREBALL_EFFECT;
	
	Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);
}
/*
void Fireball::event_callback0(const Msg& m)
{
	std::cout << "fireball is crying cause something was milled" << std::endl;
}
*/
#pragma endregion


#pragma region minigun
Minigun::Minigun()
	: Card("card_minigun", Resources(2)), _pl_vec(), _playing(false), _time_since_played(0)
{
	_bullets_properties = GameStructs::BulletProperties();
	_bullets_properties.speed = 0.5;
	_bullets_properties.height = 0.5;
	_bullets_properties.width = 0.5;
	_bullets_properties.life_time = 0.5f;
	_bullets_properties.sprite_key = "p_minigun";
	_bullets_properties.collision_filter = GameStructs::collide_with::enemy;
	_bullets_properties.damage = 1;
}
void Minigun::on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position)
{
	Card::on_play(d, player_position, target_position);
	_playing = true;
	_time_since_played = 0;
	_aim_vec = target_position;
	_pl_vec = player_position;
	_number_of_bullets_shot = 0;
	if (d.get_primed()) {
		d.set_primed(false);
		_im_primed = true;
		_number_of_shots = 30;
		_shooting_duration = 1000;
	}
	else {
		d.set_primed(true);
		_im_primed = false;
		_shooting_duration = 1500;
		_number_of_shots = 10;
	}
}

void Minigun::update(uint32_t dt)
{
	if (_playing) {
		_time_since_played += dt;
		if (_time_since_played >= _number_of_bullets_shot * (_shooting_duration / (_number_of_shots - 1))) {
			_bullets_properties.dir = ((*_aim_vec) - (*_pl_vec)).normalize();
			_bullets_properties.init_pos = *_pl_vec;
			if (!_im_primed) {
				Game::Instance()->get_currentScene()->create_proyectile(_bullets_properties, ecs::grp::BULLET);
			}
			else {
				patrons::ShotgunPatron(_bullets_properties, ecs::grp::BULLET, 324, 5);
			}
			++_number_of_bullets_shot;
			if (_number_of_bullets_shot == _number_of_shots)
				_playing = false;
		}
	}
}
#pragma endregion


#pragma region lightning
Lighting::Lighting()
	:Card("card_lighting", Resources(2))
{
}

void Lighting::on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position)
{
	Card::on_play(d, player_position, target_position);
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.dir = Vector2D(0,1);
	bp.init_pos = *target_position;
	bp.speed = 0;
	bp.height = 4.2;
	bp.width = 3.8;
	bp.life_time = 0.1;
	bp.sprite_key = "p_lighting";
	bp.collision_filter = GameStructs::collide_with::enemy;
	bp.damage = 12;
	Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);
}
#pragma endregion



#pragma region kunai
Kunai::Kunai()
	:Card("card_kunai")
{
}

void Kunai::on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position)
{
	Card::on_play(d, player_position, target_position);
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.dir = ((*target_position) - (*player_position)).normalize();
	bp.init_pos = *player_position;
	bp.speed = 0.6f;
	bp.height = 2.3;
	bp.width = 2.3;
	bp.life_time = 2;
	bp.sprite_key = "p_kunai";
	bp.damage = 5;
	bp.pierce_number = INT_MAX;
	bp.collision_filter = GameStructs::collide_with::enemy;
	
	Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);
}
#pragma endregion

#pragma region Recover
Recover::Recover() :Card("card_recover", Resources(1,2),DRAW_PILE){
}
void 
Recover::on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) {
	d.move_discard_to_draw(false);
}
#pragma endregion

#pragma region cardspray
CardSpray::CardSpray()
	:Card("card_cardSpray")
{
}

void CardSpray::on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position)
{
	Card::on_play(d, player_position, target_position);
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.dir = ((*target_position) - (*player_position)).normalize();
	bp.init_pos = *player_position;
	bp.speed = 0.05f;
	bp.height = 0.7;
	bp.width = 0.7;
	bp.life_time = 3;
	bp.damage = 4;
	bp.pierce_number = INT_MAX;
	bp.sprite_key = "p_card_spray";
	bp.collision_filter = GameStructs::collide_with::enemy;

	patrons::ShotgunPatron(bp, ecs::grp::BULLET, 75, 3);
	d.mill();
}
#pragma endregion

#pragma region eldritchblast
EldritchBlast::EldritchBlast() :Card("card_eldritchBlast", Resources(1))
{
}

void EldritchBlast::on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position)
{
	Card::on_play(d, player_position, target_position);
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.dir = ((*target_position) - (*player_position)).normalize();
	bp.init_pos = *player_position;
	bp.speed = 0.5f;
	bp.height = 2.3;
	bp.width = 2.3;
	bp.life_time = 0.13;
	bp.sprite_key = "p_eldritch_blast";
	bp.damage = 4;
	bp.collision_filter = GameStructs::collide_with::enemy;

	patrons::ShotgunPatron(bp, ecs::grp::BULLET, _amplitude * (_shot_count - 1), _shot_count);
}

Card* EldritchBlast::on_mill(Deck& d, const Vector2D* player_position)
{
	_shot_count += 2;
	return Card::on_mill(d, player_position);
}
#pragma endregion

#pragma region primordia
Primordia::Primordia() :Card("card_primordia", Resources(1), DISCARD_PILE, DISCARD_PILE, DRAW_PILE)
{
}

void Primordia::on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position)
{
	Card::on_play(d, player_position, target_position);
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.dir = ((*target_position) - (*player_position)).normalize();
	bp.init_pos = *player_position;
	bp.speed = 0.4f;
	bp.height = 2.3;
	bp.width = 2.3;
	bp.life_time = 0.3;
	bp.damage = 3;//cambiar posiblemente
	bp.collision_filter = GameStructs::collide_with::enemy;
	// Primed effect
	// TODO: Make distinct from standard effect
	if (d.get_primed()) {

		d.set_primed(false);

		bp.sprite_key = "p_eldritch_blast";
	}
	else {
		bp.sprite_key = "p_fireball";
	}
	Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);
}

Card* Primordia::on_mill(Deck& d, const Vector2D* player_position)
{
	d.set_primed(true);
	return this;
}

void Primordia::update(uint32_t dt) //TODO: Projectile must return following path end rather than set time.
{
	/*if (_playing) {
		_time_since_played += dt;
		if (_time_since_played >= 300) {
			std::cout << "back" << std::endl;
			GameStructs::BulletProperties bp = GameStructs::BulletProperties();
			bp.dir = ((*_player_pos) - (_target_pos)).normalize();
			bp.init_pos = _target_pos;
			bp.speed = 0.4f;
			bp.height = 2.3;
			bp.width = 2.3;
			bp.life_time = 0.3;
			bp.sprite_key = "p_eldritch_blast";
			
			static_cast<GameScene*>(Game::Instance()->get_currentScene())->generate_proyectile(bp, ecs::grp::BULLET);
			_playing = false;
		}
	}*/
}
#pragma endregion

#pragma region commune
Commune::Commune() :Card("card_commune", Resources(3))
{
}

void Commune::on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position)
{
	int amp = 0;
	std::pair<bool, Card*> milled_card = d.mill();
	if (milled_card.first) amp = milled_card.second->get_costs().get_mana();


	Card::on_play(d, player_position, target_position);
	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.dir = ((*target_position) - (*player_position)).normalize();
	bp.init_pos = *target_position;
	bp.speed = 0;
	bp.height = 1.0 + (amp * 1.5);
	bp.width = 1.0 + (amp * 1.5);
	bp.life_time = 0.2;
	bp.sprite_key = "p_commune";
	bp.collision_filter = GameStructs::collide_with::enemy;
	bp.damage = amp * 6+6;
	Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);
}
#pragma endregion

#pragma region evoke
Evoke::Evoke() :Card("card_evoke", Resources(1))
{
}

void Evoke::on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position)
{
	std::pair<bool, Card*> milled_card = d.mill();
	if (milled_card.first) milled_card.second->on_play(d,player_position,target_position);
}
#pragma endregion

#pragma region fulgur
Fulgur::Fulgur() : Card("card_fulgur", Resources(3)), _playing(false),_time_since_played(0)
{
	_bullets_properties = GameStructs::BulletProperties();
	_bullets_properties.speed = 0.0;
	_bullets_properties.height = 2;
	_bullets_properties.width = 2;
	_bullets_properties.life_time = 0.1f;
	_bullets_properties.dir = (Vector2D(0, 1));
	_bullets_properties.sprite_key = "p_fulgur";
	_bullets_properties.collision_filter = GameStructs::collide_with::enemy;
}
void Fulgur::on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position)
{
	Card::on_play(d, player_position, target_position);

	GameStructs::BulletProperties bp = GameStructs::BulletProperties();
	bp.dir = Vector2D(0, 1);
	bp.init_pos = *target_position;
	bp.speed = 0;
	bp.height = 4.2;
	bp.width = 3.8;
	bp.life_time = 0.5;
	bp.collision_filter = GameStructs::collide_with::enemy;
	bp.sprite_key = "p_fulgur";
	bp.damage = 5;
	Game::Instance()->get_currentScene()->create_proyectile(bp, ecs::grp::BULLET);

	// If Primed
	if (d.get_primed()) {

		d.set_primed(false);
		_playing = true;
		_time_since_played = 0;
		_aim_vec = Vector2D(target_position->getX(), target_position->getY()),
			_number_of_bullets_shot = 0;
	}
	else
		d.set_primed(true);
}
void Fulgur::update(uint32_t dt)
{
	if (_playing) {
		_time_since_played += dt;
		if (_time_since_played >= _number_of_bullets_shot * (_shooting_duration / (_number_of_shots - 1))) {

			_bullets_properties.init_pos = _aim_vec+(Vector2D(sin(_number_of_bullets_shot*2*PI/_number_of_shots),cos(_number_of_bullets_shot * 2 * PI / _number_of_shots))*2);
			
			Game::Instance()->get_currentScene()->create_proyectile(_bullets_properties, ecs::grp::BULLET);
			++_number_of_bullets_shot;
			if (_number_of_bullets_shot == _number_of_shots)
				_playing = false;
		}
	}
}
#pragma endregion
#pragma region quickfeet
QuickFeet::QuickFeet(): Card("card_quickFeet", Resources(2)), _playing(false), _time_since_played(0)
{
}
void QuickFeet::on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position)
{
	(void)player_position;
	(void)target_position;
	
	_playing = true;
	_time_since_played = 0;

	_ctrl = d.get_movement_controller();
	_ctrl->get_acceleration() += 10.0f;
	_ctrl->get_max_speed() += 0.05f;

	d.add_card_to_deck(new Ephemeral(new Kunai()));
	d.add_card_to_deck(new Ephemeral(new Kunai()));
}
void QuickFeet::update(uint32_t dt)
{
	if (_playing) {
		_time_since_played += dt;
		if (_time_since_played >= _effect_duration) {
			_ctrl->get_acceleration() -= 10.0f;
			_ctrl->get_max_speed() -= 0.05f;
			_playing = false;
		}
	}
}
#pragma endregion
#pragma region CatKuzaCard
CatKuzaCard::CatKuzaCard() : Card("card_catkuza", Resources(2)), _times_since_played(0)
{
	_can_be_replaced = false;
}

void 
CatKuzaCard::on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position)
{
	(void)d;
	(void)player_position;
	(void)target_position;
	_times_since_played++;
	std::cout << "CatKuzaCard played " << _times_since_played << " times" << std::endl;
	if(_times_since_played >= 2){
		std::cout << "Eliminarse" << std::endl;

		_play_destination = DESTROY;

	} 
}
#pragma endregion

#pragma region SuperMichiCard
SuperMichiCard::SuperMichiCard() : Card("card_super_michi", Resources(1)),_times_since_played(0){
	_can_be_replaced = false;
}

void
SuperMichiCard::on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position)
{
	(void)d;
	(void)player_position;
	(void)target_position;
	_times_since_played++;
	if (_times_since_played >= 10) {
		_play_destination = DESTROY;
	}
}
#pragma endregion

#pragma region TestCards
#pragma region prime
Prime::Prime() : Card("card_placeholder", Resources(0))
{
}

void Prime::on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position)
{
	d.set_primed(true);
}
#pragma endregion
#pragma endregion


