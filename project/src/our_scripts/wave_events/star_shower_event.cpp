
#include "star_shower_event.hpp"
#include <random>

#include "../../our_scripts/components/rendering/dyn_image.hpp"
#include "../../our_scripts/components/rendering/camera_component.hpp"
#include "../../our_scripts/components/rendering/rect_component.hpp"
#include "../../our_scripts/components/rendering/render_ordering.hpp"
#include "../../our_scripts/components/movement/Transform.h"
#include "../../our_scripts/components/Health.h"
#include "../../ecs/Entity.h"
#include "../../game/Game.h"
#include "../../game/scenes/Scene.h"
#include "../../ecs/Manager.h"

static star_drop_descriptor star_shower_event_generate_star_drop_descriptor(
    const star_drop_descriptor lower_bound,
    const star_drop_descriptor upper_bound,
    std::default_random_engine &generator
) {
    std::uniform_real_distribution<float> x_distribution{lower_bound.drop_position.x, upper_bound.drop_position.x};
    std::uniform_real_distribution<float> y_distribution{lower_bound.drop_position.y, upper_bound.drop_position.y};
    std::uniform_int_distribution<ptrdiff_t> damage_distribution{lower_bound.damage_amount, upper_bound.damage_amount};
    std::uniform_real_distribution<float> radius_distribution{lower_bound.drop_radius, upper_bound.drop_radius};
    std::uniform_real_distribution<float> fall_time_distribution{lower_bound.fall_time, upper_bound.fall_time};
    std::uniform_real_distribution<float> spawn_distance_distribution{lower_bound.spawn_distance, upper_bound.spawn_distance};

    return {
        {x_distribution(generator), y_distribution(generator)},
        damage_distribution(generator),
        radius_distribution(generator),
        fall_time_distribution(generator),
        spawn_distance_distribution(generator)
    };
}

static star_drop star_shower_event_create_star_drop(
    const star_drop_descriptor descriptor,
    ecs::Manager &manager,
    const camera_screen &camera,
    const ecs::sceneId_t scene_id
) {
    const ecs::entity_t mark_entity = manager.addEntity(scene_id, ecs::grp::STAR_DROP);
    const ecs::entity_t star_entity = manager.addEntity(scene_id, ecs::grp::STAR_DROP);

    const Vector2D shadow_position = Vector2D{descriptor.drop_position.x, descriptor.drop_position.y};
    const Vector2D spawn_position = shadow_position + Vector2D{0.0, descriptor.spawn_distance};

    const float drop_speed = descriptor.spawn_distance / descriptor.fall_time;
    Transform &star_transform = *manager.addComponent<Transform>(
        star_entity,
        spawn_position,
        Vector2D(0.0, 0.0),
        0.0,
        0.0
    );
    rect_component &star_rect = *manager.addComponent<rect_component>(
        star_entity,
        position2_f32{0.0f, 0.0f},
        size2_f32{descriptor.drop_radius * 2.0f, descriptor.drop_radius * 2.0f}
    );
    dyn_image &star_image = *manager.addComponent<dyn_image>(
        star_entity,
        rect_f32_full_subrect,
        star_rect,
        camera,
        sdlutils().images().at("meteroid"),
        star_transform
    );
    render_ordering &star_ordering = *manager.addComponent<render_ordering>(
        star_entity,
        64
    );
    (void)star_ordering;
    (void)star_image;

    Transform &shadow_transform = *manager.addComponent<Transform>(
        mark_entity,
        shadow_position,
        Vector2D(0.0, 0.0),
        0.0,
        0.0
    );
    rect_component &shadow_rect = *manager.addComponent<rect_component>(
        mark_entity,
        position2_f32{0.0f, 0.0f},
        size2_f32{descriptor.drop_radius * 2.0f, descriptor.drop_radius * 2.0f}
    );
    dyn_image &shadow_image = *manager.addComponent<dyn_image>(
        mark_entity,
        rect_f32_full_subrect,
        shadow_rect,
        camera,
        sdlutils().images().at("attack_warning"),
        shadow_transform
    );

    return star_drop{
        .mark_entity = mark_entity,
        .star_entity = star_entity,
        .star_transform = &star_transform,
        .shadow_image = &shadow_image,
        .shadow_rect = &shadow_rect,
        .damage_amount = descriptor.damage_amount,
        .remaining_fall_time = descriptor.fall_time,
        .radius = descriptor.drop_radius,
        .fall_speed = drop_speed
    };
}

