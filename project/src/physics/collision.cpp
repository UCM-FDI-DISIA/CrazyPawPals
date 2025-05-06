
#include "collision.hpp"
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <utility>

// source: adapted from https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
using outcode = uint_fast8_t;

constexpr static const outcode out_code_inside = 0b0000;
constexpr static const outcode out_code_left   = 0b0001;
constexpr static const outcode out_code_right  = 0b0010;
constexpr static const outcode out_code_bottom = 0b0100;
constexpr static const outcode out_code_top    = 0b1000;

struct rect_f32_min_max {
    position2_f32 min;
    position2_f32 max;
};
static outcode rect_f32_min_max_outcode(const rect_f32_min_max rect, const position2_f32 point) {
    outcode code = out_code_inside;
    if (point.x < rect.min.x) {
        code |= out_code_left;
    } else if (point.x > rect.max.x) {
        code |= out_code_right;
    }
    if (point.y < rect.min.y) {
        code |= out_code_bottom;
    } else if (point.y > rect.max.y) {
        code |= out_code_top;
    }
    return code;
}

enum rect_f32_intersection_segment {
    rect_f32_intersection_segment_0 = 0,
    rect_f32_intersection_segment_1,
    rect_f32_intersection_segment_2,
    rect_f32_intersection_segment_enclosed,
};
typedef uint8_t rect_f32_intersection_segment_result;

static rect_f32_intersection_segment_result cohen_sutherland_line_clip(
    const rect_f32_min_max rect,
    const position2_f32 a,
    const position2_f32 b,
    position2_f32 out_clipped[2]
) {
    outcode outcode_a = rect_f32_min_max_outcode(rect, a);
    outcode outcode_b = rect_f32_min_max_outcode(rect, b);

    if (!(outcode_a | outcode_b)) {
        out_clipped[0] = a;
        out_clipped[1] = b;

        return rect_f32_intersection_segment_enclosed;
    } else if (outcode_a & outcode_b) {
        return rect_f32_intersection_segment_0;
    }

    position2_f32 a_clipped = a;
    position2_f32 b_clipped = b;
    uint_fast8_t intersection_count = 0;

    if (outcode_a) {
        const float dx = b.x - a.x;
        const float dy = b.y - a.y;

        if (outcode_a & out_code_top) {
            a_clipped.x += dx * (rect.max.y - a.y) / dy;
            a_clipped.y = rect.max.y;
        } else if (outcode_a & out_code_bottom) {
            a_clipped.x += dx * (rect.min.y - a.y) / dy;
            a_clipped.y = rect.min.y;
        }
        
        if (outcode_a & out_code_right) {
            a_clipped.y += dy * (rect.max.x - a.x) / dx;
            a_clipped.x = rect.max.x;
        } else if (outcode_a & out_code_left) {
            a_clipped.y += dy * (rect.min.x - a.x) / dx;
            a_clipped.x = rect.min.x;
        }
        out_clipped[intersection_count] = a_clipped;
        ++intersection_count;
    }
    
    if (outcode_b) {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;

        if (outcode_b & out_code_top) {
            b_clipped.x += dx * (rect.max.y - b.y) / dy;
            b_clipped.y = rect.max.y;
        } else if (outcode_b & out_code_bottom) {
            b_clipped.x += dx * (rect.min.y - b.y) / dy;
            b_clipped.y = rect.min.y;
        }
        
        if (outcode_b & out_code_right) {
            b_clipped.y += dy * (rect.max.x - b.x) / dx;
            b_clipped.x = rect.max.x;
        } else if (outcode_b & out_code_left) {
            b_clipped.y += dy * (rect.min.x - b.x) / dx;
            b_clipped.x = rect.min.x;
        }

        out_clipped[intersection_count] = b_clipped;
        ++intersection_count;
    }
    
    switch (intersection_count) {
    case 0: {
        assert(false && "unreachable: if both out_codes were to be 0b0000, we should have taken the both inside case");
        std::exit(EXIT_FAILURE);
    }
    case 1: {
        return rect_f32_intersection_segment_1;
    }
    case 2: {
        return rect_f32_intersection_segment_2;
    }
    default: {
        assert(false && "unreachable: intersection_count should be 0, 1 or 2");
        std::exit(EXIT_FAILURE);
    }
    }
}

size_t rect_f32_intersects_segment(
    const rect_f32 rect,
    const position2_f32 a,
    const position2_f32 b,
    position2_f32 out_intersections[2]
) {
    const size2_f32 half_size = {
        .x = rect.size.x * 0.5f,
        .y = rect.size.y * 0.5f,
    };
    const rect_f32_min_max rect_min_max = {
        .min = {
            .x = rect.position.x - half_size.x,
            .y = rect.position.y - half_size.y,
        },
        .max = {
            .x = rect.position.x + half_size.x,
            .y = rect.position.y + half_size.y,
        },
    };

    const rect_f32_intersection_segment_result result = cohen_sutherland_line_clip(rect_min_max, a, b, out_intersections);
    switch (result) {
    case rect_f32_intersection_segment_0: {
        return 0;
    }
    case rect_f32_intersection_segment_1: {
        return 1;
    }
    case rect_f32_intersection_segment_2: {
        return 2;
    }
    case rect_f32_intersection_segment_enclosed: {
        return 2;
    }
    default: {
        assert(false && "unreachable: result should be 0, 1, 2 or enclosed");
        std::exit(EXIT_FAILURE);
    }
    }
}

