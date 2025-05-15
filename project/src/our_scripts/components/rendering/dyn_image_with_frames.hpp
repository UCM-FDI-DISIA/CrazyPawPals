#pragma once

#include "offset_dyn_image.hpp"

class Transform;
class Texture;
struct rect_component;

struct dyn_image_with_frames : public offset_dyn_image {
	__CMPID_DECL__(ecs::cmp::DYN_IMAGE_WITH_FRAMES);

	int ini_frame;
	int end_frame;
	int current_frame;
	uint32_t frame_duration;
	uint32_t next_frame_time;
	std::string texture_name;

	inline dyn_image_with_frames(
		const rect_f32 subrect,
		const rect_component& output_rect,
		const camera_screen& camera,
		Texture& texture,
		const Transform& transform,
		std::string texture_name=" ",
		uint32_t frame_duration = 100,
		int ini_frame = 0, int end_frame = 0
	) : offset_dyn_image(subrect, { 0.5f, 0.5f }, output_rect, camera, texture, transform),
		frame_duration(frame_duration), next_frame_time(0), current_frame(ini_frame), ini_frame(ini_frame),end_frame(end_frame), texture_name(texture_name)
	{};
	void update(uint32_t delta_time) override;
	
	void set_animation(int ini, int end, uint32_t duration = 100);
};