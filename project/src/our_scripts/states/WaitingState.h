#include "../../utils/checkML.h"
#include "State.h"

#include <functional>
class WaitingState : public State
{
public:
	WaitingState();
	void enter() override;
	void update(uint32_t delta_time) override;
	void exit() override;
};