#include "DumbWaveManager.h"
#include "../../game/Game.h"
#include "../../ecs/Manager.h"
#include "../wave_events/no_event.hpp"
#include "../wave_events/ice_skating_event.hpp"
#include "../wave_events/star_shower_event.hpp"
#include "WaveManager.h"
DumbWaveManager::DumbWaveManager() {
    _current_wave_event = std::make_unique<no_event>();
}
void DumbWaveManager::start_wave(uint16_t ev)
{
    choose_new_event(events(ev));

    _currentWaveInitTime = sdlutils().virtualTimer().currRealTime();
    _current_wave_event->start_wave_callback();
    _currentWaveTime = 0;
    _wave_active = true;
}
void DumbWaveManager::end_wave()
{
    _wave_active = false;
    change_to_rewards_time = sdlutils().virtualTimer().currTime() + 3000;
    ++_currentWave;
    _current_wave_event->end_wave_callback();
    erase_all_bullets();
    erase_all_enemies();
}
void DumbWaveManager::update(uint32_t dt) {
    if (_wave_active) {
        _currentWaveTime = sdlutils().virtualTimer().currRealTime() - _currentWaveInitTime;
        _current_wave_event->update(dt);
        _has_changed_to_rewards = false;
    }
    else {
        //RENDER WIN WAVE BUTTON
        if (!_has_changed_to_rewards&&change_to_rewards_time < sdlutils().virtualTimer().currRealTime()) {
            _has_changed_to_rewards = true;
            Game::Instance()->queue_scene(Game::REWARDSCENE);
        }
    }
}
void DumbWaveManager::choose_new_event(events new_event)
{
    _current_event = new_event;
    switch (new_event) {
    case NONE:
        _current_wave_event = (std::unique_ptr<wave_event>)new no_event();
        break;
    case ICE_SKATE:
        _current_wave_event = (std::unique_ptr<wave_event>)new ice_skating_event();
        break;
    case STAR_SHOWER: {
        constexpr static const rect_f32 event_area = {
            .position = { 0.0f, 0.0f },
            .size = { 32.0f, 16.0f },
        };
        constexpr static const size_t min_drops_inclusive = 5;
        constexpr static const size_t max_drops_exclusive = 23;
        _current_wave_event = std::make_unique<star_shower_event>(
            event_area,
            star_drop_descriptor{
                .drop_position = { 0.0f, 0.0f },
                .damage_amount = 3,
                .drop_radius = 0.25f,
                .fall_time = 1.25f,
                .spawn_distance = 16.0f,
            },
            star_drop_descriptor{
                .drop_position = { 0.0f, 0.0f },
                .damage_amount = 24,
                .drop_radius = 2.0f,
                .fall_time = 8.0f,
                .spawn_distance = 32.0f,
            },
            min_drops_inclusive,
            max_drops_exclusive
            );
        break;
    }
    default: {
        assert(false && "unrachable"); // event_choser_went_wrong
        std::exit(EXIT_FAILURE);
    }
    }

    if (_current_event == NONE) sdlutils().soundEffects().at("round_start").play();
    else sdlutils().soundEffects().at("round_start_event").play();
}
void DumbWaveManager::erase_all_bullets()
{
    auto manager = Game::Instance()->get_mngr();
    for (auto e : manager->getEntities(ecs::grp::BULLET))
        manager->setAlive(e, false);
    for (auto e : manager->getEntities(ecs::grp::PLAYERBULLETS))
        manager->setAlive(e, false);
    for (auto e : manager->getEntities(ecs::grp::ENEMYBULLETS))
        manager->setAlive(e, false);
}

void DumbWaveManager::erase_all_enemies()
{
    auto manager = Game::Instance()->get_mngr();
    for (auto e : manager->getEntities(ecs::grp::ENEMY))
        manager->setAlive(e, false);
}
