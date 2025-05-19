
#include "WeaponRataBasurera.h"
#include "../../../../game/Game.h"
#include "../../../../game/scenes/GameScene.h"
#include "../../../../sdlutils/RandomNumberGenerator.h"

WeaponRataBasurera::WeaponRataBasurera() : Weapon(0, 5000, 0.0f, 0.0f, "", 0.0f, 0.0f) {}

WeaponRataBasurera::~WeaponRataBasurera() {}

void WeaponRataBasurera::sendHealthComponent(Health *health)
{
	_health = health;
}

void WeaponRataBasurera::callback(Vector2D shootPos, Vector2D shootDir)
{

	int _randomNum = sdlutils().rand().nextInt(0, 10);
	std::cout << "randomNum: " << _randomNum << std::endl;

	Vector2D spawnPos = shootPos + Vector2D{0.5f, 0.5f};

	// Genera SarnoRata si _randomNum est� entre 0-4 (50% chance)
	if (_randomNum < 5)
	{
		GameScene::spawn_sarno_rata(spawnPos);
		// Game::Instance()->get_mngr()->getComponent<WaveManager>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::WAVE))->newEnemy();
		std::cout << "SarnoRata" << std::endl;
	}
	// Genera PlimPlim si _randomNum est� entre 5-7 (30% chance)
	else if (_randomNum < 8)
	{
		GameScene::spawn_plim_plim(spawnPos);
		// Game::Instance()->get_mngr()->getComponent<WaveManager>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::WAVE))->newEnemy();
		std::cout << "spawn_plim_plim" << std::endl;
	}
	// Genera Ratatouille si _randomNum est� entre 8-9 (20% chance)
	else
	{
		GameScene::spawn_ratatouille(spawnPos);
		// Game::Instance()->get_mngr()->getComponent<WaveManager>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::WAVE))->newEnemy();
		std::cout << "spawn_ratatouille" << std::endl;
	}
}