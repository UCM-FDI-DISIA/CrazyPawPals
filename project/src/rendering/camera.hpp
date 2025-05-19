#ifndef CAMERA_HPP
#define CAMERA_HPP


#include <SDL.h>
#include "rect.hpp"

struct camera {
    position2_f32 position;
    size2_f32 half_size;
};

struct screen_rect {
    size2_i32 pixel_size;
};

struct camera_screen {
    camera camera;
    screen_rect screen;
};

extern const rect_f32 rect_f32_full_subrect;

inline rect_f32 rect_f32_view_from_global(rect_f32 global, camera const &camera) {
    return rect_f32(
        position2_f32(
            (global.position.x - camera.position.x),
            (global.position.y - camera.position.y)
        ),
        size2_f32(
            global.size.x,
            global.size.y
        )
    );
}

inline rect_f32 rect_f32_global_from_view(rect_f32 view, camera const &camera) {
    return rect_f32(
        position2_f32(
            (view.position.x + camera.position.x),
            (view.position.y + camera.position.y)
        ),
        size2_f32(
            view.size.x,
            view.size.y
        )
    );
}

inline rect_f32 rect_f32_ndc_from_view(rect_f32 view, camera const &camera) {
    return rect_f32(
        position2_f32(
            view.position.x / camera.half_size.x,
            view.position.y / camera.half_size.y
        ),
        size2_f32(
            view.size.x / camera.half_size.x,
            view.size.y / camera.half_size.y
        )
    );
}

inline rect_f32 rect_f32_view_from_ndc(rect_f32 ndc, camera const &camera) {
    return rect_f32(
        position2_f32(
            ndc.position.x * camera.half_size.x,
            ndc.position.y * camera.half_size.y
        ),
        size2_f32(
            ndc.size.x * camera.half_size.x,
            ndc.size.y * camera.half_size.y
        )
    );
}

inline rect_f32 rect_f32_global_from_ndc(rect_f32 ndc, camera const &camera) {
    return rect_f32_global_from_view(rect_f32_view_from_ndc(ndc, camera), camera);
}


inline rect_f32 rect_f32_viewport_from_ndc(rect_f32 ndc) {
    return rect_f32(
        position2_f32(
            ndc.position.x * 0.5f + 0.5f,
            ndc.position.y * 0.5f + 0.5f
        ),
        size2_f32(
            ndc.size.x * 0.5f,
            ndc.size.y * 0.5f
        )
    );
}

inline rect_f32 rect_f32_viewport_from_ndc_flipped_y(rect_f32 ndc) {
    return rect_f32(
        position2_f32(
            ndc.position.x * 0.5f + 0.5f,
            1.0f - (ndc.position.y * 0.5f + 0.5f)
        ),
        size2_f32(
            ndc.size.x * 0.5f,
            ndc.size.y * 0.5f
        )
    );
}

inline rect_f32 rect_f32_ndc_from_viewport(rect_f32 viewport) {
    return rect_f32(
        position2_f32(
            (viewport.position.x - 0.5f) * 2.0f,
            (viewport.position.y - 0.5f) * 2.0f
        ),
        size2_f32(
            viewport.size.x * 2.0f,
            viewport.size.y * 2.0f
        )
    );
}
inline rect_f32 rect_f32_ndc_from_viewport_flipped_y(rect_f32 viewport) {
    return rect_f32(
        position2_f32(
            (viewport.position.x - 0.5f) * 2.0f,
            (0.5f - viewport.position.y) * 2.0f
        ),
        size2_f32(
            viewport.size.x * 2.0f,
            viewport.size.y * 2.0f
        )
    );
}


inline rect_f32 rect_f32_screen_rect_from_viewport(rect_f32 viewport, screen_rect const &screen) {
    return rect_f32(
        position2_f32(
            viewport.position.x * float(screen.pixel_size.x),
            viewport.position.y * float(screen.pixel_size.y)
        ),
        size2_f32(
            viewport.size.x * float(screen.pixel_size.x),
            viewport.size.y * float(screen.pixel_size.y)
        )
    );
}

inline rect_f32 rect_f32_viewport_from_screen_rect(rect_f32 in_screen_rect, screen_rect const &screen) {
    return rect_f32(
        position2_f32(
            in_screen_rect.position.x / float(screen.pixel_size.x),
            in_screen_rect.position.y / float(screen.pixel_size.y)
        ),
        size2_f32(
            in_screen_rect.size.x / float(screen.pixel_size.x),
            in_screen_rect.size.y / float(screen.pixel_size.y)
        )
    );
}

inline rect_f32 rect_f32_global_from_screen_rect_flipped_y(rect_f32 screen_rect, camera_screen const &camera_screen) {
    return rect_f32_global_from_view(
        rect_f32_view_from_ndc(
            rect_f32_ndc_from_viewport_flipped_y(
                rect_f32_viewport_from_screen_rect(screen_rect, camera_screen.screen)
            ),
            camera_screen.camera
        ),
        camera_screen.camera
    );
}
inline rect_f32 rect_f32_global_from_screen_rect(rect_f32 screen_rect, camera_screen const &camera_screen) {
    return rect_f32_global_from_view(
        rect_f32_view_from_ndc(
            rect_f32_ndc_from_viewport(
                rect_f32_viewport_from_screen_rect(screen_rect, camera_screen.screen)
            ),
            camera_screen.camera
        ),
        camera_screen.camera
    );
}

inline rect_f32 rect_f32_screen_rect_from_global(rect_f32 global, camera_screen const &camera_screen, bool flip_y = true) {
    if (flip_y) {
        return rect_f32_screen_rect_from_viewport(rect_f32_viewport_from_ndc_flipped_y(
            rect_f32_ndc_from_view(rect_f32_view_from_global(global, camera_screen.camera), camera_screen.camera)
        ), camera_screen.screen);
    } else {
        return rect_f32_screen_rect_from_viewport(rect_f32_viewport_from_ndc(
            rect_f32_ndc_from_view(rect_f32_view_from_global(global, camera_screen.camera), camera_screen.camera)
        ), camera_screen.screen);
    }
}

inline SDL_Rect SDL_Rect_screen_rect_from_global(rect_f32 global, camera_screen const &camera_screen, bool flip_y = true) {
    rect_f32 screen_rect = rect_f32_screen_rect_from_global(global, camera_screen, flip_y);
    return SDL_Rect{
        int(screen_rect.position.x),
        int(screen_rect.position.y),
        int(screen_rect.size.x),
        int(screen_rect.size.y)
    };
}

inline rect_f32 rect_f32_ndc_from_global(rect_f32 global, camera const &camera) {
    return rect_f32_ndc_from_view(rect_f32_view_from_global(global, camera), camera);
}


#endif
