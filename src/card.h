#ifndef _CARD_H_
#define _CARD_H_

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <variant>
#include <vector>

#include "guid.hpp"

#include "changeset.h"
#include "cost.h"
#include "effect.h"
#include "enum.h"
#include "mana.h"
#include "util.h"

struct ActivatedAbility;
struct Token;
struct Emblem;
struct Player;
struct Card;
class TargetingRestriction;

struct HasEffect : public clone_inherit<abstract_method<HasEffect>, Targetable> {
	EffectValue effect;
	const std::shared_ptr<const TargetingRestriction> targeting;

	std::optional<Changeset> getChangeset(const Environment& env);
	void resetEffect();

	HasEffect(EffectValue effect, std::shared_ptr<const TargetingRestriction> targeting);
};

// All subclasses of this class must inherit from Targetable as well
struct HasCost : public clone_inherit<abstract_method<HasCost>>, public std::enable_shared_from_this<HasCost> {
	const std::vector<CostValue> costs;
    const std::vector<CostValue> additionalCosts;
	const std::shared_ptr<const std::set<ZoneType>> playableFrom;
	virtual std::optional<CostValue> canPlay(const Player& player, const Environment& env) const;

	HasCost();
    HasCost(std::vector<CostValue> costs, std::vector<CostValue> additionalCosts, std::shared_ptr<const std::set<ZoneType>> playableFrom);
};

struct HasAbilities : public clone_inherit<abstract_method<HasAbilities>> {
	const std::vector<std::shared_ptr<EventHandler>> replacementEffects;
	const std::vector<std::shared_ptr<TriggerHandler>> triggerEffects;
	const std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects;
	const std::vector<size_t> thisOnlyReplacementIndexes;

	HasAbilities(std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
				 std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes);
};

struct HasEffectsAndAbilities : public HasEffect, public HasAbilities {
	HasEffectsAndAbilities(
		EffectValue effect,
		std::shared_ptr<const TargetingRestriction> targeting,
		std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
		std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes);
};

struct CardToken : public clone_inherit<abstract_method<CardToken>, HasEffectsAndAbilities> {
    bool isTapped;
	bool isSummoningSick;
	// Implement phased out

    const std::shared_ptr<const std::set<CardSuperType>> baseSuperTypes;
    const std::shared_ptr<const std::set<CardType>> baseTypes;
    const std::shared_ptr<const std::set<CardSubType>> baseSubTypes;
    // CodeReview: Implement devotion
    const int basePower;
    const int baseToughness;
    const unsigned int startingLoyalty;
    const std::string name;
    const unsigned int cmc;
    const std::set<Color> baseColors;
	const std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> activatableAbilities;
    const std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyEffects;

	Changeset applyEffect(const Environment& env) const;

    CardToken(std::shared_ptr<const std::set<CardSuperType>> superTypes, std::shared_ptr<const std::set<CardType>> types,
			  std::shared_ptr<const std::set<CardSubType>> subTypes, int power,
              int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
			  std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> activatedAbilities,
			  EffectValue effect,
			  std::shared_ptr<const TargetingRestriction> targeting,
			  std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
			  std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes);
};

struct CardTokenWithCost : public CardToken, public HasCost {
	CardTokenWithCost(std::shared_ptr<const std::set<CardSuperType>> superTypes, std::shared_ptr<const std::set<CardType>> types,
					  std::shared_ptr<const std::set<CardSubType>> subTypes, int power,
					  int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
					  std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> activatedAbilities,
					  EffectValue effect,
					  std::shared_ptr<const TargetingRestriction> targeting, std::shared_ptr<const std::set<ZoneType>> playableFrom,
					  std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
					  std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes,
					  std::vector<CostValue> costs, std::vector<CostValue> additionalCosts);
};

struct Card : public clone_inherit<Card, CardTokenWithCost> {
	using clone_inherit<Card, CardTokenWithCost>::clone_inherit;
};

struct Token : public clone_inherit<Token, CardToken> {
	using clone_inherit<Token, CardToken>::clone_inherit;
};

struct Emblem : public clone_inherit<Emblem, HasAbilities, Targetable> {
	Emblem(std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
		   std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes);
};
#endif
