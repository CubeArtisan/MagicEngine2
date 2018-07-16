#include "ability.h"

ActivatedAbility::ActivatedAbility(std::vector<std::shared_ptr<Cost>> costs, std::vector<std::shared_ptr<Cost>> additionalCosts)
    : CostedEffect(costs, additionalCosts, std::shared_ptr<Card>())
{}

ManaAbility::ManaAbility(Mana mana, std::vector<std::shared_ptr<Cost>> costs, std::vector<std::shared_ptr<Cost>> additionalCosts)
    : ActivatedAbility(costs, additionalCosts), mana(mana)
{}

Changeset ManaAbility::applyEffect(const Environment&) {
    Changeset changes;
    changes.addMana.push_back(AddMana{this->owner, mana});
    return changes;
}
