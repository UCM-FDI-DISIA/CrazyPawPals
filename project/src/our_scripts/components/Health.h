#pragma once

#include "../../ecs/Component.h"
struct 	offset_dyn_image;
class Transform;
class Health :public ecs::Component
{
public:
	__CMPID_DECL__(ecs::cmp::HEALTH);

	Health(int maxHealth, bool isPlayer = false);
	virtual ~Health();
	void takeDamage(int damage);
	void heal(int health);
	int getMaxHealth() const;
	void setMaxHeatlh(int h);
	void setHeatlh(int h);
	void resetCurrentHeatlh();
	int getHealth() const;
	int getShield() const;
	void takeShield(int s);
	void payHealth(int cost);
	void update(uint32_t delta_time) override;
	void initComponent() override;
protected:
	Transform* _tr;
	int _currentHealth;
	int _maxHealth;
	int _shield;
	uint32_t _shieldTime;
	bool _is_player;
	const int FRAME_DURATION = 1000;
	offset_dyn_image* _dy;
};
