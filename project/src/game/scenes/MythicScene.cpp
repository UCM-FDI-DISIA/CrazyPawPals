
#include "MythicScene.h"

#include "../../our_scripts/components/ui/Button.h"
#include "../GameStructs.h"
#include "../../sdlutils/SDLUtils.h"
#include "../../sdlutils/InputHandler.h"
#include "../../ecs/Entity.h"

#include "../../our_scripts/card_system/Card.hpp"
#include "../../our_scripts/card_system/CardList.h" 
#include "../../our_scripts/card_system/PlayableCards.hpp"
#include "../../our_scripts/components/cards/Deck.hpp"
#include "../../our_scripts/components/MythicComponent.h"
#include "../../our_scripts/mythic/MythicItem.h"
#include "../../our_scripts/mythic/MythicItems.h"
#include "../../our_scripts/components/rendering/transformless_dyn_image.h" 
#include "../../our_scripts/components/rendering/ImageForButton.h"
#include "../../our_Scripts/mythic/MythicDataComponent.h"

#ifdef GENERATE_LOG
#include "../../our_scripts/log_writer_to_csv.hpp"
#endif

#include <iostream>

MythicScene::MythicScene() : Scene(ecs::scene::MYTHICSCENE), _selected_mythic(nullptr), _last_my_mythic_img(nullptr),
_lm(nullptr), _selected(false), _activate_confirm_button(false), _chosen_mythic(nullptr) {}

void MythicScene::initScene() {
    create_static_background(&sdlutils().images().at("mythic_ambient"));
    create_mythic_info();
    create_reward_mythic_buttons();
    create_my_mythics();
}

void MythicScene::enterScene()
{
    auto* mngr = Game::Instance()->get_mngr();
    auto* player = mngr->getHandler(ecs::hdlr::PLAYER);
    auto _m_mythics = mngr->getComponent<MythicComponent>(player);
    auto& m = _m_mythics->get_mythics();
    refresh_mythics();
    refresh_my_mythics(m);
    _activate_confirm_button = true;
    Game::Instance()->get_mngr()->change_ent_scene(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA), ecs::scene::MYTHICSCENE);

    if (!Game::Instance()->is_network_none()) {
        Game::network_users_state& state = Game::Instance()->get_network_state();
        state.game_state.ready_users.reset();
        state.game_state.ready_user_count = 0;
}
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log();
    log_writer_to_csv::Instance()->add_new_log("ENTERED MYTHIC SCENE");
#endif
}

void MythicScene::exitScene()
{
    showing_message = false;

    _lm->resize(1.0f/1.1f);

    _lm = nullptr;
    _selected = false;

    _last_my_mythic_img = nullptr;
    _selected_mythic = nullptr;

    _chosen_mythic = nullptr;

    _activate_confirm_button = false;
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log("EXIT MYTHIC SCENE");
    log_writer_to_csv::Instance()->add_new_log();
#endif
}

void MythicScene::create_mythic_info() {
    auto e = create_entity(ecs::grp::UI,
        _scene_ID,
        new transformless_dyn_image({ { 0.315f,0.475f }, {0.4f,0.2f} },
            0,
            Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam,
            &sdlutils().images().at("initial_info")));
    Game::Instance()->get_mngr()->setHandler(ecs::hdlr::MYTHICINFO, e);
}

std::string MythicScene::select_mythic(GameStructs::MythicType wt) {
    std::string s = "";
    switch (wt)
    {
    case GameStructs::BLOODCLAW: s = "mythic_blood_claw";
        break;
    case GameStructs::DREAMRECICLER: s = "mythic_dream_recicler";
        break;
    case GameStructs::CURTAINREAPER: s = "mythic_curtain_reaper";
        break;
    case GameStructs::INCENSE: s = "mythic_incense";
        break;
    case GameStructs::QUICKDRAWDECK: s = "mythic_quick_draw_deck";
        break;
    case GameStructs::BLOODPACT: s = "mythic_blood_pact";
        break;
    case GameStructs::DEMONICSCRATCHER: s = "mythic_demonic_scratcher";
        break;
    case GameStructs::CLAWFILE: s = "mythic_claw_file";
        break;
    case GameStructs::URANIUMSOCKS: s = "mythic_uranium_socks";
        break;
    case GameStructs::ZOOMIESINDUCER: s = "mythic_zoomies_inducer";
        break;
    default:
        break;
    }
    return s;
}

