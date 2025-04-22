#include <vector>
#include <random>
#include "WaveManager.h"
#include "ecs/Manager.h"
#include "game/Game.h"
#include "game/scenes/GameScene.h"
#include "sdlutils/SDLUtils.h"
#include "../components/Fog.h"
#include "../wave_events/no_event.hpp"
#include "../wave_events/ice_skating_event.hpp"
#include "../wave_events/star_shower_event.hpp"
#include "../log_writer_to_csv.hpp"

#ifdef GENERATE_LOG
#include "Health.h"
#include "cards/Mana.h"
#include "cards/Deck.hpp"
#include "KeyboardPlayerCtrl.h"
#include "movement/MovementController.h"
#endif


// 1 segundo = 1000 ticks (ms)
WaveManager::WaveManager() :
    _currentWaveTime(0),
    _waveTime(5000), //60000 !!
    _currentWave(-1),
    _wave_active(false),
    _enemiesSpawned(0),
    _enemiesKilled(0),
    _numEnemies(0),
    enemy_index(0),
    _current_wave_event(new no_event(this))
{
    event_system::event_manager::Instance()->suscribe_to_event(event_system::enemy_dead, this, &event_system::event_receiver::event_callback0);
    event_system::event_manager::Instance()->suscribe_to_event(event_system::player_dead, this, &event_system::event_receiver::event_callback1);

    enemies_premade_waves = std::vector<std::vector<enemyType>>{
    std::vector<enemyType>{sarno_rata, michi_mafioso},
    std::vector<enemyType>{plim_plim, ratatouille, plim_plim, sarno_rata, plim_plim, plim_plim},
    std::vector<enemyType>{boom, michi_mafioso, boom, michi_mafioso, boom, boom, michi_mafioso, michi_mafioso}
    };
}

WaveManager::~WaveManager() {
    event_system::event_manager::Instance()->unsuscribe_to_event(event_system::enemy_dead, this, &event_system::event_receiver::event_callback0);
    event_system::event_manager::Instance()->unsuscribe_to_event(event_system::player_dead, this, &event_system::event_receiver::event_callback1);
}

void
WaveManager::initComponent() {
    fog = Game::Instance()->get_mngr()->getComponent<Fog>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::FOGGROUP));
    assert(fog != nullptr);
}

bool WaveManager::can_spawn_next_enemy()
{
   // std::cout << _next_spawn_time << " - " << sdlutils().virtualTimer().currTime() << std::endl;
    return _next_spawn_time < sdlutils().virtualTimer().currTime() && enemy_index < enemies_premade_waves[_currentWave].size();//tokens_for_this_wave > 0;
}

bool WaveManager::is_wave_finished()
{
    //TODO: Necesitamos no notificar los enemigos que son creados por otros
    //std::cout << "enemies_killed: " << _enemiesKilled << "   numEnemies: " << _numEnemies << "    enemies_spawned: " << _enemiesSpawned << std::endl;
    return _enemiesKilled >= _numEnemies && enemy_index == enemies_premade_waves[_currentWave].size();//_all_enemies_already_spawned;
}

void WaveManager::erase_all_enemies()
{
    auto manager = Game::Instance()->get_mngr();
    for (auto e : manager->getEntities(ecs::grp::ENEMY))
        manager->setAlive(e, false);
}

void WaveManager::erase_all_bullets()
{
    auto manager = Game::Instance()->get_mngr();
    for (auto e : manager->getEntities(ecs::grp::BULLET))
        manager->setAlive(e, false);
    for (auto e : manager->getEntities(ecs::grp::PLAYERBULLETS))
        manager->setAlive(e, false);
    for (auto e : manager->getEntities(ecs::grp::ENEMYBULLETS))
        manager->setAlive(e, false);
}

