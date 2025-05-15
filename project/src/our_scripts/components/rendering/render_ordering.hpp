#ifndef RENDER_ORDERING_HPP
#define RENDER_ORDERING_HPP


#include "../../../ecs/Component.h"

struct render_ordering : ecs::Component {
    __CMPID_DECL__(ecs::cmp::RENDER_ORDERING)
    uint_fast32_t ordinal;

    inline render_ordering(uint_fast32_t ordinal) : ordinal(ordinal) { }
};



#endif