static vec2_f32 rect_f32_normal_of_boundary_point(rect_f32 rect, const position2_f32 point) {
    const size2_f32 half_size = {
        .x = rect.size.x * 0.5f,
        .y = rect.size.y * 0.5f,
    };
    const position2_f32 center = {
        .x = rect.position.x,
        .y = rect.position.y,
    };
    const vec2_f32 to_point = {
        .x = point.x - center.x,
        .y = point.y - center.y,
    };
    const float dx = std::abs(to_point.x) - half_size.x;
    const float dy = std::abs(to_point.y) - half_size.y;
    if (dx > dy) {
        return vec2_f32{
            .x = std::copysignf(1.0f, to_point.x),
            .y = 0.0f,
        };
    } else {
        return vec2_f32{
            .x = 0.0f,
            .y = std::copysignf(1.0f, to_point.y),
        };
    }
}

bool collision_body_check(
    const collision_body &body0,
    const collision_body &body1,
    const seconds_f32 delta_time,
    collision_contact &out_contact
) {
    const rect_f32 stationary_minkowski_sum{
        .position{
            .x = 0.0f,
            .y = 0.0f,
        },
        .size{
            .x = body1.body.body.size.x + body0.body.body.size.x,
            .y = body1.body.body.size.y + body0.body.body.size.y,
        }
    };

    const vec2_f32 body0_displacement{
        .x = body0.body.space.position.x - body0.body.space.previous_position.x,
        .y = body0.body.space.position.y - body0.body.space.previous_position.y,
    };
    const vec2_f32 body1_displacement{
        .x = body1.body.space.position.x - body1.body.space.previous_position.x,
        .y = body1.body.space.position.y - body1.body.space.previous_position.y,
    };

    const vec2_f32 body0_from1_relative_displacement{
        .x = body0_displacement.x - body1_displacement.x,
        .y = body0_displacement.y - body1_displacement.y,
    };

    position2_f32 intersections[2];
    const position2_f32 origin{
        .x = (body0.body.space.previous_position.x + body0.body.body.position.x) - (body1.body.body.position.x + body1.body.space.previous_position.x),
        .y = (body0.body.space.previous_position.y + body0.body.body.position.y) - (body1.body.body.position.y + body1.body.space.previous_position.y),
    };
    const position2_f32 endpoint{
        .x = origin.x + body0_from1_relative_displacement.x,
        .y = origin.y + body0_from1_relative_displacement.y,
    };
    const size_t intersection_count = rect_f32_intersects_segment(
        stationary_minkowski_sum,
        origin,
        endpoint,
        intersections
    );
    if (intersection_count == 0) {
        return false;
    }

    position2_f32 closest_intersection;
    constexpr static const auto distance_sqr = [](const position2_f32 a, const position2_f32 b) -> float {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        return dx * dx + dy * dy;
    };

    if (intersection_count == 2) {
        const float distance_sqr_0 = distance_sqr(origin, intersections[0]);
        const float distance_sqr_1 = distance_sqr(origin, intersections[1]);
        if (distance_sqr_0 < distance_sqr_1) {
            closest_intersection = intersections[0];
        } else {
            closest_intersection = intersections[1];
        }
    } else {
        closest_intersection = intersections[0];
    }

    const vec2_f32 normal = rect_f32_normal_of_boundary_point(stationary_minkowski_sum, closest_intersection);

    constexpr static const auto inverse_lerp = [](const position2_f32 a, const position2_f32 b, const position2_f32 x) -> float {
        const float dx = b.x - a.x;
        const float dy = b.y - a.y;
        const float dx_x = x.x - a.x;
        const float dy_y = x.y - a.y;

        const float numerator = dx_x * dx + dy_y * dy;
        const float denominator = dx * dx + dy * dy;
        if (numerator == 0.0f && denominator == 0.0f) {
            return 1.0f;
        } else if (denominator == 0.0f) {
            return 0.0f;
        } else {
            return (numerator / denominator);
        }
    };
    
    out_contact = collision_contact{
        .penetration = vec2_f32{
            .x = endpoint.x - closest_intersection.x,
            .y = endpoint.y - closest_intersection.y,
        },
        .normal = normal,
        .delta_of_collision_normalised = inverse_lerp(origin, endpoint, closest_intersection), // TODO
    };
    return true;
}

