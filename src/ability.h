#ifndef _ABILITY_H_
#define _ABILITY_H_

#include <functional>
#include <variant>
#include <vector>

#include "card.h"
#include "changeset.h"

struct Ability : public Targetable, public HasEffect {
    std::variant<std::shared_ptr<Card>, std::shared_ptr<Token>, std::shared_ptr<Emblem>> origin;
    const std::set<Color> colors;

	Ability(std::shared_ptr<const TargetingRestriction> targeting);
	
	virtual std::shared_ptr<Ability> clone() const = 0;
};

struct ActivatedAbility : public Ability, public CostedEffect {
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
