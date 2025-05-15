#include "MultiplayerMenu.h"
#include "../../our_scripts/components/ui/Button.h"
#include "../../our_scripts/components/rendering/transformless_dyn_image.h"
#include "../../our_scripts/components/rendering/ImageForButton.h"
#include "../../our_scripts/components/rendering/dyn_image_with_frames.hpp"

#include "../GameStructs.h"
#include "GameScene.h"
#include "../../utils/Vector2D.h"
#include "../../sdlutils/SDLUtils.h"
#include "../../sdlutils/InputHandler.h"
#include "../../ecs/Entity.h"
#include "../../sdlutils/Texture.h"
#include "../../network/network_utility.hpp"
#include "../../network/network_message.hpp"
#include <fstream>

#ifdef GENERATE_LOG
#include "../../our_scripts/log_writer_to_csv.hpp"
#include "MultiplayerMenu.h"
#endif

MultiplayerMenu::MultiplayerMenu() : Scene(ecs::scene::MULTIPLAYERMENUSCENE),
    ip_input{
        sdlutils().renderer(),
        std::string{"Ip..."},
        sdlutils().fonts().at("ARIAL16"),
        SDL_Color{ 0, 16, 24, 255 },
        SDL_Color{ 255, 255, 255, 255 }
    }, 
    _ipHost{""},
    input_field_has_focus{false},
    host_has_pressed_play(false)
{}

MultiplayerMenu::~MultiplayerMenu()
{
}

