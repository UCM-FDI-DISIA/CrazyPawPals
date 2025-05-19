
#include "SelectionMenuScene.h"
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
SelectionMenuScene::SelectionMenuScene() 
    : Scene(ecs::scene::SELECTIONMENUSCENE), _weapon_selected(false), _deck_selected(false), _last_weapon_button(nullptr), _last_deck_button(nullptr), 
    _num_cards_of_deck(4), _activate_play_button(false) {
}

SelectionMenuScene::~SelectionMenuScene()
{
}
void SelectionMenuScene::create_weapon_buttons() {
    float umbral = 0.0075f;
    float offsetX = 0.07f;  // Distance between buttons on the X axis
    float startX = 0.71f;   // Starting position of the first button on X
    float startY = 0.2f; // Starting position of the first button on Y

    GameStructs::ButtonProperties buttonPropTemplate = {
        { { startX, startY }, {0.05f, 0.1f} },
        0.0f, "", ecs::grp::WEAPONBUTTON
    };

    //Button back
    GameStructs::ButtonProperties backB = {
        { {0.9f, 0.9f},{0.10f, 0.1f} },
        0.0f, ""
    };
    backB.sprite_key = "back";
    create_back_button(backB);

    GameStructs::ButtonProperties revolverB = buttonPropTemplate;
    revolverB.sprite_key = "revolver_button";
    create_weapon_button(GameStructs::REVOLVER, revolverB);

    buttonPropTemplate.rect.position.x += offsetX;  // Move to the right
    GameStructs::ButtonProperties rampageB = buttonPropTemplate;
    rampageB.sprite_key = "rampage_button";
    create_weapon_button(GameStructs::RAMPAGE, rampageB);

    buttonPropTemplate.rect.position.x += offsetX;  // Move to the right
    GameStructs::ButtonProperties pump_shotgun_B = buttonPropTemplate;
    pump_shotgun_B.sprite_key = "pump_shotgun_button";
    create_weapon_button(GameStructs::PUMP_SHOTGUN, pump_shotgun_B);

    buttonPropTemplate.rect.position.x += offsetX;  // Move to the right
    GameStructs::ButtonProperties rampCanonB = buttonPropTemplate;
    rampCanonB.sprite_key = "ramp_canon_button";
    create_weapon_button(GameStructs::RAMP_CANON, rampCanonB);

    //buttonPropTemplate.rect.position.x += offsetX;  // Move to the right
    //GameStructs::ButtonProperties lightbringerB = buttonPropTemplate;
    //lightbringerB.sprite_key = "lightbringer_button";
    //create_weapon_button(GameStructs::LIGHTBRINGER, lightbringerB);

    //we ll add these two when we have other weapon implemented
    //buttonPropTemplate.rect.position.x += offsetX;  // Move to the right
    //GameStructs::ButtonProperties another_B = buttonPropTemplate;
    //another_B.sprite_key = "arma6_button";
    //create_weapon_button(GameStructs::WEAPON6, another_B);

    //buttonPropTemplate.rect.position.y += umbral; // Move to the next row
    //buttonPropTemplate.rect.position.x = startX;  // Reset X to the initial position
    //buttonPropTemplate.rect.position.x += offsetX;  // Move to the right

    //GameStructs::ButtonProperties another2 = buttonPropTemplate;
    //another2.sprite_key = "arma7_button";
    //create_weapon_button(GameStructs::WEAPON7, another2);
}

