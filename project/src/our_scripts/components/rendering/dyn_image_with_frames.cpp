
#include "dyn_image_with_frames.hpp"
#include "sdlutils/Texture.h"
#include "sdlutils/SDLUtils.h"


void dyn_image_with_frames::update(uint32_t delta_time)
{
    offset_dyn_image::update(delta_time);
    next_frame_time += delta_time; 
    if (next_frame_time >= frame_duration) {
        next_frame_time = 0;
        current_frame++;
        if (current_frame >= end_frame) current_frame = ini_frame;
        subrect.position.x = current_frame * subrect.size.x;
    }
}

void dyn_image_with_frames::set_animation(int ini, int end, uint32_t duration)
{
    ini_frame = ini;
    end_frame = end;
    current_frame = ini;
    frame_duration = duration;
    next_frame_time = 0;
}
