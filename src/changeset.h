#ifndef _CHANGESET_H_
#define _CHANGESET_H_

#include <variant>
#include <vector>

#include "guid.hpp"

#include "mana.h"

class Changeset;
class Environment;

class EventHandler {
    std::vector<std::reference_wrapper<Changeset>> handleEvent(Changeset, Environment&);
};
class PropertyHandler {
};


struct Targetable {
    // This is mutable state
    xg::Guid id;

    Targetable();
};

template<typename T, typename Variant>
T& getBaseClass(Variant& variant){
    auto visitor = [](T& base) -> T&{ return base; };
    return std::visit(visitor, variant);
}

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

enum PlayerCounterType {
    POISONCOUNTER,
    EXPERIENCECOUNTER,
    ENERGYCOUNTER
};

enum PermanentCounterType {
    PLUSONEPLUSONECOUNTER
};

struct ObjectMovement {
    xg::Guid object;
    xg::Guid sourceZone;
    xg::Guid destinationZone;
    xg::Guid newObject { xg::newGuid() };
};

struct AddPlayerCounter {
    xg::Guid object;
    PlayerCounterType counterType;
    int amount;
};

struct AddPermanentCounter {
    xg::Guid object;
    PermanentCounterType counterType;
    int amount;
};

struct AddMana {
    xg::Guid player;
    Mana amount;
};

struct RemoveMana {
    xg::Guid player;
    Mana amount;
};

struct ObjectCreation {
    xg::Guid zone;
    Targetable created;
};

struct ControlChange {
    xg::Guid object;
    xg::Guid originalController;
    xg::Guid newController;
};

struct LifeTotalChange {
    xg::Guid player;
    int oldValue;
    int newValue;
};

struct RemoveObject {
    xg::Guid object;
    xg::Guid zone;
};

struct StepOrPhaseChange {
    StepOrPhase starting;
};

struct Changeset {
    std::vector<ObjectMovement> moves;
    std::vector<AddPlayerCounter> playerCounters;
    std::vector<AddPermanentCounter> permanentCounters;
    std::vector<ObjectCreation> create;
    std::vector<RemoveObject> remove;
    std::vector<ControlChange> controlChanges;
    std::vector<LifeTotalChange> lifeTotalChanges;
    std::vector<EventHandler> eventsToAdd;
    std::vector<EventHandler> eventsToRemove;
    std::vector<PropertyHandler> propertiesToAdd;
    std::vector<PropertyHandler> propertiesToRemove;
    AddMana addMana;
    RemoveMana removeMana;
    bool millOut;
    std::vector<xg::Guid> loseTheGame;

    static Changeset drawCards(xg::Guid player, unsigned int amount, Environment& env);
};
#endif
