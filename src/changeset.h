#ifndef _CHANGESET_H_
#define _CHANGESET_H_

#include <memory>
#include <variant>
#include <vector>

#include "guid.hpp"

#include "enum.h"
#include "mana.h"
#include "stateQuery.h"

struct Changeset;
struct Environment;

struct Targetable {
    // This is mutable state
    xg::Guid id;
    xg::Guid owner;

    Targetable();
};

class EventHandler : public Targetable {
public:
    virtual std::vector<std::reference_wrapper<Changeset>> handleEvent(Changeset, Environment&) = 0;
    bool operator==(EventHandler& other) {
        return this->id == other.id;
    }
};
class StateQueryHandler : public Targetable {
public:
    virtual StateQuery handleEvent(StateQuery, Environment&) = 0;
    bool operator==(StateQueryHandler& other) {
        return this->id == other.id;
    }
};

template<typename T, typename Variant>
T& getBaseClass(Variant& variant){
    auto visitor = [](T& base) -> T&{ return base; };
    return std::visit(visitor, variant);
}

template<typename T, typename Variant>
std::shared_ptr<T> getBaseClassPtr(Variant& variant){
    auto visitor = [](auto base) -> std::shared_ptr<T>{ return std::dynamic_pointer_cast<T>(base); };
    return std::visit(visitor, variant);
}

struct ObjectMovement {
    xg::Guid object;
    xg::Guid sourceZone;
    xg::Guid destinationZone;
    xg::Guid newObject { xg::newGuid() };
};

struct AddPlayerCounter {
    xg::Guid player;
    PlayerCounterType counterType;
    int amount;
};

struct AddPermanentCounter {
    xg::Guid player;
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
    std::shared_ptr<Targetable> created;
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

struct DamageToTarget {
    xg::Guid target;
    unsigned int amount;
};

struct TapTarget {
    xg::Guid target;
    bool tap;
};

struct Changeset {
    std::vector<ObjectMovement> moves;
    std::vector<AddPlayerCounter> playerCounters;
    std::vector<AddPermanentCounter> permanentCounters;
    std::vector<ObjectCreation> create;
    std::vector<RemoveObject> remove;
    std::vector<LifeTotalChange> lifeTotalChanges;
    std::vector<std::shared_ptr<EventHandler>> eventsToAdd;
    std::vector<std::shared_ptr<EventHandler>> eventsToRemove;
    std::vector<std::shared_ptr<StateQueryHandler>> propertiesToAdd;
    std::vector<std::shared_ptr<StateQueryHandler>> propertiesToRemove;
    std::vector<xg::Guid> loseTheGame;
    std::vector<AddMana> addMana;
    std::vector<RemoveMana> removeMana;
    std::vector<DamageToTarget> damage;
    std::vector<TapTarget> tap;
    StepOrPhaseChange phaseChange;

    Changeset operator+(Changeset& other);
    Changeset& operator+=(Changeset other);

    friend std::ostream& operator<<(std::ostream& os, Changeset& changeset);

    static Changeset drawCards(xg::Guid player, unsigned int amount, const Environment& env);
};
#endif
