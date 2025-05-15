#ifndef RECT_COMPONENT_HPP
#define RECT_COMPONENT_HPP


#include "../../../ecs/Component.h"
#include "../../../rendering/rect.hpp"


struct rect_component : public ecs::Component {
    __CMPID_DECL__(ecs::cmp::RECT)
    rect_f32 rect;

    inline rect_component(const rect_f32 rect) : rect(rect) {}
    inline rect_component(const position2_f32 position, const size2_f32 size)
        : rect{position, size} { }
    inline rect_component(const float x, const float y, const float w, const float h)
        : rect{{x, y}, {w, h}} {}

    inline float getSize() const { return rect.size.x; }
    inline void setSize(float _size) { rect.size = { _size, _size }; }
};


#endif