//Chooses enemies in _enemy_types_for_current_wave
void WaveManager::initialize_next_wave_params(bool normal_wave)
{
    tokens_for_this_wave = 99;//_currentWave * spawn_tokens_gained_per_wave + spawn_tokens_at_wave_0;

    uint8_t cheaper_enemy;
    for (uint8_t i = 0; i < 3; ++i) {
        uint8_t j = 0;
        do {
            j = 0;
            //Chooses new random enemy
            _enemy_types_for_current_wave[i] = sdlutils().rand().nextInt(0, (int)rata_basurera);
        } while (
            j < i && //This is false for (i==0)
            _enemy_types_for_current_wave[j] != _enemy_types_for_current_wave[i] && //This is false if enemy chosen for index 1 || 2 is alredy taken in index 0
            ++j < i && //This is false for (i==1)
            _enemy_types_for_current_wave[j] != _enemy_types_for_current_wave[i]  //This is false if enemy chosen for index 2 is alredy taken in index 1
            );
        cheaper_enemy = std::min(cheaper_enemy, enemy_spawn_data[_enemy_types_for_current_wave[i]].enemies_group_spawn_cost);
    }
    time_max_between_enemy_spawns_on_this_wave = max_spawn_wave_time / (tokens_for_this_wave / cheaper_enemy);
    _next_spawn_time = sdlutils().virtualTimer().currTime();// +time_max_between_enemy_spawns_on_this_wave;
    //Si no es normal wave spawnea tb un bos
}

void WaveManager::spawn_next_group_of_enemies()
{
    //ONLY ENTERS HERE IF TOKENS LEFT > 0 
    //rest tokens
    uint8_t index = sdlutils().rand().nextInt(0, 3);
    //tokens can only be -1 at worst at end of the round (cause I know that there will always be at least a 2 cost enemy on the group)
    while ((tokens_for_this_wave - enemy_spawn_data[_enemy_types_for_current_wave[index]].enemies_group_spawn_cost) < -1) {
        index = ++index % 3;
        std::cout << (tokens_for_this_wave - enemy_spawn_data[_enemy_types_for_current_wave[index]].enemies_group_spawn_cost) << std::endl;
    }
    tokens_for_this_wave -= enemy_spawn_data[_enemy_types_for_current_wave[index]].enemies_group_spawn_cost;
    //spawn enemies
    enemy_spawn_caller* esc;
    std::string tipoEnemigo;
    switch ((enemyType)enemies_premade_waves[_currentWave][enemy_index])
    {
    case sarno_rata:
        esc = new enemy_spawn_caller([](Vector2D v) {GameScene::spawn_sarno_rata(v); });
        tipoEnemigo = "sarno rata";
        break;
    case michi_mafioso:
        esc = new enemy_spawn_caller([](Vector2D v) {GameScene::spawn_michi_mafioso(v); });
        tipoEnemigo = "michi mafioso";
        break;
    case plim_plim:
        esc = new enemy_spawn_caller([](Vector2D v) {GameScene::spawn_plim_plim(v); });
        tipoEnemigo = "plim plim";
        break;
    case boom:
        esc = new enemy_spawn_caller([](Vector2D v) {GameScene::spawn_boom(v); });
        tipoEnemigo = "boom";
        break;
    case ratatouille:
        esc = new enemy_spawn_caller([](Vector2D v) {GameScene::spawn_ratatouille(v); });
        tipoEnemigo = "ratatouille";
        break;
    case catkuza:
        esc = new enemy_spawn_caller([](Vector2D v) {GameScene::spawn_catkuza(v); });
        tipoEnemigo = "catkuza";
        break;
    case super_michi_mafioso:
        esc = new enemy_spawn_caller([](Vector2D v) {GameScene::spawn_super_michi_mafioso(v); });
        tipoEnemigo = "super michi mafioso";
        break;
    case rata_basurera:
        esc = new enemy_spawn_caller([](Vector2D v) {GameScene::spawn_rata_basurera(v); });
        tipoEnemigo = "rata basurera";
        break;
    default: {
        assert(false && "unreachable");
        exit(EXIT_FAILURE);
        break;
    }
    }
    for (uint8_t i = 0; i < enemy_spawn_data[enemies_premade_waves[_currentWave][enemy_index]].number_of_enemies_simultaneous_spawn; ++i) {
        esc->spawn_callback();
    }
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log("SPAWN ENEMIES", "TIPO", tipoEnemigo, "Numero", std::to_string(enemy_spawn_data[_enemy_types_for_current_wave[index]].number_of_enemies_simultaneous_spawn));
#endif

    delete esc;
    _numEnemies += enemy_spawn_data[enemies_premade_waves[_currentWave][enemy_index]].number_of_enemies_simultaneous_spawn;
    //sets next spawn time
    float multiplier = ((sdlutils().rand().nextInt(0, 100) * 0.001) * 0.3 + 0.7);
    auto add_to_crono = (uint32_t)(time_max_between_enemy_spawns_on_this_wave * multiplier);
    _next_spawn_time = sdlutils().virtualTimer().currTime() + add_to_crono;

    _all_enemies_already_spawned = tokens_for_this_wave <= 0;
    ++enemy_index;
}
//---------------------------------------------------------------------------------------------------------------------------------
void WaveManager::update(uint32_t delta_time) {

    if (_wave_active) {
        _currentWaveTime = sdlutils().virtualTimer().currRealTime() - _currentWaveInitTime;
        _current_wave_event->update(delta_time);
        //tries spawning enemies
        if (can_spawn_next_enemy())
            spawn_next_group_of_enemies();

        if (is_wave_finished())
            endwave();

        if (_currentWaveTime > 50 * 1000 && !is_wave_finished()) {
            activateFog();
        }
    }
    else {
        //RENDER WIN WAVE BUTTON
        if (change_to_rewards_time < sdlutils().virtualTimer().currRealTime()) {
            enterRewardsMenu();
        }
    }
#ifdef GENERATE_LOG
    WaveManager::_ticks_on_wave++;
#endif
}
//---------------------------------------------------------------------------------------------------------------------------------

