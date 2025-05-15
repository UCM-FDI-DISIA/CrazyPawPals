#ifndef RIGIDBODY_COMPONENT_HPP
#define RIGIDBODY_COMPONENT_HPP


#include "../../ecs/Component.h"

#include "../../ecs/ecs.h"
#include "../../ecs/Entity.h"
#include "../../physics/rigidbody.hpp"
#include "../../physics/collision.hpp"
#include "movement/Transform.h"
#include "rendering/rect_component.hpp"
#include "../../game/Game.h"
#include "../../ecs/Manager.h"

#ifndef RIGIDBODY_COMPONENT_DEGEN_RESTITUTION_OFF
#define RIGIDBODY_COMPONENT_DEGEN_RESTITUTION true
#else
#define RIGIDBODY_COMPONENT_DEGEN_RESTITUTION false
#endif

struct rigidbody_component : public ecs::Component {
    __CMPID_DECL__(ecs::cmp::RIGIDBODY)
    rect_f32 rect;
    inverse_mass_f32 inverse_mass;
    float restitution_coefficient;

private:
    rigidbody_component()
        : inverse_mass{0.0f}, restitution_coefficient{0.0} {

    }

public:
    rigidbody_component(const rect_f32 rect, const mass_f32 mass, const float restitution_coefficient);
    rigidbody_component(const rect_f32 rect, const inverse_mass_f32 inverse_mass, const float restitution_coefficient);
};


enum collisionable_option {
    collisionable_option_none = 0,
    collisionable_option_trigger = 1 << 0,
};

typedef uint8_t collisionable_flags;

struct collisionable : public ecs::Component {
    __CMPID_DECL__(ecs::cmp::COLLISIONABLE)

    Transform &transform;
    rigidbody_component &rigidbody;
    rect_component &rect;
    collisionable_flags options;

    collisionable(Transform &transform, rigidbody_component &rigidbody, rect_component &rect, collisionable_option options)
        : transform(transform), rigidbody(rigidbody), rect(rect), options(options) {
    }
};

struct collision_manifold {
    collision_contact contact;
    uint64_t collision_tick;
    ecs::entity_t body0;
    ecs::entity_t body1;

    constexpr collision_manifold(collision_contact contact, uint64_t collision_tick, ecs::entity_t body0, ecs::entity_t body1)
        : contact(contact), collision_tick(collision_tick), body0(body0), body1(body1) {
    }

    constexpr collision_manifold()
        : contact(collision_contact{
                    .penetration = {0.0f, 0.0f},
                    .normal = {0.0f, 0.0f},
                    .delta_of_collision_normalised = 0.0f
            }), collision_tick(0), body0(nullptr), body1(nullptr) {
    }
};
static constexpr int8_t collision_manifolds_capacity = 32;

template <ecs::cmp::cmpId COMPONENT_ID>
struct collision_manifolds : ecs::Component {
    __CMPID_DECL__(COMPONENT_ID)
    std::array<collision_manifold, collision_manifolds_capacity> manifolds;
    uint8_t count = 0;
    /*
    collision_manifolds() {
        
    }
    */
};
using contact_manifolds = collision_manifolds<ecs::cmp::CONTACT_MANIFOLD>;
using trigger_manifolds = collision_manifolds<ecs::cmp::TRIGGER_MANIFOLD>;

static const collision_manifold uninitialized_manifold = collision_manifold{collision_contact{
    .penetration = {0.0f, 0.0f},
    .normal = {0.0f, 0.0f},
    .delta_of_collision_normalised = 0.0f
}, 0, nullptr, nullptr};

template <typename OnCollisionComponent>
struct on_collision : public ecs::Component {
    contact_manifolds *manifolds;
    std::array<uint64_t, collision_manifolds_capacity> last_collision_tick;
        
    on_collision() : manifolds{nullptr}, last_collision_tick{0} {

    }
    void initComponent() override {
        ecs::Component::initComponent();
        manifolds = Game::Instance()->get_mngr()->addComponent<contact_manifolds>(_ent);
    }

    void update(uint32_t delta_time) override {
        (void)delta_time;
        static_assert((collision_manifolds_capacity & (collision_manifolds_capacity - 1)) == 0);
        for (ptrdiff_t i = (ptrdiff_t(manifolds->count) - 1) & (manifolds->manifolds.size() - 1); i >= 0; --i) {
            auto& my_manifold = manifolds->manifolds[i];
            if (my_manifold.collision_tick != last_collision_tick[i]) {
                static_cast<OnCollisionComponent*>(this)->on_contact(my_manifold);
                last_collision_tick[i] = my_manifold.collision_tick;
            }
        }
        manifolds->count = 0;
    }
};

template <typename OnTriggerComponent>
struct on_trigger : public ecs::Component {
    trigger_manifolds* manifolds;
    std::array<uint64_t, collision_manifolds_capacity> last_collision_tick;

    on_trigger() : manifolds{nullptr}, last_collision_tick{0} {

    }
    void initComponent() override {
        ecs::Component::initComponent();
        manifolds = Game::Instance()->get_mngr()->addComponent<trigger_manifolds>(_ent );
    }
    void update(uint32_t delta_time) override {
        (void)delta_time;
        static_assert((collision_manifolds_capacity & (collision_manifolds_capacity - 1)) == 0);
        for (ptrdiff_t i = (ptrdiff_t(manifolds->count) - 1) & (manifolds->manifolds.size() - 1); i >= 0; --i) {
            auto& my_manifold = manifolds->manifolds[i];
            if (my_manifold.collision_tick != last_collision_tick[i]) {
                static_cast<OnTriggerComponent*>(this)->on_contact(my_manifold);
                last_collision_tick[i] = my_manifold.collision_tick;
            }
        }
        manifolds->count = 0;
    }
};

// DOC!

// class test_collision_callback : public on_collision<test_collision_callback> {
//     __CMPID_DECL__(ecs::cmp::/*COMP_ID*/);
//     void on_contact(const contact_manifold &manifold) {
//         // Collision event code here
//     }
// };

// class test_trigger_callback : public on_trigger<test_trigger_callback> {
//     __CMPID_DECL__(ecs::cmp::/*COMP_ID*/);
//     void on_contact(const trigger_manifold &manifold) {
//         // Triiger event code here
//     }
// };

#endif

