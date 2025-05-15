
#include "AnimationComponent.h"

#include "../../game/Game.h"
#include "../../ecs/Manager.h"

AnimationComponent::AnimationComponent()
    :_dy(nullptr), _currentAnim(){}

void AnimationComponent::initComponent()
{
    _dy = Game::Instance()->get_mngr()->getComponent<dyn_image_with_frames>(_ent);
    assert(_dy != nullptr);
}

void AnimationComponent::add_animation(const std::string& name, int ini_frame, int end_frame, uint32_t frame_duration) {
    animations[name] = Animation(ini_frame, end_frame, frame_duration);
}

void AnimationComponent::play_animation(const std::string& name) {
    auto it = animations.find(name);
    if (it != animations.end() && _currentAnim != it->first) {
        _currentAnim = it->first;
        auto anim = it->second;
        _dy->set_animation(anim.ini_frame, anim.end_frame, anim.frame_duration);
    }
}