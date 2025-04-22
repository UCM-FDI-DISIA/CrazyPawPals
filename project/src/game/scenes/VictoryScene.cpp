#include "VictoryScene.h"
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
VictoryScene::VictoryScene() : Scene(ecs::scene::VICTORYSCENE) {}

VictoryScene::~VictoryScene() {}

void VictoryScene::initScene() {
    create_static_background(&sdlutils().images().at("victory"));
    create_enter_button();
    create_exit_button();
}
void VictoryScene::enterScene()
{
    Game::Instance()->get_mngr()->change_ent_scene(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA), ecs::scene::GAMEOVERSCENE);
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log();
    log_writer_to_csv::Instance()->add_new_log("ENTERED VICTORY SCENE");
#endif
}

void VictoryScene::exitScene()
{
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log("EXIT VICTORY SCENE");
    log_writer_to_csv::Instance()->add_new_log();
#endif
}
void VictoryScene::render() {
    Scene::render();
}
void VictoryScene::create_enter_button() {
    GameStructs::ButtonProperties bp = {
         { {0.375f, 0.65f},{0.3f, 0.125f} },
         0.0f, "back"
    };
    auto* mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    auto buttonComp = mngr->getComponent<Button>(e);
    auto imgComp = mngr->getComponent<transformless_dyn_image>(e);

    buttonComp->connectClick([buttonComp, imgComp, mngr, this]() {
        imgComp->_filter = false;
        Game::Instance()->change_Scene(Game::MAINMENU);
        });
    buttonComp->connectHover([buttonComp, imgComp, this]() { imgComp->_filter = true;});
    buttonComp->connectExit([buttonComp, imgComp, this]() { imgComp->_filter = false;});
}

void VictoryScene::create_exit_button()
{
    GameStructs::ButtonProperties bp = {
       { {0.375f, 0.8f},{0.3f, 0.125f} },
       0.0f, "exit"
    };
    auto* mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    auto buttonComp = mngr->getComponent<Button>(e);

    auto imgComp = mngr->getComponent<transformless_dyn_image>(e);

    buttonComp->connectClick([buttonComp, imgComp, mngr]() {
        imgComp->_filter = false;
        Game::Instance()->set_exit(true);
        });

    buttonComp->connectHover([buttonComp, imgComp]() {
        imgComp->_filter = true;
        });

    buttonComp->connectExit([buttonComp, imgComp]() {
        imgComp->_filter = false;
        });
}