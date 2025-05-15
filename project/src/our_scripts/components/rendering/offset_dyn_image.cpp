
#include "../../../game/Game.h"
#include "../../../ecs/Manager.h"

#include "../movement/Transform.h"
#include "Image.h"
#include "offset_dyn_image.hpp"


void offset_dyn_image::render() {

    isDamaged ? texture->apply_filter(255, 0, 0) : texture->apply_filter(_current_filter.a, _current_filter.g, _current_filter.b, _current_filter.a);

    const SDL_Rect destination = SDL_Rect_screen_rect_from_global({
        .position = {
            .x = transform.getPos().getX() + output_rect.rect.position.x - offset.x * output_rect.rect.size.x,
            .y = transform.getPos().getY() + output_rect.rect.position.y + offset.y * output_rect.rect.size.y,
        },
        .size = output_rect.rect.size
    }, camera);

    texture->render(
        SDL_Rect{
            .x = int((subrect.position.x) * float(texture->width())),
            .y = int((subrect.position.y) * float(texture->height())),
            .w = int(subrect.size.x * float(texture->width())),
            .h = int(subrect.size.y * float(texture->height()))
        },
        destination, transform.getRot(), nullptr, flip);
    texture->apply_filter(255, 255, 255);   
}

void offset_dyn_image::update(uint32_t delta_time)
{
    if (isDamaged) {
        damageTimer += delta_time;
        if (damageTimer >= damage_color_duration) {
            damageTimer = 0;
            isDamaged = false;
        }
    }
    else damageTimer = 0;
}
