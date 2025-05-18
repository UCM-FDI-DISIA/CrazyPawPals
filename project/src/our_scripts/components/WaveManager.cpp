
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
#include "../../network/network_message.hpp"
#include "../components/net/GhostStateComponent.h"

#ifdef GENERATE_LOG
#include "../log_writer_to_csv.hpp"
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
    _current_wave_event(new no_event()),
    _event_pity(0)
{
    event_system::event_manager::Instance()->suscribe_to_event(event_system::enemy_dead, this, &event_system::event_receiver::event_callback0);
    event_system::event_manager::Instance()->suscribe_to_event(event_system::player_dead, this, &event_system::event_receiver::event_callback1);
}

WaveManager::~WaveManager() {
    event_system::event_manager::Instance()->unsuscribe_to_event(event_system::enemy_dead, this, &event_system::event_receiver::event_callback0);
    event_system::event_manager::Instance()->unsuscribe_to_event(event_system::player_dead, this, &event_system::event_receiver::event_callback1);
}

void
WaveManager::initComponent() {
	fog = Game::Instance()->get_mngr()->getComponent<Fog>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::FOGGROUP));
    assert(fog != nullptr);

	nPlayers = Game::Instance()->get_mngr()->getEntities(ecs::grp::PLAYER).size();
}

bool WaveManager::can_spawn_next_enemy()
{
    //std::cout << _next_spawn_time << " - " << sdlutils().virtualTimer().currTime() << std::endl;
    return _next_spawn_time < sdlutils().virtualTimer().currTime() && tokens_for_this_wave > 0;
}

bool WaveManager::is_wave_finished()
{
    //TODO: Necesitamos no notificar los enemigos que son creados por otros
    //std::cout << "enemies_killed: " << _enemiesKilled << "   numEnemies: " << _numEnemies << "    everyone out: " << _all_enemies_already_spawned << std::endl;
    return _enemiesKilled >= _numEnemies && _all_enemies_already_spawned;
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
	tokens_for_this_wave = (_currentWave * spawn_tokens_gained_per_wave) * (nPlayers == 1 ? 1 : nPlayers / 1.3f) + spawn_tokens_at_wave_0;

    float cheaper_enemy{(std::numeric_limits<uint8_t>::max)()};
    for (uint8_t i = 0; i < 3; ++i) {
        uint8_t j = 0;
        do {
            j = 0;
            //Chooses new random enemy
            _enemy_types_for_current_wave[i] = sdlutils().rand().nextInt(0, (int)rata_basurera+1);
        } while (
            j < i && //This is false for (i==0)
            _enemy_types_for_current_wave[j] != _enemy_types_for_current_wave[i] && //This is false if enemy chosen for index 1 || 2 is alredy taken in index 0
            ++j < i && //This is false for (i==1)
            _enemy_types_for_current_wave[j] != _enemy_types_for_current_wave[i]  //This is false if enemy chosen for index 2 is alredy taken in index 1
        );
        cheaper_enemy = std::min(cheaper_enemy,(float)enemy_spawn_data[_enemy_types_for_current_wave[i]].enemies_group_spawn_cost);
    }
    time_max_between_enemy_spawns_on_this_wave = std::min(max_spawn_wave_time / (tokens_for_this_wave / cheaper_enemy),5000.0f);
    _next_spawn_time = sdlutils().virtualTimer().currTime();// +time_max_between_enemy_spawns_on_this_wave;
    //Si no es normal wave spawnea tb un bos
}

void WaveManager::_spawn_boss()
{
    enemy_spawn_caller* esc;
    switch (sdlutils().rand().nextInt(0, 2)) {
    case 0:
        esc = new enemy_spawn_caller([](Vector2D v)->uint8_t{return GameScene::spawn_catkuza(v); });
        break;
    case 1:
        esc = new enemy_spawn_caller([](Vector2D v)->uint8_t{return GameScene::spawn_super_michi_mafioso(v); });
        break;
    default:
        assert(false && "unreachable");
        exit(EXIT_FAILURE);
        break;
    }
    esc->spawn_callback();
    delete esc;
    tokens_for_this_wave -= 3;
}

