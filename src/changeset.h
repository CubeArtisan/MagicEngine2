#ifndef _CHANGESET_H_
#define _CHANGESET_H_

#include <memory>
#include <variant>
#include <vector>

#include "guid.hpp"

#include "enum.h"
#include "gameAction.h"
#include "mana.h"
#include "stateQuery.h"

struct Ability;
struct Changeset;
struct Environment;

struct Targetable {
    // This is mutable state
    xg::Guid id;
    xg::Guid owner;

    Targetable();
	virtual ~Targetable() {}
};

class EventHandler : public Targetable {
public:
    virtual std::variant<std::vector<Changeset>, PassPriority> handleEvent(Changeset&, const Environment&) = 0;
    bool operator==(EventHandler& other) {
        return this->id == other.id;
    }
};
class StateQueryHandler : public Targetable {
public:
    virtual StateQuery& handleEvent(StateQuery&, const Environment&) = 0;
    bool operator==(StateQueryHandler& other) {
        return this->id == other.id;
    }
};

struct QueueTrigger {
	xg::Guid player;
	Changeset triggered;
	std::shared_ptr<Ability> ability;
};

class TriggerHandler : public EventHandler {
public:
	virtual std::variant<std::vector<Changeset>, PassPriority> handleEvent(Changeset&, const Environment&);

	template<typename Condition, typename Trigger>
	TriggerHandler(Condition doesTrigger,
				   Trigger createTrigger)
		: doesTrigger(doesTrigger), createTrigger(createTrigger)
	{}
private:
	std::function<std::vector<QueueTrigger>(const Changeset&, const Environment&)> createTriggers;
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

// These can't be const because that removes the copy assignment operator
// which is needed to be stored in a vector
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
	xg::Guid target;
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

// CodeReview: Currently need the copy operator which requires members to be non-const
struct StepOrPhaseChange {
	bool changed{ false };
	StepOrPhase starting{ UNTAP };
};

struct DamageToTarget {
	xg::Guid target;
	unsigned int amount;
};

struct TapTarget {
	xg::Guid target;
	bool tap;
};

struct CreateTargets {
	xg::Guid object;
	std::vector<xg::Guid> targets;
};

struct LandPlay {
	xg::Guid land;
	xg::Guid player;
	xg::Guid zone;
};

struct Changeset {
    std::vector<ObjectMovement> moves;
    std::vector<AddPlayerCounter> playerCounters;
    std::vector<AddPermanentCounter> permanentCounters;
    std::vector<ObjectCreation> create;
    std::vector<RemoveObject> remove;
    std::vector<LifeTotalChange> lifeTotalChanges;
    std::vector<std::shared_ptr<EventHandler>> effectsToAdd;
	// CodeReview: Should these be xg::Guid for removal?
    std::vector<std::shared_ptr<EventHandler>> effectsToRemove;
	std::vector<std::shared_ptr<TriggerHandler>> triggersToAdd;
	std::vector<std::shared_ptr<TriggerHandler>> triggersToRemove;
    std::vector<std::shared_ptr<StateQueryHandler>> propertiesToAdd;
    std::vector<std::shared_ptr<StateQueryHandler>> propertiesToRemove;
    std::vector<xg::Guid> loseTheGame;
    std::vector<AddMana> addMana;
    std::vector<RemoveMana> removeMana;
    std::vector<DamageToTarget> damage;
    std::vector<TapTarget> tap;
	std::vector<CreateTargets> target;
	std::vector<QueueTrigger> trigger;
	std::vector<LandPlay> land;
    StepOrPhaseChange phaseChange;
	bool clearTriggers{ false };
	// CodeReview: Add Destroy

    Changeset operator+(const Changeset& other);
    Changeset& operator+=(const Changeset& other);

    friend std::ostream& operator<<(std::ostream& os, const Changeset& changeset);

    static Changeset drawCards(xg::Guid player, size_t amount, const Environment& env);
};
#endif
