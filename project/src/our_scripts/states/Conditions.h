
#include <functional>
#include <string>
#include <unordered_map>
#include <iostream>

#include "../components/Health.h"
#include "../../sdlutils/SdlUtils.h"
#include "../components/movement/Transform.h"

class ConditionManager {
private:
    struct Timers {
        uint32_t duration;
        uint32_t last_used;
    };

    std::unordered_map<std::string, Timers> timers; //temporizadores de los estados, cuanto duran
    
    std::vector<std::string> patterns;
    std::unordered_map<std::string, int> patternCounters;

    size_t currentPatternIndex = 0;

    // Elegir patron aleatorio
    size_t get_random_pattern() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, patterns.size() - 1);
        return dis(gen);
    }
public:
    // Métodos para evaluar condiciones

    bool is_player_near(const Transform* _player, const Transform* _enemy, float _dist) const {
		if (!_player || !_enemy) return false;
        float distance = (_player->getPos() - _enemy->getPos()).magnitude();
        return distance < _dist;
    }

    void set_cooldown(const std::string& state, uint32_t cooldown) {
        timers[state].duration = cooldown;
        
        timers[state].last_used = 0;
    }

    void add_pattern(const std::string& pattern, int maxStates) {
        patterns.push_back(pattern);
        patternCounters[pattern] = maxStates;
    }

    // Comprobar si se puede usar la accion
    bool can_use(const std::string& action, uint32_t currentTime) {

        auto it = timers.find(action);
        if (it == timers.end()) {
            return true; // Si no tiene cooldown se puede usar
        }
        return (currentTime - timers[action].last_used) > timers[action].duration;
    }

    // Reiniciar cooldown cuando se usa la accion
    void reset_cooldown(const std::string& action, uint32_t currentTime) {
        timers[action].last_used = currentTime;
    }

    // Cambiar de patron de manera aleatoria
    void switch_pattern() {
        if (patterns.empty()) return;

        std::string currentPattern = patterns[currentPatternIndex];
        patternCounters[currentPattern]--;

        if (patternCounters[currentPattern] <= 0) {
            // Seleccionar un patrón aleatorio
            size_t newPatternIndex = get_random_pattern();
            currentPatternIndex = newPatternIndex;
            std::string nextPattern = patterns[currentPatternIndex];
            
            patternCounters[nextPattern] = patternCounters[nextPattern];
        }
    }

    std::string get_current_pattern() const {
        if (patterns.empty()) return "";
        return patterns[currentPatternIndex];
    }
    
};