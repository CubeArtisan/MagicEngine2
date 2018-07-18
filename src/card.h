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

struct CardToken : public Targetable, public HasEffect {
    bool is_tapped;

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
              std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyAbilities);
};

struct Card : public CardToken, public CostedEffect {
	Card();
	Card(std::shared_ptr<const std::set<CardSuperType>> superTypes, std::shared_ptr<const std::set<CardType>> types,
		 std::shared_ptr<const std::set<CardSubType>> subTypes, int power,
         int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
		 std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> activatedAbilities,
		 std::shared_ptr<const TargetingRestriction> targeting,
         std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyAbilities,
         std::vector<std::shared_ptr<const Cost>> costs, std::vector<std::shared_ptr<const Cost>> additionalCosts);
};

struct Token : public CardToken {
    Token(std::shared_ptr<const std::set<CardSuperType>> superTypes, std::shared_ptr<const std::set<CardType>> types,
		  std::shared_ptr<const std::set<CardSubType>> subTypes, int power,
          int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
          std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> activatedAbilities,
		  std::shared_ptr<const TargetingRestriction> targeting,
          std::vector<std::function<Changeset&(Changeset&, const Environment&, xg::Guid)>> applyAbilities);
};

struct Emblem : public Targetable {
};
#endif
