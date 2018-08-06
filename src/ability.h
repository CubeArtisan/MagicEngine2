#ifndef _ABILITY_H_
#define _ABILITY_H_

#include <functional>
#include <variant>
#include <vector>

#include "card.h"
#include "changeset.h"

struct Ability : public clone_inherit<abstract_method<Ability>, HasEffect, Targetable> {
    std::variant<std::shared_ptr<const Card>, std::shared_ptr<const Token>, std::shared_ptr<const Emblem>> source;
    const std::set<Color> colors;

	Ability(std::shared_ptr<const TargetingRestriction> targeting);
	
	Changeset operator()(const Environment& env) const;
};

struct AbilityWithCost : public Ability, public HasCost {
	AbilityWithCost(std::shared_ptr<const TargetingRestriction> targeting, std::vector<std::shared_ptr<const Cost>> costs,
					std::vector<std::shared_ptr<const Cost>> additionalCosts = {});
};

struct ActivatedAbility : public clone_inherit<abstract_method<ActivatedAbility>, AbilityWithCost> {
	using clone_inherit<abstract_method<ActivatedAbility>, AbilityWithCost>::clone_inherit;
};

struct ManaAbility : public clone_inherit<ManaAbility, ActivatedAbility> {
    Changeset applyEffect(const Environment& env) const;

    ManaAbility(Mana mana, std::vector<std::shared_ptr<const Cost>> costs, std::vector<std::shared_ptr<const Cost>> additionalCosts={});

private:
    Mana mana;
};

#endif
