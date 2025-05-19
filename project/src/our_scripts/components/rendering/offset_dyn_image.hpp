#ifndef OFFSET_DYN_IMAGE_HPP
#define OFFSET_DYN_IMAGE_HPP


#include "../../../ecs/Component.h"
#include "../../../utils/Vector2D.h"
#include "../../../rendering/camera.hpp"
#include "rect_component.hpp"

class Transform;
class Texture;
//struct rect_component;

struct filter {
	int r, g, b, a;
};
struct offset_dyn_image : public ecs::Component {
	__CMPID_DECL__(ecs::cmp::OFFSET_DYN_IMAGE);
	rect_f32 subrect;
	position2_f32 offset;
	const rect_component &output_rect;
	const camera_screen &camera;
	Texture* texture;
	const Transform &transform;
	bool isDamaged;
	uint32_t damageTimer;
	const uint32_t damage_color_duration;
	SDL_RendererFlip flip;

	filter _current_filter;

	virtual void render() override;
	void update(uint32_t delta_time) override;

	//static_assert(false);
	inline offset_dyn_image(
		const rect_f32 subrect,
		const position2_f32 offset,
		const rect_component& output_rect,
		const camera_screen& camera,
		Texture& texture,
		const Transform& transform
	) : subrect(subrect), offset(offset), output_rect(output_rect), camera(camera),
		texture(&texture), transform(transform), isDamaged(false), damageTimer(0), damage_color_duration(250), flip(SDL_FLIP_NONE)
	{
		_current_filter = { 255, 255, 255, 255 };
	}; 
};

#endif