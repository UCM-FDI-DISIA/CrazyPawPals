#pragma once

#include "../../../ecs/Component.h"
class Transform;
struct 	offset_dyn_image;
class FlipXController :public ecs::Component
{
public:
	__CMPID_DECL__(ecs::cmp::FLIPXCONTROLLER);

	FlipXController();
	virtual ~FlipXController() {};
	void initComponent() override;
	void update(uint32_t delta_time) override;
protected:
	offset_dyn_image* _dy;
	Transform* _tr;
};
