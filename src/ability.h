#ifndef _ABILITY_H_
#define _ABILITY_H_

#include <functional>
#include <variant>
#include <vector>

#include "card.h"
#include "changeset.h"

struct Ability : public Targetable, public HasEffect {
    std::variant<std::shared_ptr<Card>, std::shared_ptr<Token>, std::shared_ptr<Emblem>> origin;
    std::set<Color> colors;

	Ability(std::shared_ptr<TargetingRestriction> targeting);
	
	virtual std::shared_ptr<Ability> clone() = 0;
};

struct ActivatedAbility : public Ability, public CostedEffect {
    ActivatedAbility(std::shared_ptr<TargetingRestriction> targeting, std::vector<std::shared_ptr<Cost>> costs,
					 std::vector<std::shared_ptr<Cost>> additionalCosts={});
};

struct ManaAbility : public ActivatedAbility {
    Changeset applyEffect(const Environment& env);

    ManaAbility(Mana mana, std::vector<std::shared_ptr<Cost>> costs, std::vector<std::shared_ptr<Cost>> additionalCosts={});

private:
    Mana mana;

	virtual std::shared_ptr<Ability> clone();
};

#endif