void WaveManager::spawn_next_group_of_enemies()
{
    //ONLY ENTERS HERE IF TOKENS LEFT > 0 
    //rest tokens
    uint8_t index = sdlutils().rand().nextInt(0, 3);
    //tokens can only be -1 at worst at end of the round (cause I know that there will always be at least a 2 cost enemy on the group)
    while ((tokens_for_this_wave - enemy_spawn_data[_enemy_types_for_current_wave[index]].enemies_group_spawn_cost) < -1) {
        index = ++index % 3;
        //std::cout << (tokens_for_this_wave - enemy_spawn_data[_enemy_types_for_current_wave[index]].enemies_group_spawn_cost) << std::endl;
    }
    tokens_for_this_wave -= enemy_spawn_data[_enemy_types_for_current_wave[index]].enemies_group_spawn_cost;
    //spawn enemies
    enemy_spawn_caller* esc;
    std::string tipoEnemigo;
    //std::cout << tipoEnemigo << std::endl;

    switch ((enemyType)_enemy_types_for_current_wave[index])
    {
        case sarno_rata:
            esc = new enemy_spawn_caller([](Vector2D v) ->uint8_t{return GameScene::spawn_sarno_rata(v); });
            tipoEnemigo = "sarno rata";
            break;
        case michi_mafioso:
            esc = new enemy_spawn_caller([](Vector2D v) ->uint8_t {return GameScene::spawn_michi_mafioso(v); });
            tipoEnemigo = "michi mafioso";
            break;
        case plim_plim:
            esc = new enemy_spawn_caller([](Vector2D v) ->uint8_t {return GameScene::spawn_plim_plim(v); });
            tipoEnemigo = "plim plim";
            break;
        case boom:
            esc = new enemy_spawn_caller([](Vector2D v) ->uint8_t {return GameScene::spawn_boom(v); });
            tipoEnemigo = "boom";
            break;
        case ratatouille:
            esc = new enemy_spawn_caller([](Vector2D v) ->uint8_t {return GameScene::spawn_ratatouille(v); });
            tipoEnemigo = "ratatouille";
            break;
        case catkuza:
            esc = new enemy_spawn_caller([](Vector2D v) ->uint8_t {return  GameScene::spawn_catkuza(v); });
            tipoEnemigo = "catkuza";
            break;
        case super_michi_mafioso:
            esc = new enemy_spawn_caller([](Vector2D v) ->uint8_t {return GameScene::spawn_super_michi_mafioso(v); });
            tipoEnemigo = "super michi mafioso";
            break;
        case rata_basurera:
            esc = new enemy_spawn_caller([](Vector2D v) ->uint8_t {return GameScene::spawn_rata_basurera(v); });
            tipoEnemigo = "rata basurera";
            break;
        default: {
            assert(false && "unreachable");
            exit(EXIT_FAILURE);
            break;
        }
    }
    if(Game::Instance()->is_host()) {
        if (Game::Instance()->get_network_state().connections.connected_users > 1) {
            for (uint8_t i = 0; i < enemy_spawn_data[_enemy_types_for_current_wave[index]].number_of_enemies_simultaneous_spawn; ++i) {
                GameStructs::DumbEnemyProperties dep;
                auto aux = esc->spawn_callback();//returns initial pos Vector2D

                dep._type = (enemyType)_enemy_types_for_current_wave[index]; //enemy ID
                dep._id = aux.second;
                dep._pos = aux.first;
                network_context& network = Game::Instance()->get_network();
                //mandar a todos los clientes el mensaje de que el host a dado al boton de play
                for (network_connection_size i = 0; i < network.profile.host.sockets_to_clients.connection_count; ++i) {
                    TCPsocket& client = network.profile.host.sockets_to_clients.connections[i];
                    network_message_pack_send(
                        client,
                        network_message_pack_create(network_message_type_create_enemy, create_enemy(dep))
                    );
                    std::cout << "Creando enemigo en el client: " << (int)i << " de tipo: " << (enemyType)dep._type << "con id: " << (enemyType)dep._id << std::endl;
                }
            }
        }
    }
    else {
        esc->spawn_callback();
    }
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log("SPAWN ENEMIES", "TIPO", tipoEnemigo, "Numero", std::to_string(enemy_spawn_data[_enemy_types_for_current_wave[index]].number_of_enemies_simultaneous_spawn));
#endif

    delete esc;
    //_numEnemies += enemy_spawn_data[_enemy_types_for_current_wave[index]].number_of_enemies_simultaneous_spawn;
    //sets next spawn time
    float multiplier = ((sdlutils().rand().nextInt(0, 100)*0.001) * 0.3 + 0.7);
    auto add_to_crono = (uint32_t)(time_max_between_enemy_spawns_on_this_wave * multiplier);
    _next_spawn_time = sdlutils().virtualTimer().currTime() + add_to_crono;

    _all_enemies_already_spawned = tokens_for_this_wave <= 0;
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
        else if (_currentWaveTime > 50 * 1000) {
            activateFog();
        }
    }else{
        //RENDER WIN WAVE BUTTON
        if (change_to_rewards_time < sdlutils().virtualTimer().currRealTime()) {
            enterRewardsMenu();
        }
    }
