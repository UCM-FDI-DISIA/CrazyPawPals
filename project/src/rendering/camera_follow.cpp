
#include <cassert>
#include "camera_follow.hpp"

camera camera_update_from_follow(
    const camera &cam,
    camera_follow_descriptor &in_out_descriptor,
    const position2_f32 target,
    const seconds_f32 delta_time
) {
    assert(delta_time > 0.0f);
    const velocity2_f32 target_velocity = {
        (target.x - in_out_descriptor.previous_position.x) / delta_time,
        (target.y - in_out_descriptor.previous_position.y) / delta_time
    };
    const position2_f32 target_position = {
        target.x + target_velocity.dx * in_out_descriptor.lookahead_time,
        target.y + target_velocity.dy * in_out_descriptor.lookahead_time
    };

    displacement2_f32 direction = {
        target_position.x - cam.position.x,
        target_position.y - cam.position.y
    };
    const float distance = sqrtf(direction.dx * direction.dx + direction.dy * direction.dy);
    if (distance != 0.0) {
        direction.dx /= distance;
        direction.dy /= distance;
    }

    constexpr static const float ln2 = 0.693147180559945309417232121458176568075500134360255254120680009493393621969694715605863326996418687;
    const float closing_coefficient = ln2 / in_out_descriptor.semi_reach_time;
    float displacement = expf(-closing_coefficient * delta_time) * distance * delta_time;
    if (displacement > distance) {
        displacement = distance;
    }

    in_out_descriptor.previous_position = target;
    return camera{
        .position = {
            cam.position.x + direction.dx * displacement,
            cam.position.y + direction.dy * displacement
        },
        .half_size = cam.half_size
    };
}

camera camera_update_from_follow_multiple(
    const camera &cam,
    camera_follow_descriptor &in_out_descriptor,
    const camera_follow_target targets[],
    const size_t target_count,
    const seconds_f32 delta_time
) {
    float total_weight = 0.0f;
    position2_f32 weighted_position = {0.0f, 0.0f};
    for (size_t i = 0; i < target_count; ++i) {
        total_weight += targets[i].weight;
        weighted_position.x += targets[i].position.x * targets[i].weight;
        weighted_position.y += targets[i].position.y * targets[i].weight;
    }
    weighted_position.x /= total_weight;
    weighted_position.y /= total_weight;

    return camera_update_from_follow(cam, in_out_descriptor, weighted_position, delta_time);
}

camera camera_update_from_clamp(const camera cam, const camera_clamp_descriptor descriptor) {
    const position2_f32 bottom_left_bound = {
        descriptor.bounds.position.x - descriptor.bounds.size.x * 0.5f,
        descriptor.bounds.position.y - descriptor.bounds.size.y * 0.5f
    };
    const position2_f32 top_right_bound = {
        descriptor.bounds.position.x + descriptor.bounds.size.x * 0.5f,
        descriptor.bounds.position.y + descriptor.bounds.size.y * 0.5f
    };
    position2_f32 new_position = cam.position;
    if (cam.position.x - cam.half_size.x < bottom_left_bound.x) {
        new_position.x = bottom_left_bound.x + cam.half_size.x;
    } else if (cam.position.x + cam.half_size.x > top_right_bound.x) {
        new_position.x = top_right_bound.x - cam.half_size.x;
    }

    if (cam.position.y - cam.half_size.y < bottom_left_bound.y) {
        new_position.y = bottom_left_bound.y + cam.half_size.y;
    } else if (cam.position.y + cam.half_size.y > top_right_bound.y) {
        new_position.y = top_right_bound.y - cam.half_size.y;
    }

    return camera{
        .position = new_position,
        .half_size = cam.half_size
    };
}
