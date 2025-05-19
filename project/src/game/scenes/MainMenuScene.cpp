
#include "MainMenuScene.h"
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
#endif

MainMenuScene::MainMenuScene() : Scene(ecs::scene::MAINMENUSCENE)
{
}

MainMenuScene::~MainMenuScene()
{
}

void MainMenuScene::initScene()
{
    create_static_background(&sdlutils().images().at("background"));

    GameStructs::ButtonProperties buttonPropTemplate = {
        {{0.35f, 0.22f}, {0.30f, 0.25f}},
        0.0f,
        ""};

    // Button start
    GameStructs::ButtonProperties startB = buttonPropTemplate;
    startB.sprite_key = "enter_game";
    create_start_button(startB);

    // Button multiplauer
    buttonPropTemplate.rect.position.y += 0.18f;
    GameStructs::ButtonProperties multi = buttonPropTemplate;
    multi.sprite_key = "multiplayer";
    create_multiplayer_button(multi);

    // Button tutorial
    buttonPropTemplate.rect.position.y += 0.18f;
    GameStructs::ButtonProperties controlsB = buttonPropTemplate;
    controlsB.sprite_key = "controls_button";
    create_controls_button(controlsB);

    // Button exit
    buttonPropTemplate.rect.position.y += 0.18f;
    GameStructs::ButtonProperties exitB = buttonPropTemplate;
    exitB.sprite_key = "exit_game";
    create_exit_button(exitB);

    // Test para hacer el fondo con animaciones
    // GameStructs::ButtonProperties curtain_reaper_text = buttonPropTemplate;
    // curtain_reaper_text.rect.position.x = 0.85f;
    // curtain_reaper_text.rect.position.y = 0.86f;
    // curtain_reaper_text.rect.size.x = 0.15f;
    // curtain_reaper_text.rect.size.y = 0.15f;

    // curtain_reaper_text.sprite_key = "curtain_reaper_title";
    // create_decoration_images(curtain_reaper_text);

    // GameStructs::ButtonProperties star_derecha = buttonPropTemplate;
    // GameStructs::ButtonProperties star_izquierda = buttonPropTemplate;
    // GameStructs::ButtonProperties claw_type_1 = buttonPropTemplate;
    // GameStructs::ButtonProperties claw_type_2 = buttonPropTemplate;
    // GameStructs::ButtonProperties claw_type_3 = buttonPropTemplate;

    // image_p.sprite_key = "ui_star";
    // image_p.rect.position.y += 0.18f;
    // create_decoration_images(image_p);
    // image_p.rect.position.y += 0.18f;
    // create_decoration_images(image_p);
    // image_p.rect.position.y += 0.18f;
    // create_decoration_images(image_p);
    // image_p.rect.position.y += 0.18f;
}

void MainMenuScene::enterScene()
{
    Game::Instance()->get_mngr()->change_ent_scene(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA), ecs::scene::MAINMENUSCENE);
    sdlutils().musics().at("main_menu_bgm").play();
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log();
    log_writer_to_csv::Instance()->add_new_log("ENTERED MAIN MENU SCENE");
#endif
}

void MainMenuScene::exitScene()
{
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log("EXIT MAIN MENU SCENE");
    log_writer_to_csv::Instance()->add_new_log();
#endif
}

void MainMenuScene::create_start_button(const GameStructs::ButtonProperties &bp)
{
    auto *mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);

    auto imgComp = mngr->addComponent<ImageForButton>(e,
                                                      &sdlutils().images().at(bp.sprite_key),
                                                      &sdlutils().images().at(bp.sprite_key + "_selected"),
                                                      bp.rect,
                                                      0,
                                                      Game::Instance()->get_mngr()->getComponent<camera_component>(
                                                                                      Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))
                                                          ->cam);

    auto buttonComp = mngr->getComponent<Button>(e);
    buttonComp->connectClick([buttonComp, imgComp, mngr]()
                             {
        imgComp->_filter = false;
        imgComp->swap_textures();
        Game::Instance()->queue_scene(Game::SELECTIONMENU); });

    buttonComp->connectHover([buttonComp, imgComp]()
                             {
        imgComp->_filter = true;
        imgComp->swap_textures();
        sdlutils().soundEffects().at("button_hover").play(); });

    buttonComp->connectExit([buttonComp, imgComp]()
                            {
        imgComp->_filter = false;
        imgComp->swap_textures(); });
}

