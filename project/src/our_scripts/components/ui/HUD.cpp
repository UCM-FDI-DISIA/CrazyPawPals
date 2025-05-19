
#include "HUD.h"
#include "game/Game.h"
#include "ecs/Manager.h"
#include "../../../sdlutils/SDLUtils.h"
#include "../../components/WaveManagerFacade.h"
#include "../../components/WaveManager.h"
#include "../../components/DumbWaveManager.h"

HUD::HUD():_camera(nullptr),_wm(nullptr)
{
	_event_textures = std::vector<Texture*>(int(EVENTS_MAX)+1);
	_event_textures[int(ICE_SKATE)] = (&sdlutils().images().at("event_ice_skate")); 
	_event_textures[STAR_SHOWER] = &sdlutils().images().at("event_star_shower");
}

HUD::~HUD()
{
}

void HUD::initComponent()
{
	_wm = Game::Instance()->get_wave_manager();
	_camera = Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA));
}

void HUD::update(uint32_t delta_time)
{
	_displaying_wave_event = sdlutils().currRealTime() < _current_wave_event_time;
	//if (!_displaying_wave_event) std::cout << "END" << std::endl;
}

void HUD::render()
{
#pragma region number
	int wavenum = _wm->get_current_wave() + 1;
	rect_f32 num = rect_f32_screen_rect_from_viewport(rect_f32({ 0.85,0.1 }, { 0.07,0.07 }), _camera->cam.screen);
	SDL_Rect numtrue{ 
		int(num.position.x),
		int(num.position.y),
		int(num.size.x),
		int(num.size.y)
	};
	Texture numtex{
		sdlutils().renderer(),
		wavenum < 10 ? "0" + std::to_string(std::max(wavenum,0)) + "/10" : std::to_string(wavenum) + "/10",
		sdlutils().fonts().at("RUBIK_MONO"),
		SDL_Color(50,50,50,255) };
	numtex.render(numtrue);
#pragma endregion


#pragma region timer
	int wavetime = 60 - (_wm->get_wave_time() / 1000);
	
	rect_f32 timer = rect_f32_screen_rect_from_viewport(rect_f32({ 0.45,0.05 }, { 0.1,0.2 }), _camera->cam.screen);
	SDL_Rect timertrue{
		int(timer.position.x),
		int(timer.position.y),
		int(timer.size.x),
		int(timer.size.y)
	};
	Texture timertex{
		sdlutils().renderer(),
		wavetime < 10 ? "0" + std::to_string(std::max(wavetime,0)) : std::to_string(wavetime),
		sdlutils().fonts().at("RUBIK_MONO"),
		SDL_Color(wavetime <= 10 ? 200 : 50,50,50,255) };
	timertex.render(timertrue);
#pragma endregion

#pragma region events
	if (_displaying_wave_event) {
		
		rect_f32 event = rect_f32_screen_rect_from_viewport(rect_f32({ 0.275,0.2 }, { 0.45,0.15 }), _camera->cam.screen);
		SDL_Rect eventtrue{
			int(event.position.x),
			int(event.position.y),
			int(event.size.x),
			int(event.size.y)
		};
		_event_textures[int(_current_event)]->render(eventtrue);
	}
#pragma endregion

#pragma region wave completed
	if (_wm->wave_completed()) {

		rect_f32 wave_completed = rect_f32_screen_rect_from_viewport(rect_f32({ 0.30,0.2 }, { 0.4,0.2 }), _camera->cam.screen);
		SDL_Rect wave_completed_true{
			int(wave_completed.position.x),
			int(wave_completed.position.y),
			int(wave_completed.size.x),
			int(wave_completed.size.y)
		};
		Texture wave_completed_tex{
		sdlutils().renderer(),
		"WAVE COMPLETED NYA",
		sdlutils().fonts().at("PROTEST_GUERRILLA100"),
		SDL_Color(128, 0, 32, 255) };
		wave_completed_tex.render(wave_completed_true);
	}
#pragma endregion

}

void HUD::start_new_wave()
{
	_current_event = _wm->get_current_event();
	_current_wave_event_time = 0;
	if (int(_current_event != events::NONE)) {
		
		_current_wave_event_time = sdlutils().currRealTime() + _wave_event_timeout;
		_displaying_wave_event = true;
	}
}