void SelectionMenuScene::create_back_button(const GameStructs::ButtonProperties& bp) {
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
        Game::Instance()->queue_scene(Game::MAINMENU);});

    buttonComp->connectHover([buttonComp, imgComp]() {
        imgComp->_filter = true;
        imgComp->swap_textures();
        sdlutils().soundEffects().at("button_hover").play();
        });

    buttonComp->connectExit([buttonComp, imgComp]() {
        imgComp->_filter = false;
        imgComp->swap_textures();
        });
}
void SelectionMenuScene::create_deck_buttons() {
    float umbral = 0.1f;
    //create the first button prop
    GameStructs::ButtonProperties buttonPropTemplate = {
         { {0.0075f, 0.2f},{0.085f, 0.135f} },
         0.0f, "", ecs::grp::DECKBUTTON
    };
    GameStructs::ButtonProperties deck1B = buttonPropTemplate;
    deck1B.sprite_key = "deck1_button";
    create_deck_button(GameStructs::ONE, deck1B);

    buttonPropTemplate.rect.position.x += umbral;
    GameStructs::ButtonProperties deck2B = buttonPropTemplate;
    deck2B.sprite_key = "deck2_button";
    create_deck_button(GameStructs::TWO, deck2B);

    buttonPropTemplate.rect.position.x += umbral;
    GameStructs::ButtonProperties deck3B = buttonPropTemplate;
    deck3B.sprite_key = "deck3_button";
    create_deck_button(GameStructs::THREE, deck3B);

    buttonPropTemplate.rect.position.x += umbral;
    GameStructs::ButtonProperties deck4B = buttonPropTemplate;
    deck4B.sprite_key = "deck4_button";
    create_deck_button(GameStructs::FOUR, deck4B);
}
void SelectionMenuScene::initScene() {
    create_static_background(&sdlutils().images().at("selection"));
    
    create_weapon_info();
    create_weapon_buttons();
    create_deck_buttons();
    create_deck_infos();
    create_enter_button();
}
void SelectionMenuScene::reset() {
    _weapon_selected = false;
    _deck_selected = false;
    //we need to clean the filter of both side texture
    if (_last_deck_button != nullptr) {
        _last_deck_button->_filter = false;
        _last_deck_button->swap_textures();
        _last_deck_button->_filter = false;
    }
    _last_deck_button = nullptr;
    if (_last_weapon_button != nullptr) {
        _last_weapon_button->_filter = false;
        _last_weapon_button->swap_textures();
        _last_weapon_button->_filter = false;
    }
    _last_weapon_button = nullptr;
    _activate_play_button = false;

    auto* mngr = Game::Instance()->get_mngr();
    auto& deckInfo = mngr->getEntities(ecs::grp::DECKINFO);
    for (auto& d : deckInfo) {
        auto img = mngr->getComponent<transformless_dyn_image>(d);
        img->set_texture(&sdlutils().images().at("initial_info"));
    }
}

void SelectionMenuScene::enterScene()
{
    Game::Instance()->get_mngr()->change_ent_scene(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA), ecs::scene::SELECTIONMENUSCENE);
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log();
    log_writer_to_csv::Instance()->add_new_log("ENTERED SELECTION MENU SCENE");
#endif
    reset();
    if (!Game::Instance()->is_network_none()) {
        Game::network_users_state& state = Game::Instance()->get_network_state();
        state.game_state.ready_users.reset();
        state.game_state.ready_user_count = 0;
    }
}

void SelectionMenuScene::exitScene()
{
    auto* mngr = Game::Instance()->get_mngr();
    //if(_activate_play_button)mngr->getComponent<ImageForButton>(mngr->getHandler(ecs::hdlr::TOGAMEBUTTON))->swap_textures();

    auto playB = mngr->getHandler(ecs::hdlr::TOGAMEBUTTON);
    auto playImg = mngr->getComponent<ImageForButton>(playB);
    playImg->_filter = false;
    sdlutils().musics().at("main_menu_bgm").haltMusic();
    sdlutils().musics().at("game_bgm").play();

    auto imgComp = mngr->getComponent<ImageForButton>(mngr->getHandler(ecs::hdlr::TOGAMEBUTTON));
    imgComp->set_texture(&sdlutils().images().at("ready"));
    imgComp->destination_rect.position.x = 10.4f;


#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log("EXIT SELECTION MENU SCENE");
    log_writer_to_csv::Instance()->add_new_log();
#endif

}

void SelectionMenuScene::render()
{
    Scene::render();
    if (showing_message) {
        auto camera = Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA));
        rect_f32 message_rect = rect_f32_screen_rect_from_viewport(rect_f32({ { 0.4,0.7 }, { 0.2,0.05 } }), camera->cam.screen);
        SDL_Rect message_true{
            int(message_rect.position.x),
            int(message_rect.position.y),
            int(message_rect.size.x),
            int(message_rect.size.y)
        };
        Texture message_tex{
        sdlutils().renderer(),
        message,
        sdlutils().fonts().at("RUBIK_MONO"),
        SDL_Color({128, 0, 32, 255}) };
        message_tex.render(message_true);
    }
}


