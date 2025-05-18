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
#include <limits>
#include <cassert>
#include <cstdlib>

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
    _lastChosenSkin(nullptr),
    _playButton(nullptr)
{}

MultiplayerMenu::~MultiplayerMenu()
{
}

void MultiplayerMenu::initScene()
{
    create_static_background(&sdlutils().images().at("multiplayer_background"));

    //Button back
    GameStructs::ButtonProperties backB = {
        { {0.035f, 0.025f},{0.10f, 0.1f} },
        0.0f, ""
    };
    backB.sprite_key = "back2";
    create_back_button(backB);

    //Button play
    //0.55f
    GameStructs::ButtonProperties playB = {
        { {100.55f, 0.65f}, { 0.25f, 0.2f } },
            0.0f, ""
    };
    playB.sprite_key = "enter_game";
    create_play_button(playB);

    // --- BUTTONS FOR SKINS ---
     //Button mimi
    GameStructs::ButtonProperties skinB = {
        { {0.04f, 0.4f}, { 0.15f, 0.25f } },
            0.0f, ""
    };
    skinB.sprite_key = "mimibutton";
    create_skin_button(skinB, "mimi");

    //Button piu
    skinB.rect.position.x = 0.21f;
    skinB.sprite_key = "piubutton";
    create_skin_button(skinB, "piu");


    // --- BUTTONS ABOUT MULTIPLAYER ---
    //Button host
    GameStructs::ButtonProperties hostB = {
        { {0.45f, 0.05f}, { 0.20f, 0.2f } },
            0.0f, ""
    };
    hostB.sprite_key = "host";
    create_host_button(hostB);

    GameStructs::ButtonProperties edit_ip_button_descriptor{
        rect_f32{
            position2_f32{0.645f + 0.15f + 0.05f, 0.315f},
            size2_f32{ 0.075f, 0.12f }
        },
        0.0f,
        std::string{"edit_ip"},
        ecs::grp::DEFAULT
    };
    create_edit_ip_button(edit_ip_button_descriptor);
    
    //Button copy ip
    GameStructs::ButtonProperties copyB = {
        { {0.65f, 0.05f}, { 0.20f, 0.2f } },
            0.0f, ""
    };
    copyB.sprite_key = "copyip";
    create_copy_ip_button(copyB);

    //Button client
    GameStructs::ButtonProperties clientB = {
        { {0.45f, 0.275f}, { 0.20f, 0.2f } },
            0.0f, ""
    };
    clientB.sprite_key = "client";
    create_client_button(clientB);
}

void MultiplayerMenu::enterScene()
{
    Game::Instance()->get_mngr()->change_ent_scene(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA), ecs::scene::MULTIPLAYERMENUSCENE);
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log();
    log_writer_to_csv::Instance()->add_new_log("ENTERED MULTIPLAYER MENU SCENE");
#endif
}

void MultiplayerMenu::exitScene()
{
    if (!Game::Instance()->is_network_none()) {
        Game::network_users_state& state = Game::Instance()->get_network_state();
        state.game_state.ready_users.reset();
        state.game_state.ready_user_count = 0;
    }
#ifdef GENERATE_LOG
    log_writer_to_csv::Instance()->add_new_log("EXIT MULTIPLAYER MENU SCENE");
    log_writer_to_csv::Instance()->add_new_log();
#endif
    if (_lastChosenSkin != nullptr) {
        _lastChosenSkin->_filter = false;
        _lastChosenSkin->swap_textures();
        _lastChosenSkin->_filter = false;
        _lastChosenSkin = nullptr;
    }
}

