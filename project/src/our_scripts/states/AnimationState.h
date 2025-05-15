
#include "State.h"
#include "../components/rendering/dyn_image_with_frames.hpp"

class AnimationState : public State
{
public:
	AnimationState(dyn_image_with_frames& anim, int start_frame, int end_frame, uint32_t frame_duration);
	void enter() override;
    void update(uint32_t delta_time) override {};
    void exit() override {};

private:
    dyn_image_with_frames& _anim;
    const int _start_frame;
    const int _end_frame;
    const uint32_t _frame_duration;
};