void SelectionMenuScene::create_weapon_button(GameStructs::WeaponType wt, const GameStructs::ButtonProperties& bp) {
    auto* mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    auto buttonComp = mngr->getComponent<Button>(e);
    //used for change the sprite once a button is clicked
    auto imgComp = mngr->addComponent<ImageForButton>(e,
        &sdlutils().images().at(bp.sprite_key),
        &sdlutils().images().at(bp.sprite_key + "_selected"),
        bp.rect,
        0,
        Game::Instance()->get_mngr()->getComponent<camera_component>(
            Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam
    );
    auto player = mngr->getHandler(ecs::hdlr::PLAYER);

    buttonComp->connectClick([buttonComp, imgComp, mngr, wt, player, this]() {
        
        std::string s;

        switch (wt) {
        case GameStructs::REVOLVER:
            mngr->addComponent<Revolver>(player);
            break;
        case GameStructs::RAMPAGE:
            mngr->addComponent<Rampage>(player);
            break;
        case GameStructs::PUMP_SHOTGUN:
            mngr->addComponent<PumpShotgun>(player);
            break;
        case GameStructs::RAMP_CANON:
            mngr->addComponent<RampCanon>(player);
            break;
        case GameStructs::LIGHTBRINGER:
            mngr->addComponent<Lightbringer>(player);
            break;
        case GameStructs::WEAPON6:
            mngr->addComponent<Rampage>(player);
            break;
        case GameStructs::WEAPON7:
            mngr->addComponent<Revolver>(player);
            break;
        default:
            break;
        }

        _weapon_selected = true;

        if (imgComp != _last_weapon_button) {
            //swap the actual buttons textures
            imgComp->_filter = false;
            imgComp->swap_textures();
        }

        //register the last clicked button
        if (_last_weapon_button != nullptr && _last_weapon_button != imgComp) {
            _last_weapon_button->swap_textures();
        }
        _last_weapon_button = imgComp;
        

    });
    buttonComp->connectHover([buttonComp, imgComp, mngr, wt]() {
        sdlutils().soundEffects().at("button_hover").play();
        std::string s;
        auto info = mngr->getHandler(ecs::hdlr::WEAPONINFO);
        auto infoImg = mngr->getComponent<transformless_dyn_image>(info);
        switch (wt) {
        case GameStructs::REVOLVER:
            s = "revolver_";
            break;
        case GameStructs::RAMPAGE:
            s = "rampage_";
            break;
        case GameStructs::PUMP_SHOTGUN:
            s = "pump_shotgun_";
            break;
        case GameStructs::RAMP_CANON:
            s = "ramp_canon_";
            break;
        case GameStructs::LIGHTBRINGER:
            s = "lightbringer_";
            break;
        case GameStructs::WEAPON6:
            s = "arma6_";
            break;
        case GameStructs::WEAPON7:
            s = "arma7_";
            break;
        default:
            break;
        }
        s += "info";
        infoImg->set_texture(&sdlutils().images().at(s));
        imgComp->_filter = true;
    });
    buttonComp->connectExit([buttonComp, imgComp,mngr]() {
        auto info = mngr->getHandler(ecs::hdlr::WEAPONINFO); 
        auto infoImg = mngr->getComponent<transformless_dyn_image>(info); 
        infoImg->set_texture(&sdlutils().images().at("initial_info"));
        imgComp->_filter = false;
    });
}
void SelectionMenuScene::create_deck_button(GameStructs::DeckType dt, const GameStructs::ButtonProperties& bp) {
    auto* mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    auto buttonComp = mngr->getComponent<Button>(e);
    auto player = mngr->getHandler(ecs::hdlr::PLAYER);
    //used for change the sprite once a button is clicked
    auto imgComp = mngr->addComponent<ImageForButton>(e,
        &sdlutils().images().at(bp.sprite_key),
        &sdlutils().images().at(bp.sprite_key + "_selected"),
        bp.rect,
        0,
        Game::Instance()->get_mngr()->getComponent<camera_component>(
            Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam
    );
    buttonComp->connectClick([buttonComp, imgComp, mngr, player, dt, this]() {
        std::list<Card*> cl = {};
        
        switch (dt)
        {
        case GameStructs::ONE:
            cl = { new Evoke(), new Fireball(), new Minigun(), new Commune()};
            break;
        case GameStructs::TWO: 
            cl = { new Fireball(), new CardSpray(), new Lighting(), new Minigun()};
            break;
        case GameStructs::THREE:
            cl = { new Fulgur(), new Fireball(), new Minigun(), new Kunai()};
            break;
        case GameStructs::FOUR:
            cl = { new Kunai(), new EldritchBlast(), new QuickFeet (), new CardSpray()};
            break;
        default:
            break;
        }

        set_concrete_deck_info(cl);

        mngr->addComponent<Deck>(player, cl);
        _deck_selected = true;

        //swap the actual buttons textures
        if (_last_deck_button != nullptr && _last_deck_button != imgComp) {
            imgComp->_filter = false;
            imgComp->swap_textures();
            _last_deck_button->swap_textures();
            //register the clicked button
        }
        else if (_last_deck_button == nullptr) { //special case: first click
            imgComp->_filter = false;
            imgComp->swap_textures();
        }
        _last_deck_button = imgComp;
    });

    buttonComp->connectHover([buttonComp, imgComp]() {
        imgComp->_filter = true;
        sdlutils().soundEffects().at("button_hover").play();
        });
    buttonComp->connectExit([buttonComp, imgComp]() {
        imgComp->_filter = false;
        });
}
void SelectionMenuScene::create_deck_info(const rect_f32& rect) {
    auto& cam = Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam;
    ecs::entity_t e = create_entity(
        ecs::grp::DECKINFO,
        ecs::scene::SELECTIONMENUSCENE,
        new transformless_dyn_image
        (rect, 
        0,
        cam,
        &sdlutils().images().at("initial_info"))
    );
}
void SelectionMenuScene::create_deck_infos() {
    float umbral = 0.14f;
    rect_f32 r = {{ 0.0375f, 0.385f }, { 0.325f, 0.14f }};
    for (int i = 0; i < _num_cards_of_deck; ++i) {
        create_deck_info(r); 
        r.position.y += umbral;
    }
}
void SelectionMenuScene::create_weapon_info() {
   // rect_f32 rect = {{1.3f, 0.25f} ,{0.75f, 0.5f}};
    rect_f32 rect = { {0.695f, 0.315f} ,{0.3f, 0.25f} };
    ecs::entity_t e = create_entity(
        ecs::grp::UI,
        ecs::scene::SELECTIONMENUSCENE,
        new transformless_dyn_image
        (rect,
            0,
            Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam,
            &sdlutils().images().at("initial_info"))
    );
    auto mngr = Game::Instance()->get_mngr();
    mngr->setHandler(ecs::hdlr::WEAPONINFO, e);
}

void SelectionMenuScene::set_concrete_deck_info(const std::list<Card*>& cl) {
    auto mngr = Game::Instance()->get_mngr();
    auto& infos = mngr->getEntities(ecs::grp::DECKINFO);

    auto itInfo = infos.begin();
    for (auto it = cl.begin(); it != cl.end() && itInfo != infos.end(); ++it, ++itInfo) {
        auto img = mngr->getComponent<transformless_dyn_image>(*itInfo);
        std::string typeName = (*it)->get_name();
        img->set_texture(&sdlutils().images().at(typeName + "_info"));
    }
}
void SelectionMenuScene::create_enter_button() {
    GameStructs::ButtonProperties bp = {
         { {10.0f, 0.4f},{0.2f, 0.15f} },
         0.0f, "ready", ecs::grp::UI
    };
    auto* mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    mngr->setHandler(ecs::hdlr::TOGAMEBUTTON, e);
    auto imgComp = mngr->addComponent<ImageForButton>(e,
        &sdlutils().images().at("initial_info"),
        &sdlutils().images().at(bp.sprite_key+"_selected"),
        bp.rect,
        0,
        Game::Instance()->get_mngr()->getComponent<camera_component>(
            Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam
    );


    auto buttonComp = mngr->getComponent<Button>(e);

    buttonComp->connectClick([buttonComp, mngr, imgComp, this]() {
        if (_weapon_selected && _deck_selected) {
            if (!Game::Instance()->is_network_none()) {
                network_context& network = Game::Instance()->get_network();
                Game::network_users_state& state = Game::Instance()->get_network_state();
                uint32_t id = state.connections.local_user_index;
                if (Game::Instance()->is_host()) {
                    state.game_state.ready_users.set(0, true);
                    state.game_state.ready_user_count++;
                    if (checkAllPlayersReady())Game::Instance()->queue_scene(Game::GAMESCENE);
                }
                else if(!state.game_state.ready_users.test(id)){
                    state.game_state.ready_users.set(id, true);

                    network_message_pack_send(
                        network.profile.client.socket_to_host,
                        network_message_pack_create(network_message_type_player_ready,
                            create_player_id_message(id))
                    );
                }
                show_message("Esperando a los otros michis...", 5 * 1000);
            }
            else {
                imgComp->_filter = false;
                Game::Instance()->queue_scene(Game::GAMESCENE);
                imgComp->destination_rect.position.x = 10.0f;
            }
        }
    });
    buttonComp->connectHover([buttonComp, imgComp, this]() {     
        if (!_activate_play_button) return;
        sdlutils().soundEffects().at("button_hover").play(); 
        imgComp->_filter = true;
        imgComp->swap_textures();
        });
    buttonComp->connectExit([buttonComp, imgComp, this]() { 
        if (!_activate_play_button) return;
        imgComp->_filter = false;
    imgComp->swap_textures();});
}
void SelectionMenuScene::update(uint32_t delta_time) {

    Scene::update(delta_time);

    if (!_activate_play_button && _last_weapon_button && _last_deck_button) {
        auto* mngr = Game::Instance()->get_mngr();
        auto imgComp = mngr->getComponent<ImageForButton>(mngr->getHandler(ecs::hdlr::TOGAMEBUTTON));
        imgComp->set_texture(&sdlutils().images().at("ready"));
        _activate_play_button = true;
        imgComp->destination_rect.position.x = 0.4f;
    }

    if (!Game::Instance()->is_network_none()) {
        network_context& network = Game::Instance()->get_network();

        if (Game::Instance()->is_host()) {
            if (SDLNet_CheckSockets(network.profile.host.clients_host_set, 0) > 0) {
                for (network_connection_size i = 0; i < network.profile.host.sockets_to_clients.connection_count; ++i) {
                    TCPsocket& connection = network.profile.host.sockets_to_clients.connections[i];
                    if (SDLNet_SocketReady(connection)) {
                        network_message_dynamic_pack dyn_message = network_message_dynamic_pack_receive(connection);
                        const uint16_t type_n{ dyn_message->header.type_n };
                        const uint16_t type_h{ SDLNet_Read16(&type_n) };
                        switch (type_h) {
                        case network_message_type_player_ready: {
                            auto message = network_message_dynamic_pack_into<network_message_player_id>(std::move(dyn_message));
                            auto&& payload = message->payload.content;
                            uint32_t id = SDLNet_Read32(&payload.id_n);

                            Game::network_users_state& state = Game::Instance()->get_network_state();
                            state.game_state.ready_users.set(id, true);
                            state.game_state.ready_user_count++;

                            if (checkAllPlayersReady())Game::Instance()->queue_scene(Game::GAMESCENE);
                            break;
                        }
                        default: {
                            break;
                        }
                        }
                    }
                }
            }
        }
        else {
            int active_sockets = SDLNet_CheckSockets(network.profile.client.client_set, 0);
            if (active_sockets > 0 && SDLNet_SocketReady(network.profile.client.socket_to_host)) {
                auto msg = network_message_dynamic_pack_receive(network.profile.client.socket_to_host);
                const uint16_t type_n{ msg->header.type_n };
                const uint16_t type_h{ SDLNet_Read16(&type_n) };
                switch (type_h) {
                case network_message_type_start_game: {
                    std::cout << "mensaje de ir al juego" << std::endl;
                    Game::Instance()->queue_scene(Game::GAMESCENE);
                    break;
                }
                default: break;
                }
            }

        }
       
    }
}
