#include "card.h"
#include "targeting.h"

HasEffect::HasEffect(std::shared_ptr<const TargetingRestriction> targeting)
	: targeting(targeting)
{}

HasCost::HasCost() {}

HasCost::HasCost(std::vector<std::shared_ptr<const Cost>> costs, std::vector<std::shared_ptr<const Cost>> additionalCosts)
    : costs(costs), additionalCosts(additionalCosts)
{}

std::shared_ptr<const Cost> HasCost::canPlay(const Player& player, const Environment& env) const {
    // CodeReview: Can choose targets if *this is a HasEffect - Might not be neccesary

	SourceType source;
	if (const Card* self = dynamic_cast<const Card*>(this)) {
		// CodeReview: Check are in a valid zone to be played from
		if (!env.goodTiming(self->id)) return std::shared_ptr<Cost>();
		source = std::dynamic_pointer_cast<const Card>(env.gameObjects.at(self->id));
	}
	if (const Ability* self = dynamic_cast<const Ability*>(this)) {
		// CodeReview: Check source is in a valid zone to activate from
		source = self->source;
	}
	// CodeReview: Figure out timing for abilities

    for(const std::shared_ptr<const Cost>& cost : this->costs) {
        if(cost->canPay(player, env, source)) return cost;
    }
    return std::shared_ptr<Cost>();
}

Changeset CardToken::applyEffect(const Environment& env) const {
    Changeset change;
    for(auto& c : this->applyEffects) c(change, env, this->id);
    return change;
}

HasAbilities::HasAbilities(std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
						   std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes)
	: replacementEffects(replacementEffects), triggerEffects(triggerEffects), staticEffects(staticEffects), thisOnlyReplacementIndexes(thisOnlyReplacementIndexes)
{}

CardToken::CardToken()
	: CardToken(std::shared_ptr<std::set<CardSuperType>>(new std::set<CardSuperType>{}), std::shared_ptr<std::set<CardType>>(new std::set<CardType>{}),
		std::shared_ptr<std::set<CardSubType>>(new std::set<CardSubType>{}), 0,
		0, 0, "Unnamed", 0, std::set<Color>{},
		std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>>(new std::vector<std::shared_ptr<const ActivatedAbility>>{}),
		std::shared_ptr<TargetingRestriction>(new NoTargets()),
		std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>>{},
		std::vector<std::shared_ptr<EventHandler>>{}, std::vector<std::shared_ptr<TriggerHandler>>{},
		std::vector<std::shared_ptr<StaticEffectHandler>>{}, std::vector<size_t>{})
{}

HasEffectsAndAbilities::HasEffectsAndAbilities(std::shared_ptr<const TargetingRestriction> targeting,
											   std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
											   std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes)
	: HasEffect(targeting), HasAbilities(replacementEffects, triggerEffects, staticEffects, thisOnlyReplacementIndexes)
{}

CardToken::CardToken(std::shared_ptr<const std::set<CardSuperType>> superTypes, std::shared_ptr<const std::set<CardType>> types,
					 std::shared_ptr<const std::set<CardSubType>> subTypes, int power,
                     int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
					 std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> activatedAbilities,
                     std::shared_ptr<const TargetingRestriction> targeting,
					 std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyEffects,
					 std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
					 std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes)
    : clone_inherit(targeting, replacementEffects, triggerEffects, staticEffects, thisOnlyReplacementIndexes), baseSuperTypes(superTypes),
	  baseTypes(types), baseSubTypes(subTypes), basePower(power), baseToughness(toughness), startingLoyalty(loyalty), name(name), cmc(cmc),
	  baseColors(colors), activatableAbilities(activatedAbilities), applyEffects(applyEffects)
    {}

CardTokenWithCost::CardTokenWithCost(std::shared_ptr<const std::set<CardSuperType>> superTypes, std::shared_ptr<const std::set<CardType>> types,
	       std::shared_ptr<const std::set<CardSubType>> subTypes, int power,
           int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
		   std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> activatedAbilities,
		   std::shared_ptr<const TargetingRestriction> targeting,
           std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyAbilities,
		   std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
		   std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes,
           std::vector<std::shared_ptr<const Cost>> costs, std::vector<std::shared_ptr<const Cost>> additionalCosts)
    : CardToken(superTypes, types, subTypes, power, toughness, loyalty, name, cmc, colors, activatedAbilities,
                targeting, applyAbilities, replacementEffects, triggerEffects, staticEffects, thisOnlyReplacementIndexes),
	  HasCost(costs, additionalCosts)
    {}

Emblem::Emblem(std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
			   std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes)
	: HasAbilities(replacementEffects, triggerEffects, staticEffects, thisOnlyReplacementIndexes)
{}