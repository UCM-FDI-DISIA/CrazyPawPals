#ifndef CAMERA_FOLLOW_HPP
#define CAMERA_FOLLOW_HPP


#include "camera.hpp"
#include "units.hpp"

struct camera_follow_descriptor {
    position2_f32 previous_position;
    float lookahead_time;
    float semi_reach_time;
};


template <typename Numeric>
struct displacement2 {
    Numeric dx;
    Numeric dy;
};

using displacement2_f32 = displacement2<float>;
using velocity2_f32 = displacement2_f32;
camera camera_update_from_follow(
    const camera &cam,
    camera_follow_descriptor &in_out_descriptor,
    const position2_f32 target,
    const seconds_f32 delta_time
);


struct camera_follow_target {
    position2_f32 position;
    float weight;
};
camera camera_update_from_follow_multiple(
    const camera &cam,
    camera_follow_descriptor &in_out_descriptor,
    const camera_follow_target targets[],
    const size_t target_count,
    const seconds_f32 delta_time
);


struct camera_clamp_descriptor {
    rect_f32 bounds;
};
camera camera_update_from_clamp(
    const camera cam,
    const camera_clamp_descriptor descriptor
);

#endif
