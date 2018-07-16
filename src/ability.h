#ifndef _ABILITY_H_
#define _ABILITY_H_

#include <functional>
#include <variant>
#include <vector>

#include "card.h"
#include "changeset.h"

struct Ability : public Targetable, public HasEffect {
    std::variant<std::shared_ptr<Card>, std::shared_ptr<Token>, std::shared_ptr<Emblem>> source;
    std::set<Color> colors;
};

struct ActivatedAbility : public Ability, public CostedEffect {
    ActivatedAbility(std::vector<std::shared_ptr<Cost>> costs, std::vector<std::shared_ptr<Cost>> additionalCosts={});
};

struct ManaAbility : public ActivatedAbility {
    Changeset applyEffect(const Environment& env);

    ManaAbility(Mana mana, std::vector<std::shared_ptr<Cost>> costs, std::vector<std::shared_ptr<Cost>> additionalCosts={});

private:
    Mana mana;
};

#endif
