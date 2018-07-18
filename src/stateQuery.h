#ifndef _STATEQUERY_H_
#define _STATEQUERY_H_

#include <optional>
#include <variant>

#include "enum.h"
struct ActivatedAbility;
struct CardToken;
struct CostedEffect;
struct Targetable;
struct HasAbilities;
class EventHandler;
class TriggerHandler;
class StateQueryHandler;

struct PowerQuery {
    const CardToken& target;
    int currentValue;
};

struct ToughnessQuery {
    const CardToken& target;
    int currentValue;
};

struct TimingQuery {
	const CostedEffect& effect;
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
	xg::Guid player;
	unsigned int amount;
};

struct ReplacementEffectsQuery {
	const HasAbilities& target;
	ZoneType destinationZone;
	std::optional<ZoneType> originZone;
	std::vector<std::shared_ptr<EventHandler>> effects;
};

struct TriggerEffectsQuery {
	const HasAbilities& target;
	ZoneType destinationZone;
	std::optional<ZoneType> originZone;
	std::vector<std::shared_ptr<TriggerHandler>> effects;
};

struct StaticEffectsQuery {
	const HasAbilities& target;
	ZoneType destinationZone;
	std::optional<ZoneType> originZone;
	std::vector<std::shared_ptr<StateQueryHandler>> effects;
};

struct SelfReplacementEffectsQuery {
	const HasAbilities& target;
	ZoneType destinationZone;
	std::optional<ZoneType> originZone;
	std::vector<std::shared_ptr<EventHandler>> effects;
};

// CodeReview: Add cost calculation, can tap, valid attackers, valid blockers
using StateQuery = std::variant<PowerQuery, ToughnessQuery, TimingQuery, SuperTypesQuery, TypesQuery, SubTypesQuery,
                                ColorsQuery, ControllerQuery, ActivatedAbilitiesQuery, LandPlaysQuery, ReplacementEffectsQuery,
								TriggerEffectsQuery, StaticEffectsQuery, SelfReplacementEffectsQuery>;

#endif
