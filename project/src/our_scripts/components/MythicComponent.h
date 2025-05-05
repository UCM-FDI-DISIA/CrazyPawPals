#pragma once
#include "../../utils/checkML.h"
#include <vector>
#include "../../ecs/Component.h"
#include "../../utils/checkML.h"

class MythicItem;

class MythicComponent: public ecs::Component {
private:
	std::vector<MythicItem*> _obtained_mythics;
public:
    __CMPID_DECL__(ecs::cmp::MYTHICCOMPONENT);
    MythicComponent();
    ~MythicComponent();
    void initComponent() override;
    void add_mythic(MythicItem* mythic);
    void reset();
    void update(uint32_t dt) override;
    inline const std::vector<MythicItem*>& get_mythics() const { return _obtained_mythics; }
};