void MultiplayerMenu::initScene()
{
    create_static_background(&sdlutils().images().at("multiplayer_background"));

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
    create_skin_button(skinMimiB, "mimi");

    //Button piu
    GameStructs::ButtonProperties skinPiuB = {
        { {0.2f, 0.5f}, { 0.15f, 0.25f } },
            0.0f, ""
    };
    skinPiuB.sprite_key = "piubutton";
    create_skin_button(skinPiuB, "piu");


    // --- BUTTONS ABOUT MULTIPLAYER ---
    //Button host
    GameStructs::ButtonProperties hostB = {
        { {0.5f, 0.15f}, { 0.20f, 0.15f } },
            0.0f, ""
    };
    hostB.sprite_key = "host";
    create_host_button(hostB);

    GameStructs::ButtonProperties edit_ip_button_descriptor{
        rect_f32{
            position2_f32{0.7f + 0.15f + 0.05f, 0.39f},
            size2_f32{ 0.075f, 0.075f }
        },
        0.0f,
        std::string{"edit_ip"},
        ecs::grp::DEFAULT
    };
    create_edit_ip_button(edit_ip_button_descriptor);
    
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

void MultiplayerMenu::enterScene()
{
    Game::Instance()->get_mngr()->change_ent_scene(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA), ecs::scene::MAINMENUSCENE);
    host_has_pressed_play = false;
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

void MultiplayerMenu:: multiplayer_menu_host_loop(network_context &ctx) {
    if (SDLNet_CheckSockets(ctx.profile.host.clients_host_set, 0) > 0) {
        if (SDLNet_SocketReady(ctx.profile.host.host_socket)) {
            network_connection_size connection_index;
            network_context_host_accept_connection_status_flags status = network_context_host_accept_connection(
                ctx.profile.host,
                connection_index
            );
            if (status & network_context_host_accept_connection_status_accepted) {
                std::cout << "message: accepted connection. Connection index: " << connection_index << std::endl;
            } else if (status & network_context_host_accept_connection_status_rejected) {
                std::cout << "warning: rejected connection" << std::endl;
            }
            
            if (status & network_context_host_accept_connection_status_full) {
                std::cout << "warning: connection full" << std::endl;
            } else if (status & network_context_host_accept_connection_status_error) {
                std::cerr << "error: error accepting connection" << std::endl;
            }
        }
        for (network_connection_size i = 0; i < ctx.profile.host.sockets_to_clients.connection_count; ++i) {
            TCPsocket &connection = ctx.profile.host.sockets_to_clients.connections[i];
            if (SDLNet_SocketReady(connection)) {
                network_message_dynamic_pack dyn_message = network_message_dynamic_pack_receive(connection);
                const uint16_t type_n{dyn_message->header.type_n};
                const uint16_t type_h{SDLNet_Read16(&type_n)};
                switch (type_h) {
                case network_message_type::network_message_type_dbg_print: {
                    // std::unique_ptr<network_message_pack<network_message_payload_dbg_print<64>>, typename network_message_pack<network_message_payload_dbg_print<64>>::deleter> m;
                    auto message =
                        network_message_dynamic_pack_into<network_message_payload_dbg_print<64>>(std::move(dyn_message));
                    auto&& payload = message->payload.content;

                    const uint32_t args_size_h{ SDLNet_Read32(&payload.args_size_n) };
                    assert(
                        args_size_h < sizeof(payload.args)
                        && "error: payload size must be less than the size of the payload"
                        "There is no space for the null terminator"
                    );
                    payload.args[args_size_h] = '\0';
                    std::cout << "message: " << payload.args.data() << std::endl;
                    break;
                }
                case network_message_type_player_connect: {
                    auto message = network_message_dynamic_pack_into<network_message_player_connect>(std::move(dyn_message));
                    auto&& payload = message->payload.content;

                    static uint32_t next_id = 1;
                    uint32_t new_id = next_id++;

                    uint32_t  key_length = SDLNet_Read32(&payload.sprite_key_length);
                    std::string sprite_key(payload.sprite_key, key_length);

                    //crear un player local
                    GameScene::create_dumb_player(ecs::scene::GAMESCENE, new_id, sprite_key);

                    //asignarle al cliente nuevo un id
                    auto id_msg = network_message_pack_create(
                        network_message_type_client_id,
                        create_client_id_message(new_id)
                    );
                    network_message_pack_send(connection, id_msg);

                    //mandarle al cliente nuevo los clientes ya conectados
                    for (auto player : Game::Instance()->get_network_players()) {
                        uint32_t id = player.first;
                        if (id != new_id) {
                            ecs::entity_t player_entity = player.second;
                            auto* dyn_img = Game::Instance()->get_mngr()->getComponent<dyn_image_with_frames>(player_entity);
                           
                            if (dyn_img) {
                                std::string sprite_key = dyn_img->texture_name;
                                network_message_pack_send(
                                    connection,
                                    network_message_pack_create(network_message_type_new_player,
                                        create_player_connect_message(id, sprite_key)));
                            }
                            else std::cout << "Error: textura del player " << std::to_string(id) << std::endl;    
                        }
                    }

                    std::cout << "Player conectado. ID asignado: " << std::to_string(new_id) << ", sprite: " << payload.sprite_key << std::endl;

                    //avisar a otros clientes que se ha conectado un nuevo cliente
                    auto new_player_msg = create_player_connect_message(new_id, sprite_key);

                    for (network_connection_size i = 0; i < ctx.profile.host.sockets_to_clients.connection_count; ++i) {
                        TCPsocket& client = ctx.profile.host.sockets_to_clients.connections[i];

                        if (client != connection) {
                            network_message_pack_send(
                                client,
                                network_message_pack_create(network_message_type_new_player, new_player_msg));
                        }
                    }

                    break;
                }
                default: {
                    break;
                }
                }
                // TODO: listen to custom messages
            }
        }
    }
}

void MultiplayerMenu::multiplayer_menu_client_loop(network_context& ctx) {
    int active_sockets = SDLNet_CheckSockets(ctx.profile.client.client_set, 0);
    if (active_sockets > 0 && SDLNet_SocketReady(ctx.profile.client.socket_to_host)) {

        auto msg = network_message_dynamic_pack_receive(ctx.profile.client.socket_to_host);
        const uint16_t type_n{ msg->header.type_n };
        const uint16_t type_h{ SDLNet_Read16(&type_n) };
        switch (type_h) {
        case network_message_type::network_message_type_client_id: {
            auto message = network_message_dynamic_pack_into<network_message_client_id_from_host>(std::move(msg));
            auto&& payload = message->payload.content;

            uint32_t id = SDLNet_Read32(&payload.client_id);

            Game::Instance()->set_local_player_id(id);
            Game::Instance()->add_network_player(id, Game::Instance()->get_mngr()->getHandler(ecs::hdlr::PLAYER));
            std::cout << "player id:" << std::to_string(id) << std::endl;

            break;
        }
        case network_message_type::network_message_type_new_player: {
            auto message = network_message_dynamic_pack_into<network_message_player_connect>(std::move(msg));
            auto&& payload = message->payload.content;

            uint32_t id = SDLNet_Read32(&payload.player_id);
            std::cout << "nuevo player con id:" << std::to_string(id) << std::endl;

            uint32_t  key_length = SDLNet_Read32(&payload.sprite_key_length);
            std::string sprite_key(payload.sprite_key, key_length);

            GameScene::create_dumb_player(ecs::scene::GAMESCENE, id, sprite_key);

            break;
        }
        case network_message_type::network_message_type_host_has_pressed_play: {
            host_has_pressed_play = true;
            break;
        }
        default: {
            break;
        }
        }
    }
}

struct mulitplayer_menu_handle_text_input_result {
    bool lost_focus : 1;
    bool regenerate_text : 1;
    uint8_t unused : 6;
};
static mulitplayer_menu_handle_text_input_result mulitplayer_menu_handle_text_input(const InputHandler &input, std::string &ip_text) {
    mulitplayer_menu_handle_text_input_result result{
        .lost_focus = false,
        .regenerate_text = false,
        .unused = 0
    };
    for (const auto &event : input.get_last_events()) {
        if (event.type == SDL_TEXTINPUT) {
            ip_text += event.text.text;
            if (ip_text.size() > network_utility_write_canonical_ip_buffer_size - 1) {
                ip_text.resize(network_utility_write_canonical_ip_buffer_size - 1);
            }
            result.regenerate_text |= true;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.mod & KMOD_CTRL) {
                if (event.key.keysym.sym == SDLK_v) {
                    char *clipboard_text = SDL_GetClipboardText();
                    if (clipboard_text != nullptr) {
                        ip_text += clipboard_text;
                        SDL_free(clipboard_text);

                        if (ip_text.size() > network_utility_write_canonical_ip_buffer_size - 1) {
                            ip_text.resize(network_utility_write_canonical_ip_buffer_size - 1);
                        }
                        result.regenerate_text |= true;
                    }
                } else if (event.key.keysym.sym == SDLK_c) {
                    SDL_SetClipboardText(ip_text.c_str());
                }
            } else if (event.key.keysym.sym == SDLK_BACKSPACE) {
                if (!ip_text.empty()) {
                    ip_text.pop_back();
                }
                result.regenerate_text |= true;
            } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_ESCAPE) {
                result.lost_focus |= true;
            }
        }
    }
    return result;
}

