#include "ability.h"
#include "targeting.h"

Ability::Ability(EffectValue effect, std::shared_ptr<const TargetingRestriction> targeting)
	: clone_inherit(effect, targeting)
{}

AbilityWithCost::AbilityWithCost(EffectValue effect, std::shared_ptr<const TargetingRestriction> targeting, std::vector<CostValue> costs,
								 std::vector<CostValue> additionalCosts, std::shared_ptr<const std::set<ZoneType>> playableFrom)
    : Ability(effect, targeting), HasCost(costs, additionalCosts, playableFrom)
{}

ManaAbility::ManaAbility(Mana mana, std::vector<CostValue> costs, std::vector<CostValue> additionalCosts, std::shared_ptr<const std::set<ZoneType>> playableFrom)
	: clone_inherit(LambdaEffects([=](xg::Guid s, const Environment& env)
					{
						Changeset changes;
						changes.addMana.push_back(AddMana{ env.getController(s), mana });
						return changes;
					}),
					std::shared_ptr<TargetingRestriction>(new NoTargets()), costs, additionalCosts), mana(mana)
{}

Changeset ManaAbility::applyEffect(const Environment&) const {
    Changeset changes;
    changes.addMana.push_back(AddMana{this->owner, mana});
    return changes;
}