static void multiplayer_menu_host_loop(network_context &ctx) {
    if (ctx.profile_status != network_context_profile_status::network_context_profile_status_host) {
        assert(false && "fatal error: host loop called when not in host mode");
        std::exit(EXIT_FAILURE);
    }

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
                case network_message_type::network_message_type_new_connection_sync_request: {
                    auto message = network_message_dynamic_pack_into<network_message_payload_new_connection_sync_request>(std::move(dyn_message));
                    const auto &payload = message->payload.content;
                    Game::network_users_state &state = Game::Instance()->get_network_state();
                    
                    const uint8_t sprite_key_length{payload.sprite_key.sprite_key_length};
                    assert(
                        0 < sprite_key_length
                        && "error: sprite key length must be greater than 0"
                    );
                    assert(
                        sprite_key_length < network_user_sprite_key_maximum_buffer_size
                        && "error: sprite key length must be less than the maximum buffer size"
                    );
                    const std::string_view sprite_key{
                        payload.sprite_key.sprite_key.data(),
                        sprite_key_length
                    };
                    std::cout << "message: new connection sync request. Sprite key: " << sprite_key << ". With length: " << uint16_t(sprite_key_length) << std::endl;
                    const size_t network_user_index{network_state_next_user_index(state)}; 
                    const ecs::entity_t player_entity{GameScene::create_dumb_player(ecs::scene::GAMESCENE, network_user_index, std::string{sprite_key})};

                    const size_t assigned_user_index{network_state_add_client_user(ctx, state, player_entity, std::string{sprite_key})};
                    if (assigned_user_index > (std::numeric_limits<uint8_t>::max)()) {
                        assert(false && "fatal error: assigned user index is greater than uint8_t max");
                        std::exit(EXIT_FAILURE);
                    }
                    if (assigned_user_index != network_user_index) {
                        assert(false && "fatal error: assigned user index is not equal to the network user index");
                        std::exit(EXIT_FAILURE);
                    }
                    
                    std::vector<std::string_view> sprite_keys;
                    sprite_keys.reserve(state.connections.connected_users);
                    for (size_t i = 0; i < state.connections.connected_users; ++i) {
                        sprite_keys.emplace_back(state.game_state.users_sprite_keys.at(i));
                    }
                    const auto response_payload{
                        network_message_payload_new_connection_sync_response_create<network_context_maximum_connections>(
                            network_connections{
                                .connected_users = state.connections.connected_users,
                                .local_user_index = uint8_t(assigned_user_index),
                                .oldest_non_host_index = state.connections.oldest_non_host_index
                            },
                            sprite_keys
                        )
                    };
                    const auto response = network_message_pack_create(
                        network_message_type_new_connection_sync_response,
                        response_payload
                    );
                    for (network_connection_size i = 0; i < ctx.profile.host.sockets_to_clients.connection_count; ++i) {
                        TCPsocket &client = ctx.profile.host.sockets_to_clients.connections[i];
                        network_message_pack_send(client, response);
                    }
                    break;
                }
                case network_message_type::network_message_type_new_connection_sync_response: {
                    assert(false && "error: new connection sync response should not be received by the host");
                    std::exit(EXIT_FAILURE);
                    break;
                }
                case network_message_type::network_message_type_skin_selection_request: {
                    auto message = network_message_dynamic_pack_into<network_payload_skin_selection_request>(std::move(dyn_message));
                    const auto &payload = message->payload.content;
                    const uint8_t requester_id{payload.requester_id};
                    const std::string_view sprite_key{
                        payload.sprite_key.sprite_key.data(),
                        payload.sprite_key.sprite_key_length
                    };
                    Game::Instance()->get_network_state().game_state.users_sprite_keys.at(requester_id) = std::string{sprite_key};
                    for (network_connection_size i = 0; i < ctx.profile.host.sockets_to_clients.connection_count; ++i) {
                        TCPsocket &client = ctx.profile.host.sockets_to_clients.connections[i];
                        if (client == connection) {
                            continue;
                        }
                        const auto response = network_message_pack_create(
                            network_message_type_skin_selection_response,
                            message->payload
                        );
                        network_message_pack_send(client, response);
                    }
                    break;
                }
                case network_message_type::network_message_type_skin_selection_response: {
                    assert(false && "error: skin selection response should not be received by the host");
                    std::exit(EXIT_FAILURE);
                    break;
                }
                case network_message_type::network_message_type_player_ready: {
                    auto message = network_message_dynamic_pack_into<network_message_player_id>(std::move(dyn_message));
                    const auto& payload = message->payload.content;
                    Game::network_users_state& state = Game::Instance()->get_network_state();

                    uint32_t player_id = SDLNet_Read32(&payload.id_n);
                    state.game_state.ready_users.set(player_id, true);
                    break;
                }
                default: {
                    assert(false && "unreachable: invalid network message type");
                    std::exit(EXIT_FAILURE);
                    break;
                }
                }
                // TODO: listen to custom messages
            }
        }
    }
}

