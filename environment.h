#include <functional>
#include <map>
#include <set>
#include <variant>
#include <vector>

#include "guid.hpp"

class Targetable {
public:
    xg::Guid id;
};

enum PlayerCounterType {
    POISONCOUNTER
};

enum PermanentCounterType {
    PLUSONEPLUSONECOUNTER
};

class Environment;

class Changeset {
};

class Strategy {
};

class Player : public Targetable {
public:
    Strategy strategy;
};


class Emblem : public Targetable {
public:
    Player& owner;
};

class Token : public Targetable {
public:
    Player& owner;
    
    Changeset apply_effect(const Environment& env);
};
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

enum Color {
    WHITE,
    BLUE,
    BLACK,
    RED,
    GREEN
};

class Cost {
public:
    virtual bool canPay(Environement& env);
    virtual Changeset payCost(Environement& env);
};

class Ability;

class Card : public Targetable {
public:
    Player& owner;
    Player& controller;

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
    std::vector<std::reference_wrapper<Ability>> activatableAbilities;
    
    Changeset apply_effect(const Environment& env);
};

class Ability : public Targetable {
public:
    std::vector<std::reference_wrapper<Targetable>> targets;
    std::variant<std::reference_wrapper<Card>, std::reference_wrapper<Token>, std::reference_wrapper<Emblem>> source;
    Player& controller;

    bool is_legal(int pos, Targetable& target);
    std::vector<bool> are_legal(std::vector<std::reference_wrapper<Targetable>>);

    Changeset apply_effect(const Environment& env);
};

class EventHandler {
};

class PropertyHandler {
};

template<typename T>
class PrivateZone : public Targetable {
    public:
        std::map<xg::Guid, std::vector<T>> objects;
};

template<typename T>
class PublicZone : public Targetable {
    public:
        std::vector<T> objects;
};

class Environment {
public:
    PrivateZone<std::variant<Card, Token>> hand;
    PrivateZone<std::variant<Card, Token>> library;
    PublicZone<std::variant<Card, Token>> graveyard;
    PublicZone<std::variant<Card, Token>> battlefield;
    PublicZone<std::variant<Card, Token, Ability>> stack;
    PublicZone<std::variant<Card, Token>> exile;
    PublicZone<std::variant<Card, Token, Emblem>> commandZone;

    std::map<xg::Guid, PermanentCounterType> PermanantCounters;
    std::map<xg::Guid, PlayerCounterType> PlayerCounters;
    std::map<xg::Guid, int> lifeTotals;

    std::vector<EventHandler> triggerHandlers;
    std::vector<EventHandler> replacementEffects;
    std::vector<PropertyHandler> propertyHandlers;

    std::vector<Changeset> changes;

private:
};
