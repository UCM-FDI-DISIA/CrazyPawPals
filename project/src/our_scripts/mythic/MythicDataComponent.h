#pragma once

#include "../../ecs/Component.h"
#include "../../game/GameStructs.h"
#include <string>

struct MythicDataComponent : public ecs::Component {
public:
    __CMPID_DECL__(ecs::cmp::MYTHICDATACOMPONENT);
    MythicDataComponent();
    MythicDataComponent(const std::string& s);
    virtual ~MythicDataComponent();
    void set_data(const std::string& s, GameStructs::MythicType mt);
    const GameStructs::MythicType& MT() { return _mt; };
    const std::string& sprite() { return _data; };
protected:
    GameStructs::MythicType _mt;
    std::string _data;
};