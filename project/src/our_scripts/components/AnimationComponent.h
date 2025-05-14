#pragma once

#include <unordered_map>
#include <string>
#include "../../ecs/Component.h"
#include "rendering/dyn_image_with_frames.hpp"

struct Animation {
    int ini_frame;
    int end_frame;
    uint32_t frame_duration;

    Animation() : ini_frame(0), end_frame(0), frame_duration(0) {} //constructor por defecto
    Animation(int ini, int end, uint32_t duration): ini_frame(ini), end_frame(end), frame_duration(duration) {}
};

class AnimationComponent :public ecs::Component {
public:
    __CMPID_DECL__(ecs::cmp::ANIMATIONCOMPONENT);

    std::unordered_map<std::string, Animation> animations;

    AnimationComponent();

    virtual ~AnimationComponent() {};

    void initComponent() override;

    void add_animation(const std::string& name, int ini_frame, int end_frame, uint32_t frame_duration);

    void play_animation(const std::string& name);

    std::string get_current_Anim() { return _currentAnim; };

private:
    dyn_image_with_frames* _dy;
    std::string _currentAnim;
};