void MultiplayerMenu::update(uint32_t delta_time) {
    Scene::update(delta_time);
    
    if (input_field_has_focus) {
        const auto &input = ih();
        const auto result = mulitplayer_menu_handle_text_input(input, _ipHost);

        if (result.regenerate_text) {
            ip_input = Texture{
                sdlutils().renderer(),
                _ipHost.empty() ? std::string{"Ip..."} : _ipHost,
                sdlutils().fonts().at("ARIAL16"),
                SDL_Color{0, 16, 24, 255},
                SDL_Color{255, 255, 255, 0},
            };
        }

        if (result.lost_focus) {
            input_field_has_focus = false;
            SDL_StopTextInput();
        }
    }

    if (showing_message) {
        if (sdlutils().currRealTime() > message_time) {
            showing_message = false;
        }
    }
    network_context &network = Game::Instance()->get_network();
    switch (network.profile_status) {
    case network_context_profile_status_none:
        break;
    case network_context_profile_status_host: {
        multiplayer_menu_host_loop(network);
        break;
    }
    case network_context_profile_status_client: {
        multiplayer_menu_client_loop(network);
        break;
    }
    default: {
        assert(false && "unreachable: invalid network profile status");
        std::exit(EXIT_FAILURE);
    }
    }
}

void MultiplayerMenu::render() {
    Scene::render();

    const auto &_cam = *Game::Instance()->get_mngr()->getComponent<camera_component>(
        Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA)
    );

    //Updates text input 
    //Adapted to screep
    const rect_f32 textInput = rect_f32_screen_rect_from_viewport(
        rect_f32{
            position2_f32{ 0.7f, 0.39f },
            size2_f32{ 0.15f + 0.05f, 0.075f }
        },
        _cam.cam.screen
    );
    //The real field
    const SDL_Rect textField{
        int(textInput.position.x),
        int(textInput.position.y),
        int(textInput.size.x),
        int(textInput.size.y)
    };
    auto &renderer = *sdlutils().renderer();
    SDL_SetRenderDrawColor(&renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(&renderer, &textField);
    
    const SDL_Rect occupied_text_field{
        textField.x + ((textField.w - ip_input.width()) >> 1),
        textField.y + ((textField.h - ip_input.height()) >> 1),
        ip_input.width(),
        ip_input.height()
    };
    ip_input.render(occupied_text_field);


    if (!Game::Instance()->is_network_none()) {
        //mostar el num de jugadores conectados
        auto camera = Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA));
        rect_f32 num_player_rect = rect_f32_screen_rect_from_viewport(rect_f32({ { 0.50,0.05 }, { 0.2,0.05 } }), camera->cam.screen);
        SDL_Rect num_player_true{
            int(num_player_rect.position.x),
            int(num_player_rect.position.y),
            int(num_player_rect.size.x),
            int(num_player_rect.size.y)
        };
        int num_players = Game::Instance()->get_network_players_num();
        Texture num_player_tex{
        sdlutils().renderer(),
        "Jugadores conectados: " + std::to_string(num_players),
        sdlutils().fonts().at("RUBIK_MONO"),
        SDL_Color({128, 0, 32, 255}) };
        num_player_tex.render(num_player_true);

        //mostar mensaje cuando sea necesario
        if (showing_message) {
            rect_f32 message_rect = rect_f32_screen_rect_from_viewport(rect_f32({ { 0.4,0.7 }, { 0.5,0.1 } }), camera->cam.screen);
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
    
}

