
/*#include "transformless_dyn_image.h"


//#include <SDL_rect.h>
#include "../../rendering/camera.hpp"
//#include "rect_component.hpp"
//#include "camera_component.hpp"

void transformless_dyn_image::render()
{
    //Texture& card_texture = sdlutils().images().at(descriptor.card_image_key.data());
    const SDL_Rect destination = SDL_Rect_screen_rect_from_global(destination_rect, my_camera_screen);
    const SDL_Rect source = { 0, 0, texture.width(), texture.height() };

        //card_texture.render(source, destination, angle, nullptr, flip);
        const SDL_Rect subsource = {
            int(source_subrect.position.x * source.w),
            int(source_subrect.position.y * source.h),
            int(source_subrect.size.x * source.w),
            int(source_subrect.size.y * source.h)
        };
        //card_texture.render(subsource, destination, my_rotation, nullptr, flip);
    //}
    texture.render(subsource, destination, my_rotation, nullptr);
}

transformless_dyn_image::transformless_dyn_image(const rect_f32 subrect, const rect_f32 offset, float rotation, const camera_screen& camera, Texture& texture)
    : destination_rect(subrect), source_subrect(offset), my_camera_screen(camera), texture(texture), my_rotation(rotation)
{
}
*/