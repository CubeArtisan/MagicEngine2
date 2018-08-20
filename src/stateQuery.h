#ifndef _STATEQUERY_H_
#define _STATEQUERY_H_

#include <optional>
#include <variant>

#include "enum.h"
#include "mana.h"
struct ActivatedAbility;
struct CardToken;
struct HasCost;
struct Targetable;
struct HasAbilities;
class EventHandler;
class TriggerHandler;
class StaticEffectHandler;
class AttackRequirementValue;
class AttackRestrictionValue;
class BlockRequirementValue;
class BlockRestrictionValue;

struct PowerQuery {
    const CardToken& target;
    int power;
	bool operator==(const PowerQuery& other) {
		return this->power == other.power;
	}
};

struct ToughnessQuery {
    const CardToken& target;
    int toughness;
	bool operator==(const ToughnessQuery& other) {
		return this->toughness == other.toughness;
	}
};

struct TimingQuery {
	const HasCost& effect;
    bool timing;
	bool operator==(const TimingQuery& other) {
		return this->timing == other.timing;
	}
};

struct SuperTypesQuery {
    const CardToken& target;
	std::shared_ptr<const std::set<CardSuperType>> superTypes;
	bool operator==(const SuperTypesQuery& other) {
		return *this->superTypes == *other.superTypes;
	}
};

struct TypesQuery {
    const CardToken& target;
	std::shared_ptr<const std::set<CardType>> types;
	bool operator==(const TypesQuery& other) {
		return *this->types == *other.types;
	}
};

struct SubTypesQuery {
    const CardToken& target;
	std::shared_ptr<const std::set<CardSubType>> subTypes;
	bool operator==(const SubTypesQuery& other) {
		return *this->subTypes == *other.subTypes;
	}
};

struct ColorsQuery {
	const CardToken& target;
    std::set<Color> colors;
	bool operator==(const ColorsQuery& other) {
		return this->colors == other.colors;
	}
};

struct ControllerQuery {
	const Targetable& target;
    xg::Guid controller;
	bool operator==(const ControllerQuery& other) {
		return this->controller == other.controller;
	}
};

// CodeReview: How to handle losing static/continuous abilities
struct ActivatedAbilitiesQuery {
	const CardToken& target;
    std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> abilities;
	bool operator==(const ActivatedAbilitiesQuery& other) {
		// CodeReview: Should this dereference the abilities for comparison?
		return this->abilities == other.abilities;
	}
};

struct LandPlaysQuery {
	const xg::Guid player;
	unsigned int amount;
	bool operator==(const LandPlaysQuery& other) {
		return this->amount == other.amount;
	}
};

struct ReplacementEffectsQuery {
	const CardToken& target;
	const ZoneType destinationZone;
	const std::optional<ZoneType> originZone;
	std::vector<std::shared_ptr<EventHandler>> effects;
	bool operator==(const ReplacementEffectsQuery& other) {
		// CodeReview: Should this dereference the effects for comparison?
		return this->effects == other.effects;
	}
};

struct ActiveReplacementEffectsQuery {
	std::vector<std::shared_ptr<EventHandler>> effects;
	bool operator==(const ActiveReplacementEffectsQuery& other) {
		// CodeReview: Should this dereference the effects for comparison?
		return this->effects == other.effects;
	}
};

struct TriggerEffectsQuery {
	const CardToken& target;
	const ZoneType destinationZone;
	const std::optional<ZoneType> originZone;
	std::vector<std::shared_ptr<TriggerHandler>> effects;
	bool operator==(const TriggerEffectsQuery& other) {
		// CodeReview: Should this dereference the effects for comparison?
		return this->effects == other.effects;
	}
};

struct ActiveTriggerEffectsQuery {
	std::vector<std::shared_ptr<TriggerHandler>> effects;
	bool operator==(const ActiveTriggerEffectsQuery& other) {
		// CodeReview: Should this dereference the effects for comparison?
		return this->effects == other.effects;
	}
};

struct StaticEffectsQuery {
	const CardToken& target;
	const ZoneType destinationZone;
	const std::optional<ZoneType> originZone;
	std::vector<std::shared_ptr<StaticEffectHandler>> effects;
	bool operator==(const StaticEffectsQuery& other) {
		// CodeReview: Should this dereference the effects for comparison?
		return this->effects == other.effects;
	}
};

struct ActiveStaticEffectsQuery {
	std::vector<std::shared_ptr<StaticEffectHandler>> effects;
	bool operator==(const ActiveStaticEffectsQuery& other) {
		// CodeReview: Should this dereference the effects for comparison?
		return this->effects == other.effects;
	}
};

struct SelfReplacementEffectsQuery {
	const HasAbilities& target;
	const ZoneType destinationZone;
	const std::optional<ZoneType> originZone;
	std::vector<std::shared_ptr<EventHandler>> effects;
	bool operator==(const SelfReplacementEffectsQuery& other) {
		// CodeReview: Should this dereference the effects for comparison?
		return this->effects == other.effects;
	}
};

struct CanAttackQuery {
	const CardToken& target;
	bool canAttack;
	bool operator==(const CanAttackQuery& other) {
		return this->canAttack == other.canAttack;
	}
};

struct CanBlockQuery {
	const CardToken& target;
	bool canBlock;
	bool operator==(const CanBlockQuery& other) {
		return this->canBlock == other.canBlock;
	}
};

struct LethalDamageQuery {
	const CardToken& attacker;
	const CardToken& blocker;
	int damage;
	bool operator==(const LethalDamageQuery& other) {
		return this->damage == other.damage;
	}
};

struct AttackRestrictionQuery {
	const CardToken& attacker;
	std::vector<AttackRestrictionValue> restrictions;
	bool operator==(const AttackRestrictionQuery& other) {
		return this->restrictions == other.restrictions;
	}
};

struct AttackRequirementQuery {
	const CardToken& attacker;
	std::vector<AttackRequirementValue> requirements;
	bool operator==(const AttackRequirementQuery& other) {
		return this->requirements == other.requirements;
	}
};

struct BlockRestrictionQuery {
	const CardToken& blocker;
	std::vector<BlockRestrictionValue> restrictions;
	bool operator==(const BlockRestrictionQuery& other) {
		return this->restrictions == other.restrictions;
	}
};

struct BlockRequirementQuery {
	const CardToken& blocker;
	std::vector<BlockRequirementValue> requirements;
	bool operator==(const BlockRequirementQuery& other) {
		return this->requirements == other.requirements;
	}
};

// CodeReview: Add cost calculation, can tap, valid attackers, valid blockers
using StaticEffectQuery = std::variant<PowerQuery, ToughnessQuery, TimingQuery, SuperTypesQuery, TypesQuery, SubTypesQuery,
                                       ColorsQuery, ControllerQuery, ActivatedAbilitiesQuery, LandPlaysQuery, ReplacementEffectsQuery,
								       TriggerEffectsQuery, StaticEffectsQuery, SelfReplacementEffectsQuery, CanAttackQuery,
									   CanBlockQuery, LethalDamageQuery, AttackRestrictionQuery, AttackRequirementQuery,
									   BlockRestrictionQuery, BlockRequirementQuery, ActiveReplacementEffectsQuery, ActiveTriggerEffectsQuery,
									   ActiveStaticEffectsQuery>;
#endif
