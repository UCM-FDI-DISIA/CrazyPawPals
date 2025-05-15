#pragma once

#include "../../ecs/Component.h"
#include "../../sdlutils/Texture.h"

class EraseTextureComponent : public ecs::Component {
private:
	Texture* _text_ref;
public:
	__CMPID_DECL__(ecs::cmp::ERASE_TEXTURE);
	inline EraseTextureComponent(Texture* t) {
		_text_ref = t;
	}
	~EraseTextureComponent() override {
		if (_text_ref != nullptr)
			delete _text_ref;
	}
};