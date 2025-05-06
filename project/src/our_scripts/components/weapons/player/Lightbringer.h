#pragma once

#include "../Weapon.h"

class Deck;
class Lightbringer: public Weapon {
protected:
	int _base_damage;
	int _amp_damage;
	float _base_cooldown;
	float _amp_cooldown;
	std::string _base_tex;
	std::string _amp_tex;
	Deck* _deck;
	float _offset;
	void callback(Vector2D shootPos, Vector2D shootDir) override;
public:
	__CMPID_DECL__(ecs::cmp::WEAPON);
	Lightbringer();
	virtual ~Lightbringer();
	void initComponent() override;
	void update(uint32_t) override;
};