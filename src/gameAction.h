#ifndef _GAMEACTION_H_
#define _GAMEACTION_H_

#include <variant>

#include "environment.h"

struct CastSpell {
    xg::Guid spell;
    // Modes
    std::vector<xg::Guid> targets;
    Cost cost;
    std::vector<Cost> additionalCost;
    unsigned int x;
};

struct PlayLand {
    xg::Guid land;
};

struct ActivateAnAbility {
    xg::Guid source;
    ActivatedAbility& ability;
    std::vector<xg::Guid> targets;
    Cost cost;
    unsigned int x;
};

struct PassPriority {};

using GameAction = std::variant<CastSpell, PlayLand, ActivatedAbility, PassPriority>;

#endif
