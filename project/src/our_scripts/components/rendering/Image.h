
#pragma once

#include "../../../ecs/Component.h"

class Transform;
class Texture;

class Image: public ecs::Component {
public:

	__CMPID_DECL__(ecs::cmp::IMAGE)

	Image();
	Image(Texture *tex);
	virtual ~Image();

	void setTexture(Texture *tex) {
		_tex = tex;
	}

	void initComponent() override;
	void render() override;

protected:
	Transform *_tr;
	Texture *_tex;
};

