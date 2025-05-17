
#include "Fog.h"

#include "../../ecs/Manager.h"
#include "../../game/Game.h"

#include "movement/Transform.h"
#include "../../our_scripts/components/rendering/dyn_image.hpp"
#include "../../our_scripts/components/rendering/rect_component.hpp"


void
Fog::initComponent() {
	fogTransform = Game::Instance()->get_mngr()->getComponent<Transform>(_ent);
	fogImage = Game::Instance()->get_mngr()->getComponent<dyn_image>(_ent);
	fogRect = Game::Instance()->get_mngr()->getComponent<rect_component>(_ent);
	orSize = fogRect->getSize();
	orW = fogTransform->getWidth();
	orH = fogTransform->getHeight();
	orY = fogTransform->getPos().getY();
}

void Fog::update(uint32_t delta_time) {
	(void)delta_time;
	if (!fogActive) {
		fogPercentage = 0;
	}
	else {
		if (fogPercentage <= 1 - 0.0005) {
			//10 seconds of wave * 1000 ms is a second
			fogPercentage += delta_time / (float)(10000);
		}
		else {
			fogTransform->getPos().setY(orY+10000.0f);
		}
		//std::cout << "Fog: " << fogPercentage * 100 << "%" << std::endl;

		fogRect->setSize(orSize * (1 - fogPercentage));
	}
}

void Fog::setFog(bool fogActive_)
{
	if (!fogActive_) {
		fogRect->setSize(orSize);
		fogTransform->setWidth(orW);
		fogTransform->setHeight(orH);
		fogTransform->getPos().setY(orY);

	}fogActive = fogActive_;
}