std::pair<std::string, GameStructs::MythicType> 
MythicScene::get_unique_mythic(std::unordered_set<std::string>& appeared_mythics) {
    std::string sprite;
    GameStructs::MythicType wt;
    do {
        wt = (GameStructs::MythicType)sdlutils().rand().nextInt(0, GameStructs::LAST_MYTHIC);
        sprite = select_mythic(wt);
    } while (appeared_mythics.find(sprite) != appeared_mythics.end()); // Repite si la carta ya apareci�

    appeared_mythics.insert(sprite);
    return { sprite, wt };
}

void MythicScene::create_reward_mythic_buttons() {
    float umbral = 0.2f;
    GameStructs::ButtonProperties buttonPropTemplate = {
        {{0.44f, 0.25f}, {0.125f, 0.2f}},
        0.0f, "", ecs::grp::MYTHICOBJS
    };

    // Auxiliar set to check if we repeat some card
    std::unordered_set<std::string> appeared_mythics;

    //three card reward buttons
    buttonPropTemplate.sprite_key = get_unique_mythic(appeared_mythics).first;
    create_reward_mythic_button(buttonPropTemplate);

    buttonPropTemplate.sprite_key = get_unique_mythic(appeared_mythics).first;
    buttonPropTemplate.rect.position.x -= umbral;
    create_reward_mythic_button(buttonPropTemplate);

    buttonPropTemplate.sprite_key = get_unique_mythic(appeared_mythics).first;
    buttonPropTemplate.rect.position.x += umbral * 2;
    create_reward_mythic_button(buttonPropTemplate);

    buttonPropTemplate.ID = ecs::grp::UI;
    buttonPropTemplate.sprite_key = "confirm_reward";
    buttonPropTemplate.rect.position = { 0.36f, 0.65f };
    buttonPropTemplate.rect.position.y = 15.0f;
    buttonPropTemplate.rect.size = { 0.3, 0.2f };
    create_mythic_selected_button(buttonPropTemplate);

    //next round button
    buttonPropTemplate.ID = ecs::grp::UI;
    buttonPropTemplate.sprite_key = "next"; 
    buttonPropTemplate.rect.position.y = 10.0f;
    create_next_round_button(buttonPropTemplate); 
}

