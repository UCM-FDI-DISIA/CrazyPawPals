#pragma once

#include "MythicItem.h"
#include "../../utils/EventsSystem.hpp"

/**
 * @class BloodClaw
 * @brief BloodClaw is a MythicItem that applies effects to the player's health and weapon.
 *
 * This class reduces the player's health by 50% and doubles weapon damage when applied.
 */
class BloodClaw : public MythicItem {
	Health* _health;
	Weapon* _weapon;
public:
	BloodClaw();

	void apply_effects() override;

	void set_health(Health* h) { _health = h; }
	void set_weapon(Weapon* w) { _weapon = w; }
};

/**
 * @class DreamRecicler
 * @brief DreamRecicler is a MythicItem that affects the player's mana management.
 *
 * When a card is milled, the player gains mana equal to its cost.
 * However, mana regeneration is reduced by 50%.
 */
class DreamRecicler :public event_system::event_receiver, public MythicItem {
	ManaComponent* _mana;
public:
	DreamRecicler();
	~DreamRecicler();
	void apply_effects() override;
	void event_callback0(const event_system::event_receiver::Msg& m);

	void set_mana(ManaComponent* m) { _mana = m; }
};

/**
 * @class CurtainReaper
 * @brief CurtainReaper is a MythicItem that modifies the player's health mechanics.
 *
 * The player’s health is reduced by 50%.
 * Upon defeating an enemy, the player gains 20% of the enemy's health as a shield.
 */
class CurtainReaper : public event_system::event_receiver, public MythicItem {
	Health* _health;
public:
	CurtainReaper();
	~CurtainReaper();
	void apply_effects() override;
	void event_callback0(const event_system::event_receiver::Msg& m);

	void set_health(Health* h) { _health = h; }
};

/**
 * @class Incense
 * @brief Incense is a MythicItem that enhances mana regeneration and reduces player's damage.
 *
 * Mana regeneration is doubled.
 * However, base weapon damage is reduced by 50%.
 */
class Incense : public MythicItem {
	ManaComponent* _mana;
	Weapon* _weapon;
public:
	Incense();
	void apply_effects() override;

	void set_mana(ManaComponent* m) { _mana = m; }
	void set_weapon(Weapon* w) { _weapon = w; }
};

/**
 * @class ArcaneSurge
 * @brief ArcaneSurge is a MythicItem that enhances mana regeneration while affecting reload speed.
 *
 * While reloading, mana regeneration increases by 50%.
 * However, reload speed is reduced by 50%.
 */
class QuickDrawDeck : public MythicItem {
	ManaComponent* _mana;
	Deck* _deck;

	bool _set = false;
	int _ini_mana;
public:
	QuickDrawDeck();

	void apply_effects() override;
	void update(uint32_t dt) override;

	void set_mana(ManaComponent* m) { _mana = m; }
	void set_deck(Deck* d) { _deck = d; }
};
	
/**
 * @class BloodPact
 * @brief BloodPact is a MythicItem that enhances mana regeneration while affecting the player's health.
 *
 * This class reduces the player's health by 50% and doubles mana regeneration.
 */
class BloodPact : public MythicItem {
	ManaComponent* _mana;
	Health* _health;
public:
	BloodPact();

	void apply_effects() override;

	void set_mana(ManaComponent* m) { _mana = m; }
	void set_health(Health* h) { _health = h; }
};
	
/**
 * @class PreternaturalForce
 * @brief PreternaturalForce is a MythicItem that enhances player's damage and reduces mana production.
 *
 * This class reduces the player's  mana regeneration by 50% and doubles damage.
 */
class DemonicScratcher : public MythicItem {
	ManaComponent* _mana;
	Weapon* _weapon;
public:
	DemonicScratcher();

	void apply_effects() override;

	void set_mana(ManaComponent* m) { _mana = m; }
	void set_weapon(Weapon* w) { _weapon = w; }
};

/**
 * @class ClawFile
 * @brief ClawFile is a MythicItem that enhances player's reload speed and reduces movement speed.
 *
 * While reloading, movement speed reduced by 50%.
 * However, reload speed is reduced by 50%.
 */
class ClawFile : public MythicItem {
	MovementController* _mc;
	Deck* _deck;

	bool _set = false;
	float _ini_mc;
public:
	ClawFile();

	void apply_effects() override;
	void update(uint32_t dt) override;

	void set_deck(Deck* d) { _deck = d; }
	void set_movement_controller(MovementController* mc) { _mc = mc; }
};

/**
 * @class MeowOrNever
 * @brief MeowOrNever is a MythicItem that enhances player's movement speed and reduces player's health.
 *
 * The player’s health is reduced by 50%.
 * However, movement speed is enhanced by 100%.
 */
class UraniumSocks : public MythicItem {
	MovementController* _mc;
	Health* _health;
public:
	UraniumSocks();

	void apply_effects() override;

	void set_health(Health* h) { _health = h; }
	void set_movement_controller(MovementController* mc) { _mc = mc; }
};

/**
 * @class ZoomiesInducer
 * @brief ZoomiesInducer is a MythicItem that enhances player's movement speed and reduces player's health.
 *
 * The player’s health is reduced by 50%.
 * However, movement speed is enhanced by 100%.
 */
class ZoomiesInducer : public MythicItem {
	MovementController* _mc;
	Transform* _tr;

	uint32_t _timer;
	uint32_t _last_time;

	uint32_t _duration;

	float _distance;
public:
	ZoomiesInducer();
	ZoomiesInducer(MovementController* mc, Transform* tr, uint32_t time, uint32_t duration, float distance);

	void apply_effects() override;
	void update(uint32_t dt) override;

	void set_movement_controller(MovementController* mc) { _mc = mc; }
	void set_transform(Transform* tr) { _tr = tr; }
};