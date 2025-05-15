#pragma once

#include "Image.h"
#include "transformless_dyn_image.h"

class ImageForButton : public transformless_dyn_image {
public:
	__CMPID_DECL__(ecs::cmp::TRANSFORMLESS_DYN_IMAGE)
	ImageForButton() = delete;
	ImageForButton(Texture* tex, Texture* stex, const rect_f32& viewportRect, float rotation, const camera_screen& cam);
	virtual ~ImageForButton() { };

	void swap_textures();

protected:
	Texture* _selected_tex;

public:
	inline Texture &get_selected_texture() {
		return *_selected_tex;
	}
	inline const Texture &get_selected_texture() const {
		return *_selected_tex;
	}
};
