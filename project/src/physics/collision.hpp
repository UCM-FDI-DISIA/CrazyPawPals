#ifndef COLLISION_HPP
#define COLLISION_HPP


#include "../rendering/rect.hpp"
#include "../rendering/units.hpp"
#include "rigidbody.hpp"
#include <array>

size_t rect_f32_intersects_segment(
    const rect_f32 rect,
    const position2_f32 a,
    const position2_f32 b,
    position2_f32 out_intersections[2]
);

struct collision_body {
    rigidbody body;
    float restitution;
};

struct collision_penetration {
    vec2_f32 penetration;
};

struct collision_contact {
    collision_penetration penetration;
    vec2_f32 normal;
    float delta_of_collision_normalised;
};

bool collision_body_check(
    const collision_body &body0,
    const collision_body &body1,
    const seconds_f32 delta_time,
    collision_contact &out_contact
);

bool collision_body_check_broad(
    const collision_body &body0,
    const collision_body &body1
);


struct collision_penetration_response {
    vec2_f32 separation;
};

struct collision_restitution_response {
    vec2_f32 restitution_displacement;
};

struct collision_response_pairs {
    std::array<collision_penetration_response, 2> penetration_responses;
    std::array<collision_restitution_response, 2> restitution_responses;
};

collision_response_pairs collision_body_resolve(
    const collision_body &body0,
    const collision_body &body1,
    const collision_contact &contact
);

#endif