void MythicScene::create_reward_mythic_button(const GameStructs::ButtonProperties& bp)
{
    auto* mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    auto buttonComp = mngr->getComponent<Button>(e);

    //used for change the sprite once a button is clicked
    auto imgComp = mngr->addComponent<ImageForButton>(e,
        &sdlutils().images().at(bp.sprite_key),
        &sdlutils().images().at("reward_selected"),
        bp.rect,
        0,
        Game::Instance()->get_mngr()->getComponent<camera_component>(
            Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam
    );
    //used to get the sprite key for show a specific info
    mngr->addComponent<MythicDataComponent>(e, bp.sprite_key);
    auto ri = mngr->getComponent<transformless_dyn_image>(mngr->getHandler(ecs::hdlr::MYTHICINFO));

    buttonComp->connectClick([buttonComp, imgComp, this, e]() {
        if (_selected) {
            //std::cout << "already selected" << std::endl;
        }
        else {
            //std::cout << "left click -> reward card button" << std::endl;
            if (_lm == nullptr) {
                imgComp->resize(1.1f);
            }
            else if (_lm != nullptr && _lm != imgComp) {
                imgComp->resize(1.1f);
                _lm->resize(1.0f/1.1f);
            }
            _lm = imgComp;
            _chosen_mythic = e;
        }
        });
    buttonComp->connectHover([mngr, buttonComp, imgComp, ri, e, this]() {
        if (_selected) return;
        //std::cout << "hover -> Reward button: " << std::endl;
        //filter
        imgComp->_filter = true;
        auto& sp = mngr->getComponent<MythicDataComponent>(e)->sprite();
        ri->set_texture(&sdlutils().images().at(sp + "_info"));
        sdlutils().soundEffects().at("button_hover").play();
        });
    buttonComp->connectExit([buttonComp, imgComp, ri]() {
        //std::cout << "exit -> Reward button: " << std::endl;
        //filter
        imgComp->_filter = false;
        ri->set_texture(&sdlutils().images().at("initial_info"));
        });

}

void MythicScene::refresh_mythics() {
    auto* mngr = Game::Instance()->get_mngr();
    auto& mo = mngr->getEntities(ecs::grp::MYTHICOBJS);
    auto player = mngr->getHandler(ecs::hdlr::PLAYER);
    auto& pMythics = mngr->getComponent<MythicComponent>(player)->get_mythics();
    std::unordered_set<std::string> appeared_mythics; 
    //convert my mythic vector into an unordered set of string
    for (auto m : pMythics) {
        appeared_mythics.insert("mythic_"+m->get_name());
    }
    //refresh the three reward card button
    for (auto& e : mo) {
        auto s = get_unique_mythic(appeared_mythics);
        auto img = mngr->getComponent<transformless_dyn_image>(e);
        img->set_texture(&sdlutils().images().at(s.first));
        auto data = mngr->getComponent<MythicDataComponent>(e);
        data->set_data(s.first, s.second);
    }
}

void MythicScene::add_new_reward_mythic() {
    assert(_chosen_mythic != nullptr);

    //Once reward is picked, checks number of cards in deck
    auto* mngr = Game::Instance()->get_mngr();
    auto* player = mngr->getHandler(ecs::hdlr::PLAYER);
    auto _m_mythics = mngr->getComponent<MythicComponent>(player);
    //We add the cards to the deck
    MythicItem* m = nullptr;
    GameStructs::MythicType wt = mngr->getComponent<MythicDataComponent>(_chosen_mythic)->MT();
    switch (wt)
    {
    case GameStructs::BLOODCLAW: m = new BloodClaw();
        break;
    case GameStructs::DREAMRECICLER: m = new DreamRecicler();
        break;
    case GameStructs::CURTAINREAPER: m = new CurtainReaper();
        break;
    case GameStructs::INCENSE: m = new Incense();
        break;
    case GameStructs::QUICKDRAWDECK: m = new QuickDrawDeck();
        break;
    case GameStructs::BLOODPACT: m = new BloodPact();
        break;
    case GameStructs::DEMONICSCRATCHER:m = new DemonicScratcher();
        break;
    case GameStructs::CLAWFILE:m = new ClawFile();
        break;
    case GameStructs::URANIUMSOCKS:m = new UraniumSocks();
        break;
    case GameStructs::ZOOMIESINDUCER:m = new ZoomiesInducer();
        break;
    default:
        break;
    }
    if (m != nullptr) {
        _m_mythics->add_mythic(m);
        //Refresh my actual mythic representation
        refresh_my_mythics(_m_mythics->get_mythics());
    }
}

void MythicScene::create_mythic_selected_button(const GameStructs::ButtonProperties& bp) {
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
    mngr->setHandler(ecs::hdlr::CONFIRMMYTHIC, e);
    buttonComp->connectClick([buttonComp, this, imgComp, mngr] {
        if (_lm != nullptr && !_selected) {
            _lm->swap_textures();
            _selected = true;
            add_new_reward_mythic();
            imgComp->_filter = false;
            imgComp->swap_textures();
            auto imgGameScene = mngr->getComponent<ImageForButton>(mngr->getHandler(ecs::hdlr::NEXTROUNDMYTHIC));
            imgGameScene->destination_rect.position.y = imgComp->destination_rect.position.y-0.15f;
            imgComp->destination_rect.position.y = 100.0f;
        }
        });
    buttonComp->connectHover([buttonComp, imgComp, this]() {
        if (_selected) return;
        //std::cout << "hover -> Reward selected button: " << std::endl;
        //filter
        imgComp->swap_textures();
        imgComp->_filter = true;
        sdlutils().soundEffects().at("button_hover").play();
        });
    buttonComp->connectExit([buttonComp, imgComp, this]() {
        //std::cout << "exit -> Reward selected button: " << std::endl;
        //filter
        imgComp->_filter = false;
        imgComp->swap_textures();
        });
}

ecs::entity_t MythicScene::create_mythic_button(const GameStructs::MythicButtonProperties& bp) {
    auto* mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    auto b = mngr->addComponent<MythicButton>(e);
    b->initComponent();
    b->set_it(bp.iterator);
    return e;
}

void MythicScene::create_a_mythic(const GameStructs::MythicButtonProperties& bp) {
    auto* mngr = Game::Instance()->get_mngr();

    auto e = create_mythic_button(bp);
    auto buttonComp = mngr->getComponent<MythicButton>(e); 
    auto imgComp = mngr->getComponent<transformless_dyn_image>(e);
    auto ri = mngr->getComponent<transformless_dyn_image>(mngr->getHandler(ecs::hdlr::MYTHICINFO));

    buttonComp->connectClick([buttonComp, imgComp, this, bp] {
        if (_selected) return;
        imgComp->destination_rect.size = { imgComp->_original_w,  imgComp->_original_h };
        auto it = buttonComp->It();
        //only assign a valid iterator
        if (it != _selected_mythic) {

            _selected_mythic = it;
            _last_my_mythic_img = static_cast<ImageForButton*>(imgComp);
            //std::cout << "card selected: "<< std::endl;
        }
        });

    buttonComp->connectHover([buttonComp, imgComp, mngr, ri, e, this]() {
        //std::cout << "hover -> Reward button: " << std::endl;
        //filter
        if (_selected) return;
        imgComp->_filter = true;
        auto& sp = buttonComp->Name();
        if (sp != "") ri->set_texture(&sdlutils().images().at(sp + "_info"));
        /*imgComp->destination_rect.position.y -= 0.125f;*/
       /* imgComp->destination_rect.size = { imgComp->destination_rect.size.x * 1.25f,  imgComp->destination_rect.size.y * 1.25f };*/
        });
    buttonComp->connectExit([buttonComp, imgComp, mngr, ri, this]() {
        //std::cout << "exit -> Reward button: " << std::endl;
        /*imgComp->destination_rect.position.y += 0.125f;*/
        //filter
        if (_selected) return;
        imgComp->_filter = false;
        ri->set_texture(&sdlutils().images().at("initial_info"));
        //imgComp->destination_rect.size = { imgComp->destination_rect.size.x / 1.25f,  imgComp->destination_rect.size.y / 1.25f };
        });
}

void MythicScene::create_my_mythics() {
    auto* mngr = Game::Instance()->get_mngr();

    //GET PLAYERS MYTHIC VECTOR REFERENCE
    auto* player = mngr->getHandler(ecs::hdlr::PLAYER);
    if (player && !mngr->hasComponent<MythicComponent>(player)) {
        //when we add these entities, our olayer doesnt have any deck as component
        mngr->addComponent<MythicComponent>(player);
    }
    auto _m_mythic = mngr->getComponent<MythicComponent>(player);
    auto& vMyth = _m_mythic->get_mythics();

    float umbral = 0.095f;
    //when your starts a game, you wont have any mythic item 
    //thats why we create 10 empty mythic visual elements and then we refresh them
    GameStructs::MythicButtonProperties propTemplate = {
            { {0.01f, 0.8f}, {0.1f, 0.175f} },
            0.0f, "", ecs::grp::MYMYTHICOBJS, nullptr
    };
    for (int i = 0; i < 10; ++i) {
        propTemplate.sprite_key = "initial_info";
        create_a_mythic(propTemplate);
        propTemplate.rect.position.x += umbral;
    }

}

void MythicScene::refresh_my_mythics(const std::vector<MythicItem*>& cl) {
    auto* mngr = Game::Instance()->get_mngr();
    auto& infos = mngr->getEntities(ecs::grp::MYMYTHICOBJS);

    if (infos.size() == 0) return;
    auto itMythicInfo = infos.begin();

    //refresh my mythic info and represent it
    for (auto& c : cl) {
        //obtain each ones component
        auto img = mngr->getComponent<transformless_dyn_image>(*itMythicInfo);

        std::string typeName = c->get_name();

        //change to the newest texture
        img->set_texture(&sdlutils().images().at("mythic_"+typeName));
        //Refresh the pointer saved in the component
        auto buttonComp = mngr->getComponent<Button>(*itMythicInfo);
        if (buttonComp) {
            static_cast<MythicButton*>(buttonComp)->set_it(c);
        }
        ++itMythicInfo;
    }

    //refresh rest of the mythic infos (blank infos)
    for (; itMythicInfo != infos.end(); ++itMythicInfo) {
        auto img = mngr->getComponent<transformless_dyn_image>(*itMythicInfo);
        img->set_texture(&sdlutils().images().at("initial_info"));

        auto buttonComp = mngr->getComponent<Button>(*itMythicInfo);
        if (buttonComp) {
            static_cast<MythicButton*>(buttonComp)->set_it(nullptr); // Poner puntero a nullptr si no hay carta
        }
    }
}
void MythicScene::render()
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

void MythicScene::update(uint32_t delta_time) {
    Scene::update(delta_time);

    if (_activate_confirm_button && _lm != nullptr) {
        auto mngr = Game::Instance()->get_mngr();
        auto imgCompConfirm = mngr->getComponent<ImageForButton>(mngr->getHandler(ecs::hdlr::CONFIRMMYTHIC));
        imgCompConfirm->destination_rect.position.y = 0.65f;
        _activate_confirm_button = false;
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

void MythicScene::create_next_round_button(const GameStructs::ButtonProperties& bp) {
    auto* mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);
    mngr->setHandler(ecs::hdlr::NEXTROUNDMYTHIC, e);
    auto imgComp = mngr->addComponent<ImageForButton>(e,
        &sdlutils().images().at(bp.sprite_key),
        &sdlutils().images().at(bp.sprite_key+"_selected"),
        bp.rect,
        0,
        Game::Instance()->get_mngr()->getComponent<camera_component>(
            Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam
    );
    auto buttonComp = mngr->getComponent<Button>(e);

    buttonComp->connectClick([buttonComp, mngr, imgComp, this]() { if (_selected) {
        _lm->swap_textures();
        
        if (!Game::Instance()->is_network_none()) {
            network_context& network = Game::Instance()->get_network();
            Game::network_users_state& state = Game::Instance()->get_network_state();
            uint32_t id = state.connections.local_user_index;
            if (Game::Instance()->is_host()) {
                state.game_state.ready_users.set(0, true);
                state.game_state.ready_user_count++;
                if (checkAllPlayersReady())Game::Instance()->queue_scene(Game::GAMESCENE);
            }
            else if (!state.game_state.ready_users.test(id)) {
                state.game_state.ready_users.set(id, true);
                uint32_t id = Game::Instance()->client_id();
                network_message_pack_send(
                    network.profile.client.socket_to_host,
                    network_message_pack_create(network_message_type_player_ready,
                        create_player_id_message(id))
                );
            }
            show_message("Esperando a los otros michis...", 5 * 1000);
        }
        else {
            Game::Instance()->queue_scene(Game::GAMESCENE);
        }
        imgComp->destination_rect.position.y = 2.0f;
        imgComp->_filter = false;
        imgComp->swap_textures();
    }});
    buttonComp->connectHover([buttonComp, imgComp, this]() { 
        imgComp->swap_textures();
        imgComp->_filter = true;
        sdlutils().soundEffects().at("button_hover").play();
        });
    buttonComp->connectExit([buttonComp, imgComp, this]() { 
        imgComp->_filter = false;
        imgComp->swap_textures();});
}