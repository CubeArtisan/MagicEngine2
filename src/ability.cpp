#include "ability.h"
#include "targeting.h"

Ability::Ability(EffectValue effect, std::shared_ptr<const TargetingRestriction> targeting)
	: clone_inherit(effect, targeting)
{}

AbilityWithCost::AbilityWithCost(EffectValue effect, std::shared_ptr<const TargetingRestriction> targeting, std::vector<std::shared_ptr<const Cost>> costs,
								 std::vector<std::shared_ptr<const Cost>> additionalCosts)
    : Ability(effect, targeting), HasCost(costs, additionalCosts)
{}

ManaAbility::ManaAbility(Mana mana, std::vector<std::shared_ptr<const Cost>> costs, std::vector<std::shared_ptr<const Cost>> additionalCosts)
	: clone_inherit(LambdaEffects([=](xg::Guid source, const Environment& env)
					{
						Changeset changes;
						changes.addMana.push_back(AddMana{ env.getController(source), mana });
						return changes;
					}),
					std::shared_ptr<TargetingRestriction>(new NoTargets()), costs, additionalCosts), mana(mana)
{}

Changeset ManaAbility::applyEffect(const Environment&) const {
    Changeset changes;
    changes.addMana.push_back(AddMana{this->owner, mana});
    return changes;
}