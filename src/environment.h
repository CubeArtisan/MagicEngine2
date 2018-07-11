#include <functional>
#include <map>
#include <set>
#include <variant>
#include <vector>

#include "guid.hpp"

#include "changeset.h"
#include "player.h"
#include "card.h"
#include "ability.h"

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
    
    std::map<xg::Guid, Mana> manaPools;

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
