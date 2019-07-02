#ifndef _CHANGESET_H_
#define _CHANGESET_H_

#include <memory>
#include <variant>
#include <vector>

#include "guid.hpp"

#include "enum.h"
#include "handlers.h"
#include "linq/linq.h"
#include "mana.h"
#include "stateQuery.h"
#include "linq/util.h"

struct Ability;
struct Changeset;
struct Environment;
struct ManaAbility;
class Runner;

struct GameChange {
	virtual bool ApplyTo(Environment& env, Runner& runner) = 0;

	virtual std::string ToString() const = 0;
	virtual std::string ToString(Environment&) const {
		return this->ToString();
	}
};

struct AddPlayerCounter : public GameChange {
	xg::Guid player;
	PlayerCounterType counterType;
	int amount{ 0 };

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	AddPlayerCounter(xg::Guid player = {}, PlayerCounterType counterType = {}, int amount = 0)
		: player(player), counterType(counterType), amount(amount)
	{}
};

struct AddPermanentCounter : public GameChange {
	xg::Guid target;
	PermanentCounterType counterType;
	int amount{ 0 };

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	AddPermanentCounter(xg::Guid target = {}, PermanentCounterType counterType = {}, int amount = 0)
		: target(target), counterType(counterType), amount(amount)
	{}
};

struct CreateObject : public GameChange {
	xg::Guid zone;
	std::shared_ptr<Targetable> created;
	int index{ 0 };

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString(Environment& env) const override;
	std::string ToString() const override;

	CreateObject(xg::Guid zone = xg::Guid(), std::shared_ptr<Targetable> created = {}, int index = 0)
		: zone(zone), created(created), index(index)
	{}
};

struct RemoveObject : public GameChange {
	xg::Guid object;
	xg::Guid zone;

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString(Environment& env) const override;
	std::string ToString() const override;

	RemoveObject(xg::Guid object = {}, xg::Guid zone = {})
		: object(object), zone(zone)
	{}
};

struct LifeTotalChange : public GameChange {
	xg::Guid player;
	int amount{ 0 };

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	LifeTotalChange(xg::Guid player = {}, int amount = 0)
		: player(player), amount(amount)
	{}
};

struct AddReplacementEffect : public GameChange {
	std::shared_ptr<const EventHandler> handler;

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	AddReplacementEffect(std::shared_ptr<const EventHandler> handler = {})
		: handler(handler)
	{}
};

struct RemoveReplacementEffect : public GameChange {
	std::shared_ptr<const EventHandler> handler;

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	RemoveReplacementEffect(std::shared_ptr<const EventHandler> handler = {})
		: handler(handler)
	{}
};

struct AddTriggerEffect : public GameChange {
	std::shared_ptr<const TriggerHandler> handler;

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	AddTriggerEffect(std::shared_ptr<const TriggerHandler> handler = {})
		: handler(handler)
	{}
};

struct RemoveTriggerEffect : public GameChange {
	std::shared_ptr<const TriggerHandler> handler;

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	RemoveTriggerEffect(std::shared_ptr<const TriggerHandler> handler = {})
		: handler(handler)
	{}
};

struct AddStaticEffect : public GameChange {
	std::shared_ptr<const StaticEffectHandler> handler;

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	AddStaticEffect(std::shared_ptr<const StaticEffectHandler> handler = {})
		: handler(handler)
	{}
};

struct RemoveStaticEffect : public GameChange {
	std::shared_ptr<const StaticEffectHandler> handler;

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	RemoveStaticEffect(std::shared_ptr<const StaticEffectHandler> handler = {})
		: handler(handler)
	{}
};

struct AddMana : public GameChange {
	xg::Guid player;
	Mana amount;

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	AddMana(xg::Guid player = {}, Mana amount = {})
		: player(player), amount(amount)
	{}
};

struct RemoveMana : public GameChange {
	xg::Guid player;
	Mana amount;

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	RemoveMana(xg::Guid player = {}, Mana amount = {})
		: player(player), amount(amount)
	{}
};

struct DamageToTarget : public GameChange {
	xg::Guid target;
	unsigned int amount{ 0 };
	xg::Guid source;

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	DamageToTarget(xg::Guid target = {}, unsigned int amount = 0, xg::Guid source = {})
		: target(target), amount(amount), source(source)
	{}
};

struct CombatDamageToTarget : public DamageToTarget {
	std::string ToString() const override;

	CombatDamageToTarget(xg::Guid target = {}, unsigned int amount = 0, xg::Guid source = {})
		: DamageToTarget(target, amount, source)
	{}
};

struct TapTarget : public GameChange {
	xg::Guid target;
	bool tap{ true };

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;
	std::string ToString(Environment& env) const override;

	TapTarget(xg::Guid target = {}, bool tap = true)
		: target(target), tap(tap)
	{}
};

