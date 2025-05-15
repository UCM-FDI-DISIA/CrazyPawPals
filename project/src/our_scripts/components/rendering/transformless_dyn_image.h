#pragma once


#include "../../../ecs/Component.h"
#include "../../../rendering/rect.hpp"
#include "../../../sdlutils/Texture.h"
#include "../../../rendering/camera.hpp"
#include "../rendering/camera_component.hpp"
class Texture;
struct camera_screen;
#include <iostream>
struct transformless_dyn_image : public ecs::Component {

	__CMPID_DECL__(ecs::cmp::TRANSFORMLESS_DYN_IMAGE);
	Texture* texture;
	//takes 0 to 1 values on viewport
	rect_f32 destination_rect;
	const camera_screen& my_camera_screen;
	float my_rotation;
	bool active = true;
	float _original_w;
	float _original_h;
	bool _filter;
	transformless_dyn_image(
		const rect_f32 subrect,
		float rotation,
		const camera_screen& camera,
		Texture* texture
	) : destination_rect(subrect), my_camera_screen(camera), texture(texture), my_rotation(rotation), _original_w(subrect.size.x), _original_h(subrect.size.y), _filter(false)
	{
	}

	inline SDL_Rect get_destination_rect() const {
		rect_f32 rect = rect_f32_screen_rect_from_viewport(destination_rect, my_camera_screen.screen);
		//std::cout << "camera_size:  x: " << my_camera_screen.screen.pixel_size.x << "  y: " << my_camera_screen.screen.pixel_size.y << "          ";
		SDL_Rect destination = { int(rect.position.x), int(rect.position.y), int(rect.size.x), int(rect.size.y) };
		return destination;
	};

	inline virtual void render() override {
		if (!active) return;
		/*
		rect_f32 rect = rect_f32_screen_rect_from_viewport(destination_rect, my_camera_screen.screen);
		const SDL_Rect destination = {
			int(rect.position.x),
			int(rect.position.y),
			int(rect.size.x),
			int(rect.size.y)
		}; //SDL_Rect_screen_rect_from_global(destination_rect, my_camera_screen);
		*/
		_filter ? texture->apply_filter(220, 220, 220) : texture->apply_filter(255, 255, 255);
		SDL_Rect destination = get_destination_rect();
		const SDL_Rect source = { 0, 0, texture->width(), texture->height() };
		if (!source.x) {
			//std::cout << "w: " << source.w << "   h: " << source.h << std::endl;
		}
		texture->render(source, destination, my_rotation, nullptr);
	}
	inline void set_texture(Texture* t) { texture = t; };

	inline void set_active(bool v) {
		active = v;
	}

	inline void resize(float value) { 
		destination_rect.size = { destination_rect.size.x * value, destination_rect.size.y * value };
	};

	inline Texture &get_texture() {
		return *texture;
	}
	inline const Texture &get_texture() const {
		return *texture;
	}
};
