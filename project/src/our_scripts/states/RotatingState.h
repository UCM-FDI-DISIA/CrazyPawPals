
#include "State.h"
#include "../../utils/Vector2D.h"

class MovementController;
class Transform;
class Follow;
class RotatingState : public State
{
public:
	RotatingState(Transform* tr, Follow* follow, MovementController* mc);
	void enter() override;
	void update(uint32_t delta_time) override;
	void exit() override;

protected:
	Transform* _tr;
	MovementController* _movementController;
	Follow* _fll;
	float _angle;
	Vector2D _center;
	float _radius;
	const float _ANGULAR_SPEED = 1.5f;

};