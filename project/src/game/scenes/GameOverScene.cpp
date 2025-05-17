
#include "GameOverScene.h"
#include "../../our_scripts/components/ui/Button.h"
#include "../GameStructs.h"
#include "../../utils/Vector2D.h"
#include "../../sdlutils/SDLUtils.h"
#include "../../sdlutils/InputHandler.h"
#include "../../ecs/Entity.h"

#include "../../our_scripts/components/weapons/player/Revolver.h"
#include "../../our_scripts/components/weapons/player/Rampage.h"
#include "../../our_scripts/components/weapons/player/PumpShotgun.h"
#include "../../our_scripts/components/weapons/player/RampCanon.h"
#include "../../our_scripts/components/weapons/player/Lightbringer.h"

#include "../../our_scripts/card_system/Card.hpp"
#include "../../our_scripts/card_system/PlayableCards.hpp"
#include "../../our_scripts/components/cards/Deck.hpp"
#include "../../our_scripts/components/rendering/Image.h"
#include "../../our_scripts/components/rendering/ImageForButton.h"
#ifdef GENERATE_LOG
#include "../../our_scripts/log_writer_to_csv.hpp"
#endif

#include <iostream>
#include <typeinfo>
#include <algorithm>
GameOverScene::GameOverScene()
    : Scene(ecs::scene::GAMEOVERSCENE) {}

GameOverScene::~GameOverScene()
{
}
void GameOverScene::initScene() {
    create_static_background(&sdlutils().images().at("game_over"));
    create_enter_button();
    create_exit_button();
}
void GameOverScene::enterScene()
{
    sdlutils().musics().at("game_bgm").haltMusic();
    sdlutils().soundEffects().at("round_start_event").haltChannel();
    sdlutils().soundEffects().at("round_start").haltChannel();
    sdlutils().soundEffects().at("game_over").play();
    Game::Instance()->get_mngr()->change_ent_scene(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA), ecs::scene::GAMEOVERSCENE);
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log();
    log_writer_to_csv::Instance()->add_new_log("ENTERED GAMEOVER SCENE");
#endif
}

void GameOverScene::exitScene()
{
    sdlutils().soundEffects().at("game_over").haltChannel();
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log("EXIT GAMEOVER SCENE");
    log_writer_to_csv::Instance()->add_new_log();
#endif
}
void GameOverScene::render() {
    Scene::render();
}
void GameOverScene::create_enter_button() {
    GameStructs::ButtonProperties bp = {
         { {0.375f, 0.5f},{0.3f, 0.2} },
         0.0f, "back"
    };
    auto* mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    auto imgComp = mngr->addComponent<ImageForButton>(e,
        &sdlutils().images().at(bp.sprite_key),
        &sdlutils().images().at(bp.sprite_key + "_selected"),
        bp.rect,
        0,
        Game::Instance()->get_mngr()->getComponent<camera_component>(
            Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam
    );
    auto buttonComp = mngr->getComponent<Button>(e);

    buttonComp->connectClick([buttonComp, imgComp, mngr, this]() {
        imgComp->_filter = false;
        imgComp->swap_textures();
        Game::Instance()->queue_scene(Game::MAINMENU);
    }); 
    buttonComp->connectHover([buttonComp, imgComp, this]() { 
        imgComp->swap_textures();
        imgComp->_filter = true;
        sdlutils().soundEffects().at("button_hover").play();
        });
    buttonComp->connectExit([buttonComp, imgComp, this]() { 
        imgComp->_filter = false;
        imgComp->swap_textures();});
}

void GameOverScene::create_exit_button()
{
    GameStructs::ButtonProperties bp = {
       { {0.375f, 0.7f},{0.3f, 0.2f} },
       0.0f, "exit_game"
    };
    auto* mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    auto buttonComp = mngr->getComponent<Button>(e);

    auto imgComp = mngr->addComponent<ImageForButton>(e,
        &sdlutils().images().at(bp.sprite_key),
        &sdlutils().images().at(bp.sprite_key + "_selected"),
        bp.rect,
        0,
        Game::Instance()->get_mngr()->getComponent<camera_component>(
            Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam
    );

    buttonComp->connectClick([buttonComp, imgComp, mngr]() {
        Game::Instance()->set_exit(true);
        imgComp->_filter = false;
        imgComp->swap_textures();
        });

    buttonComp->connectHover([buttonComp, imgComp]() {
        imgComp->swap_textures();
        imgComp->_filter = true;
        sdlutils().soundEffects().at("button_hover").play();
        });

    buttonComp->connectExit([buttonComp, imgComp]() {
        imgComp->_filter = false;
        imgComp->swap_textures();
        });
}
