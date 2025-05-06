
#include "StateMachine.h"
#include "../states/State.h"
#include "../states/Conditions.h"
#include "../states/AttackingState.h"
#include "../states/WalkingState.h"
#include "../components/movement/Transform.h"

#include <iostream>

StateMachine::StateMachine() {
	_condition_manager = new ConditionManager();
}

StateMachine::~StateMachine() {
	delete _condition_manager;
}
void StateMachine::add_state(const std::string& name, StatePtr state) {
	_states[name] = state;
}

void StateMachine::add_transition(const std::string& from, const std::string& to, TransitionCondition condition) {
	_transitions[from].push_back({to, condition});
}

void StateMachine::set_initial_state(const std::string& name) {
	_currentState = name;
	if (_states.count(_currentState)) {
		_states[_currentState]->enter();
	}
}

void StateMachine::transitionTo(const std::string& name) {
	_currentState = name;
	if (_states.count(_currentState)) {
		_states[_currentState]->enter();
	}
}

void StateMachine::update(uint32_t delta_time) {
	if (_states.count(_currentState)) {
		_states[_currentState]->update(delta_time);
		//std::cout << "Current state: " << _currentState << std::endl;
		// Check for transitions
		// Iterate through the transitions for the current state
		for (const auto& transition : _transitions[_currentState]) {
			if (transition.condition()) {
				_states[_currentState]->exit();
				_currentState = transition.targetState;
				_states[_currentState]->enter();
				break;
			}
		}
	}
}

void StateMachine::enter() {
	if (_states.count(_currentState)) {
		_states[_currentState]->enter();
	}
}

void StateMachine::exit() {
	if (_states.count(_currentState)) {
		_states[_currentState]->exit();
	}
}

