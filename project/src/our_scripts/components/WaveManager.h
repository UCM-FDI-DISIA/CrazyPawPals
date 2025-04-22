#pragma once

#include <functional>
#include "../../ecs/Component.h"
#include "../../sdlutils/SDLUtils.h"
#include "../wave_events/wave_event.hpp"
#include "rendering/transformless_dyn_image.h"
#include "../../utils/EventsSystem.hpp"

enum events {
    NONE = -1, // 0xffffffffff
    ICE_SKATE = 0,  // 0x00000000
    STAR_SHOWER = 1,
    EVENTS_MAX
};

enum enemyType {
    sarno_rata = 0,
    michi_mafioso = 1,
    plim_plim = 2,
    boom = 3,
    ratatouille = 4,
    rata_basurera = 5, //TODAVIA NO TIENE SPRITE POR ESO PETA, CUANDO TENGA --> CAMBIAR RATA BASURERA POR CATKUZA en el enemy randomizer
    catkuza = 6,
    super_michi_mafioso = 7,
};

struct enemy_spawn_definition {
    uint8_t number_of_enemies_simultaneous_spawn;//always more than 1
    uint8_t enemies_group_spawn_cost;//always more than 1
};
//index equals enemyType
constexpr enemy_spawn_definition enemy_spawn_data[8] = {
    {2,2}, //0
    {1,1}, //1
    {1,1}, //2
    {1,2}, //3
    {3,1}, //4
    {1,3}, //5
    {1,3}, //6
    {1,3}  //7
};
class enemy_spawn_caller {
    std::function<void(Vector2D)> spawn_call;
public:
    enemy_spawn_caller(std::function<void(Vector2D)> c): spawn_call(c) {}
    void spawn_callback();
};

class Fog;
constexpr int max_spawn_wave_time = 30 * 1000; //30 sec
constexpr int spawn_tokens_gained_per_wave = 4; 
constexpr int spawn_tokens_at_wave_0 = 3;

class WaveManager : public event_system::event_receiver, public ecs::Component {
private:
    void choose_new_event();
    void endwave();
    void activateFog();
    bool areAllEnemiesDead();
    void enterRewardsMenu();
    bool can_spawn_next_enemy();
    bool is_wave_finished();
    void erase_all_enemies();
    void erase_all_bullets();
    void select_event();

    uint32_t change_to_rewards_time;
#ifdef GENERATE_LOG
    inline static uint32_t _ticks_on_wave = 1;
#endif

    void spawn_next_group_of_enemies();
    void initialize_next_wave_params(bool normal_wave);
    Uint32 _currentWaveTime = 0; //tiempo actual (post calculo, inicial en constructor)
    Uint32 _currentWaveInitTime; // cuándo empezó la oleada
    Uint32 _waveTime; // cuánto dura la oleada (CONSTRUCTOR)

    events _current_event = NONE;
    int _currentWave = 0;
    int _enemy_types_for_current_wave[3];
    int time_max_between_enemy_spawns_on_this_wave;
    int tokens_for_this_wave;
    std::unique_ptr<wave_event> _current_wave_event;

    int _event_pity; // tracks waves since last event
    const int _max_event_pity = 3; // maximum number of waves w/o events

    bool _wave_active = false;

    int _numEnemies; // enemigos total en la oleada (post calculo)
    int _enemiesSpawned; // número de enemigos spawneados (post calculo)
	int _enemiesKilled; // número de enemigos muertos (post calculo)

    uint32_t _next_spawn_time; // cuándo se spawnea el siguiente enemigo (post calculo)

    // tiempo de spawn del siguiente enemigo (post calculo)
    float _min_time;
    float _op_time;

    bool _all_enemies_already_spawned = false;

    //transformless_dyn_image* _tdi;

    Fog* fog;
public:
    __CMPID_DECL__(ecs::cmp::WAVEMANAGER)
        WaveManager();
    virtual ~WaveManager() override;
    void update(uint32_t delta_time) override;
    void initComponent() override;
    void start_new_wave();
    void reset_wave_manager();

    inline Uint32 get_wave_time() { return _currentWaveTime; }
    inline void reset_wave_time() { _currentWaveTime = 0; }
    inline int get_current_wave() { return _currentWave; }
    inline events get_current_event() { return _current_event; }
    inline bool wave_completed() { return !_wave_active; }
    void event_callback0(const Msg& m) override;
    void event_callback1(const Msg& m) override;
    void newEnemy() { _numEnemies++; _enemiesSpawned++; };
#ifdef GENERATE_LOG
    static inline uint32_t get_ticks_on_wave() {
        return _ticks_on_wave;
    };
#endif
};