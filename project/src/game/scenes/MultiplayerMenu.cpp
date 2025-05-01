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

#ifdef GENERATE_LOG
#include "../../our_scripts/log_writer_to_csv.hpp"
#include "MultiplayerMenu.h"
#endif

MultiplayerMenu::MultiplayerMenu() : Scene(ecs::scene::MULTIPLAYERMENUSCENE),
    _ipHost(""),
    _isClient(false),
    _ipInputActive(false),
    input_field_has_focus{false},
    ip_input_field{nullptr} {
  
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

    GameStructs::ButtonProperties ipB = {
        { {0.725f, 0.39f}, { 0.15f, 0.075f } },
            0.0f, ""
    };
    ipB.sprite_key = "floor";
    ip_input_field = Game::Instance()->get_mngr()->getComponent<ImageForButton>(create_ip_input_field(ipB));
    assert(ip_input_field != nullptr && "error: ip_input_field is null");

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

void MultiplayerMenu::update(uint32_t delta_time)
{
    Scene::update(delta_time);

    //If is Client, changes textInput
    if (_isClient) {
        _ipHost = "Hi";
    }

    if (input_field_has_focus) {
        bool regenerate_text_surface = false;
        auto &input = ih();

        for(const auto &event : input.get_last_events()) {
            if (event.type == SDL_TEXTINPUT && event.text.text[0] == '\n') {
                // Handle the Enter key press
                _ipInputActive = false;
                input_field_has_focus = false;
                SDL_StopTextInput();
            } else if (event.type == SDL_TEXTINPUT) {
                // Append text from the event to the input string
                _ipHost += event.text.text;
                regenerate_text_surface = true;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_BACKSPACE) {
                // Remove the last character from the input string
                if (!_ipHost.empty()) {
                    _ipHost.pop_back();
                    regenerate_text_surface = true;
                } else {
                    _ipHost = ""; // Reset to default if empty
                    regenerate_text_surface = true;
                }
            }
        }

        if (regenerate_text_surface) {
            // Update the text surface with the new input string
            Texture &selected = ip_input_field->get_selected_texture();
            selected = Texture{
                sdlutils().renderer(),
                _ipHost.empty() ? std::string{(char [network_utility_write_canonical_ip_buffer_size]){
                    "Ip..."
                }} : _ipHost,
                sdlutils().fonts().at("ARIAL16"),
                SDL_Color{0, 0, 0, 255},
                SDL_Color{255, 255, 255, 255},
            };
        }
    }
}

void MultiplayerMenu::render()
{
    Scene::render();

    // auto _cam = Game::Instance()->get_mngr()->getComponent<camera_component>(
    //     Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA));

    // //Updates text input 
    // //Adapted to screep
    // rect_f32 textInput = rect_f32_screen_rect_from_viewport(rect_f32{position2_f32{ 0.725f, 0.39f }, size2_f32{ 0.15f, 0.075f }}, _cam->cam.screen);
    // //The real field
    // SDL_Rect textField{
    //     int(textInput.position.x),
    //     int(textInput.position.y),
    //     int(textInput.size.x),
    //     int(textInput.size.y)
    // };
    // //The text
    // Texture textFieldText{
    //     sdlutils().renderer(),
    //     _ipHost,
    //     sdlutils().fonts().at("ARIAL16"),
    //     SDL_Color{0, 0, 0, 255},
    //     SDL_Color{255, 255, 255, 255},
    // };
    // //Renders text
    // textFieldText.render(textField);

}

ecs::entity_t MultiplayerMenu::create_ip_input_field(const GameStructs::ButtonProperties& bp) {
    auto* mngr = Game::Instance()->get_mngr();
    auto e = create_button(bp);

    auto imgComp = mngr->addComponent<ImageForButton>(e,
        new Texture{
            sdlutils().renderer(),
            std::string{(char [network_utility_write_canonical_ip_buffer_size]){
                "Ip..."
            }},
            sdlutils().fonts().at("ARIAL16"),
            SDL_Color{0, 0, 0, 255},
            SDL_Color{255, 255, 255, 255},
        },
        new Texture{
            sdlutils().renderer(),
            std::string{network_utility_write_canonical_ip_buffer_size, ' '},
            sdlutils().fonts().at("ARIAL16"),
            SDL_Color{0, 0, 0, 255},
            SDL_Color{255, 255, 255, 255},
        },
        bp.rect,
        0,
        Game::Instance()->get_mngr()->getComponent<camera_component>(
            Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam
    );

    auto buttonComp = mngr->getComponent<Button>(e);
    buttonComp->connectClick([buttonComp, imgComp, this]() {
        imgComp->_filter = false;
        imgComp->swap_textures();
        _ipInputActive = true;
        input_field_has_focus = true;
        SDL_StartTextInput();
    });

    // buttonComp->connectHover([buttonComp, imgComp]() {
    //     imgComp->_filter = true;
    //     imgComp->swap_textures();
    // });

    // buttonComp->connectExit([buttonComp, imgComp]() {
    //     imgComp->_filter = false;
    //     imgComp->swap_textures();
    // });
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

static std::string multiplayer_menu_get_ip(const uint16_t port) {
    const IPaddress ip{
        .host = INADDR_ANY,
        .port = port,
    };
    std::string ip_host{network_utility_write_canonical_ip_buffer_size, '\0'};
    network_utility_write_canonical_ip(
        network_utility_get_host_ip(
            network_utility_get_host_name(ip),
            port
        ),
        ip_host.data()
    );
    return ip_host;
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
        imgComp->_filter = false;
        imgComp->swap_textures();
        
        _isClient = false;
        _ipInputActive = false;

        std::cout << "You are the host.";
        //Activates the button regarding copy ip
        network_context &network = Game::Instance()->get_network();
        network = network_context_create_host(nullptr, Game::default_port);
        network_context_host_connect_alloc(network.profile.host);

        // TODO: allow start client acceptance loop
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
    buttonComp->connectClick([buttonComp, imgComp, this]() {
        imgComp->_filter = false;
        imgComp->swap_textures();

        if (!_isClient) {
            const IPaddress ip{
                .host = INADDR_ANY,
                .port = Game::default_port,
            };

            _ipHost = multiplayer_menu_get_ip(Game::default_port);
            SDL_SetClipboardText(_ipHost.c_str());
            std::cout << "Your ip is copied." << std::endl;
        }
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
    buttonComp->connectClick([buttonComp, imgComp, this]() {
        imgComp->_filter = false;
        imgComp->swap_textures();

        _isClient = true;
        _ipInputActive = true;

        std::cout << "Your are a client." << std::endl;
        //Activates the button regarding enter ip
        
        network_context &network = Game::Instance()->get_network();
        network = network_context_create_client(_ipHost.c_str(), Game::default_port);
        auto connection = network_context_client_connect_alloc(network.profile.client);
        if (connection & network_context_client_connect_status_connected) {
            std::cout << "Connected to host." << std::endl;
        } else if (connection & network_context_client_connect_status_rejected) {
            std::cout << "Connection rejected." << std::endl;
        } else if (connection & network_context_client_connect_status_error) {
            if (connection & network_context_client_connect_status_invalid) {
                std::cout << "Invalid connection." << std::endl;
            } else {
                std::cout << "Error connecting to host." << std::endl;
            }
        } else {
            std::cout << "Unknown error." << std::endl;
        }

        //TODO: intial message if any
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
        // but for testing it's commented out
        //multiplayer_menu_destroy_network_context(Game::Instance()->get_network());
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


