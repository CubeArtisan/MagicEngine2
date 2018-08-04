#ifndef _ABILITY_H_
#define _ABILITY_H_

#include <functional>
#include <variant>
#include <vector>

#include "card.h"
#include "changeset.h"

struct Ability : public Targetable, public HasEffect {
    std::variant<std::shared_ptr<const Card>, std::shared_ptr<const Token>, std::shared_ptr<const Emblem>> source;
    const std::set<Color> colors;

	Ability(std::shared_ptr<const TargetingRestriction> targeting);
	
	Changeset operator()(const Environment& env) const;
	virtual std::shared_ptr<Ability> clone() const = 0;
};

struct ActivatedAbility : public Ability, public HasCost {
    ActivatedAbility(std::shared_ptr<const TargetingRestriction> targeting, std::vector<std::shared_ptr<const Cost>> costs,
					 std::vector<std::shared_ptr<const Cost>> additionalCosts={});
};

struct ManaAbility : public ActivatedAbility {
    Changeset applyEffect(const Environment& env) const;

    ManaAbility(Mana mana, std::vector<std::shared_ptr<const Cost>> costs, std::vector<std::shared_ptr<const Cost>> additionalCosts={});

	virtual std::shared_ptr<Ability> clone() const;

private:
    Mana mana;
};

#endif