ecs::entity_t MultiplayerMenu::create_edit_ip_button(const GameStructs::ButtonProperties& bp) {
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
    buttonComp->connectClick([buttonComp, imgComp, this]() {
        imgComp->_filter = false;
        imgComp->swap_textures();
        
        input_field_has_focus = true;
        SDL_StartTextInput();
    });

    buttonComp->connectHover([buttonComp, imgComp]() {
        imgComp->_filter = true;
        imgComp->swap_textures();
    });

    buttonComp->connectExit([buttonComp, imgComp]() {
        imgComp->_filter = false;
        imgComp->swap_textures();
    });
    return e;
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
    buttonComp->connectClick([buttonComp, imgComp, mngr, this]() {
        if (Game::Instance()->is_network_none()) {
            return;
        }
        if (Game::Instance()->is_host()) {
            if (Game::Instance()->get_network_players_num() > 1) {
                host_has_pressed_play = true;
                network_context& network = Game::Instance()->get_network();
                //mandar a todos los clientes el mensaje de que el host a dado al boton de play
                for (network_connection_size i = 0; i < network.profile.host.sockets_to_clients.connection_count; ++i) {
                    TCPsocket& client = network.profile.host.sockets_to_clients.connections[i];
                    network_message_pack_send(
                        client,
                        network_message_pack_create(network_message_type_host_has_pressed_play, create_payload_empty_message()));
                }
            }
            else {
                show_message("Espera a que lleguen los otros michis!");
            }

        }
        else if (Game::Instance()->is_client() && !host_has_pressed_play)show_message("Esperando instrucciones del michi operador... ");
        
        if (host_has_pressed_play) Game::Instance()->change_Scene(Game::SELECTIONMENU);
        
        imgComp->_filter = false;
        imgComp->swap_textures();
    });

    buttonComp->connectHover([buttonComp, imgComp]() {
        if (Game::Instance()->is_network_none()) {
            return;
        }
        imgComp->_filter = true;
        imgComp->swap_textures();
    });

    buttonComp->connectExit([buttonComp, imgComp]() {
        if (Game::Instance()->is_network_none()) {
            return;
        }
        imgComp->_filter = false;
        imgComp->swap_textures();
    });
}

