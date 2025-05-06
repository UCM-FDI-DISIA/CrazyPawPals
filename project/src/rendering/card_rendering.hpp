#ifndef CARD_RENDERING_HPP
#define CARD_RENDERING_HPP


#include <string>
#include <SDL.h>
#include <cstdint>
#include "camera.hpp"

struct card_rendering_descriptor {
    rect_f32 mana_cost_subrect;
    rect_f32 health_cost_subrect;
    std::string_view card_image_key;
    std::string_view mana_cost_font_key;
    SDL_Color mana_cost_color;
    uint16_t mana_cost;
    SDL_Color health_cost_color;
    uint16_t health_cost;
};
enum card_rendering_descriptor_options {
    card_rendering_descriptor_options_none = 0,
    card_rendering_descriptor_options_flip_horizontal = 1 << 0,
    card_rendering_descriptor_options_flip_vertical = 1 << 1,
    card_rendering_descriptor_options_full_subrect = 1 << 2,
};
typedef uint8_t card_rendering_descriptor_options_flags;
extern const card_rendering_descriptor_options_flags flip_flags;

SDL_Rect card_rendering_descriptor_render(
    const card_rendering_descriptor &descriptor,
    const camera_screen &camera_screen,
    SDL_Renderer &renderer,
    const rect_f32 destination_rect,
    const rect_f32 source_subrect,
    const float angle,
    const card_rendering_descriptor_options_flags flags
);


struct card_description_rendering_descriptor {
    rect_f32 name_subrect;
    rect_f32 description_subrect;
    std::string_view background_key;
    std::string_view font_key;
    std::string_view name;
    std::string_view description;
};

SDL_Rect card_description_rendering_descriptor_render(
    const card_description_rendering_descriptor &descriptor,
    const camera_screen &camera_screen,
    SDL_Renderer &renderer,
    const rect_f32 destination_rect,
    const SDL_Color text_color,
    const SDL_Color background_color
);

#endif