struct CreateTargets : public GameChange {
	xg::Guid object;
	std::vector<xg::Guid> targets;

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	CreateTargets(xg::Guid object = {}, std::vector<xg::Guid> targets = {})
		: object(object), targets(targets)
	{}
};

struct StepOrPhaseChange : public GameChange {
public:
	StepOrPhaseId starting{ UNTAP };

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	StepOrPhaseChange(StepOrPhaseId starting)
		: starting(starting)
	{}

private:
	StepOrPhaseId nextStepOrPhase(Environment& env) const;
};

struct ClearTriggers : public GameChange {
	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;
};

struct QueueTrigger;

struct LandPlay : public GameChange {
	xg::Guid land;
	xg::Guid player;
	xg::Guid zone;

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString(Environment& env) const override;
	std::string ToString() const override;

	LandPlay(xg::Guid land = {}, xg::Guid player = {}, xg::Guid zone = {})
		: land(land), player(player), zone(zone)
	{}
};

struct ApplyManaAbility : public GameChange {
	std::shared_ptr<ManaAbility> manaAbility;

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	ApplyManaAbility(std::shared_ptr<ManaAbility> manaAbility)
		: manaAbility(manaAbility)
	{}
};

struct ObjectMovement : public GameChange {
	xg::Guid object;
	xg::Guid sourceZone;
	xg::Guid destinationZone;
	int index{ 0 };
	MovementType type{ DEFAULTMOVEMENTTYPE };
	xg::Guid newObject{ xg::newGuid() };

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	ObjectMovement(xg::Guid object = xg::Guid(), xg::Guid sourceZone = xg::Guid(), xg::Guid destinationZone = xg::Guid(), int index = 0,
		MovementType type = DEFAULTMOVEMENTTYPE, xg::Guid newObject = xg::newGuid())
		: object(object), sourceZone(sourceZone), destinationZone(destinationZone), index(index), type(type), newObject(newObject)
	{ }
};

struct LoseTheGame : public GameChange {
	xg::Guid player;

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;

	LoseTheGame(xg::Guid player = {})
		: player(player)
	{}
};

struct DeclareAttack : public GameChange {
	xg::Guid attacker;
	xg::Guid defender;

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString() const override;
	std::string ToString(Environment& env) const override;

	DeclareAttack(xg::Guid attacker = {}, xg::Guid defender = {})
		: attacker(attacker), defender(defender)
	{}
};

struct Changeset {
	std::vector<std::shared_ptr<GameChange>> changes;

	Changeset operator+(const Changeset& other) const;
	Changeset& operator+=(const Changeset& other);

	bool empty() const;

	Changeset() {}
	Changeset(std::shared_ptr<GameChange> change)
		: changes{ change }
	{}
	template<typename T>
	Changeset(T change)
		: changes{ std::shared_ptr<GameChange>(dynamic_cast<GameChange*>(new T(change))) }
	{}
	Changeset(std::vector<std::shared_ptr<GameChange>> changes)
		: changes(changes)
	{}

	template<typename T, typename... Args>
	std::enable_if_t<std::is_base_of_v<GameChange, T>, Changeset&> push_back(Args... args) {
		this->changes.push_back(std::shared_ptr<GameChange>(dynamic_cast<GameChange*>(new T(args...))));
		return *this;
	}

	template<typename T>
	auto push_back(T change) -> std::enable_if_t<std::is_base_of_v<GameChange, T>, Changeset&> {
		this->changes.push_back(std::shared_ptr<GameChange>(dynamic_cast<GameChange*>(new T(change))));
		return *this;
	}

	using iterator = std::vector<std::shared_ptr<GameChange>>::iterator;
	using const_iterator = std::vector<std::shared_ptr<GameChange>>::const_iterator;

	template<typename U>
	auto ofType() {
		return linq::id(this->changes).ofType<U>();
	}

	template<typename U>
	auto ofType() const {
		return linq::id(this->changes).ofType<U>();
	}

	auto linq() {
		return linq::id{ this->changes };
	}

	auto linq() const {
		return linq::id{ this->changes };
	}

	auto from() {
		return this->linq();
	}

	auto from() const {
		return this->linq();
	}

	friend std::ostream& operator<<(std::ostream& os, const Changeset& changeset);

	static Changeset drawCards(xg::Guid player, size_t amount, const Environment& env);
	static Changeset discardCards(xg::Guid player, size_t amount, const Environment& env);
	static Changeset scryCards(xg::Guid player, size_t amount, const Environment& env);
};

struct QueueTrigger : public GameChange {
	xg::Guid player;
	xg::Guid source;
	Changeset triggered;
	std::shared_ptr<Ability> ability;

	bool ApplyTo(Environment& env, Runner& runner) override;

	std::string ToString(Environment& env) const override;
	std::string ToString() const override;

	QueueTrigger(xg::Guid player = {}, xg::Guid source = {}, Changeset triggered = {}, std::shared_ptr<Ability> ability = {})
		: player(player), source(source), triggered(triggered), ability(ability)
	{}
};
#endif