[[maybe_unused]]
static std::string multiplayer_menu_get_ip(const uint16_t port) {
    const IPaddress ip{
        .host = INADDR_ANY,
        .port = port,
    };
    char ip_host[network_utility_write_canonical_ip_buffer_size] = {0};
    network_utility_write_canonical_ip(
        network_utility_get_host_local_ip(
            network_utility_get_host_name(ip),
            port
        ),
        ip_host
    );
    return std::string{ip_host};
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
    buttonComp->connectClick([buttonComp, imgComp,mngr, this]() {
        if (!Game::Instance()->is_network_none()) {
            return;
        }
        imgComp->_filter = false;
        imgComp->swap_textures();
        
        network_context &network = Game::Instance()->get_network();
        // std::string canonical_ip = multiplayer_menu_get_ip(Game::default_port);
        network = network_context_create_host(nullptr, Game::default_port);
        network_context_host_connect_alloc(network.profile.host);
        std::cout << "Host at port: " << Game::default_port << std::endl;

        Game::Instance()->set_local_player_id(0);
        auto player = Game::Instance()->get_mngr()->getHandler(ecs::hdlr::PLAYER);
        Game::Instance()->add_network_player(0, player);

    });

    buttonComp->connectHover([buttonComp, imgComp]() {
        if (!Game::Instance()->is_network_none()) {
            return;
        }
        imgComp->_filter = true;
        imgComp->swap_textures();
    });

    buttonComp->connectExit([buttonComp, imgComp]() {
        if (!Game::Instance()->is_network_none()) {
            return;
        }
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
    buttonComp->connectClick([buttonComp, imgComp, this]() {
        imgComp->_filter = false;
        imgComp->swap_textures();

        std::string canonical_public_ip{
            network_utility_get_host_canonical_public_ip(Game::default_port)
        };
        
        SDL_SetClipboardText(canonical_public_ip.c_str());
        std::cout << "Congratulations! Your IP: " << canonical_public_ip << " has been copied to the clipboard." << std::endl;
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


static void multiplayer_menu_destroy_network_context(network_context &ctx) {
	switch (ctx.profile_status) {
	case network_context_profile_status_none:
		break;
	case network_context_profile_status_host: {
		if (network_context_host_connected(ctx.profile.host)) {
			network_context_host_destroy(ctx.profile.host);
		} else {
			ctx.profile.host.ip_self.host = INADDR_NONE;
		}
		break;
	}
	case network_context_profile_status_client: {
		if (network_context_client_connected(ctx.profile.client)) {
			network_context_client_destroy(ctx.profile.client);
		} else {
			ctx.profile.client.ip_host.host = INADDR_NONE;
		}
		break;
	}
	default: {
		assert(false && "fatal error: invalid network context profile status");
		std::exit(EXIT_FAILURE);
	}
	}
	ctx.profile_status = network_context_profile_status_none;
}

void MultiplayerMenu::create_client_button(const GameStructs::ButtonProperties& bp) {
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
        if (!Game::Instance()->is_network_none()) {
            return;
        }

        imgComp->_filter = false;
        imgComp->swap_textures();

        char ip_buffer[network_utility_write_canonical_ip_buffer_size] = {0};
        if (_ipHost == "localhost") {
            std::copy(_ipHost.begin(), _ipHost.end(), ip_buffer);
        } else if (
            !network_utility_canonicalize_ip(_ipHost.c_str(), ip_buffer)
            || !network_context_client_can_resolve(ip_buffer, Game::default_port)
        ) {
            std::cerr << "warning: could not resolve host with ip: " << _ipHost << " at port: " << Game::default_port << std::endl;
            return;
        }
        
        network_context &network = Game::Instance()->get_network();
        network = network_context_create_client(ip_buffer, Game::default_port);

        auto connection = network_context_client_connect_alloc(network.profile.client);
        if (connection & network_context_client_connect_status_connected) {
            std::cout << "message: connected to host. Host public ip: " << ip_buffer
                << " Host port: " << Game::default_port << std::endl;
        } else if (connection & network_context_client_connect_status_error) {
            if (connection & network_context_client_connect_status_invalid) {
                std::cerr << "warning: invalid connection. Machine with public ip: " << ip_buffer
                    << " is not accepting connections at port: " << Game::default_port << std::endl;
            } else if (connection & network_context_client_connect_status_rejected) {
                std::cerr << "warning: connection rejected. A connection with machine (public): " << ip_buffer
                    << " was made but rejected the connection. Maybe the host application is full." << std::endl;
            } else {
                assert(false && "unreachable: invalid connection status");
                std::exit(EXIT_FAILURE);
            }
            std::cout << "error log: SDLNet error: " << SDLNet_GetError() << std::endl;
            multiplayer_menu_destroy_network_context(network);
        } else {
            assert(false && "unreachable: invalid connection status");
            std::exit(EXIT_FAILURE);
        }

        network_message_pack_send(
            network.profile.client.socket_to_host,
            network_message_pack_create(
                network_message_type::network_message_type_dbg_print_two_byte_test,
                network_message_payload_dbg_print_create<32>(
                    "Hello from client!"
                )
            )
        );

        auto player = mngr->getHandler(ecs::hdlr::PLAYER);
        auto name = mngr->getComponent<dyn_image_with_frames>(player)->texture_name;
        auto connect_msg = create_player_connect_message(0,name);

        std::cout << "Player solicita conexion, con textura " << name << std::endl;

        network_message_pack_send(
            network.profile.client.socket_to_host,
            network_message_pack_create(
                network_message_type_player_connect,
                connect_msg
            )
        );
    });

    buttonComp->connectHover([buttonComp, imgComp]() {
        if (!Game::Instance()->is_network_none()) {
            return;
        }
        imgComp->_filter = true;
        imgComp->swap_textures();
    });

    buttonComp->connectExit([buttonComp, imgComp]() {
        if (!Game::Instance()->is_network_none()) {
            return;
        }
        imgComp->_filter = false;
        imgComp->swap_textures();
    });
}


void MultiplayerMenu::create_back_button(const GameStructs::ButtonProperties& bp) {
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

        // TODO: if they back out they should really disconnect
        multiplayer_menu_destroy_network_context(Game::Instance()->get_network());
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

void MultiplayerMenu::create_skin_button(const GameStructs::ButtonProperties& bp, const std::string& tex_name) {   
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
    buttonComp->connectClick([buttonComp, imgComp, mngr, tex_name]() {
        imgComp->_filter = false;
        imgComp->swap_textures();

        GameScene::change_player_tex(tex_name);
        //TODO
        std::cout << "You choosed your skin.";
        //Sends it to players
        // TODO: send message probably to all players
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



