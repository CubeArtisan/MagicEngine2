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

class ActivatedAbility;
class Token;
class Emblem;
class Player;

class HasEffect {
public:
    virtual Changeset applyEffect(const Environment& env) = 0;
    bool is_legal(int pos, Targetable& target);
    std::vector<bool> are_legal(std::vector<std::reference_wrapper<Targetable>>);
    // CodeReview: Move to environment
    std::vector<xg::Guid> targets;
};

class CardToken : public Targetable, public HasEffect {
public:
    // CodeReview: Move to environment
    bool is_tapped;

    std::set<CardSuperType> superType;
    std::set<CardType> type;
    std::set<CardSubType> subType;

    unsigned int basePower;
    unsigned int baseToughness;
    unsigned int startingLoyalty;
    std::string name;
    unsigned int cmc;
    std::set<Color> colors;
    std::vector<std::reference_wrapper<Cost>> costs;
    std::vector<std::reference_wrapper<Cost>> additionalCosts;
    std::vector<std::shared_ptr<ActivatedAbility>> activatableAbilities;
};

class Card : public CardToken {
public:

    virtual bool canCast() = 0;
};

class Token : public CardToken {
public:
};

class Emblem : public Targetable {
public:
};
#endif