//Verifica si todos los enemigos estan muertos
bool
WaveManager::areAllEnemiesDead() {
    //std::cout << _enemiesKilled << " vs " << _numEnemies;
    return _enemiesKilled >= _numEnemies;
}

//Activa la niebla
void
WaveManager::activateFog() {
    fog->setFog(true);
    //std::cout << "Niebla activada!" << std::endl;
}


void
WaveManager::enterRewardsMenu() {
    Game::Instance()->change_Scene(Game::REWARDSCENE);
}

void WaveManager::start_new_wave()
{
    _currentWaveInitTime = sdlutils().virtualTimer().currRealTime();
    //Si es oleada de boss es true
    initialize_next_wave_params(_currentWave % 5 == 0);

    // Esto tiene que ir después del menu de recompensas
    _currentWave++;
    _currentWaveTime = 0;
    _enemiesSpawned = 0;
    _enemiesKilled = 0;
    _numEnemies = 0;
    enemy_index = 0;
    _wave_active = true;
#ifdef GENERATE_LOG
    WaveManager::_ticks_on_wave = 1;
#endif
    fog->setFog(false);

    auto& mngr = *Game::Instance()->get_mngr();
    auto enemies = mngr.getEntities(ecs::grp::ENEMY);
    for (auto e : enemies) {
        mngr.setAlive(e, false);
    }

    choose_new_event();
}

void WaveManager::reset_wave_manager()
{
    _currentWave = -1;
}


