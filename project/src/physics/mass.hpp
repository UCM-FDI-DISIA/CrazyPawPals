#ifndef MASS_HPP
#define MASS_HPP


template <typename Numeric>
struct mass {
    Numeric value;
};
using mass_f32 = mass<float>;

template <typename Numeric>
struct inverse_mass {
    Numeric value;
};
using inverse_mass_f32 = inverse_mass<float>;

#endif
