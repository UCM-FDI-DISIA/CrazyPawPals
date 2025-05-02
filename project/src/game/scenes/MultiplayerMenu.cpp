#include "MultiplayerMenu.h"
#include "../../our_scripts/components/ui/Button.h"
#include "../../our_scripts/components/rendering/transformless_dyn_image.h"
#include "../../our_scripts/components/rendering/ImageForButton.h"

#include "../GameStructs.h"
#include "GameScene.h"
#include "../../utils/Vector2D.h"
#include "../../sdlutils/SDLUtils.h"
#include "../../sdlutils/InputHandler.h"
#include "../../ecs/Entity.h"
#include "../../sdlutils/Texture.h"
#include "../../network/network_utility.hpp"
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
        SDL_Color{ 0, 0, 0, 255 },
        SDL_Color{ 255, 255, 255, 255 }
    }, 
    _ipHost{""},
    input_field_has_focus{false} {
  
}

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
            position2_f32{0.725f + 0.15f, 0.39f},
            size2_f32{ 0.15f, 0.075f }
        },
        0.0f,
        std::string{"confirm_reward"},
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

static void multiplayer_menu_host_loop(network_context &ctx) {
    if (SDLNet_CheckSockets(ctx.profile.host.clients_host_set, 0) > 0) {
        if (SDLNet_SocketReady(ctx.profile.host.host_socket)) {
            network_connection_size connection_index;
            network_context_host_accept_connection_status_flags status = network_context_host_accept_connection(
                ctx.profile.host,
                connection_index
            );
            if (status & network_context_host_accept_connection_status_accepted) {
                std::cout << "Accepted connection. Connection index: " << connection_index << std::endl;
            }
            else if (status & network_context_host_accept_connection_status_rejected) {
                std::cout << "Rejected connection" << std::endl;
            }
            
            if (status & network_context_host_accept_connection_status_full) {
                std::cout << "Connection full" << std::endl;
            }
            else if (status & network_context_host_accept_connection_status_error) {
                std::cerr << "Error accepting connection" << std::endl;
            }
        }
        for (network_connection_size i = 0; i < ctx.profile.host.sockets_to_clients.connection_count; ++i) {
            TCPsocket &connection = ctx.profile.host.sockets_to_clients.connections[i];
            if (SDLNet_SocketReady(connection)) {
                // TODO: listen to custom messages
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
                SDL_Color{0, 0, 0, 255},
                SDL_Color{255, 255, 255, 255},
            };
        }

        if (result.lost_focus) {
            input_field_has_focus = false;
            SDL_StopTextInput();
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
            position2_f32{ 0.725f, 0.39f },
            size2_f32{ 0.15f, 0.075f }
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
    ip_input.render(textField);
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
    buttonComp->connectClick([buttonComp, imgComp, mngr]() {
        if (!Game::Instance()->is_host()) {
            return;
        }
        imgComp->_filter = false;
        imgComp->swap_textures();
        Game::Instance()->change_Scene(Game::SELECTIONMENU);
    });

    buttonComp->connectHover([buttonComp, imgComp]() {
        if (!Game::Instance()->is_host()) {
            return;
        }
        imgComp->_filter = true;
        imgComp->swap_textures();
    });

    buttonComp->connectExit([buttonComp, imgComp]() {
        if (!Game::Instance()->is_host()) {
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
        network_utility_get_host_ip(
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
    buttonComp->connectClick([buttonComp, imgComp, this]() {
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

        std::system("curl \"https://api.ipify.org\" >czpp_ip.txt");
        std::string canonical_external_ip; {
            std::ifstream ip_file{"czpp_ip.txt"};
            canonical_external_ip = std::string{
                std::istreambuf_iterator<char>(ip_file),
                std::istreambuf_iterator<char>()
            };
        }
        std::remove("czpp_ip.txt");
        
        SDL_SetClipboardText(canonical_external_ip.c_str());
        std::cout << "Congratulations! Your IP: " << canonical_external_ip << " has been copied to the clipboard." << std::endl;
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
    buttonComp->connectClick([buttonComp, imgComp, this]() {
        if (!Game::Instance()->is_network_none()) {
            return;
        }

        imgComp->_filter = false;
        imgComp->swap_textures();

        char ip_buffer[network_utility_write_canonical_ip_buffer_size] = {0};
        if (
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

        //TODO: intial message if any
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

void MultiplayerMenu::handleIPInput() {
    auto& ihdlr = ih();
}


