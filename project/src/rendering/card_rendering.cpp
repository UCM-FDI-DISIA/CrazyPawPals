
#include "card_rendering.hpp"
#include "../sdlutils/SDLUtils.h"

const card_rendering_descriptor_options_flags flip_flags =
    (card_rendering_descriptor_options_flip_horizontal | card_rendering_descriptor_options_flip_vertical);

extern inline SDL_Rect SDL_Rect_screen_rect_from_global(rect_f32 global, camera_screen const &camera_screen, bool flip_y);
SDL_Rect card_rendering_descriptor_render(
    const card_rendering_descriptor &descriptor,
    const camera_screen &camera_screen,
    SDL_Renderer &renderer,
    const rect_f32 destination_rect,
    const rect_f32 source_subrect,
    const float angle,
    const card_rendering_descriptor_options_flags flags
) {
    Texture &card_texture = sdlutils().images().at(descriptor.card_image_key.data());
    const SDL_RendererFlip flip = static_cast<SDL_RendererFlip>(flags & (flip_flags));

    const SDL_Rect destination = SDL_Rect_screen_rect_from_global(destination_rect, camera_screen);
    const SDL_Rect source = { 0, 0, card_texture.width(), card_texture.height() };

    if (flags & card_rendering_descriptor_options_full_subrect) {
        card_texture.render(source, destination, angle, nullptr, flip);
    } else {
        const SDL_Rect subsource = {
            int(source_subrect.position.x * source.w),
            int(source_subrect.position.y * source.h),
            int(source_subrect.size.x * source.w),
            int(source_subrect.size.y * source.h)
        };
        card_texture.render(subsource, destination, angle, nullptr, flip);
    }

    const SDL_Rect mana_cost_destination = {
        int(destination.x + destination.w * descriptor.mana_cost_subrect.position.x),
        int(destination.y + destination.h * descriptor.mana_cost_subrect.position.y),
        int(destination.w * descriptor.mana_cost_subrect.size.x),
        int(destination.h * descriptor.mana_cost_subrect.size.y)
    };

    
    Font &font = sdlutils().fonts().at(descriptor.mana_cost_font_key.data());
    Texture mana_cost{&renderer, std::to_string(descriptor.mana_cost).c_str(), font, descriptor.mana_cost_color};
    mana_cost.render(mana_cost_destination);

    if (descriptor.health_cost > 0) {
        const SDL_Rect health_cost_destination = {
        int(destination.x + destination.w * descriptor.health_cost_subrect.position.x),
        int(destination.y + destination.h * descriptor.health_cost_subrect.position.y),
        int(destination.w * descriptor.health_cost_subrect.size.x),
        int(destination.h * descriptor.health_cost_subrect.size.y)
        };
        Texture health_cost{ &renderer, std::to_string(descriptor.health_cost).c_str(), font, descriptor.health_cost_color };
        health_cost.render(health_cost_destination);
    }

    return destination;
}

SDL_Rect card_description_rendering_descriptor_render(
    const card_description_rendering_descriptor &descriptor,
    const camera_screen &camera_screen,
    SDL_Renderer &renderer,
    const rect_f32 destination_rect,
    const SDL_Color text_color,
    const SDL_Color background_color
) {
    Texture &background = sdlutils().images().at(descriptor.background_key.data());
    SDL_SetTextureColorMod(&background.get_texture(), background_color.r, background_color.g, background_color.b);
    
    const SDL_Rect destination = SDL_Rect_screen_rect_from_global(destination_rect, camera_screen);
    SDL_SetRenderDrawColor(&renderer, background_color.r, background_color.g, background_color.b, background_color.a);
	SDL_RenderFillRect(&renderer, &destination);
    SDL_SetRenderDrawColor(&renderer, 255, 255, 255, 255);
    
    const rect_f32 name_rect = {
        {
            destination_rect.position.x + descriptor.name_subrect.position.x,
            destination_rect.position.y + descriptor.name_subrect.position.y,
        },
        {
            destination_rect.size.x * descriptor.name_subrect.size.x,
            destination_rect.size.y * descriptor.name_subrect.size.y,
        }
    };
    const rect_f32 description_rect = {
        {
            destination_rect.position.x + descriptor.description_subrect.position.x,
            destination_rect.position.y + descriptor.description_subrect.position.y,
        },
        {
            destination_rect.size.x * descriptor.description_subrect.size.x,
            destination_rect.size.y * descriptor.description_subrect.size.y
        }
    };

    Font &font = sdlutils().fonts().at(descriptor.font_key.data());
	Texture description{&renderer, descriptor.description.data(), font, text_color};
    description.render(SDL_Rect_screen_rect_from_global(description_rect, camera_screen));

    Texture name{&renderer, descriptor.name.data(), font, text_color};
    name.render(SDL_Rect_screen_rect_from_global(name_rect, camera_screen));
    return destination;
}