void WaveManager::endwave()
{
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log();
    log_writer_to_csv::Instance()->add_new_log("WAVE", _currentWave, "FINISHED");
    auto player = Game::Instance()->get_mngr()->getEntities(ecs::grp::PLAYER)[0];
    auto player_hp = Game::Instance()->get_mngr()->getComponent<Health>(player);
    auto player_mana = Game::Instance()->get_mngr()->getComponent<ManaComponent>(player);
    auto player_deck = Game::Instance()->get_mngr()->getComponent<Deck>(player);
    auto player_keyboard_controller = Game::Instance()->get_mngr()->getComponent<KeyboardPlayerCtrl>(player);
    auto player_movement_controller = Game::Instance()->get_mngr()->getComponent<MovementController>(player);
    log_writer_to_csv::Instance()->add_new_log("VIDA PLAYER", "CURRENT", player_hp->getHealth(), "MAX", player_hp->getMaxHealth());
    log_writer_to_csv::Instance()->add_new_log("ENEMIGOS SPAWNEADOS", _numEnemies);
    log_writer_to_csv::Instance()->add_new_log("MANA", "MID", player_mana->get_mana_mid(), "MAX", player_mana->get_mana_max());
    log_writer_to_csv::Instance()->add_new_log("TIMES RELOADED", std::to_string(player_deck->times_reloaded));
    log_writer_to_csv::Instance()->add_new_log("TIMES M1 USED", player_keyboard_controller->times_m1_used);
    log_writer_to_csv::Instance()->add_new_log("TIMES M2 USED", player_keyboard_controller->times_m2_used_cards, "TIMES M2 COULDNT USE CARD", player_keyboard_controller->times_m2_failed_to_use_cards);
    log_writer_to_csv::Instance()->add_new_log("DISTANCIA RECORRIDA", player_movement_controller->total_movement);
    log_writer_to_csv::Instance()->add_new_log("USOS DE CADA CARTA EN ESTA RONDA");
    for (auto c : player_keyboard_controller->cards_used_this_round) {
        log_writer_to_csv::Instance()->add_new_log(c.first, std::to_string(c.second));
    }
    log_writer_to_csv::Instance()->add_new_log("DESCARTES DE CADA CARTA ESTA RONDA");
    for (auto c : player_keyboard_controller->cards_discarded_this_round) {
        log_writer_to_csv::Instance()->add_new_log(c.first, std::to_string(c.second));
    }
    player_mana->reset_mana_mid();
    player_deck->times_reloaded = 0;
    player_keyboard_controller->times_m1_used = 0;
    player_keyboard_controller->times_m2_used_cards = 0;
    player_keyboard_controller->times_m2_failed_to_use_cards = 0;
    player_keyboard_controller->cards_used_this_round = player_keyboard_controller->cards_discarded_this_round = std::unordered_map<std::string, uint8_t>();
    player_movement_controller->total_movement = 0;

#endif
    if (_currentWave == 2) {
        Game::Instance()->get_mngr()->getComponent<WaveManager>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::WAVE))->reset_wave_manager();
        Game::Instance()->change_Scene(Game::State::VICTORY);
    }
    else {
        _wave_active = false;
        change_to_rewards_time = sdlutils().virtualTimer().currTime() + 3000;
        _current_wave_event->end_wave_callback();
        _all_enemies_already_spawned = false;
        fog->setFog(false);
        erase_all_bullets();
        erase_all_enemies();
        //enterRewardsMenu();
    }

}

void WaveManager::event_callback0(const Msg& m)
{
    (void)m;
    _enemiesKilled++;
}

void WaveManager::event_callback1(const Msg& m)
{
    _current_wave_event->end_wave_callback();
    erase_all_enemies();
    erase_all_bullets();
    _current_wave_event = std::make_unique<no_event>(this);
    fog->setFog(false);
}

void WaveManager::choose_new_event()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rnd_gen(NONE, EVENTS_MAX - 1);
    _current_event = events(rnd_gen(gen));

    _current_event = _currentWave == 1 ? STAR_SHOWER : NONE;

    switch (_current_event) {
    case NONE:
        _current_wave_event = (std::unique_ptr<wave_event>)new no_event(this);
        break;
    case ICE_SKATE:
        _current_wave_event = (std::unique_ptr<wave_event>)new ice_skating_event(this);
        break;
    case STAR_SHOWER: {
        constexpr static const rect_f32 event_area = {
            .position = { 0.0f, 0.0f },
            .size = { 32.0f, 16.0f },
        };
        constexpr static const size_t min_drops_inclusive = 5;
        constexpr static const size_t max_drops_exclusive = 23;
        _current_wave_event = std::make_unique<star_shower_event>(
            *this,
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


    _current_wave_event->start_wave_callback();

    //TODO elegir evento y llamar a la función de iniciar
}

void enemy_spawn_caller::spawn_callback()
{
    //Choose random spawn pos
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> rAngGen(0.0f, 360.0f);
    float rAng = rAngGen(gen); // (0, 360)
    Vector2D posVec = Vector2D(cos(rAng) * Game::Instance()->get_world_half_size().first, Game::Instance()->get_world_half_size().second * sin(rAng));
    //spawn
    spawn_call(posVec);
}
