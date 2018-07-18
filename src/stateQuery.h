#ifndef _STATEQUERY_H_
#define _STATEQUERY_H_

#include <variant>

#include "enum.h"
struct ActivatedAbility;
struct CardToken;
struct CostedEffect;
struct Targetable;

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

// CodeReview: Add cost calculation, can tap, valid attackers, valid blockers
using StateQuery = std::variant<PowerQuery, ToughnessQuery, TimingQuery, SuperTypesQuery, TypesQuery, SubTypesQuery,
                                ColorsQuery, ControllerQuery, ActivatedAbilitiesQuery, LandPlaysQuery>;

#endif
