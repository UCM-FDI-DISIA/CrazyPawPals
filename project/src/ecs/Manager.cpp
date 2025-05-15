
#include "Manager.h"
#include "../our_scripts/components/rendering/render_ordering.hpp"
#include "../our_scripts/components/rigidbody_component.hpp"
#include "../physics/collision.hpp"
#include "../our_scripts/components/rendering/camera_component.hpp"
#include <cstdlib>
#include <unordered_set>

struct contact_pair {
	ecs::entity_t body0;
	ecs::entity_t body1;
};

static inline bool operator==(const contact_pair &lhs, const contact_pair &rhs) {
	return lhs.body0 == rhs.body0 && lhs.body1 == rhs.body1;
}
static inline bool operator<(const contact_pair &lhs, const contact_pair &rhs) {
	return lhs.body0 < rhs.body0 || (lhs.body0 == rhs.body0 && lhs.body1 < rhs.body1);
}

namespace std {
	template <>
	struct hash<contact_pair> {
		size_t operator()(const contact_pair &pair) const {
			return std::hash<ecs::entity_t>()(pair.body0) ^ std::hash<ecs::entity_t>()(pair.body1);
		}
	};
}

namespace ecs {

Manager::Manager() :
		_hdlrs(), //
		_entsByGroup() //
{

	// for each group we reserve space for 100 entities,
	// just to avoid copies
	//
	for (auto &groupEntities : _entsByGroup) {
		groupEntities.reserve(100);
	}
}

Manager::~Manager() {

	// delete all entities
	//
	for (auto &ents : _entsByGroup) {
		for (auto e : ents)
			if (e != nullptr && e->get_currCmps_size()) delete e;
	}
}


#ifndef DBG_COLLISIONS
#define DBG_COLLISIONS_DEFAULT true
#define DBG_COLLISIONS DBG_COLLISIONS_DEFAULT
#endif

#if DBG_COLLISIONS
#define DBG_COLLISIONS_BUFFER_SIZE 4096
struct dbg_collisions {
	SDL_Rect dbg_rect_col[DBG_COLLISIONS_BUFFER_SIZE];
	size_t dbg_rect_col_size;