static void multiplayer_menu_client_loop(network_context& ctx) {
    int active_sockets = SDLNet_CheckSockets(ctx.profile.client.client_set, 0);
    if (active_sockets > 0 && SDLNet_SocketReady(ctx.profile.client.socket_to_host)) {

        auto msg = network_message_dynamic_pack_receive(ctx.profile.client.socket_to_host);
        const uint16_t type_n{ msg->header.type_n };
        const uint16_t type_h{ SDLNet_Read16(&type_n) };
        switch (type_h) {
        case network_message_type::network_message_type_new_connection_sync_request: {
            assert(false && "error: new connection sync request should not be received by the client");
            std::exit(EXIT_FAILURE);
            break;
        }
        case network_message_type::network_message_type_new_connection_sync_response: {
            auto message = network_message_dynamic_pack_into<network_message_payload_new_connection_sync_response<network_context_maximum_connections>>(std::move(msg));
            const auto &payload = message->payload.content;
            Game::network_users_state &state = Game::Instance()->get_network_state();
            const ecs::entity_t own_player_entity{Game::Instance()->get_mngr()->getHandler(ecs::hdlr::PLAYER)};

            if (state.connections.local_user_index < payload.connections.local_user_index) {
                // update
                assert(
                    state.connections.oldest_non_host_index == payload.connections.oldest_non_host_index
                    && "error: oldest non host index must be equal to the payload on a new connection sync response"
                );
            } else if (state.connections.local_user_index == network_context_maximum_connections){
                // new
                ecs::entity_t &local_player_slot = state.game_state.user_players.at(payload.connections.local_user_index);
                assert(local_player_slot == nullptr && "error: local player slot must be empty before assigning");
                
                // we assign it bellow
                // local_player_slot = own_player_entity;
                state.connections = network_connections{
                    .connected_users = 0,
                    .local_user_index = payload.connections.local_user_index,
                    .oldest_non_host_index = payload.connections.oldest_non_host_index
                };
            } else {
                assert(false && "unreachable: local user index may be uninitialized (set to maximum connections) or set to a value less than the number of connected users");
                std::exit(EXIT_FAILURE);
            }

            for (size_t i = state.connections.connected_users; i < payload.connections.connected_users; ++i) {
                const auto sprite_key_length{payload.sprite_keys[i].sprite_key_length};
                const std::string_view sprite_key{
                    payload.sprite_keys[i].sprite_key.data(),
                    sprite_key_length
                };
                const size_t network_user_index{network_state_next_user_index(state)};

                ecs::entity_t player_entity;
                if (i != state.connections.local_user_index) {  
                    player_entity = GameScene::create_dumb_player(ecs::scene::GAMESCENE, network_user_index, std::string{sprite_key});   
                } else {
                    player_entity = own_player_entity;
                }
                const size_t assigned_user_index{network_state_add_user(state, player_entity, std::string{sprite_key})};
                if (assigned_user_index > (std::numeric_limits<uint8_t>::max)()) {
                    assert(false && "fatal error: assigned user index is greater than uint8_t max");
                    std::exit(EXIT_FAILURE);
                }
                if (assigned_user_index != network_user_index) {
                    assert(false && "fatal error: assigned user index is not equal to the network user index");
                    std::exit(EXIT_FAILURE);
                }
            }
            if (state.connections.connected_users != payload.connections.connected_users) {
                assert(false && "fatal error: connected users count must be equal to the payload");
                std::exit(EXIT_FAILURE);
            }
            break;
        }
        case network_message_type::network_message_type_skin_selection_request: {
            assert(false && "error: skin selection request should not be received by the client");
            std::exit(EXIT_FAILURE);
            break;
        }
        case network_message_type::network_message_type_skin_selection_response: {
            auto message = network_message_dynamic_pack_into<network_payload_skin_selection_request>(std::move(msg));
            const auto &payload = message->payload.content;
            const uint8_t requester_id{payload.requester_id};
            const std::string_view sprite_key{
                payload.sprite_key.sprite_key.data(),
                payload.sprite_key.sprite_key_length
            };
            Game::Instance()->get_network_state().game_state.users_sprite_keys.at(requester_id) = std::string{sprite_key};
            break;
        } 
        case network_message_type::network_message_type_player_ready: {
            auto message = network_message_dynamic_pack_into<network_message_player_id>(std::move(msg));
            const auto& payload = message->payload.content;
            Game::network_users_state& state = Game::Instance()->get_network_state();
            uint32_t id = SDLNet_Read32(&payload.id_n);

            state.game_state.ready_users.set(id, true);
            bool local_player_ready = state.game_state.ready_users.test(state.connections.local_user_index);
            if (id == 0 && local_player_ready) {
                Game::Instance()->queue_scene(Game::SELECTIONMENU);
            }
            break;
        }
        default:
        {
            assert(false && "unreachable: invalid network message type");
            std::exit(EXIT_FAILURE);
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
                sdlutils().fonts().at("RUBIK_MONO_12"),
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
            position2_f32{ 0.65f, 0.34f },
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
        rect_f32 num_player_rect = rect_f32_screen_rect_from_viewport(rect_f32({ { 0.475,0.45 }, { 0.425,0.085 } }), camera->cam.screen);
        SDL_Rect num_player_true{
            int(num_player_rect.position.x),
            int(num_player_rect.position.y),
            int(num_player_rect.size.x),
            int(num_player_rect.size.y)
        };
        int num_players = Game::Instance()->get_network_state().connections.connected_users;
        Texture num_player_tex{
        sdlutils().renderer(),
        "Jugadores conectados: " + std::to_string(num_players),
        sdlutils().fonts().at("RUBIK_MONO"),
        SDL_Color({104, 64, 38, 255}) };
        num_player_tex.render(num_player_true);

        //mostar mensaje cuando sea necesario
        if (showing_message) {
            rect_f32 message_rect = rect_f32_screen_rect_from_viewport(rect_f32({ { 0.45,0.55 }, { 0.5,0.1 } }), camera->cam.screen);
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
    _playButton = imgComp;
    auto buttonComp = mngr->getComponent<Button>(e);
    buttonComp->connectClick([buttonComp, imgComp, mngr, this]() {
        if (Game::Instance()->is_network_none()) {
            return;
        }

        auto& network = Game::Instance()->get_network();
        auto& network_state = Game::Instance()->get_network_state();
        uint8_t id = network_state.connections.local_user_index;


        if (Game::Instance()->is_host()) {
            if (network_state.connections.connected_users > 1) {

                network_state.game_state.ready_users.set(id, true);
                network_state.game_state.ready_user_count++;

                //mandar a todos los clientes el mensaje 
                for (network_connection_size i = 0; i < network.profile.host.sockets_to_clients.connection_count; ++i) {
                    TCPsocket& client = network.profile.host.sockets_to_clients.connections[i];
                    network_message_pack_send(
                        client,
                        network_message_pack_create(
                            network_message_type_player_ready,
                            create_player_id_message(0)));
                }
                Game::Instance()->queue_scene(Game::SELECTIONMENU);
            }
            else {
                show_message("Espera a que lleguen los otros michis!", 5 * 1000);
            }
        }
        else {
        
            if (network_state.game_state.ready_users.test(0)) {
                Game::Instance()->queue_scene(Game::SELECTIONMENU);
            }
            else {

                if (!network_state.game_state.ready_users.test(id)) {

                    network_state.game_state.ready_users.set(id, true);
                    network_state.game_state.ready_user_count++;

                    network_message_pack_send(
                        network.profile.client.socket_to_host,
                        network_message_pack_create(
                            network_message_type_player_ready,
                            create_player_id_message(id)
                        )
                    );
                }
                else show_message("Esperando instrucciones del michi operador...", 5 * 1000);
                
            }   
        }
        
        imgComp->_filter = false;
        imgComp->swap_textures();
    });

    buttonComp->connectHover([buttonComp, imgComp]() {
        if (Game::Instance()->is_network_none()) {
            return;
        }
        imgComp->_filter = true;
        imgComp->swap_textures();
        sdlutils().soundEffects().at("button_hover").play();
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

        auto player = Game::Instance()->get_mngr()->getHandler(ecs::hdlr::PLAYER);
        
        _playButton->destination_rect.position.x = 0.55f;

        Game &game = *Game::Instance();
        const size_t host_user_index = network_state_add_host_user_as_self(
            network,
            game.get_network_state(),
            player,
            std::string{game.get_mngr()->getComponent<dyn_image_with_frames>(player)->texture_name}
        );
        assert(
            host_user_index == 0
            && "error: host user index should be 0 as it is the first user to be added"
        );
    });

    buttonComp->connectHover([buttonComp, imgComp]() {
        if (!Game::Instance()->is_network_none()) {
            return;
        }
        imgComp->_filter = true;
        imgComp->swap_textures();
        sdlutils().soundEffects().at("button_hover").play();
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
        sdlutils().soundEffects().at("button_hover").play();
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
                network_message_type::network_message_type_dbg_print,
                network_message_payload_dbg_print_create<32>(
                    "Hello from client!"
                )
            )
        );

        auto player = mngr->getHandler(ecs::hdlr::PLAYER);
        const std::string_view sprite_key = mngr->getComponent<dyn_image_with_frames>(player)->texture_name;
        std::cout << "Player solicita conexion, con textura " << sprite_key << std::endl;

        network_message_pack_send(
            network.profile.client.socket_to_host,
            network_message_pack_create(
                network_message_type_new_connection_sync_request,
                network_message_payload_new_connection_sync_create(sprite_key)
            )
        );
        _playButton->destination_rect.position.x = 0.55f;
    });

    buttonComp->connectHover([buttonComp, imgComp]() {
        if (!Game::Instance()->is_network_none()) {
            return;
        }
        imgComp->_filter = true;
        imgComp->swap_textures();
        sdlutils().soundEffects().at("button_hover").play();
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

        Game::Instance()->queue_scene(Game::MAINMENU);

        // TODO: if they back out they should really disconnect
        multiplayer_menu_destroy_network_context(Game::Instance()->get_network());
    });

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

/// @brief 
/// @param bp 
/// @param tex_name is very very dangerous as it is, thankfully we are passing statically stored string literals but should we pass a scoped std::string and after that many components would be pointing to a deallocated string
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

    const std::string captured_texture_name = std::string{tex_name};
    buttonComp->connectClick([buttonComp, imgComp, mngr, captured_texture_name,this]() {
        imgComp->_filter = false;
        if (_lastChosenSkin != imgComp) {
            if (_lastChosenSkin != nullptr) {
                _lastChosenSkin->swap_textures();
            }
            _lastChosenSkin = imgComp;
            imgComp->swap_textures();
        }

        GameScene::change_player_tex(captured_texture_name);
        //TODO
        std::cout << "You choosed your skin." << std::endl;
        //Sends it to players
        // TODO: send message probably to all players
        Game &game = *Game::Instance();
        network_context &network = game.get_network();
        if (game.is_host()) {
            game.get_network_state().game_state.users_sprite_keys.at(game.client_id()) = std::string{captured_texture_name};
            const network_payload_skin_selection_response payload{
                network_payload_skin_selection_create(
                    game.client_id(),
                    captured_texture_name
                )
            };
            const auto message = network_message_pack_create(
                network_message_type_skin_selection_response,
                payload
            );
            for (network_connection_size i = 0; i < network.profile.host.sockets_to_clients.connection_count; ++i) {
                TCPsocket &client = network.profile.host.sockets_to_clients.connections[i];
                network_message_pack_send(
                    client,
                    message
                );
            }
        } else if (game.is_client()) {
            game.get_network_state().game_state.users_sprite_keys.at(game.client_id()) = std::string{captured_texture_name};
            const network_payload_skin_selection_request payload{
                network_payload_skin_selection_create(
                    game.client_id(),
                    captured_texture_name
                )
            };
            const auto message = network_message_pack_create(
                network_message_type_skin_selection_request,
                payload
            );
            network_message_pack_send(
                network.profile.client.socket_to_host,
                message
            );
        }
    });

    buttonComp->connectHover([buttonComp, imgComp]() {
        imgComp->_filter = true;
        sdlutils().soundEffects().at("button_hover").play();
    });

    buttonComp->connectExit([buttonComp, imgComp]() {
        imgComp->_filter = false;
    });
}



