#include "ControlsScene.h"
#include "../../our_scripts/components/ui/Button.h"
#include "../GameStructs.h"
#include "../../utils/Vector2D.h"
#include "../../sdlutils/SDLUtils.h"
#include "../../sdlutils/InputHandler.h"
#include "../../ecs/Entity.h"
#include "../../sdlutils/Texture.h"
#ifdef GENERATE_LOG
#include "../../our_scripts/log_writer_to_csv.hpp"
#endif

ControlsScene::ControlsScene() : Scene(ecs::scene::CONTROLSSCENE)
{


}

ControlsScene::~ControlsScene()
{

}

void ControlsScene::initScene()
{
    auto* mngr = Game::Instance()->get_mngr();

    _background = &sdlutils().images().at("controls");

    //Boton back to main menu
    GameStructs::ButtonProperties mainMenuB = {
        { {0.1f, 0.1f},{0.1f, 0.1f} },
        0.0f, ""
    };
    mainMenuB.sprite_key = "heart";
    create_mainmenu_button(mainMenuB);
}

void ControlsScene::enterScene()
{
    Game::Instance()->get_mngr()->change_ent_scene(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA), ecs::scene::CONTROLSSCENE);
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log();
    log_writer_to_csv::Instance()->add_new_log("ENTERED CONTROLS SCENE");
#endif
}

void ControlsScene::exitScene()
{
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log("EXIT CONTROLS SCENE");
    log_writer_to_csv::Instance()->add_new_log();
#endif
}
void ControlsScene::render() {
    _background->render(200, 300);
    Scene::render();
}

void ControlsScene::create_mainmenu_button(const GameStructs::ButtonProperties& bp) {
    auto* mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    auto buttonComp = mngr->getComponent<Button>(e);
    buttonComp->connectClick([buttonComp, &mngr]() {
        

        Game::Instance()->change_Scene(Game::MAINMENU);
        });

    buttonComp->connectHover([buttonComp]() {
        (void)buttonComp;
        });
}