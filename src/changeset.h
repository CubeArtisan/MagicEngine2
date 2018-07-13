#ifndef _CHANGESET_H_
#define _CHANGESET_H_

#include <variant>
#include <vector>

#include "guid.hpp"

#include "enum.h"
#include "mana.h"
#include "stateQuery.h"

class Changeset;
class Environment;

class EventHandler {
    virtual std::vector<std::reference_wrapper<Changeset>> handleEvent(Changeset, Environment&) = 0;
};
class StateQueryHandler {
    virtual StateQuery handleEvent(StateQuery, Environment&) = 0;
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
    bool changed;
    StepOrPhase starting;
};

struct Changeset {
    std::vector<ObjectMovement> moves;
    std::vector<AddPlayerCounter> playerCounters;
    std::vector<AddPermanentCounter> permanentCounters;
    std::vector<ObjectCreation> create;
    std::vector<RemoveObject> remove;
    std::vector<LifeTotalChange> lifeTotalChanges;
    std::vector<std::reference_wrapper<EventHandler>> eventsToAdd;
    std::vector<std::reference_wrapper<EventHandler>> eventsToRemove;
    std::vector<std::reference_wrapper<StateQueryHandler>> propertiesToAdd;
    std::vector<std::reference_wrapper<StateQueryHandler>> propertiesToRemove;
    std::vector<xg::Guid> loseTheGame;
    std::vector<AddMana> addMana;
    std::vector<RemoveMana> removeMana;
    StepOrPhaseChange phaseChange;
    bool millOut;

    Changeset operator+(Changeset& other);
    Changeset& operator+=(Changeset other);

    static Changeset drawCards(xg::Guid player, unsigned int amount, Environment& env);
};
#endif
