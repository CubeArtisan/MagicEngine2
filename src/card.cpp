#include "ability.h"
#include "card.h"
#include "cost.h"
#include "targeting.h"

HasEffect::HasEffect(EffectValue effect, std::shared_ptr<const TargetingRestriction> targeting)
	: effect(effect), targeting(targeting)
{}

std::optional<Changeset> HasEffect::getChangeset(const Environment& env) {
	return this->effect.getChangeset(this->id, env);
}

void HasEffect::resetEffect() {
	this->effect.reset();
}

HasCost::HasCost() {}

HasCost::HasCost(std::vector<CostValue> costs, std::vector<CostValue> additionalCosts, std::shared_ptr<const std::set<ZoneType>> playableFrom)
    : costs(costs), additionalCosts(additionalCosts), playableFrom(playableFrom)
{}

bool inZone(SourceType source, ZoneType zone, const Environment& env) {
	std::shared_ptr<const Targetable> s = getBaseClassPtr<const Targetable>(source);
	switch (zone) {
	case BATTLEFIELD:
		return (bool)env.battlefield->findObject(s->id);
	case EXILE:
		return (bool)env.exile->findObject(s->id);
	case HAND:
		return (bool)env.hands.at(s->owner)->findObject(s->id);
	case GRAVEYARD:
		return (bool)env.graveyards.at(s->owner)->findObject(s->id);
	case LIBRARY:
		return (bool)env.libraries.at(s->owner)->findObject(s->id);
	case COMMAND:
		return (bool)env.command->findObject(s->id);
	case STACK:
		return (bool)env.stack->findObject(s->id);
	}
	return false;
}

std::optional<CostValue> HasCost::canPlay(const Player& player, const Environment& env) const {
    // CodeReview: Can choose targets if *this is a HasEffect - Might not be neccesary

	SourceType source;
	if (const Card* self = dynamic_cast<const Card*>(this)) {
		if (!env.goodTiming(self->id)) return std::nullopt;
		source = std::dynamic_pointer_cast<const Card>(env.gameObjects.at(self->id));
	}
	else if (const Ability* self2 = dynamic_cast<const Ability*>(this)) {
		source = self2->source;
	}
	else {
		throw "Cannot determine source";
	}

	bool inGoodZone = false;
	for (ZoneType zone : *this->playableFrom) {
		if (inZone(source, zone, env)) {
			inGoodZone = true;
			break;
		}
	}
	if (!inGoodZone) return std::nullopt;
	// CodeReview: Figure out timing for abilities

    for(const CostValue& cost : this->costs) {
        if(cost.value().canPay(player, env, source)) return cost;
    }
    return std::nullopt;
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

HasEffectsAndAbilities::HasEffectsAndAbilities(EffectValue effect, std::shared_ptr<const TargetingRestriction> targeting,
											   std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
											   std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes)
	: HasEffect(effect, targeting), HasAbilities(replacementEffects, triggerEffects, staticEffects, thisOnlyReplacementIndexes)
{}

CardToken::CardToken(std::shared_ptr<const std::set<CardSuperType>> superTypes, std::shared_ptr<const std::set<CardType>> types,
					 std::shared_ptr<const std::set<CardSubType>> subTypes, int power,
                     int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
					 std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> activatedAbilities,
                     EffectValue effect,
					 std::shared_ptr<const TargetingRestriction> targeting,
					 std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
					 std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes)
    : clone_inherit(effect, targeting, replacementEffects, triggerEffects, staticEffects, thisOnlyReplacementIndexes), baseSuperTypes(superTypes),
	  baseTypes(types), baseSubTypes(subTypes), basePower(power), baseToughness(toughness), startingLoyalty(loyalty), name(name), cmc(cmc),
	  baseColors(colors), activatableAbilities(activatedAbilities)
    {}

CardTokenWithCost::CardTokenWithCost(std::shared_ptr<const std::set<CardSuperType>> superTypes, std::shared_ptr<const std::set<CardType>> types,
	       std::shared_ptr<const std::set<CardSubType>> subTypes, int power,
           int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
		   std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> activatedAbilities,
		   EffectValue effect,
		   std::shared_ptr<const TargetingRestriction> targeting, std::shared_ptr<const std::set<ZoneType>>,
		   std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
		   std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes,
           std::vector<CostValue> costs, std::vector<CostValue> additionalCosts)
    : CardToken(superTypes, types, subTypes, power, toughness, loyalty, name, cmc, colors, activatedAbilities,
                effect, targeting, replacementEffects, triggerEffects, staticEffects, thisOnlyReplacementIndexes),
	  HasCost(costs, additionalCosts, playableFrom)
    {}

Emblem::Emblem(std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
			   std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes)
	: clone_inherit(replacementEffects, triggerEffects, staticEffects, thisOnlyReplacementIndexes)
{}