#pragma once

#include "../../ecs/Component.h"
#include "../../sdlutils/SDLUtils.h"
#include <SDL.h> // Add this include to resolve the incomplete type error

class Transform;
class dyn_image;
class rect_component;

class Fog : public ecs::Component
{
public:
	__CMPID_DECL__(ecs::cmp::FOG);

	Fog() { };
	virtual ~Fog() {};
	void initComponent() override;
	void update(uint32_t delta_time) override;

	bool getFogActive() { return fogActive; }
	void setFog(bool fogActive_);

private:
	bool fogActive = false;
	float fogPercentage = 0;

	Transform* fogTransform;
	dyn_image* fogImage;
	rect_component* fogRect;
	
	float orSize;
	int orW;
	int orH;
	float orY;
};