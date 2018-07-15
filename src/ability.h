#ifndef _ABILITY_H_
#define _ABILITY_H_

#include <functional>
#include <variant>
#include <vector>

#include "card.h"
#include "changeset.h"

struct Ability : public Targetable, public HasEffect {
    std::variant<std::reference_wrapper<Card>, std::reference_wrapper<Token>, std::reference_wrapper<Emblem>> source;
    std::set<Color> colors;
};

struct ActivatedAbility : public Ability {
    std::vector<std::reference_wrapper<Cost>> costs;
};

#endif
