#pragma once
#include "../utils/Vector2D.h"
#include "../ecs/ecs.h"
#include "../rendering/rect.hpp"
#include <list>
class Card;
class MythicItem;
class Transform;
namespace GameStructs
{
	enum WeaponType
	{
		DEFAULT,
		REVOLVER,
		RAMPAGE,
		PUMP_SHOTGUN,
		RAMP_CANON,
		LIGHTBRINGER,
		WEAPON6,
		WEAPON7,
		CATKUZA_WEAPON,
		SUPER_MICHI,
		LAST_WEAPON,
	};
	enum DeckType
	{
		ONE,
		TWO,
		THREE,
		FOUR,
		LAST_DECK,
	};
	enum collide_with
	{
		player = 0,
		enemy = 1,
		all = 2,
		none = 4
	};
	struct BulletProperties
	{
		Vector2D init_pos;
		Vector2D dir;
		float speed = 0.0f;
		int damage = 0;
		int pierce_number = 0;
		float life_time = 1.0f;
		float width = 40;
		float height = 40;
		std::string sprite_key;
		WeaponType weapon_type = DEFAULT;
		collide_with collision_filter;
	};
	enum EnemyFollow
	{
		CLOSEST,
		FURTHEST,
		LOWREST_LIFE,
		HIGHEST_LIFE,
	};
	struct EnemyProperties
	{
		// Sprite
		std::string sprite_key;
		Vector2D start_pos;
		
		// Weapon
		WeaponType weapon_type = DEFAULT;

		int health;

		// rect_component
		float width;
		float height;

		EnemyFollow follow = CLOSEST;

		// Transform properties
		Vector2D dir = {0.0f, 0.0f};
		float r = 0.0f;
		float s = 2.0f;

		// MovementController
		float max_speed = 0.1f;
		float acceleration = 5.0f;
		float decceleration = 20.0f;
	};
	struct ButtonProperties
	{
		rect_f32 rect;
		float rot = 0.0f;
		std::string sprite_key;
		ecs::grpId_t ID;
	};
	struct CardButtonProperties : public ButtonProperties
	{
		Card *iterator;
		// constructor
		CardButtonProperties(const rect_f32 &rect, float rot, const std::string &sprite_key, ecs::grpId_t ID, Card *it)
			: ButtonProperties{rect, rot, sprite_key, ID}, iterator(it) {}
	};
	enum CardType
	{
		FIREBALL,
		LIGHTING,
		KUNAI,
		RECOVER,
		MINIGUN,
		SPRAY,
		ELDRITCH_BLAST,
		COMMUNE,
		EVOKE,
		FULGUR,
		QUICK_FEET,
		LAST_CARD,
	};
	struct MythicButtonProperties : public ButtonProperties
	{
		MythicItem *iterator;
		// constructor
		MythicButtonProperties(const rect_f32 &rect, float rot, const std::string &sprite_key, ecs::grpId_t ID, MythicItem *it)
			: ButtonProperties{rect, rot, sprite_key, ID}, iterator(it) {}
	};
	enum MythicType
	{
		BLOODCLAW,
		DREAMRECICLER,
		CURTAINREAPER,
		INCENSE,
		QUICKDRAWDECK,
		BLOODPACT,
		DEMONICSCRATCHER,
		CLAWFILE,
		URANIUMSOCKS,
		ZOOMIESINDUCER,
		LAST_MYTHIC,
	};

	struct PlayerData {
		int weapon_damage;
		float max_speed;
		float cooldown;
		int reload;
		int mana_regen;
	};
}