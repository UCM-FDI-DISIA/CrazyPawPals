
#include "State.h"
#include <functional>

class Transform;
class MovementController;
class Follow;

class WalkingState : public State {
public:

	WalkingState(Transform* tr, MovementController* movement_controller, Follow* follow, bool toDestination = false);
	~WalkingState() {};
	void enter() override;
	void update(uint32_t delta_time) override;
	void exit() override;

protected:
	Transform* _tr;
	MovementController* _movement_controller;
	Follow* _fll;
	bool _to_destination;
};