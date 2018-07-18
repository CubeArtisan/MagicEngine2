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

struct ActivatedAbility;
struct Token;
struct Emblem;
struct Player;
struct Card;
class TargetingRestriction;

struct HasEffect {
    virtual Changeset applyEffect(const Environment& env) const = 0;
	const std::shared_ptr<const TargetingRestriction> targeting;

	HasEffect(std::shared_ptr<const TargetingRestriction> targeting);
};

// All subclasses of this class must inherit from Targetable as well
struct CostedEffect {
	std::variant<std::shared_ptr<const Card>, std::shared_ptr<const Token>> source;

	const std::vector<std::shared_ptr<const Cost>> costs;
    const std::vector<std::shared_ptr<const Cost>> additionalCosts;
	virtual std::shared_ptr<const Cost> canPlay(const Player& player, const Environment& env) const;

	CostedEffect();
    CostedEffect(std::vector<std::shared_ptr<const Cost>> costs, std::vector<std::shared_ptr<const Cost>> additionalCosts,
		         std::variant<std::shared_ptr<const Card>, std::shared_ptr<const Token>> source);
};

struct HasAbilities {
	const std::vector<std::shared_ptr<EventHandler>> replacementEffects;
	const std::vector<std::shared_ptr<TriggerHandler>> triggerEffects;
	const std::vector<std::shared_ptr<StateQueryHandler>> staticEffects;
	const std::vector<size_t> thisOnlyReplacementIndexes;

	HasAbilities(std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
				 std::vector<std::shared_ptr<StateQueryHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes);
};

struct CardToken : public Targetable, public HasEffect, public HasAbilities {
    bool is_tapped;
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
			  std::vector<std::shared_ptr<StateQueryHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes);
};

struct Card : public CardToken, public CostedEffect {
	Card();
	Card(std::shared_ptr<const std::set<CardSuperType>> superTypes, std::shared_ptr<const std::set<CardType>> types,
		 std::shared_ptr<const std::set<CardSubType>> subTypes, int power,
         int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
		 std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> activatedAbilities,
		 std::shared_ptr<const TargetingRestriction> targeting,
         std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyAbilities,
		 std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
		 std::vector<std::shared_ptr<StateQueryHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes,
         std::vector<std::shared_ptr<const Cost>> costs, std::vector<std::shared_ptr<const Cost>> additionalCosts);

	// std::shared_ptr<Token> createTokenCopy();
};

struct Token : public CardToken {
    Token(std::shared_ptr<const std::set<CardSuperType>> superTypes, std::shared_ptr<const std::set<CardType>> types,
		  std::shared_ptr<const std::set<CardSubType>> subTypes, int power,
          int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
          std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> activatedAbilities,
		  std::shared_ptr<const TargetingRestriction> targeting,
          std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyAbilities,
		  std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
		  std::vector<std::shared_ptr<StateQueryHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes);
};

struct Emblem : public Targetable, public HasAbilities {
	Emblem(std::vector<std::shared_ptr<EventHandler>> replacementEffects, std::vector<std::shared_ptr<TriggerHandler>> triggerEffects,
		   std::vector<std::shared_ptr<StateQueryHandler>> staticEffects, std::vector<size_t> thisOnlyReplacementIndexes);
};
#endif
