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
#include "mana.h"
#include "player.h"

enum CardSuperType {
    LEGENDARY,
    BASIC,
    SNOW,
    WORLD
};

enum CardType {
    CREATURE,
    LAND,
    PLANESWALKER,
    ARTIFACT,
    ENCHANTMENT,
    TRIBAL,
    INSTANT,
    SORCERY
};

enum CardSubType {
    FIRSTCREATURETYPE,
    DJINN,
    HUMAN,
    PIRATE,
    SHAPESHIFTER,
    SIREN,
    THOPTER,
    WIZARD,
    FUNGUS,
    SAPROLING,
    LASTCREATURETYPE,

    FIRSTLANDTYPE,
    PLAINS,
    ISLAND,
    SWAMP,
    MOUNTAIN,
    FOREST,
    DESERT,
    LASTLANDTYPE,
    
    FIRSTARTIFACTTYPE,
    VEHICLE,
    LASTARTIFACTTYPE,
    
    FIRSTENCHANTMENTTYPE,
    AURA,
    SAGA,
    LASTENCHANTMENTTYPE,
    
    FIRSTSORCERYTYPE,
    ARCANE,
    TRAP,
    LASTSORCERYTYPE,
    
    FIRSTTRIBALTYPE = FIRSTCREATURETYPE,
    LASTTRIBALTYPE = LASTCREATURETYPE,
    
    FIRSTINSTANTTYPE = FIRSTSORCERYTYPE,
    LASTINSTANTTYPE = LASTSORCERYTYPE
};

class ActivatedAbility;
class Token;
class Emblem;

class Card : public Targetable {
public:
    Player& owner;
    Player& controller;
    std::vector<std::reference_wrapper<Targetable>> targets;
    std::variant<std::reference_wrapper<Card>, std::reference_wrapper<Token>, std::reference_wrapper<Emblem>> source;

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
    std::vector<Cost> costs;
    std::vector<Cost> additionalCosts;
    std::vector<std::reference_wrapper<ActivatedAbility>> activatableAbilities;
    
    Changeset apply_effect(const Environment& env);
};

class Token : public Targetable {
public:
    Player& owner;
    
    Changeset apply_effect(const Environment& env);
};

class Emblem : public Targetable {
public:
    Player& owner;
};
#endif
