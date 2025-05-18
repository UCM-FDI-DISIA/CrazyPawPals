
#include "Scene.h"
#include <cassert>

#include "../../our_scripts/components/movement/Transform.h"
#include "../../our_scripts/components/rendering/camera_component.hpp"

#include "../../our_scripts/components/rendering/transformless_dyn_image.h"
#include "../../our_scripts/components/rendering/dyn_image.hpp"
#include "../../our_scripts/components/rendering/RotationComponent.h"
#include "../../our_scripts/components/ui/Button.h"

#include <string>
Scene::Scene(ecs::sceneId_t id) : _scene_ID(id) {}

ecs::entity_t Scene::rendering::create_camera(
    const ecs::sceneId_t scene,
    const camera_creation_descriptor_flags flags,
    CZPP_NULLABLE const Transform *optional_follow_target)
{
    assert(
        (flags & camera_creation_descriptor_options_follow) == (optional_follow_target != nullptr) && "error: follow target must be provided if and only if the follow flag is set");

    auto &&manager = *Game::Instance()->get_mngr();
    auto camera = manager.addEntity(scene);
    if (flags & camera_creation_descriptor_options_set_handler)
    {
        manager.setHandler(ecs::hdlr::CAMERA, camera);
    }

    auto transform = manager.addComponent<Transform>(camera, Vector2D(0.0, 0.0), Vector2D(0.0, 0.0), 0.0, 0.0);
    assert(transform != nullptr && "error: failed to add camera transform");

    constexpr static const ::camera default_camera{
        .position = {0.0, 0.0},
        .half_size = {8.0, 4.5},
    };
    auto &&device = *manager.addComponent<camera_component>(camera, camera_screen{
                                                                        .camera = default_camera,
                                                                        .screen = {
                                                                            .pixel_size = {sdlutils().width(), sdlutils().height()},
                                                                        },
                                                                    });

    constexpr static const float default_lookahead_time = 1.0;
    constexpr static const float default_semi_reach_time = 2.5;
    if (flags & camera_creation_descriptor_options_follow)
    {
        auto follow = manager.addComponent<camera_follow>(camera, camera_follow_descriptor{.previous_position = device.cam.camera.position, .lookahead_time = default_lookahead_time, .semi_reach_time = default_semi_reach_time}, device, *optional_follow_target);
        assert(follow != nullptr && "error: failed to add camera follow");
    }

    if (flags & camera_creation_descriptor_options_clamp)
    {
        auto clamp = manager.addComponent<camera_clamp>(camera, camera_clamp_descriptor{.bounds = default_scene_bounds}, device);
        assert(clamp != nullptr && "error: failed to add camera clamp");
    }
    return camera;
}

ecs::entity_t
Scene::create_button(const GameStructs::ButtonProperties &bp)
{
    auto b = new Button();
    ecs::entity_t e = create_entity(
        bp.ID,
        _scene_ID,
        new transformless_dyn_image(bp.rect,
                                    0,
                                    Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam,
                                    &sdlutils().images().at(bp.sprite_key)),
        b);
    b->initComponent();
    return e;
}

ecs::entity_t Scene::create_decoration_image(const GameStructs::ButtonProperties & ip)
{
    Vector2D pos = {ip.rect.position.x, ip.rect.position.y};
    auto &&transform = *new Transform(pos, {0.0f,0.0f}, 0.0f, 0.0f);
    auto manager = Game::Instance()->get_mngr();
    auto e = create_entity(
        ip.ID,
        _scene_ID,
        &transform,
        new transformless_dyn_image(ip.rect,
            0,
            Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam,
            &sdlutils().images().at(ip.sprite_key)),
        new RotationComponent(RotationComponent::Mode::CONTINUOUS,
            0.5f,
            360.0f)
    );

    return e;
}

void Scene::create_static_background(Texture *bg)
{
    create_entity(ecs::grp::UI,
                  _scene_ID,
                  new transformless_dyn_image({{0.0f, 0.0f}, {1.0f, 1.0f}},
                                              0,
                                              Game::Instance()->get_mngr()->getComponent<camera_component>(Game::Instance()->get_mngr()->getHandler(ecs::hdlr::CAMERA))->cam,
                                              bg));
}

bool Scene::checkAllPlayersReady()
{
    Game::network_users_state& state = Game::Instance()->get_network_state();
    if (!Game::Instance()->is_host() || !state.game_state.ready_users.test(0)) return false;

    if (state.game_state.ready_users.count() == state.connections.connected_users) {
        network_context& network = Game::Instance()->get_network();
        // enviar start a todos los clientes
        for (auto& client : network.profile.host.sockets_to_clients.connections) {
            if (client) {
                network_message_pack_send(
                    client,
                    network_message_pack_create(network_message_type_start_game,
                        create_payload_empty_message())
                );
            }
        }
        return true;
    }
    else return false;
}

void Scene::update(uint32_t delta_time)
{
    Game::Instance()->get_mngr()->update(_scene_ID, delta_time);
}

void Scene::render()
{
    Game::Instance()->get_mngr()->render(_scene_ID);
}

void Scene::show_message(const std::string& text, uint32_t time)
{
    message = text;
    message_time = sdlutils().currRealTime() + time;
    showing_message = true;
}
