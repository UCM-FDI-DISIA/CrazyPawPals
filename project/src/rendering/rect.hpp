#ifndef RECT_HPP
#define RECT_HPP

#include <cstdint>

template <typename Numeric>
struct vec2 {
    Numeric x;
    Numeric y;
};

using vec2_f32 = vec2<float>;
using vec2_i32 = vec2<int32_t>;

template <typename Numeric>
struct position2 {
    Numeric x;
    Numeric y;
};

using position2_f32 = position2<float>;
using position2_i32 = position2<int32_t>;

template <typename Numeric>
struct size2 {
    Numeric x;
    Numeric y;
};

using size2_f32 = size2<float>;
using size2_i32 = size2<int32_t>;

template <typename Numeric>
struct rect {
    position2<Numeric> position;
    size2<Numeric> size;
};

using rect_f32 = rect<float>;
using rect_i32 = rect<int32_t>;

#endif
