
#include "Card.hpp"
class MovementController;

class Fireball : public Card /*, public event_system::event_receiver*/ {
public:
	Fireball();
	void on_play(Deck& d,const Vector2D* player_position, const Vector2D* target_position) override;
	//void event_callback0(const Msg& m) override;
};

class Lighting : public Card {
public:
	Lighting();
	void on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) override;
};

// Archetypes: Cycle
class Kunai : public Card {
public:
	Kunai();
	void on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) override;
};

class Recover : public Card {
public:
	Recover();
	void on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) override;
};

// Archetypes: Multi-hit
class Minigun : public Card {
public:
	Minigun();
	void on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) override;
	void update(uint32_t) override;
private:
	GameStructs::BulletProperties _bullets_properties;
	const Vector2D* _aim_vec;
	const Vector2D* _pl_vec;
	bool _playing;
	int _time_since_played;
	int _number_of_bullets_shot = 0;
	int _number_of_shots = 10;
	int _shooting_duration = 1500;
	bool _im_primed = false;
};

// Archetypes: Mill
class CardSpray : public Card {
public:
	CardSpray();
	void on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) override;
};

// Archetypes: Mill
class EldritchBlast : public Card { // TODO: Reset on round end
public:
	EldritchBlast();
	void on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) override;
	Card* on_mill(Deck& d, const Vector2D* player_position) override;
private:
	int _shot_count = 1;
	const int _amplitude = 10;
};

// Archetypes: Mill, Prime
class Primordia : public Card { 
public: 
	Primordia();
	void on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) override;
	Card* on_mill(Deck& d, const Vector2D* player_position) override;
	void update(uint32_t) override;
};

// Archetypes: Mill, High Costs
class Commune : public Card {
public:
	Commune();
	void on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) override;
};

// Archetypes: Mill, High Costs
class Evoke : public Card {
public:
	Evoke();
	void on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) override;

};

//Archetypes: Prime, High Costs
class Fulgur : public Card {
public:
	Fulgur();
	void on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) override;
	void update(uint32_t) override;
private:
	GameStructs::BulletProperties _bullets_properties;
	Vector2D _aim_vec;
	bool _playing;
	int _time_since_played;
	int _number_of_bullets_shot = 0;
	int _number_of_shots = 8;
	int _shooting_duration = 500;
};

class CatKuzaCard : public Card {
public:
	CatKuzaCard();
	void on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) override;

private:
	int _times_since_played;
};

class SuperMichiCard : public Card {
public:
	SuperMichiCard();
	void on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) override;
private:
	int _times_since_played;
};

class QuickFeet : public Card {
public:
	QuickFeet();
	void on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) override;
	void update(uint32_t) override;
private:
	bool _playing;
	int _time_since_played;
	int _effect_duration = 1000;
	MovementController* _ctrl;
};

// The following cards exist for testing purposes and may not be playable in the finished product. These are subject to change.
#pragma region TestCards
class Prime : public Card {
public:
	Prime();
	void on_play(Deck& d, const Vector2D* player_position, const Vector2D* target_position) override;
};
#pragma endregion

