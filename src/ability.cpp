#include "ability.h"
#include "targeting.h"

Ability::Ability(std::shared_ptr<const TargetingRestriction> targeting)
	: clone_inherit(targeting)
{}

Changeset Ability::operator()(const Environment& env) const {
	return this->applyEffect(env);
}

AbilityWithCost::AbilityWithCost(std::shared_ptr<const TargetingRestriction> targeting, std::vector<std::shared_ptr<const Cost>> costs,
								 std::vector<std::shared_ptr<const Cost>> additionalCosts)
    : Ability(targeting), HasCost(costs, additionalCosts)
{}

ManaAbility::ManaAbility(Mana mana, std::vector<std::shared_ptr<const Cost>> costs, std::vector<std::shared_ptr<const Cost>> additionalCosts)
    : clone_inherit(std::shared_ptr<TargetingRestriction>(new NoTargets()), costs, additionalCosts), mana(mana)
{}

Changeset ManaAbility::applyEffect(const Environment&) const {
    Changeset changes;
    changes.addMana.push_back(AddMana{this->owner, mana});
    return changes;
}