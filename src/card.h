#ifndef _CARD_H_
#define _CARD_H_

#include <functional>
#include <map>
#include <set>
#include <variant>
#include <vector>

#include "guid.hpp"

#include "changeset.h"
#include "cost.h"
#include "enum.h"
#include "mana.h"
#include "player.h"

class ActivatedAbility;
class Token;
class Emblem;
class Player;

class HasEffect {
public:
    virtual Changeset applyEffect(const Environment& env) = 0;
};

class Card : public Targetable, public HasEffect {
public:
    xg::Guid owner;
    // CodeReview: Move to environment
    std::vector<std::reference_wrapper<Targetable>> targets;
    bool is_tapped;

    bool is_legal(int pos, Targetable& target);
    std::vector<bool> are_legal(std::vector<std::reference_wrapper<Targetable>>);

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
    std::vector<std::reference_wrapper<ActivatedAbility>> activatableAbilities;
};

class Token : public Targetable, public HasEffect {
public:
    xg::Guid owner;
    
    Changeset applyEffect(const Environment& env);
};

class Emblem : public Targetable {
public:
    Player& owner;
};
#endif
