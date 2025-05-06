#pragma once


#include "../../ecs/Component.h"
#include "../../game/scenes/GameScene.h"
#include "../../our_scripts/states/State.h"
#include <functional>

class ConditionManager;
class Transform;
class StateMachine : public ecs::Component {
public:

	__CMPID_DECL__(ecs::cmp::STATEMACHINE);

	using StatePtr = std::shared_ptr<State>;
    using TransitionCondition = std::function<bool()>;

	StateMachine();

	//StateMachine(ConditionManager& conditionManager, Transform* playerTransform, Transform* enemyTransform, float dist);

	void add_state(const std::string& name, StatePtr state);

	void add_transition(const std::string& from, const std::string& to, TransitionCondition condition);

    void set_initial_state(const std::string& name);

    void transitionTo(const std::string& name);

	inline ConditionManager* getConditionManager() { return _condition_manager; };

    void enter();

    void exit();


	virtual ~StateMachine();

	void update(uint32_t delta_time) override;

	//void initComponent() override;

protected:
	State* _state;

	std::unordered_map<std::string, StatePtr> _states; // Mapa de estados
    std::string _currentState; // Estado actual

    struct Transition {
        std::string targetState;
        TransitionCondition condition;
    };

    std::unordered_map<std::string, std::vector<Transition>> _transitions; // Transiciones
	ConditionManager* _condition_manager;
};

