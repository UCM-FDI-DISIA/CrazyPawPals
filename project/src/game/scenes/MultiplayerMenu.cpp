#include "MultiplayerMenu.h"
#include "../../our_scripts/components/ui/Button.h"
#include "../../our_scripts/components/rendering/transformless_dyn_image.h"
#include "../../our_scripts/components/rendering/ImageForButton.h"

#include "../GameStructs.h"
#include "../../utils/Vector2D.h"
#include "../../sdlutils/SDLUtils.h"
#include "../../sdlutils/InputHandler.h"
#include "../../ecs/Entity.h"
#include "../../sdlutils/Texture.h"
#ifdef GENERATE_LOG
#include "../../our_scripts/log_writer_to_csv.hpp"
#include "MultiplayerMenu.h"
#endif

MultiplayerMenu::MultiplayerMenu() : Scene(ecs::scene::MULTIPLAYERMENUSCENE), _ipHost("Introduce IP"), _isClient(false)
{
    create_static_background(&sdlutils().images().at("selection"));

    //Button back
    GameStructs::ButtonProperties backB = {
        { {0.025f, 0.025f},{0.10f, 0.07f} },
        0.0f, ""
    };
    backB.sprite_key = "back2";
    create_back_button(backB);

    //Button play
    GameStructs::ButtonProperties playB = {
        { {0.35f, 0.75f}, { 0.30f, 0.25f } },
            0.0f, ""
    };
    playB.sprite_key = "enter_game";
    create_play_button(playB);

    // --- BUTTONS FOR SKINS ---
     //Button mimi
    GameStructs::ButtonProperties skinMimiB = {
        { {0.2f, 0.15f}, { 0.15f, 0.25f } },
            0.0f, ""
    };
    skinMimiB.sprite_key = "mimibutton";
    create_skin_button(skinMimiB);

    //Button piu
    GameStructs::ButtonProperties skinPiuB = {
        { {0.2f, 0.5f}, { 0.15f, 0.25f } },
            0.0f, ""
    };
    skinPiuB.sprite_key = "piubutton";
    create_skin_button(skinPiuB);


    // --- BUTTONS ABOUT MULTIPLAYER ---
    //Button host
    GameStructs::ButtonProperties hostB = {
        { {0.5f, 0.15f}, { 0.20f, 0.15f } },
            0.0f, ""
    };
    hostB.sprite_key = "host";
    create_host_button(hostB);

    //Button copy ip
    GameStructs::ButtonProperties copyB = {
        { {0.7f, 0.15f}, { 0.20f, 0.15f } },
            0.0f, ""
    };
    copyB.sprite_key = "copyip";
    create_copy_ip_button(copyB);

    //Button client
    GameStructs::ButtonProperties clientB = {
        { {0.5f, 0.35f}, { 0.20f, 0.15f } },
            0.0f, ""
    };
    clientB.sprite_key = "client";
    create_client_button(clientB);

}

MultiplayerMenu::~MultiplayerMenu()
{
}

void MultiplayerMenu::initScene()
{
}

void MultiplayerMenu::enterScene()
{
    Game::Instance()->get_mngr()->change_ent_scene(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA), ecs::scene::MAINMENUSCENE);
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log();
    log_writer_to_csv::Instance()->add_new_log("ENTERED MULTIPLAYER MENU SCENE");
#endif
}

void MultiplayerMenu::exitScene()
{
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log("EXIT MULTIPLAYER MENU SCENE");
    log_writer_to_csv::Instance()->add_new_log();
#endif
}

void MultiplayerMenu::update(uint32_t delta_time)
{
    Scene::update(delta_time);

    //If is Client, changes textInput
    if (_isClient) {
        _ipHost = "Hi";
    }


}