#ifdef GENERATE_LOG
    WaveManager::_ticks_on_wave++;
    #endif
    
    // std::cout << _numEnemies << " vs " << _enemiesKilled << std::endl;
}
//---------------------------------------------------------------------------------------------------------------------------------

//Activa la niebla
void 
WaveManager::activateFog() {
    fog->setFog(true);
    //std::cout << "Niebla activada!" << std::endl;
}


void 
WaveManager::enterRewardsMenu() {
    Game::Instance()->queue_scene(Game::REWARDSCENE);
}

void WaveManager::start_new_wave()
{
    _currentWave++;
    _currentWaveInitTime = sdlutils().virtualTimer().currRealTime();
    //Si es oleada de boss es true
    initialize_next_wave_params(_currentWave%5==0);

    // Esto tiene que ir después del menu de recompensas
    _currentWaveTime = 0;
    _enemiesSpawned = 0;
    _enemiesKilled = 0;
    _numEnemies = 0;
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

    auto player = mngr.getHandler(ecs::hdlr::PLAYER);
    if (mngr.hasComponent<GhostStateComponent>(player))mngr.removeComponent<GhostStateComponent>(player);
    choose_new_event();

    if ((_currentWave + 1) % 5 == 0)
        _spawn_boss();

    //START WAVE MESSAGE
    network_context& network = Game::Instance()->get_network();
    for (network_connection_size i = 0; i < network.profile.host.sockets_to_clients.connection_count; ++i) {
        TCPsocket& client = network.profile.host.sockets_to_clients.connections[i];
        network_message_pack_send(
            client,
            network_message_pack_create(network_message_type_start_wave, create_start_wave_message((uint16_t)get_current_event()))
        );
    }
}

void WaveManager::reset_wave_manager()
{
    _currentWave = -1;
    _event_pity = 0;
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
        log_writer_to_csv::Instance()->add_new_log(c.first,std::to_string(c.second));
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
    if (_currentWave == 9) {
        Game::Instance()->queue_scene(Game::State::VICTORY);
    }
    else {
        fog->setFog(false);
        _wave_active = false;
        change_to_rewards_time = sdlutils().virtualTimer().currTime() + 3000;
        _current_wave_event->end_wave_callback();
        _all_enemies_already_spawned = false;
        erase_all_bullets();
        erase_all_enemies();

        std::cout << "mensaje fin de oleada" << std::endl;

        //SEND END WAVE MESSAGE
        network_context& network = Game::Instance()->get_network();
        for (network_connection_size i = 0; i < network.profile.host.sockets_to_clients.connection_count; ++i) {
            TCPsocket& client = network.profile.host.sockets_to_clients.connections[i];
            network_message_pack_send(
                client,
                network_message_pack_create(network_message_type_end_wave, create_end_wave_message())
            );
        }
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
    _current_wave_event = std::make_unique<no_event>();
    fog->setFog(false);
}

void WaveManager::newEnemy()
{
    if (!Game::Instance()->is_client()) { _numEnemies++; _enemiesSpawned++; }
}

void WaveManager::select_event()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rnd_gen(ICE_SKATE, EVENTS_MAX - 1);
    _current_event = events(rnd_gen(gen));
    _event_pity = 0;
}
void WaveManager::choose_new_event()
{
    if (_event_pity == _max_event_pity) {
        select_event();
    }
    else {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> has_event(0, 2);
        if (has_event(gen) == 2) {
            select_event();
        }
        else {
            _current_event = NONE;
            _event_pity++;
        }
    }

    switch(_current_event) {
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


    _current_wave_event->start_wave_callback();

    //TODO elegir evento y llamar a la función de iniciar
}

std::pair<Vector2D,uint8_t> enemy_spawn_caller::spawn_callback()
{
    //Choose random spawn pos
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> rAngGen(0.0f, 360.0f);
    float rAng = rAngGen(gen); // (0, 360)
    Vector2D posVec = Vector2D(cos(rAng) * Game::Instance()->get_world_half_size().first, Game::Instance()->get_world_half_size().second * sin(rAng));
    //spawn
    uint8_t aux = spawn_call(posVec);

    return std::make_pair(posVec,aux);
}
