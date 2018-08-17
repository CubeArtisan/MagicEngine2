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
#include "util.h"

struct Ability;
struct Changeset;
struct Environment;
struct ManaAbility;

struct Targetable : public clone_inherit<abstract_method<Targetable>> {
    // This is mutable state
    xg::Guid id;
    // CodeReview: potentially make const
	xg::Guid owner;

    Targetable();
	virtual ~Targetable() {}
};

class Handler : public clone_inherit<abstract_method<Handler>, Targetable> {
public:
	const std::set<ZoneType> activeSourceZones;
	const std::set<ZoneType> activeDestinationZones;
	bool operator==(Handler& other) const {
		return this->id == other.id;
	}

	Handler(std::set<ZoneType> activeSourceZones, std::set<ZoneType> activeDestinationZones)
		: activeSourceZones(activeSourceZones), activeDestinationZones(activeDestinationZones)
	{}
};

class EventHandler : public clone_inherit<abstract_method<EventHandler>, Handler> {
public:
    virtual std::variant<std::vector<Changeset>, PassPriority> handleEvent(Changeset&, const Environment&) const = 0;
	using clone_inherit<abstract_method<EventHandler>, Handler>::clone_inherit;
	static inline bool selfReplacing = false;
};

class StaticEffectHandler : public clone_inherit<abstract_method<StaticEffectHandler>, Handler> {
public:
    virtual StaticEffectQuery& handleEvent(StaticEffectQuery&, const Environment&) const = 0;
	virtual bool appliesTo(StaticEffectQuery&, const Environment&) const = 0;
	virtual bool dependsOn(StaticEffectQuery& start, StaticEffectQuery& end, const Environment& env) const = 0;

	// CodeReview: Is CDA
	using clone_inherit<abstract_method<StaticEffectHandler>, Handler>::clone_inherit;
};

struct QueueTrigger;

class TriggerHandler : public clone_inherit<abstract_method<TriggerHandler>, EventHandler> {
public:
	std::variant<std::vector<Changeset>, PassPriority> handleEvent(Changeset&, const Environment&) const;
	using clone_inherit<abstract_method<TriggerHandler>, EventHandler>::clone_inherit;

protected:
	virtual std::vector<QueueTrigger> createTriggers(const Changeset&, const Environment&) const = 0;
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
	int index{ 0 };
	MovementType type{ DEFAULTMOVEMENTTYPE };
    xg::Guid newObject{ xg::newGuid() };
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
	int index{ 0 };
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
	int amount;
	xg::Guid source;
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

struct DeclareAttack {
	xg::Guid attacker;
	xg::Guid defender;
};

struct Changeset {
    std::vector<ObjectMovement> moves;
    std::vector<AddPlayerCounter> playerCounters;
    std::vector<AddPermanentCounter> permanentCounters;
    std::vector<ObjectCreation> create;
    std::vector<RemoveObject> remove;
    std::vector<LifeTotalChange> lifeTotalChanges;
    std::vector<std::shared_ptr<EventHandler>> effectsToAdd;
    std::vector<std::shared_ptr<EventHandler>> effectsToRemove;
	std::vector<std::shared_ptr<TriggerHandler>> triggersToAdd;
	std::vector<std::shared_ptr<TriggerHandler>> triggersToRemove;
    std::vector<std::shared_ptr<StaticEffectHandler>> propertiesToAdd;
    std::vector<std::shared_ptr<StaticEffectHandler>> propertiesToRemove;
    std::vector<xg::Guid> loseTheGame;
    std::vector<AddMana> addMana;
    std::vector<RemoveMana> removeMana;
    std::vector<DamageToTarget> damage;
	std::vector<DamageToTarget> combatDamage;
    std::vector<TapTarget> tap;
	std::vector<CreateTargets> target;
	std::vector<QueueTrigger> trigger;
	std::vector<LandPlay> land;
	std::vector<std::shared_ptr<ManaAbility>> manaAbility;
	std::vector<DeclareAttack> attacks;
    StepOrPhaseChange phaseChange;
	bool clearTriggers{ false };

    Changeset operator+(const Changeset& other);
    Changeset& operator+=(const Changeset& other);

	// CodeReview: implement then use to filter out empty Changesets
	// bool empty();

    friend std::ostream& operator<<(std::ostream& os, const Changeset& changeset);

    static Changeset drawCards(xg::Guid player, size_t amount, const Environment& env);
	static Changeset discardCards(xg::Guid player, size_t amount, const Environment& env);
	static Changeset scryCards(xg::Guid player, size_t amount, const Environment& env);
};

struct QueueTrigger {
	xg::Guid player;
	xg::Guid source;
	Changeset triggered;
	std::shared_ptr<Ability> ability;
};
#endif