	position2_f32 dbg_pos[2][DBG_COLLISIONS_BUFFER_SIZE];
	size_t dbg_pos_size;
};

static dbg_collisions dbg_col = {0};
static void dbg_collision_fill_rect(ecs::Manager &manager, const collision_body &body0, const collision_body &body1) {
	auto &&camera = *manager.getComponent<camera_component>(manager.getHandler(ecs::hdlr::CAMERA));
	
	auto &&dbg_rect_col = dbg_col.dbg_rect_col;
	auto &&dbg_rect_col_size = dbg_col.dbg_rect_col_size;
	
	assert(dbg_rect_col_size < sizeof(dbg_rect_col) / sizeof(dbg_rect_col[0]));
	dbg_rect_col[dbg_rect_col_size] = SDL_Rect_screen_rect_from_global(
		rect_f32{
			.position = {
				.x = body0.body.body.position.x + body0.body.space.position.x - body0.body.body.size.x * 0.5f,
				.y = body0.body.body.position.y + body0.body.space.position.y + body0.body.body.size.y * 0.5f,
			},
			.size = {
				.x = body0.body.body.size.x,
				.y = body0.body.body.size.y,
			},
		},
		camera.cam
	);
	++dbg_rect_col_size;
	dbg_rect_col[dbg_rect_col_size] = SDL_Rect_screen_rect_from_global(
		rect_f32{
			.position = {
				.x = body1.body.body.position.x + body1.body.space.position.x - body1.body.body.size.x * 0.5f,
				.y = body1.body.body.position.y + body1.body.space.position.y + body1.body.body.size.y * 0.5f,
			},
			.size = {
				.x = body1.body.body.size.x,
				.y = body1.body.body.size.y,
			},
		},
		camera.cam
	);
	++dbg_rect_col_size;

	dbg_rect_col[dbg_rect_col_size] = SDL_Rect_screen_rect_from_global(
		rect_f32{
			.position = {
				.x = body1.body.body.position.x + body1.body.space.position.x - (body1.body.body.size.x + body0.body.body.size.x) * 0.5f,
				.y = body1.body.body.position.y + body1.body.space.position.y + (body1.body.body.size.y + body0.body.body.size.y) * 0.5f,
			},
			.size = {
				.x = body1.body.body.size.x + body0.body.body.size.x,
				.y = body1.body.body.size.y + body0.body.body.size.y,
			},
		},
		camera.cam
	);
	++dbg_rect_col_size;

	dbg_rect_col[dbg_rect_col_size] = SDL_Rect_screen_rect_from_global(
		rect_f32{
			.position = {
				.x = - (body1.body.body.size.x + body0.body.body.size.x) * 0.5f,
				.y = + (body1.body.body.size.y + body0.body.body.size.y) * 0.5f,
			},
			.size = {
				.x = body1.body.body.size.x + body0.body.body.size.x,
				.y = body1.body.body.size.y + body0.body.body.size.y,
			},
		},
		camera.cam
	);
	++dbg_rect_col_size;

	dbg_rect_col[dbg_rect_col_size] = SDL_Rect_screen_rect_from_global(
		rect_f32{
			.position = {
				.x = (body0.body.space.previous_position.x + body0.body.body.position.x - (body1.body.body.position.x + body1.body.space.previous_position.x)),
				.y = (body0.body.space.previous_position.y + body0.body.body.position.y - (body1.body.body.position.y + body1.body.space.previous_position.y)),
			},
			.size = {
				.x = 0.0f,
				.y = 0.0f,
			},
		},
		camera.cam
	);
	++dbg_rect_col_size;
}

static void dbg_collsion_fill_point_pair(ecs::Manager &manager, const collision_contact &contact, const collision_body &body0, const collision_body &body1) {
	(void)manager;
	(void)body1;

	auto &&dbg_pos = dbg_col.dbg_pos;
	auto &&dbg_pos_size = dbg_col.dbg_pos_size;

	dbg_pos[0][dbg_pos_size] = body0.body.space.position;
	dbg_pos[1][dbg_pos_size] = position2_f32{
		.x = body0.body.space.position.x + contact.penetration.penetration.x * 20.0f,
		.y = body0.body.space.position.y + contact.penetration.penetration.y * 20.0f,
	};
	++dbg_pos_size;
}

static void dbg_collision_render_drain(ecs::Manager &manager) {
	auto &&dbg_rect_col = dbg_col.dbg_rect_col;
	auto &&dbg_rect_col_size = dbg_col.dbg_rect_col_size;

	auto &&dbg_pos = dbg_col.dbg_pos;
	auto &&dbg_pos_size = dbg_col.dbg_pos_size;

	for (size_t i = 0; i < dbg_rect_col_size; i++) {
		SDL_SetRenderDrawColor(sdlutils().renderer(), 255, 0, 0, 255);
		SDL_RenderDrawRect(sdlutils().renderer(), &dbg_rect_col[i]);
	}
	dbg_rect_col_size = 0;

	const camera_screen &camera = manager.getComponent<camera_component>(manager.getHandler(ecs::hdlr::CAMERA))->cam;
	for (size_t i = 0; i < dbg_pos_size; i++) {
		SDL_SetRenderDrawColor(sdlutils().renderer(), 0, 255, 0, 255);

		auto a =SDL_Rect_screen_rect_from_global(
			rect_f32{
				.position = {
					.x = dbg_pos[0][i].x,
					.y = dbg_pos[0][i].y,
				},
				.size = {
					.x = 0.1f,
					.y = 0.1f,
				},
			},
			camera
		);
		auto b = SDL_Rect_screen_rect_from_global(
			rect_f32{
				.position = {
					.x = dbg_pos[1][i].x,
					.y = dbg_pos[1][i].y,
				},
				.size = {
					.x = 0.1f,
					.y = 0.1f,
				},
			},
			camera
		);
		SDL_RenderDrawLine(
			sdlutils().renderer(),
			a.x,
			a.y,
			b.x,
			b.y
		);
	}
	dbg_pos_size = 0;
}
#endif

static position2_f32 position2_f32_from_vec2(const Vector2D vec) {
	return position2_f32{
		.x = vec.getX(),
		.y = vec.getY(),
	};
}
static collision_body collision_body_from_collisionable(const collisionable c) {
	return collision_body{
		.body{
			.body = rect_f32{
				.position = position2_f32{
					.x = c.rigidbody.rect.position.x + c.rect.rect.position.x,
					.y = c.rigidbody.rect.position.y + c.rect.rect.position.y,
				},
				.size = size2_f32{
					.x = c.rigidbody.rect.size.x * c.rect.rect.size.x,
					.y = c.rigidbody.rect.size.y * c.rect.rect.size.y,
				},
			},
			.space{
				.position = position2_f32_from_vec2(c.transform.getPos()),
				.previous_position = position2_f32_from_vec2(c.transform.get_previous_position()),
			},
			.mass_rcp = c.rigidbody.inverse_mass,
		},
		.restitution = c.rigidbody.restitution_coefficient,
	};
}

enum collision_response_options {
	collision_response_option_none = 0,
	collision_response_option_body0_trigger = 1 << 0,
	collision_response_option_body1_trigger = 1 << 1,
};
typedef uint8_t collision_response_flags;

enum manager_collision_handle_response {
	manager_collision_handle_response_none = 0,
	manager_collision_handle_response_collision = 1 << 0,
	manager_collision_handle_response_trigger = 1 << 1,
};
typedef uint8_t manager_collision_handle_response_flags;



static manager_collision_handle_response manager_handle_collision_bodies(
	Manager &manager,
	const ecs::entity_t entity0,
	const ecs::entity_t entity1,
	collision_body &body0,
	collision_body &body1,
	seconds_f32 delta_time,
	std::unordered_set<contact_pair> &pairs_already_checked,
	collision_contact &out_contact,
	const collision_response_flags flags
) {
#if DBG_COLLISIONS
	dbg_collision_fill_rect(manager, body0, body1);
#endif
	if (!collision_body_check_broad(body0, body1)) {
		out_contact = collision_contact{};
		return manager_collision_handle_response_none;
	}

	collision_contact &contact = out_contact;
	if (collision_body_check(body0, body1, delta_time, contact)) {
#if DBG_COLLISIONS
		//dbg_collsion_fill_point_pair(manager, contact, body0, body1);
#endif
		switch (flags & (collision_response_option_body0_trigger | collision_response_option_body1_trigger)) {
		case collision_response_option_none: {
			const collision_response_pairs responses = collision_body_resolve(body0, body1, contact);	

			auto &response0_separation = responses.penetration_responses[0];
			auto &response1_separation = responses.penetration_responses[1];
	
			auto &response0_restitution = responses.restitution_responses[0];
			auto &response1_restitution = responses.restitution_responses[1];
			{
				auto &space0 = body0.body.space;
				auto &space1 = body1.body.space;
				
				space0.position.x += response0_separation.separation.x;
				space0.position.y += response0_separation.separation.y;
	
				space1.position.x += response1_separation.separation.x;
				space1.position.y += response1_separation.separation.y;
	
				space0.previous_position.x = space0.position.x - response0_restitution.restitution_displacement.x;
				space0.previous_position.y = space0.position.y - response0_restitution.restitution_displacement.y;
				
				space1.previous_position.x = space1.position.x - response1_restitution.restitution_displacement.x;
				space1.previous_position.y = space1.position.y - response1_restitution.restitution_displacement.y;
			}
			auto p0 = contact_pair{entity0,entity1};
			auto p1 = contact_pair{entity1, entity0};
			if (pairs_already_checked.find(p0) == pairs_already_checked.end()) {

				contact_manifolds* contact0 = manager.getComponent<contact_manifolds>(entity0);
				contact_manifolds* contact1 = manager.getComponent<contact_manifolds>(entity1);
				if (contact0 != nullptr) {
					contact0->manifolds[contact0->count & (contact0->manifolds.size() - 1)].contact = contact;
					contact0->manifolds[contact0->count & (contact0->manifolds.size() - 1)].collision_tick = sdlutils().virtualTimer().currTime();
					contact0->manifolds[contact0->count & (contact0->manifolds.size() - 1)].body0 = entity0;
					contact0->manifolds[contact0->count & (contact0->manifolds.size() - 1)].body1 = entity1;
					++contact0->count;
				}
				if (contact1 != nullptr) {
					contact1->manifolds[contact1->count & (contact1->manifolds.size() - 1)].contact = contact;
					contact1->manifolds[contact1->count & (contact1->manifolds.size() - 1)].collision_tick = sdlutils().virtualTimer().currTime();
					contact1->manifolds[contact1->count & (contact1->manifolds.size() - 1)].body0 = entity0;
					contact1->manifolds[contact1->count & (contact1->manifolds.size() - 1)].body1 = entity1;
					++contact1->count;
				}
				auto it0 = pairs_already_checked.insert(p0);
				auto it1 = pairs_already_checked.insert(p1);
				assert(it0.second);
				assert(it1.second);
			}
			return manager_collision_handle_response_collision;
		}
		case collision_response_option_body0_trigger: {
			auto p0 = contact_pair{entity0,entity1};
			auto p1 = contact_pair{entity1, entity0};
			if (pairs_already_checked.find(p0) == pairs_already_checked.end()) {
				trigger_manifolds* trigger0 = manager.getComponent<trigger_manifolds>(entity0);
				if (trigger0 != nullptr) {
					trigger0->manifolds[trigger0->count & (trigger0->manifolds.size() - 1)].contact = contact;
					trigger0->manifolds[trigger0->count & (trigger0->manifolds.size() - 1)].collision_tick = sdlutils().virtualTimer().currTime();
					trigger0->manifolds[trigger0->count & (trigger0->manifolds.size() - 1)].body0 = entity0;
					trigger0->manifolds[trigger0->count & (trigger0->manifolds.size() - 1)].body1 = entity1;
					++trigger0->count;
				}
				auto it0 = pairs_already_checked.insert(p0);
				auto it1 = pairs_already_checked.insert(p1);
				assert(it0.second);
				assert(it1.second);
			}
			return manager_collision_handle_response_trigger;
		}
		case collision_response_option_body1_trigger: {
			auto p0 = contact_pair{entity0,entity1};
			auto p1 = contact_pair{entity1, entity0};
			if (pairs_already_checked.find(p0) == pairs_already_checked.end()) {
				trigger_manifolds* trigger1 = manager.getComponent<trigger_manifolds>(entity1);
				if (trigger1 != nullptr) {
					trigger1->manifolds[trigger1->count & (trigger1->manifolds.size() - 1)].contact = contact;
					trigger1->manifolds[trigger1->count & (trigger1->manifolds.size() - 1)].collision_tick = sdlutils().virtualTimer().currTime();
					trigger1->manifolds[trigger1->count & (trigger1->manifolds.size() - 1)].body0 = entity0;
					trigger1->manifolds[trigger1->count & (trigger1->manifolds.size() - 1)].body1 = entity1;
					++trigger1->count;
				}
				auto it0 = pairs_already_checked.insert(p0);
				auto it1 = pairs_already_checked.insert(p1);
				assert(it0.second);
				assert(it1.second);
			}
			return manager_collision_handle_response_trigger;
		}
		case collision_response_option_body0_trigger | collision_response_option_body1_trigger: {
			auto p0 = contact_pair{entity0,entity1};
			auto p1 = contact_pair{entity1, entity0};
			if (pairs_already_checked.find(p0) == pairs_already_checked.end()) {
				trigger_manifolds* trigger0 = manager.getComponent<trigger_manifolds>(entity0);
				trigger_manifolds* trigger1 = manager.getComponent<trigger_manifolds>(entity1);

				if (trigger0 != nullptr) {
					trigger0->manifolds[trigger0->count & (trigger0->manifolds.size() - 1)].contact = contact;
					trigger0->manifolds[trigger0->count & (trigger0->manifolds.size() - 1)].collision_tick = sdlutils().virtualTimer().currTime();
					trigger0->manifolds[trigger0->count & (trigger0->manifolds.size() - 1)].body0 = entity0;
					trigger0->manifolds[trigger0->count & (trigger0->manifolds.size() - 1)].body1 = entity1;
					++trigger0->count;
				}
				if (trigger1 != nullptr) {
					trigger1->manifolds[trigger1->count & (trigger1->manifolds.size() - 1)].contact = contact;
					trigger1->manifolds[trigger1->count & (trigger1->manifolds.size() - 1)].collision_tick = sdlutils().virtualTimer().currTime();
					trigger1->manifolds[trigger1->count & (trigger1->manifolds.size() - 1)].body0 = entity0;
					trigger1->manifolds[trigger1->count & (trigger1->manifolds.size() - 1)].body1 = entity1;
					++trigger1->count;
				}
				auto it0 = pairs_already_checked.insert(p0);
				auto it1 = pairs_already_checked.insert(p1);
				assert(it0.second);
				assert(it1.second);
			}
			return manager_collision_handle_response_trigger;
		}
		default: {
			assert(false && "unreachable");
			std::exit(EXIT_FAILURE);
		}
		}
	} else {
		return manager_collision_handle_response_none;
	}
}

static void manager_update_collisions(Manager &manager, const std::vector<ecs::entity_t> &entities, const uint32_t delta_time_milliseconds) {
	switch (entities.size()) {
	case 0:
		break;
	case 1:
		break;
	default: {
		const float delta_time_seconds = float(delta_time_milliseconds) / 1000.0f;

		
		struct collision_check {
			ecs::entity_t entity0;
			collision_body body0;
			collisionable *collisionable0;
			
			ecs::entity_t entity1;
			collision_body body1;
			collisionable *collisionable1;
		};
		std::vector<collision_check> collision_checks;
		for (size_t i = 0; i < entities.size(); i++) {
			auto entity = entities[i];
			auto entity_collisionable = manager.getComponent<collisionable>(entity);
			if (entity_collisionable != nullptr) {
				collision_body body = collision_body_from_collisionable(*entity_collisionable);
				
				for (size_t j = i + 1; j < entities.size(); j++) {
					auto other_entity = entities[j];
					auto other_collisionable = manager.getComponent<collisionable>(other_entity);
					if (other_collisionable != nullptr) {
						collision_body other_body = collision_body_from_collisionable(*other_collisionable);
						
						collision_checks.push_back(collision_check{
							.entity0 = entity,
							.body0 = body,
							.collisionable0 = entity_collisionable,
							.entity1 = other_entity,
							.body1 = other_body,
							.collisionable1 = other_collisionable,
						});
					}
				}
			}
		}
		constexpr static const size_t max_collision_passes = 4;
		size_t last_pass_collision_count;
		size_t pass_count = 0;
		std::unordered_set<contact_pair> aux_set{};
		do {
			last_pass_collision_count = 0;
			float previous_interpenetration_sqr = std::numeric_limits<float>::max();
			for (size_t i = 0; i < collision_checks.size(); ++i) {
				auto &&check = collision_checks[i];
				auto &&entity = check.entity0;
				auto &&body = check.body0;
				auto &&entity_collisionable = *check.collisionable0;
				auto &&other_entity = check.entity1;
				auto &&other_body = check.body1;
				auto &&other_collisionable = *check.collisionable1;

				collision_contact contact;
				manager_collision_handle_response collided = manager_handle_collision_bodies(
					manager, entity, other_entity, body, other_body, delta_time_seconds, aux_set, contact, (
					((entity_collisionable.options & collisionable_option_trigger) != 0)
						| (((other_collisionable.options & collisionable_option_trigger) != 0) << 1)
					)
				);
	
				switch (collided) {
				case manager_collision_handle_response_none:
					break;
				case manager_collision_handle_response_collision: {
					const vec2_f32 displacement = vec2_f32{
						.x = body.body.space.position.x - body.body.space.previous_position.x,
						.y = body.body.space.position.y - body.body.space.previous_position.y,
					};
					const float displacement_length_sqr = 
						displacement.x * displacement.x + displacement.y * displacement.y;
					constexpr static const float epsilon_displacement_length_sqr = 0.0000001f;
					//if (displacement_length_sqr > epsilon_displacement_length_sqr) {	
					{
						// this is done with assignment operator instead of setPos method to prevent the displacement vector to be mutated this frame while the correction happens
						const auto previous0{entity_collisionable.transform.getPos()};
						const auto previous1{other_collisionable.transform.getPos()};
						const vec2_f32 restitution0{
							.x = (body.body.space.position.x - body.body.space.previous_position.x),
							.y = (body.body.space.position.y - body.body.space.previous_position.y),
						};
						const vec2_f32 restitution1{
							.x = (other_body.body.space.position.x - other_body.body.space.previous_position.x),
							.y = (other_body.body.space.position.y - other_body.body.space.previous_position.y),
						};
						
#if DBG_COLLISIONS
						dbg_col.dbg_pos[0][dbg_col.dbg_pos_size] = position2_f32{
							other_collisionable.transform.getPos().getX(),
							other_collisionable.transform.getPos().getY(),
						};
						dbg_col.dbg_pos[1][dbg_col.dbg_pos_size] = position2_f32{
							.x = other_collisionable.transform.getPos().getX() + (other_body.body.space.position.x - other_collisionable.transform.getPos().getX()) * (1.0f + 20.0f),
							.y = other_collisionable.transform.getPos().getY() + (other_body.body.space.position.y - other_collisionable.transform.getPos().getY()) * (1.0f + 20.0f),
						};
						++dbg_col.dbg_pos_size;
						
						dbg_col.dbg_pos[0][dbg_col.dbg_pos_size] = position2_f32{
							other_collisionable.transform.getPos().getX(),
							other_collisionable.transform.getPos().getY(),
						};
						dbg_col.dbg_pos[1][dbg_col.dbg_pos_size] = position2_f32{
							.x = other_collisionable.transform.getPos().getX() + restitution1.x * 200,
							.y = other_collisionable.transform.getPos().getY() + restitution1.y * 200,
						};
						++dbg_col.dbg_pos_size;
#endif
						
						entity_collisionable.transform.getPos() = (Vector2D{
							body.body.space.position.x + restitution0.x * delta_time_seconds,	
							body.body.space.position.y + restitution0.y * delta_time_seconds,	
						});
						other_collisionable.transform.getPos() = (Vector2D{
							other_body.body.space.position.x + restitution1.x * delta_time_seconds,
							other_body.body.space.position.y + restitution1.y * delta_time_seconds,
						});	
						++last_pass_collision_count;
					}
					break;
				}
				case manager_collision_handle_response_trigger:
					break;
				default: {
					assert(false && "unreachable");
					std::exit(EXIT_FAILURE);
				}
				}

				constexpr static const auto dot = [](const vec2_f32 a, const vec2_f32 b) -> float {
					return a.x * b.x + a.y * b.y;
				};
				const float interpenetration_sqr = dot(contact.penetration.penetration, contact.penetration.penetration);
				if (interpenetration_sqr > previous_interpenetration_sqr) {
					assert(i > 0 && "fatal error: on first iteration interpenetration_sqr is greater than previous_interpenetration_sqr");
					std::swap(collision_checks[i], collision_checks[i - 1]);
				}
				previous_interpenetration_sqr = interpenetration_sqr;
			}
			++pass_count;
		} while (last_pass_collision_count > 0 && pass_count < max_collision_passes);
	}
	}
}

void Manager::update(sceneId_t sId, Uint32 dt) {
	auto& entities = getEntitiesByScene(sId);
	for (auto &ents : entities) {
		update(ents, dt);
	}

	refresh();
	manager_update_collisions(*this, entities, dt);
}

void Manager::render(sceneId_t sId) {
    auto& _entity = getEntitiesByScene(sId);
	switch (_entity.size()) {
	case 0:
		break;
	case 1:
		render(_entity.front());
		break;
	default: {
		auto last_ordered_entity = _entity.front();
		for (size_t i = 1; i < _entity.size(); i++) {
			auto current_entity = _entity[i];
			
			render_ordering *last_ordered = getComponent<render_ordering>(last_ordered_entity);
			render_ordering *current_ordered = getComponent<render_ordering>(current_entity);

			if (last_ordered != nullptr && current_ordered != nullptr) {
				if (current_ordered->ordinal < last_ordered->ordinal) {
					render(current_entity);
				} else {
					render(last_ordered_entity);
					last_ordered_entity = current_entity;
				}
			} else {
				render(last_ordered_entity);
				last_ordered_entity = current_entity;
			}
		}
		assert(last_ordered_entity != nullptr);
		render(last_ordered_entity);
	}
	}
#if DBG_COLLISIONS
	dbg_collision_render_drain(*this);
#endif
}

void Manager::refresh()
{

    // remove dead entities from the groups lists, and also those
	// do not belong to the group anymore
	std::unordered_set<ecs::entity_t> to_remove;
	std::unordered_set<ecs::entity_t> to_swap;
	for (ecs::grpId_t gId = 0; gId < ecs::maxGroupId; gId++) {
		auto &groupEntities = _entsByGroup[gId];
		for (auto entity : groupEntities) {
			if (!isAlive(entity)) {
				to_remove.insert(entity);
			}

		}
	}
	for (ecs::sceneId_t sId = 0; sId < ecs::maxSceneId; sId++) {
		auto &sceneEntities = _entsByScene[sId];
		for (auto entity : sceneEntities) {
			if (!isAlive(entity)) {
				to_remove.insert(entity);
			}
			else if (entity->_sId != sId) {
				to_swap.insert(entity);
				_pendingEntities.push_back(entity);
			}
		}
	}
	for (auto &&scene : _entsByScene) {
		std::erase_if(scene, [&to_remove](ecs::entity_t e) {
			return to_remove.find(e) != to_remove.end();
		});
		std::erase_if(scene, [&to_swap](ecs::entity_t e) {
			return to_swap.find(e) != to_swap.end();
		});
	}
	for (auto &&group : _entsByGroup) {
		std::erase_if(group, [&to_remove](ecs::entity_t e) {
			return to_remove.find(e) != to_remove.end();
		});
	}

	// std::cout << "removed " << to_remove.size() << " entities" << std::endl;
	for (auto e : to_remove) {
		delete e;
		e = nullptr;
	}

	for (auto e : _pendingEntities) {
		_entsByGroup[e->_gId].push_back(e);
		_entsByScene[e->_sId].push_back(e);
	}
	_pendingEntities.clear();
}
} // end of namespace
