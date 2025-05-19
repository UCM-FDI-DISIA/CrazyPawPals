#ifndef STAR_SHOWER_EVENT_HPP
#define STAR_SHOWER_EVENT_HPP


#include "wave_event.hpp"
#include "../../rendering/rect.hpp"
#include "../../rendering/units.hpp"

#include "../../ecs/Entity.h"

#include "../../our_scripts/components/rendering/dyn_image.hpp"
#include "../../our_scripts/components/movement/Transform.h"
#include "../../our_scripts/components/rendering/rect_component.hpp"

#include <vector>
#include <cassert>

struct star_drop_descriptor {
    position2_f32 drop_position;
    ptrdiff_t damage_amount;
    float drop_radius;
    seconds_f32 fall_time;
    float spawn_distance;
};

struct star_drop {
    ecs::entity_t mark_entity;
    ecs::entity_t star_entity;

    Transform *star_transform;
    dyn_image *shadow_image;
    rect_component *shadow_rect;
    ptrdiff_t damage_amount;
    seconds_f32 remaining_fall_time;
    float radius;
    float fall_speed;
};

class star_shower_event : public wave_event {
    rect_f32 event_area;
    star_drop_descriptor drop_lower_bound;
    star_drop_descriptor drop_upper_bound;
    
    std::vector<star_drop> star_drops;
    
    size_t min_drops_inclusive;
    size_t max_drops_exclusive;
    size_t drops_destroyed;

public:
    inline star_shower_event(
        const rect_f32 event_area,
        const star_drop_descriptor drop_lower_bound,
        const star_drop_descriptor drop_upper_bound,
        const size_t min_drops_inclusive,
        const size_t max_drops_exclusive
    ) : wave_event(),
        event_area(event_area),
        drop_lower_bound(drop_lower_bound),
        drop_upper_bound(drop_upper_bound),
        star_drops{},
        min_drops_inclusive(min_drops_inclusive),
        max_drops_exclusive(max_drops_exclusive),
        drops_destroyed{0}
    {
        assert(min_drops_inclusive < max_drops_exclusive && "error: min_drops_inclusive must be less than max_drops_exclusive");
        assert(min_drops_inclusive > 0 && "error: min_drops_inclusive must be greater than 0");
    }

    void start_wave_callback() override;
    void end_wave_callback() override;
    void update(unsigned int delta_time) override;
};

#endif