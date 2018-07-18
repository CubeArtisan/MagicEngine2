#include "ability.h"
#include "targeting.h"

Ability::Ability(std::shared_ptr<const TargetingRestriction> targeting)
	: HasEffect(targeting)
{}

ActivatedAbility::ActivatedAbility(std::shared_ptr<const TargetingRestriction> targeting, std::vector<std::shared_ptr<const Cost>> costs,
								   std::vector<std::shared_ptr<const Cost>> additionalCosts)
    : Ability(targeting), CostedEffect(costs, additionalCosts, std::shared_ptr<Card>())
{}

ManaAbility::ManaAbility(Mana mana, std::vector<std::shared_ptr<const Cost>> costs, std::vector<std::shared_ptr<const Cost>> additionalCosts)
    : ActivatedAbility(std::shared_ptr<TargetingRestriction>(new NoTargets()), costs, additionalCosts), mana(mana)
{}

Changeset ManaAbility::applyEffect(const Environment&) const {
    Changeset changes;
    changes.addMana.push_back(AddMana{this->owner, mana});
    return changes;
}

std::shared_ptr<Ability> ManaAbility::clone() const {
	return std::shared_ptr<Ability>(new ManaAbility(*this));
}