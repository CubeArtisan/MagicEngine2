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
#include "enum.h"
#include "mana.h"
#include "util.h"

struct ActivatedAbility;
struct Token;
struct Emblem;
struct Player;
struct Card;
class TargetingRestriction;

struct HasEffect : public clone_inherit<abstract_method<HasEffect>> {
    virtual Changeset applyEffect(const Environment& env) const = 0;
	const std::shared_ptr<const TargetingRestriction> targeting;

	HasEffect(std::shared_ptr<const TargetingRestriction> targeting);
};

// All subclasses of this class must inherit from Targetable as well
struct HasCost : public clone_inherit<abstract_method<HasCost>> {
	const std::vector<std::shared_ptr<const Cost>> costs;
    const std::vector<std::shared_ptr<const Cost>> additionalCosts;
	virtual std::shared_ptr<const Cost> canPlay(const Player& player, const Environment& env) const;

	HasCost();
    HasCost(std::vector<std::shared_ptr<const Cost>> costs, std::vector<std::shared_ptr<const Cost>> additionalCosts);
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
		std::shared_ptr<const TargetingRestriction> targeting,
		std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
		std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes);
};

struct CardToken : public clone_inherit<abstract_method<CardToken>, HasEffectsAndAbilities, Targetable> {
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

	CardToken();
    CardToken(std::shared_ptr<const std::set<CardSuperType>> superTypes, std::shared_ptr<const std::set<CardType>> types,
			  std::shared_ptr<const std::set<CardSubType>> subTypes, int power,
              int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
			  std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> activatedAbilities,
			  std::shared_ptr<const TargetingRestriction> targeting,
              std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyAbilities,
			  std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
			  std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes);
};

struct CardTokenWithCost : public CardToken, public HasCost {
	CardTokenWithCost(std::shared_ptr<const std::set<CardSuperType>> superTypes, std::shared_ptr<const std::set<CardType>> types,
					  std::shared_ptr<const std::set<CardSubType>> subTypes, int power,
					  int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
					  std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> activatedAbilities,
					  std::shared_ptr<const TargetingRestriction> targeting,
					  std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyAbilities,
					  std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
					  std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes,
					  std::vector<std::shared_ptr<const Cost>> costs, std::vector<std::shared_ptr<const Cost>> additionalCosts);
};

struct Card : public clone_inherit<Card, CardTokenWithCost> {
	using clone_inherit<Card, CardTokenWithCost>::clone_inherit;

	// std::shared_ptr<Token> createTokenCopy();
};

struct Token : public clone_inherit<Token, CardToken> {
	using clone_inherit<Token, CardToken>::clone_inherit;
};

struct Emblem : public Targetable, public HasAbilities {
	Emblem(std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
		   std::vector<std::shared_ptr<StaticEffectHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes);
};
#endif