void star_shower_event::start_wave_callback() {
    assert(star_drops.empty() && "fatal error: star_drops must be empty at the start of the wave");

    std::default_random_engine generator;
    std::uniform_int_distribution<size_t> distribution{min_drops_inclusive, max_drops_exclusive};
    const size_t drop_count = distribution(generator);

    std::vector<star_drop_descriptor> star_drop_descriptors{drop_count};
    star_drops.reserve(drop_count);
    const star_drop_descriptor lower_bound{
        .drop_position = position2_f32{
            .x = event_area.position.x - event_area.size.x * 0.5f,
            .y = event_area.position.y - event_area.size.y * 0.5f
        },
        .damage_amount = drop_lower_bound.damage_amount,
        .drop_radius = drop_lower_bound.drop_radius,
        .fall_time = drop_lower_bound.fall_time,
        .spawn_distance = drop_lower_bound.spawn_distance
    };
    const star_drop_descriptor upper_bound{
        .drop_position = position2_f32{
            .x = event_area.position.x + event_area.size.x * 0.5f,
            .y = event_area.position.y + event_area.size.y * 0.5f
        },
        .damage_amount = drop_upper_bound.damage_amount,
        .drop_radius = drop_upper_bound.drop_radius,
        .fall_time = drop_upper_bound.fall_time,
        .spawn_distance = drop_upper_bound.spawn_distance
    };
    for (size_t i = 0; i < drop_count; ++i) {
        star_drop_descriptors.at(i) = (
            star_shower_event_generate_star_drop_descriptor(lower_bound, upper_bound, generator)
        );
    }
    
    Game &game = *Game::Instance();
    const ecs::sceneId_t scene_id = game.get_currentScene()->get_scene_id();
    ecs::Manager &manager = *game.get_mngr();
    const camera_screen &camera = manager.getComponent<camera_component>(manager.getHandler(ecs::hdlr::CAMERA))->cam;
    for (size_t i = 0; i < drop_count; ++i) {
        auto drop = star_shower_event_create_star_drop(
            star_drop_descriptors[i], *game.get_mngr(), camera, scene_id
        );
        star_drops.push_back(drop);
    }
}

void star_shower_event::end_wave_callback() {
    auto &&manager = *Game::Instance()->get_mngr();
    for (auto star_drop : star_drops) {
        assert(star_drop.mark_entity != nullptr && "fatal error: star_drop.mark_entity must not be nullptr");
        assert(star_drop.star_entity != nullptr && "fatal error: star_drop.star_entity must not be nullptr");
        
        manager.setAlive(star_drop.mark_entity, false);
        manager.setAlive(star_drop.star_entity, false);
    }
    star_drops.clear();
}

static void star_shower_event_on_impact(    
    const star_drop &star_drop,
    ecs::Manager &manager,
    const seconds_f32 delta_time_seconds
) {
    (void)delta_time_seconds;
    assert(star_drop.remaining_fall_time <= 0.0f && "fatal error: star_drop.remaining_fall_time must be less than or equal to 0.0f");
    
    auto &&player = manager.getHandler(ecs::hdlr::PLAYER);
    auto &&player_transform = manager.getComponent<Transform>(player);
    auto &&player_rect = manager.getComponent<rect_component>(player);

    const position2_f32 player_centre{
        .x = player_transform->getPos().getX() + player_rect->rect.position.x,
        .y = player_transform->getPos().getY() + player_rect->rect.position.y
    };
    const vec2_f32 player_displacement{
        .x = player_centre.x - star_drop.star_transform->getPos().getX(),
        .y = player_centre.y - star_drop.star_transform->getPos().getY()
    };
    
    const float player_distance_sqr = player_displacement.x * player_displacement.x + player_displacement.y * player_displacement.y;
    if (player_distance_sqr < star_drop.radius) {
        auto &&player_health = *manager.getComponent<Health>(player);
        player_health.takeDamage(star_drop.damage_amount);
    }

    manager.setAlive(star_drop.mark_entity, false);
    manager.setAlive(star_drop.star_entity, false);
}

void star_shower_event::update(unsigned int delta_time) {
    if (!star_drops.empty()) {
        const size_t drop_count = star_drops.size();
        assert(drop_count + drops_destroyed >= min_drops_inclusive && "fatal error: drop_count must be greater than or equal to min_drops_inclusive");
        assert(drop_count + drops_destroyed < max_drops_exclusive && "fatal error: drop_count must be less than max_drops_exclusive");

        seconds_f32 delta_time_seconds = seconds_f32{float(delta_time) / 1000.0f};
        auto &&manager = *Game::Instance()->get_mngr();
        for (ptrdiff_t i = drop_count - 1; i >= 0 && drop_count==star_drops.size(); --i) {
            auto &&star_drop = star_drops[i];            
            if (star_drop.remaining_fall_time <= 0.0f) {
                star_shower_event_on_impact(star_drop, manager, delta_time_seconds);
                star_drops[i] = star_drops.back();
                star_drops.pop_back();
                
                ++drops_destroyed;
                assert(drops_destroyed < max_drops_exclusive && "fatal error: drops_destroyed must be less than max_drops_exclusive");
            } else {
                star_drop.star_transform->setPos(
                    star_drop.star_transform->getPos()
                        + Vector2D{0.0, -1.0} * (star_drop.fall_speed * delta_time_seconds)
                );
                star_drop.remaining_fall_time -= delta_time_seconds;
            }
        }
    }
}
