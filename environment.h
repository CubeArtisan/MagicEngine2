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

class ObjectMovement {
public:
    xg::Guid sourceZone;
    xg::Guid destinationZone;
    xg::Guid object;
};

class AddPlayerCounter {
public:
    xg::Guid object;
    PlayerCounterType counterType;
    int amount;
};

class AddPermanentCounter {
public:
    xg::Guid object;
    PermanentCounterType counterType;
    int amount;
};

class ObjectCreation {
public:
    xg::Guid zone;
    Targetable created;
};

class Changeset {
    std::vector<ObjectMovement> moves;
    std::vector<AddPlayerCounter> playerCounters;
    std::vector<AddPermanentCounter> permanentCounters;
    std::vector<ObjectCreation> creation;
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
    virtual bool canPay(Environment& env);
    virtual Changeset payCost(Environment& env);
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

using EventHandler = std::function<std::vector<Changeset>(Changeset, Environment&)>;
class PropertyHandler {
};

enum ZoneType {
    HAND,
    GRAVEYARD,
    LIBRARY,
    BATTLEFIELD,
    STACK,
    EXILE,
    COMMAND
};

template<typename T>
class Zone : public Targetable {
public:
    ZoneType type;
    std::vector<T> objects;
};

template<typename T>
using PrivateZone = std::map<xg::Guid, Zone<T>>;

enum StepOrPhase {
    UNTAP,
    UPKEEP,
    DRAW,
    PRECOMBATMAIN,
    BEGINCOMBAT,
    DECLAREATTACKERS,
    DECLAREBLOCKERS,
    FIRSTSTRIKEDAMAGE,
    COMBATDAMAGE,
    ENDCOMBAT,
    POSTCOMBATMAIN,
    END,
    CLEANUP
};

class Environment {
public:
    std::map<xg::Guid, Targetable> gameObjects;

    PrivateZone<std::variant<Card, Token>> hand;
    PrivateZone<std::variant<Card, Token>> library;
    Zone<std::variant<Card, Token>> graveyard;
    Zone<std::variant<Card, Token>> battlefield;
    Zone<std::variant<Card, Token, Ability>> stack;
    Zone<std::variant<Card, Token>> exile;
    Zone<std::variant<Card, Token, Emblem>> command;

    std::map<xg::Guid, PermanentCounterType> PermanantCounters;
    std::map<xg::Guid, PlayerCounterType> PlayerCounters;
    std::map<xg::Guid, int> lifeTotals;

    std::vector<EventHandler> triggerHandlers;
    std::vector<EventHandler> replacementEffects;
    std::vector<PropertyHandler> propertyHandlers;

    std::vector<Changeset> changes;

    StepOrPhase currentPhase;

private:
};
