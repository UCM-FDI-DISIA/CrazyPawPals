
#include "rigidbody_component.hpp"
#include <cassert>


rigidbody_component::rigidbody_component(const rect_f32 rect, mass_f32 mass, float restitution_coefficient)
    : rect(rect) {
    assert(mass.value > 0.0f && "error: must be strictly positive");
    assert(restitution_coefficient >= 0.0f && "error: restitution must be non-negative");
#if !RIGIDBODY_COMPONENT_DEGEN_RESTITUTION
    assert(restitution_coefficient <= 1.0f && "error: restitution must be less or equal to one if the RIGIDBODY_COMPONENT_DEGEN_RESTITUTION flag is enabled");
#endif
    inverse_mass = {1.0f / mass.value};
    this->restitution_coefficient = restitution_coefficient;
}

rigidbody_component::rigidbody_component(const rect_f32 rect, inverse_mass_f32 inverse_mass, float restitution_coefficient) 
    : rect(rect), inverse_mass{inverse_mass} {

#if !RIGIDBODY_COMPONENT_DEGEN_RESTITUTION
    assert(restitution_coefficient <= 1.0f && "error: restitution must be less or equal to one if the RIGIDBODY_COMPONENT_DEGEN_RESTITUTION flag is enabled");
#endif
    this->restitution_coefficient = restitution_coefficient;
}
