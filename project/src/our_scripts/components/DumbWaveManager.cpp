#include "DumbWaveManager.h"
#include "../../game/Game.h"
#include "../../ecs/Manager.h"
/*
void DumbWaveManager::initComponent()
{
}
*/
void DumbWaveManager::start_wave()
{
    _currentWaveInitTime = sdlutils().virtualTimer().currRealTime();
    _current_wave_event->start_wave_callback();
    _wave_active = true;
}
void DumbWaveManager::end_wave()
{
    _wave_active = false;
    ++_currentWave;
    _current_wave_event->end_wave_callback();
    erase_all_bullets();
    erase_all_enemies();
}
void DumbWaveManager::update(uint32_t dt) {
    if (_wave_active) {
        _currentWaveTime = sdlutils().virtualTimer().currRealTime() - _currentWaveInitTime;
        _current_wave_event->update(dt);
    }
    else {
        //RENDER WIN WAVE BUTTON
        if (change_to_rewards_time < sdlutils().virtualTimer().currRealTime()) {
            Game::Instance()->change_Scene(Game::REWARDSCENE);
        }
    }
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
