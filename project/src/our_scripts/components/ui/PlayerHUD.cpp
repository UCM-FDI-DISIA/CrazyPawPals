
#include "PlayerHUD.h"
#include "../../../game/Game.h"
#include "../../../ecs/Manager.h"
#include "../../../sdlutils/SDLUtils.h"
#include "../../../rendering/card_rendering.hpp"
#include "../WaveManager.h"
#include "../../mythic/MythicItem.h"
#include "../../components/GamePadPlayerCtrl.hpp"
#include "../../components/KeyboardPlayerCtrl.h"
#include <sdlutils/InputHandler.h>
PlayerHUD::PlayerHUD() 
	: _tex_orb(&sdlutils().images().at("manaorb")), _tex_orb_empty(&sdlutils().images().at("manaorbempty")), 
	_tex_prime(&sdlutils().images().at("prime")), _tex_reticle(&sdlutils().images().at("reticle"))
{

}

PlayerHUD::~PlayerHUD()
{
}

void PlayerHUD::initComponent()
{
	_tr = Game::Instance()->get_mngr()->getComponent<Transform>(_ent);
	_mana = Game::Instance()->get_mngr()->getComponent<ManaComponent>(_ent);
	_health = Game::Instance()->get_mngr()->getComponent<Health>(_ent);
	_deck = Game::Instance()->get_mngr()->getComponent<Deck>(_ent);
	_camera = Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA));
	_mythics = Game::Instance()->get_mngr()->getComponent<MythicComponent>(_ent);
	_gpc = Game::Instance()->get_mngr()->getComponent<GamePadPlayerCtrl>(_ent);
	_kpc = Game::Instance()->get_mngr()->getComponent<KeyboardPlayerCtrl>(_ent);
}

void PlayerHUD::update(uint32_t delta_time)
{
}