void MainMenuScene::create_decoration_images(const GameStructs::ButtonProperties &ip)
{
    std::cout << "Creating decoration image" << std::endl;
    create_decoration_image(ip);
}

void MainMenuScene::create_multiplayer_button(const GameStructs::ButtonProperties &bp)
{
    auto *mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);

    auto imgComp = mngr->addComponent<ImageForButton>(e,
                                                      &sdlutils().images().at(bp.sprite_key),
                                                      &sdlutils().images().at(bp.sprite_key + "_selected"),
                                                      bp.rect,
                                                      0,
                                                      Game::Instance()->get_mngr()->getComponent<camera_component>(
                                                                                      Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))
                                                          ->cam);

    auto buttonComp = mngr->getComponent<Button>(e);
    buttonComp->connectClick([buttonComp, imgComp, mngr]()
                             {
        imgComp->_filter = false;
        imgComp->swap_textures();
        Game::Instance()->queue_scene(Game::MULTIPLAYERMENU); });

    buttonComp->connectHover([buttonComp, imgComp]()
                             {
        imgComp->_filter = true;
        imgComp->swap_textures();
        sdlutils().soundEffects().at("button_hover").play(); });

    buttonComp->connectExit([buttonComp, imgComp]()
                            {
        imgComp->_filter = false;
        imgComp->swap_textures(); });
}

void MainMenuScene::create_controls_button(const GameStructs::ButtonProperties &bp)
{
    auto *mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    auto imgComp = mngr->addComponent<ImageForButton>(e,
                                                      &sdlutils().images().at(bp.sprite_key),
                                                      &sdlutils().images().at(bp.sprite_key + "_selected"),
                                                      bp.rect,
                                                      0,
                                                      Game::Instance()->get_mngr()->getComponent<camera_component>(
                                                                                      Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))
                                                          ->cam);

    auto buttonComp = mngr->getComponent<Button>(e);
    buttonComp->connectClick([buttonComp, imgComp, mngr]()
                             {
        imgComp->_filter = false;
        imgComp->swap_textures();
        imgComp->_filter = false;
        Game::Instance()->queue_scene(Game::TUTORIAL); });

    buttonComp->connectHover([buttonComp, imgComp]()
                             {
                                 imgComp->_filter = true;
                                 sdlutils().soundEffects().at("button_hover").play();
                                 imgComp->swap_textures();
                             });

    buttonComp->connectExit([buttonComp, imgComp]()
                            {
        imgComp->_filter = false;
        imgComp->swap_textures(); });
}

void MainMenuScene::create_exit_button(const GameStructs::ButtonProperties &bp)
{
    auto *mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    auto imgComp = mngr->addComponent<ImageForButton>(e,
                                                      &sdlutils().images().at(bp.sprite_key),
                                                      &sdlutils().images().at(bp.sprite_key + "_selected"),
                                                      bp.rect,
                                                      0,
                                                      Game::Instance()->get_mngr()->getComponent<camera_component>(
                                                                                      Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))
                                                          ->cam);

    auto buttonComp = mngr->getComponent<Button>(e);

    buttonComp->connectClick([buttonComp, imgComp, mngr]()
                             {
        imgComp->_filter = true;
        imgComp->swap_textures();
        Game::Instance()->set_exit(true); });

    buttonComp->connectHover([buttonComp, imgComp]()
                             {
        imgComp->_filter = true;
        auto sfx = &sdlutils().soundEffects().at("button_hover");
        sfx->play();
        imgComp->swap_textures(); });

    buttonComp->connectExit([buttonComp, imgComp]()
                            {
        imgComp->_filter = false;
        imgComp->swap_textures(); });
}