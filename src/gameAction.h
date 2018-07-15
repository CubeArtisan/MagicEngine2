#ifndef _GAMEACTION_H_
#define _GAMEACTION_H_

#include <variant>

#include "ability.h"
#include "cost.h"

struct CastSpell {
    xg::Guid spell;
    // Modes
    std::vector<xg::Guid> targets;
    Cost& cost;
    std::vector<std::reference_wrapper<Cost>> additionalCosts;
    unsigned int x;
};

struct PlayLand {
    xg::Guid land;
};

struct ActivateAnAbility {
    std::variant<std::reference_wrapper<Card>, std::reference_wrapper<Token>, std::reference_wrapper<Emblem>> source;
    std::shared_ptr<ActivatedAbility> ability;
    std::vector<xg::Guid> targets;
    Cost& cost;
    unsigned int x;
};

struct PassPriority {};

using GameAction = std::variant<CastSpell, PlayLand, ActivateAnAbility, PassPriority>;

#endif
