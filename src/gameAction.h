#ifndef _GAMEACTION_H_
#define _GAMEACTION_H_

#include <variant>

#include "ability.h"
#include "cost.h"

// CodeReview: Only handles casting from hand currently
// CodeReview: Implement Modes
struct CastSpell {
    xg::Guid spell;
    std::vector<xg::Guid> targets;
    Cost& cost;
    std::vector<std::shared_ptr<Cost>> additionalCosts;
    unsigned int x;
};

struct PlayLand {
    xg::Guid land;
};

struct ActivateAnAbility {
    std::variant<std::shared_ptr<Card>, std::shared_ptr<Token>> source;
    std::shared_ptr<ActivatedAbility> ability;
    std::vector<xg::Guid> targets;
    Cost& cost;
    unsigned int x;
};

struct PassPriority {};

using GameAction = std::variant<CastSpell, PlayLand, ActivateAnAbility, PassPriority>;

#endif