void MultiplayerMenu::render()
{
    Scene::render();

    auto _cam = Game::Instance()->get_mngr()->getComponent<camera_component>(
        Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA));

    //Updates text input 
    //Adapted to screep
    rect_f32 textInput = rect_f32_screen_rect_from_viewport(rect_f32({ 0.725f, 0.39f }, { 0.15f, 0.075f }), _cam->cam.screen);
    //The real field
    SDL_Rect textField{
        int(textInput.position.x),
        int(textInput.position.y),
        int(textInput.size.x),
        int(textInput.size.y)
    };
    //The text
    Texture textFieldText{
        sdlutils().renderer(),
        _ipHost,
        sdlutils().fonts().at("ARIAL16"),
        SDL_Color(0, 0, 0, 255),
        SDL_Color(255, 255, 255, 255),
    };
    //Renders text
    textFieldText.render(textField);

}

void MultiplayerMenu::create_play_button(const GameStructs::ButtonProperties& bp)
{
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
    buttonComp->connectClick([buttonComp, imgComp, mngr]() {
        imgComp->_filter = false;
        imgComp->swap_textures();
        Game::Instance()->change_Scene(Game::SELECTIONMENU);
        });

    buttonComp->connectHover([buttonComp, imgComp]() {
        imgComp->_filter = true;
        imgComp->swap_textures();
        });

    buttonComp->connectExit([buttonComp, imgComp]() {
        imgComp->_filter = false;
        imgComp->swap_textures();
        });
}

void MultiplayerMenu::create_host_button(const GameStructs::ButtonProperties& bp)
{
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
    buttonComp->connectClick([buttonComp, imgComp, mngr]() {
        imgComp->_filter = false;
        imgComp->swap_textures();
        
        //TODO
        std::cout << "You are the host.";
        //Activates the button regarding copy ip

        });

    buttonComp->connectHover([buttonComp, imgComp]() {
        imgComp->_filter = true;
        imgComp->swap_textures();
        });

    buttonComp->connectExit([buttonComp, imgComp]() {
        imgComp->_filter = false;
        imgComp->swap_textures();
        });
}

void MultiplayerMenu::create_copy_ip_button(const GameStructs::ButtonProperties& bp)
{
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
    buttonComp->connectClick([buttonComp, imgComp, mngr]() {
        imgComp->_filter = false;
        imgComp->swap_textures();

        //TODO
        std::cout << "Your ip is copied.";
        //Sends it to players


        });

    buttonComp->connectHover([buttonComp, imgComp]() {
        imgComp->_filter = true;
        imgComp->swap_textures();
        });

    buttonComp->connectExit([buttonComp, imgComp]() {
        imgComp->_filter = false;
        imgComp->swap_textures();
        });
}

void MultiplayerMenu::create_client_button(const GameStructs::ButtonProperties& bp)
{
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
    buttonComp->connectClick([buttonComp, imgComp, mngr]() {
        imgComp->_filter = false;
        imgComp->swap_textures();

        //TODO
        std::cout << "Your are a client.";
        //Activates the button regarding enter ip

        });

    buttonComp->connectHover([buttonComp, imgComp]() {
        imgComp->_filter = true;
        imgComp->swap_textures();
        });

    buttonComp->connectExit([buttonComp, imgComp]() {
        imgComp->_filter = false;
        imgComp->swap_textures();
        });
}

void MultiplayerMenu::create_back_button(const GameStructs::ButtonProperties& bp)
{
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
    buttonComp->connectClick([buttonComp, imgComp, mngr]() {
        imgComp->_filter = false;
        imgComp->swap_textures();

        Game::Instance()->change_Scene(Game::MAINMENU);

        });

    buttonComp->connectHover([buttonComp, imgComp]() {
        imgComp->_filter = true;
        imgComp->swap_textures();
        });

    buttonComp->connectExit([buttonComp, imgComp]() {
        imgComp->_filter = false;
        imgComp->swap_textures();
        });
}

void MultiplayerMenu::create_skin_button(const GameStructs::ButtonProperties& bp)
{
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
    buttonComp->connectClick([buttonComp, imgComp, mngr]() {
        imgComp->_filter = false;
        imgComp->swap_textures();

        //TODO
        std::cout << "You choosed your skin.";
        //Sends it to players

        });

    buttonComp->connectHover([buttonComp, imgComp]() {
        imgComp->_filter = true;
        imgComp->swap_textures();
        });

    buttonComp->connectExit([buttonComp, imgComp]() {
        imgComp->_filter = false;
        imgComp->swap_textures();
        });
}


