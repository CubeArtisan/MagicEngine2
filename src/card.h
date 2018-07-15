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

struct HasEffect {
public:
    virtual Changeset applyEffect(const Environment& env) = 0;
    virtual bool is_legal(int pos, Targetable& target) = 0;
    virtual std::vector<bool> are_legal(std::vector<std::reference_wrapper<Targetable>>) = 0;
    // CodeReview: Move to environment
    std::vector<xg::Guid> targets;
};

struct CardToken : public Targetable, public HasEffect {
public:
    Changeset applyEffect(const Environment& env); 
    bool is_legal(int, Targetable&) { return true; };
    std::vector<bool> are_legal(std::vector<std::reference_wrapper<Targetable>>) { return std::vector<bool>(); };
    // CodeReview: Move to environment
    bool is_tapped;

    std::set<CardSuperType> baseSuperTypes;
    std::set<CardType> baseTypes;
    std::set<CardSubType> baseSubTypes;

    unsigned int basePower;
    unsigned int baseToughness;
    unsigned int startingLoyalty;
    std::string name;
    unsigned int cmc;
    std::set<Color> baseColors;
    std::vector<std::shared_ptr<Cost>> costs;
    std::vector<std::shared_ptr<Cost>> additionalCosts;
    std::vector<std::shared_ptr<ActivatedAbility>> activatableAbilities;
    std::vector<std::function<Changeset&(Changeset&, const Environment&)>> applyEffects;
    CardToken(std::set<CardSuperType> superTypes, std::set<CardType> types, std::set<CardSubType> subTypes, int power,
              int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
              std::vector<std::shared_ptr<Cost>> costs, std::vector<std::shared_ptr<Cost>> additionalCosts,
              std::vector<std::shared_ptr<ActivatedAbility>> activatedAbilities,
              std::vector<std::function<Changeset&(Changeset&, const Environment&)>> applyAbilities);
};

struct Card : public CardToken {
public:
    bool canPlay(Player& player, Environment& env);

    Card(std::set<CardSuperType> superTypes, std::set<CardType> types, std::set<CardSubType> subTypes, int power,
         int toughness, int loyalty, std::string name, unsigned int cmc, std::set<Color> colors,
         std::vector<std::shared_ptr<Cost>> costs, std::vector<std::shared_ptr<Cost>> additionalCosts,
         std::vector<std::shared_ptr<ActivatedAbility>> activatedAbilities,
         std::vector<std::function<Changeset&(Changeset&, const Environment&)>> applyAbilities);
};

struct Token : public CardToken {
public:
};

struct Emblem : public Targetable {
public:
};
#endif