void PlayerHUD::render()
{
	SDL_SetRenderDrawBlendMode(sdlutils().renderer(), SDL_BLENDMODE_NONE);

#pragma region health
	int health = _health->getHealth();
	int max_health = _health->getMaxHealth();
	int shield = _health->getShield();
	static constexpr float health_scale = 0.0015;

	//bg
	SDL_SetRenderDrawColor(sdlutils().renderer(), 80, 55, 60, 255);
	rect_f32 health1 = rect_f32_screen_rect_from_viewport(rect_f32({ 0.13,0.82 }, { max_health * health_scale,0.03 }),_camera->cam.screen);
	SDL_Rect health1true {
		int(health1.position.x),
		int(health1.position.y),
		int(health1.size.x),
		int(health1.size.y)
	};
	SDL_RenderFillRect(sdlutils().renderer(), &health1true);

	//remaining
	SDL_SetRenderDrawColor(sdlutils().renderer(), 200, 80, 100, 255);
	rect_f32 health2 = rect_f32_screen_rect_from_viewport(rect_f32({ 0.13,0.82 }, { health * health_scale,0.03 }), _camera->cam.screen);
	SDL_Rect health2true {
		int(health2.position.x),
		int(health2.position.y),
		int(health2.size.x),
		int(health2.size.y)
	};
	SDL_RenderFillRect(sdlutils().renderer(), &health2true);

	//shield (overlaps health)
	SDL_SetRenderDrawColor(sdlutils().renderer(), 190, 210, 255, 125);
	rect_f32 shieldrect = rect_f32_screen_rect_from_viewport(rect_f32({ 0.13,0.815 }, { shield * health_scale,0.04 }), _camera->cam.screen);
	SDL_Rect shieldrecttrue{
		int(shieldrect.position.x),
		int(shieldrect.position.y),
		int(shieldrect.size.x),
		int(shieldrect.size.y)
	};
	SDL_RenderFillRect(sdlutils().renderer(), &shieldrecttrue);

#pragma endregion

#pragma region mana
	int mana_count = _mana->raw_mana_count();
	int max_mana = _mana->max_mana();
	static constexpr float mana_scale = 0.000025;

	//bg
	SDL_SetRenderDrawColor(sdlutils().renderer(), 60, 55, 80, 255);
	rect_f32 mana1 = rect_f32_screen_rect_from_viewport(rect_f32({ 0.13,0.86 }, { max_mana*mana_scale,0.024 }), _camera->cam.screen);
	SDL_Rect mana1true{
		int(mana1.position.x),
		int(mana1.position.y),
		int(mana1.size.x),
		int(mana1.size.y)
	};
	SDL_RenderFillRect(sdlutils().renderer(), &mana1true);

	//progress
	SDL_SetRenderDrawColor(sdlutils().renderer(), 81, 100, 222, 255);
	rect_f32 mana2 = rect_f32_screen_rect_from_viewport(rect_f32({ 0.13,0.86 }, { (mana_count%1000)*5 * mana_scale,0.024 }), _camera->cam.screen);
	SDL_Rect mana2true{
		int(mana2.position.x),
		int(mana2.position.y),
		int(mana2.size.x),
		int(mana2.size.y)
	};
	SDL_RenderFillRect(sdlutils().renderer(), &mana2true);

	// full mana orbs
	int display = max_mana / 1000;
	for (int i = 1; i <= display; i++) {
		rect_f32 output = rect_f32_screen_rect_from_viewport(rect_f32({ 0.13f + (0.024f * float(i-1)), 0.894f}, {0.022f, 0.022f}), _camera->cam.screen);
		SDL_Rect outputtrue{
			int(output.position.x),
			int(output.position.y),
			int(output.size.x),
			int(output.size.x)
		};
		if (mana_count >= i * 1000) _tex_orb->render(outputtrue);
		else _tex_orb_empty->render(outputtrue);
	}

	
#pragma endregion

#pragma region deck
#pragma region reload
	if (_deck->is_reloading()) {
		int reload_time = _deck->reload_time();
		int time_till_reload_finishes = _deck->time_till_reload_finishes();
		SDL_SetRenderDrawBlendMode(sdlutils().renderer(), SDL_BLENDMODE_NONE);

		//bg
		SDL_SetRenderDrawColor(sdlutils().renderer(), 100, 100, 100, 255);
		rect_f32 baroutput1{ {_tr->getPos().getX() - 0.35f, _tr->getPos().getY() + 1.2f}, {(reload_time) / 1000.0f, 0.2 } };
		SDL_Rect trueoutput1 = SDL_Rect_screen_rect_from_global(baroutput1, _camera->cam);
		SDL_RenderFillRect(sdlutils().renderer(), &trueoutput1);


		//progress
		SDL_SetRenderDrawColor(sdlutils().renderer(), 220, 220, 220, 255);
		rect_f32 baroutput2{ {_tr->getPos().getX() - 0.35f, _tr->getPos().getY() + 1.2f}, {(reload_time - time_till_reload_finishes) / 1000.0f, 0.2 } };
		SDL_Rect trueoutput2 = SDL_Rect_screen_rect_from_global(baroutput2, _camera->cam);
		SDL_RenderFillRect(sdlutils().renderer(), &trueoutput2);
	}
#pragma endregion

#pragma region cards
	card_rendering_descriptor crd = card_rendering_descriptor();
	crd.mana_cost_font_key = "RUBIK_MONO";
	crd.mana_cost_color = { 81, 100, 222, 255 };
	crd.health_cost_color = { 200, 80, 100, 255 };
	AnimationVars av = _deck->animation_vars();
#pragma region camera_definition
	camera_screen cam_screen = camera_screen();
	std::pair<int, int> position = { sdlutils().width(), sdlutils().height() };
	//camera position, similar to aspect ratio but in world units (suppose player is 1 world unit, how many players will fit on camera kinda)
	cam_screen.camera = { {0,0},{8,6} };
	//camera screen on pixels size
	cam_screen.screen = { position.first, position.second };
#pragma endregion
	float percentual_time_to_card_in_position = (sdlutils().virtualTimer().currTime() - av._last_card_draw_time) / (float)av._card_draw_anim_duration;

#pragma region cards_in_queue
	auto it = _deck->get_draw_pile().rbegin();
	int8_t card_pos_mod = _deck->get_draw_pile().size();

	while (it != _deck->get_draw_pile().rend()) {
		crd.mana_cost_subrect = { {0.1,0.2},{0,0} };
		crd.card_image_key = "card_back";// : (*it)->get_name().data();
		crd.mana_cost = (*it)->get_costs().get_mana();
		crd.health_cost = (*it)->get_costs().get_health();
		crd.health_cost_subrect = { {0.55,0.2},{0,0} };

		card_rendering_descriptor_render(
			crd,
			cam_screen,
			//take renderer
			*sdlutils().renderer(),
			//destination rect --> where will the card be placed (position, size in world units)
			{ {-8,std::lerp((float)card_pos_mod + (_deck->last_milled_card()!=nullptr ? 2.0f : 1.0f),(float)card_pos_mod,std::min(percentual_time_to_card_in_position,1.0f))*1.0f - 3.5f},{2,2.5f}},
			//src subrect --> if our image is only 1 take this parameters
			//if we have a map of 5x6 cards and we wanted to render card (3,2) being first card(0,0), and last (4,5)
			//values would be --> { {3/5, 2/6}, {1/5,1/6} }
			{ {0,0},{1,1} },
			//rotation
			0,
			//adittional options
			//card_rendering_descriptor_options_none,
			//card_rendering_descriptor_options_flip_horizontal,
			//card_rendering_descriptor_options_flip_vertical,
			//card_rendering_descriptor_options_full_subrect
			card_rendering_descriptor_options_none
		);
		--card_pos_mod;
		++it;
	}
#pragma endregion

#pragma region hand_card
	//Mostrar carta en la mano
	//Mostrar n? cartas draw_pile and discard_pile
	//Position and scale for the cost --> both values from 0 to 1

	//Funci�n que calcula la posici�n de una carta seg�n el tiempo
	
	if (_deck->hand() == nullptr) {
		crd.card_image_key = "card_reloading";
		crd.mana_cost_subrect = { {0,0.2},{0,0} };
	}
	else {
		crd.mana_cost_subrect = { {0.18,0.25},{0.2,0.4} };
		crd.card_image_key = percentual_time_to_card_in_position < 0.5f ? "card_back" : _deck->hand()->get_name().data();
		crd.mana_cost = _deck->hand()->get_costs().get_mana();
		crd.health_cost = _deck->hand()->get_costs().get_health();
		crd.health_cost_subrect = { {0.6,0.25},{0.2,0.4} };
	}

	//Function for rendering a card
	card_rendering_descriptor_render(
		crd,
		cam_screen,
		//take renderer
		*sdlutils().renderer(),
		//destination rect --> where will the card be placed (position, size in world units)
		{ {-8.0f,std::lerp(-2.5f,-3.5f,std::min(1.0f,percentual_time_to_card_in_position))},{(percentual_time_to_card_in_position < 0.5f) ? (float)std::lerp(2,0,std::min(percentual_time_to_card_in_position * 2,1.0f)) : (float)std::lerp(0,2,std::min((percentual_time_to_card_in_position - 0.5f) * 2,1.0f)),2.5f}},
		//src subrect --> if our image is only 1 take this parameters
		//if we have a map of 5x6 cards and we wanted to render card (3,2) being first card(0,0), and last (4,5)
		//values would be --> { {3/5, 2/6}, {1/5,1/6} }
		{ {0,0},{1,1} },
		//rotation
		0,
		//adittional options
		//card_rendering_descriptor_options_none,
		//card_rendering_descriptor_options_flip_horizontal,
		//card_rendering_descriptor_options_flip_vertical,
		//card_rendering_descriptor_options_full_subrect
		card_rendering_descriptor_options_none
	);
#pragma endregion

#pragma region milled_card
	if (_deck->last_milled_card() != nullptr) {
		//Funci�n que calcula la posici�n de una carta seg�n el tiempo
		percentual_time_to_card_in_position = (sdlutils().virtualTimer().currTime() - av._last_milled_card_time) / (float)av._mill_card_anim_duration;

		float scale = std::lerp(1.0f, 0.0f, std::max(0.0f, std::min((percentual_time_to_card_in_position - 0.75f) / 0.25f, 1.0f)));

		crd.mana_cost_font_key = "RUBIK_MONO";
		crd.mana_cost_subrect = { {0.0f,0.4f - 0.2f * scale},{0.4f,0.4f * scale} };
		crd.health_cost_subrect = { {0.0f,0.7f - 0.2f * scale},{0.4f,0.4f * scale} };
		crd.mana_cost_color = { 81, 100, 222, 255 };
		crd.health_cost_color = { 200, 80, 100, 255 };
		crd.card_image_key = percentual_time_to_card_in_position < 0.5f ? "card_back" : _deck->last_milled_card()->get_name().data();
		crd.mana_cost = _deck->last_milled_card()->get_costs().get_mana();

		//Function for rendering a card
		card_rendering_descriptor_render(
			crd,
			cam_screen,
			//take renderer
			*sdlutils().renderer(),
			//destination rect --> where will the card be placed (position, size in world units)
			{ {std::lerp(-8.0f,-6.0f, std::min(percentual_time_to_card_in_position*4,1.0f)),std::lerp(-3.0f,-2.5f,percentual_time_to_card_in_position)},{(percentual_time_to_card_in_position < 0.25f) ? (float)std::lerp(2,0,std::min(percentual_time_to_card_in_position * 4,1.0f)) : (float)std::lerp(0,2,std::min((percentual_time_to_card_in_position - 0.25f) * 4,1.0f)),2.5f} },
			//src subrect --> if our image is only 1 take this parameters
			//if we have a map of 5x6 cards and we wanted to render card (3,2) being first card(0,0), and last (4,5)
			//values would be --> { {3/5, 2/6}, {1/5,1/6} }
			{ {0,0},{1,scale} },
			//rotation
			0,
			//adittional options
			//card_rendering_descriptor_options_none,
			//card_rendering_descriptor_options_flip_horizontal,
			//card_rendering_descriptor_options_flip_vertical,
			//card_rendering_descriptor_options_full_subrect
			card_rendering_descriptor_options_none
		);
	}
#pragma endregion
#pragma endregion

#pragma region prime
	if (_deck->get_primed()) {
		rect_f32 prime = rect_f32_screen_rect_from_viewport(rect_f32({0.013,0.805 }, { 0.1,0.1 }), _camera->cam.screen);
		SDL_Rect primetrue{
			int(prime.position.x),
			int(prime.position.y),
			int(prime.size.x),
			int(prime.size.x)
		};
		_tex_prime->render(primetrue);
	}
#pragma endregion
#pragma endregion

#pragma region reticle
	Vector2D my_ret_pos_aux = ih().getLastDevice() == InputHandler::CONTROLLER ? _gpc->get_reticle_position() : _kpc->get_reticle_position();
	my_ret_pos_aux += Vector2D({-0.35,0.35});
	rect_f32 r_a_r = rect_f32_screen_rect_from_global(rect_f32({my_ret_pos_aux.getX(),my_ret_pos_aux.getY()}, {0.7,0.7}), _camera->cam);
	SDL_Rect reticle_rect = build_sdlrect(r_a_r.position.x, r_a_r.position.y, r_a_r.size.x, r_a_r.size.y);
	_tex_reticle->render(reticle_rect);
#pragma endregion


#pragma region mythics
	const auto& mythics = _mythics->get_mythics();
	if (mythics.size() > 0) {
		float icon_size = 0.05f; 
		float start_x = 0.02;
		float start_y = 0.02;

		for (auto mythic : mythics) {
			Texture* texture = mythic->get_texture();
			rect_f32 mythics_rect = 
				rect_f32_screen_rect_from_viewport(rect_f32({ start_x, start_y }, { icon_size, icon_size * 1.75f }), _camera->cam.screen);
			SDL_Rect mythics_rect_true{ (int)mythics_rect.position.x, (int)mythics_rect.position.y, (int)mythics_rect.size.x, (int)mythics_rect.size.y };
			texture->render(mythics_rect_true);
			start_x += icon_size + 0.01f;
		}
	}

#pragma endregion

#pragma region timer
	//TODO: Move to general HUD component
	//auto ent = Game::Instance()->get_mngr()->getHandler(ecs::hdlr::WAVE);
	//WaveManager* wavemanager = Game::Instance()->get_mngr()->getComponent<WaveManager>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::WAVE));
	/*if (Game::Instance()->get_mngr()->hasComponent<WaveManager>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::FOGGROUP))) {
		int wavetime = Game::Instance()->get_mngr()->getComponent<WaveManager>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::FOGGROUP))->get_wave_time();
		rect_f32 timer = rect_f32_screen_rect_from_viewport(rect_f32({ 0.4,0.05 }, { 0.2,0.1 }), _camera->cam.screen);
		SDL_Rect timertrue{ timer.position.x,timer.position.y,timer.size.x,timer.size.y };
		Texture timertex{ sdlutils().renderer(),std::to_string(wavetime),sdlutils().fonts().at("ARIAL16"),{50,50,50,255} };
		timertex.render(timertrue);
	}
	rect_f32 timer = rect_f32_screen_rect_from_viewport(rect_f32({ 0.4,0.01 }, { 0.2,0.15 }), _camera->cam.screen);
	SDL_Rect timertrue{ timer.position.x,timer.position.y,timer.size.x,timer.size.y };
	Texture timertex{ sdlutils().renderer(),std::to_string(60),sdlutils().fonts().at("ARIAL16"),{50,50,50,255} };
	timertex.render(timertrue);*/
#pragma endregion

}
