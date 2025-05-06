#ifndef RIGIDBODY_HPP
#define RIGIDBODY_HPP


#include "../rendering/rect.hpp"
#include "integration.hpp"
#include "mass.hpp"

struct rigidbody {
    rect_f32 body;
    verlet space;
    inverse_mass_f32 mass_rcp;
};

#endif
