
#include "State.h"

#include <functional>
class Transform;
class Follow;
class Weapon;
class AttackingState : public State
{
public:
	using OnAttackCallback = std::function<bool()>;

	AttackingState(Transform* tr, Follow* follow, Weapon* weapon, bool can_attack = true, OnAttackCallback onAttackCallback = nullptr, int attact_times = 1);
	void enter() override;
	void update(uint32_t delta_time) override;
	void exit() override;

protected:
	Transform* _tr;
	Follow* _fll;

	Weapon* _weapon;

	OnAttackCallback _on_attack_callback;

	int _attack_times;
	int _contador;

	bool _can_attack;
};