bool collision_body_check_broad(const collision_body &body0, const collision_body &body1) {
    const size2_f32 max_size_doubled{
        std::max(body0.body.body.size.x, body1.body.body.size.x) * 2.0f,
        std::max(body0.body.body.size.y, body1.body.body.size.y) * 2.0f,
    };
    
    const position2_f32 broad0{
        body0.body.space.position.x / max_size_doubled.x,
        body0.body.space.position.y / max_size_doubled.y,
    };
    const position2_f32 broad1{
        body1.body.space.position.x / max_size_doubled.x,
        body1.body.space.position.y / max_size_doubled.y,
    };

    return std::abs(broad0.x - broad1.x) < 1.0f && std::abs(broad0.y - broad1.y) < 1.0f;
}

static vec2_f32 vec2_f32_reflect(const vec2_f32 normal, const vec2_f32 v) {
    const float dot = v.x * normal.x + v.y * normal.y;

    // v' = v - 2 * (v . n) * n
    return vec2_f32{
        .x = v.x - 2.0f * dot * normal.x,
        .y = v.y - 2.0f * dot * normal.y,
    };
}

#include <iostream>

collision_response_pairs collision_body_resolve(
    const collision_body &body0,
    const collision_body &body1,
    const collision_contact &contact
) {
    const inverse_mass_f32 inverse_mass_sum = {body0.body.mass_rcp.value + body1.body.mass_rcp.value};
    const float body0_separation_coefficient = body0.body.mass_rcp.value / inverse_mass_sum.value;
    const float body1_separation_coefficient = 1.0f - body0_separation_coefficient;

    const collision_penetration_response body0_penetration_response{
        .separation = vec2_f32{
            .x = -contact.penetration.penetration.x * body0_separation_coefficient,
            .y = -contact.penetration.penetration.y * body0_separation_coefficient,
        },
    };
    const collision_penetration_response body1_penetration_response{
        .separation = vec2_f32{
            .x = contact.penetration.penetration.x * body1_separation_coefficient,
            .y = contact.penetration.penetration.y * body1_separation_coefficient,
        },
    };

    constexpr static const auto dot = [](const vec2_f32 a, const vec2_f32 b) -> float {
        return a.x * b.x + a.y * b.y;
    };
    const vec2_f32 displacement_body0{
        .x = body0.body.space.position.x - body0.body.space.previous_position.x,
        .y = body0.body.space.position.y - body0.body.space.previous_position.y,
    };
    const vec2_f32 displacement_body1{
        .x = body1.body.space.position.x - body1.body.space.previous_position.x,
        .y = body1.body.space.position.y - body1.body.space.previous_position.y,
    };
    const seconds_f32 delta_of_collision_complement = 1.0f - contact.delta_of_collision_normalised;
    const vec2_f32 remaining_displacement_body0{
        .x = displacement_body0.x * delta_of_collision_complement,
        .y = displacement_body0.y * delta_of_collision_complement,
    };
    const vec2_f32 remaining_displacement_body1{
        .x = displacement_body1.x * delta_of_collision_complement,
        .y = displacement_body1.y * delta_of_collision_complement,
    };

    const float restitution_sum = body0.restitution + body1.restitution;
    const float max_restitution = (std::max)(body0.restitution, body1.restitution);

    const float restitution_body0_coefficient = (body0.restitution / restitution_sum) * max_restitution;
    const float restitution_body1_coefficient = (body1.restitution / restitution_sum) * max_restitution;

    const vec2_f32 reflected_displacement_body0 = vec2_f32_reflect(contact.normal, remaining_displacement_body0);
    const collision_restitution_response body0_restitution_response{
        .restitution_displacement = vec2_f32{
            .x = reflected_displacement_body0.x * restitution_body0_coefficient,
            .y = reflected_displacement_body0.y * restitution_body0_coefficient,
        },
    };
    
    const vec2_f32 reflected_displacement_body1 = vec2_f32_reflect(contact.normal, remaining_displacement_body1);
    const collision_restitution_response body1_restitution_response{
        .restitution_displacement = vec2_f32{
            .x = reflected_displacement_body1.x * restitution_body1_coefficient,
            .y = reflected_displacement_body1.y * restitution_body1_coefficient,
        },
    };
    
    const collision_restitution_response r0 =
        dot(remaining_displacement_body0, contact.normal) > dot(body0_restitution_response.restitution_displacement, contact.normal)
        ? collision_restitution_response{remaining_displacement_body0}
        : body0_restitution_response;
    const collision_restitution_response r1 =
        dot(remaining_displacement_body1, contact.normal) < dot(body1_restitution_response.restitution_displacement, contact.normal)
        ? collision_restitution_response{remaining_displacement_body1}
        : body1_restitution_response;
    
    const collision_penetration_response p0 =
        dot(remaining_displacement_body0, contact.normal) > 0.0f
        ? collision_penetration_response{remaining_displacement_body0}
        : body0_penetration_response;
    const collision_penetration_response p1 =
        dot(remaining_displacement_body1, contact.normal) < 0.0f
        ? collision_penetration_response{remaining_displacement_body1}
        : body1_penetration_response;
    return collision_response_pairs{
        .penetration_responses = {p0, p1},
        .restitution_responses = {r0, r1},
    };
}
