
#include "AnimationState.h"


AnimationState::AnimationState(dyn_image_with_frames& anim, int start_frame, int end_frame, uint32_t frame_duration)
    : _anim(anim),_start_frame(start_frame),_end_frame(end_frame),
    _frame_duration(frame_duration)
{}

void AnimationState::enter() {
    _anim.ini_frame = _start_frame;
    _anim.end_frame = _end_frame;
    _anim.current_frame = _start_frame;
    _anim.frame_duration = _frame_duration;
    _anim.next_frame_time = 0;
}
