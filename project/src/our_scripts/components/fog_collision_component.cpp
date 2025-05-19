
#include "fog_collision_component.hpp"
#include "rigidbody_component.hpp"
#include "collision_triggerers.hpp"
#include "../../game/Game.h"
#include "../../ecs/Manager.h"
#include "../../game/GameStructs.h"
#include "Health.h"
#include "collision_registration_by_id.h"
#include "id_component.h"
#include "../../sdlutils/SDLUtils.h"

void fog_collision_component::on_contact(const collision_manifold& tm)
{
    //takes pointer to the other body
    ecs::entity_t entity_collided_with = _ent == tm.body0 ? tm.body1 : tm.body0;
    if (entity_collided_with->get_currCmps_size() > 0 && Game::Instance()->get_mngr()->hasComponent<player_collision_triggerer>(entity_collided_with)) {
        last_fog_time_collided_by_player[Game::Instance()->get_mngr()->getComponent<id_component>(entity_collided_with)->getId()]= std::make_pair(entity_collided_with,sdlutils().virtualTimer().currRealTime());
    }
}

void fog_collision_component::reset()
{
    last_fog_time_collided_by_player.clear();
}

void fog_collision_component::update(uint32_t delta_time) {
	on_trigger<fog_collision_component>::update(delta_time);
    for (auto& pair : last_fog_time_collided_by_player) {
        if (sdlutils().virtualTimer().currRealTime() > pair.second.second + 499) {
            pair.second.second = sdlutils().virtualTimer().currRealTime();
            Game::Instance()->get_mngr()->getComponent<Health>(pair.second.first)->takeDamage(fogDamage);
        }
    }
}

