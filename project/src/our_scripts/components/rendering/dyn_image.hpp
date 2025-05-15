#ifndef DYN_IMAGE_HPP
#define DYN_IMAGE_HPP


#include "offset_dyn_image.hpp"

class Transform;
class Texture;
struct rect_component;

struct dyn_image : public offset_dyn_image {
	__CMPID_DECL__(ecs::cmp::DYN_IMAGE);

	//static_assert(false);
	inline dyn_image(
		const rect_f32 subrect,
		const rect_component &output_rect,
		const camera_screen &camera,
		Texture &texture,
		const Transform &transform
	) : offset_dyn_image(subrect, {0.5f, 0.5f}, output_rect, camera, texture, transform) {};
};

#endif