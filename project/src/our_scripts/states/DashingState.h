
#include "State.h"

class Transform;
class MovementController;
class Follow;

class DashingState : public State {
public:
	DashingState(Transform* tr, MovementController* movementController, Follow* follow, uint32_t time = 1000, float extra_space = 1.8f);
	~DashingState() {};
	void enter() override;
	void update(uint32_t delta_time) override{
		(void)delta_time;
	};
	void exit() override;

protected:
	Transform* _tr;
	MovementController* _movementController;
	Follow* _fll;
	
	float _extra_space;
	uint32_t _time;
};