
#include "AttackingState.h"

#include "../../game/scenes/GameScene.h"
#include "../components/movement/Transform.h"
#include "../components/weapons/Weapon.h"
#include "../components/movement/Follow.h"


AttackingState::AttackingState(Transform* tr, Follow* follow, Weapon* weapon, bool can_attack, OnAttackCallback onAttackCallback, int attact_times) :
	_tr(tr), _fll(follow), _weapon(weapon), _on_attack_callback(onAttackCallback), _attack_times(attact_times), _contador(0), _can_attack(can_attack) 
{
}

void AttackingState::enter() {
	assert(_tr != nullptr);
    assert(_weapon != nullptr);
    assert(_fll != nullptr);

	if (_contador < _attack_times) {
		_contador++;
		Vector2D _target = _fll->get_act_follow()->getPos();
		_weapon->shoot(_target);
		if (_weapon->shoot(_target))	sdlutils().soundEffects().at("enemy_shot").play();
		if (_on_attack_callback) _on_attack_callback();
	}
}

void AttackingState::update(uint32_t delta_time) {
	(void)delta_time;
	if (_tr == nullptr || _fll == nullptr ||_weapon==nullptr) return;

	if (_contador < _attack_times || _can_attack) {
		_contador++;   
		Vector2D _target = _fll->get_act_follow()->getPos();
		_weapon->shoot(_target);

		if (_on_attack_callback) {
			if(_on_attack_callback())
			 	_can_attack = false;;
		}
	}
}

void AttackingState::exit() {
	_contador = 0;
}
