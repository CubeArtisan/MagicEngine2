#ifndef _STATEQUERY_H_
#define _STATEQUERY_H_

#include <optional>
#include <variant>

#include "enum.h"
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
    int currentValue;
};

struct ToughnessQuery {
    const CardToken& target;
    int currentValue;
};

struct TimingQuery {
	const HasCost& effect;
    bool timing;
};

struct SuperTypesQuery {
    const CardToken& target;
	std::shared_ptr<const std::set<CardSuperType>> superTypes;
};

struct TypesQuery {
    const CardToken& target;
	std::shared_ptr<const std::set<CardType>> types;
};

struct SubTypesQuery {
    const CardToken& target;
	std::shared_ptr<const std::set<CardSubType>> subTypes;
};

struct ColorsQuery {
	const CardToken& target;
    std::set<Color> colors;
};

struct ControllerQuery {
	const Targetable& target;
    xg::Guid controller;
};

// CodeReview: How to handle losing static/continuous abilities
struct ActivatedAbilitiesQuery {
	const CardToken& target;
    std::shared_ptr<const std::vector<std::shared_ptr<const ActivatedAbility>>> abilities;
};

struct LandPlaysQuery {
	const xg::Guid player;
	unsigned int amount;
};

struct ReplacementEffectsQuery {
	const HasAbilities& target;
	const ZoneType destinationZone;
	const std::optional<ZoneType> originZone;
	std::vector<std::shared_ptr<EventHandler>> effects;
};

struct TriggerEffectsQuery {
	const HasAbilities& target;
	const ZoneType destinationZone;
	const std::optional<ZoneType> originZone;
	std::vector<std::shared_ptr<TriggerHandler>> effects;
};

struct StaticEffectsQuery {
	const HasAbilities& target;
	const ZoneType destinationZone;
	const std::optional<ZoneType> originZone;
	std::vector<std::shared_ptr<StaticEffectHandler>> effects;
};

struct SelfReplacementEffectsQuery {
	const HasAbilities& target;
	const ZoneType destinationZone;
	const std::optional<ZoneType> originZone;
	std::vector<std::shared_ptr<EventHandler>> effects;
};

struct CanAttackQuery {
	const CardToken& target;
	bool canAttack;
};

struct CanBlockQuery {
	const CardToken& target;
	bool canBlock;
};

struct LethalDamageQuery {
	const CardToken& attacker;
	const CardToken& blocker;
	int damage;
};

struct AttackRestrictionQuery {
	const CardToken& attacker;
	std::vector<AttackRestrictionValue> restrictions;
};

struct AttackRequirementQuery {
	const CardToken& attacker;
	std::vector<AttackRequirementValue> requirements;
};

struct BlockRestrictionQuery {
	const CardToken& blocker;
	std::vector<BlockRestrictionValue> restrictions;
};

struct BlockRequirementQuery {
	const CardToken& blocker;
	std::vector<BlockRequirementValue> requirements;
};

// CodeReview: Add cost calculation, can tap, valid attackers, valid blockers
using StaticEffectQuery = std::variant<PowerQuery, ToughnessQuery, TimingQuery, SuperTypesQuery, TypesQuery, SubTypesQuery,
                                       ColorsQuery, ControllerQuery, ActivatedAbilitiesQuery, LandPlaysQuery, ReplacementEffectsQuery,
								       TriggerEffectsQuery, StaticEffectsQuery, SelfReplacementEffectsQuery, CanAttackQuery,
									   CanBlockQuery, LethalDamageQuery, AttackRestrictionQuery, AttackRequirementQuery,
									   BlockRestrictionQuery, BlockRequirementQuery>;
#endif
