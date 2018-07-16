#ifndef _STATEQUERY_H_
#define _STATEQUERY_H_

#include <variant>

#include "enum.h"
struct ActivatedAbility;
struct CardToken;
struct CostedEffect;
struct Targetable;

struct PowerQuery {
    CardToken& target;
    int currentValue;
};

struct ToughnessQuery {
    CardToken& target;
    int currentValue;
};

struct TimingQuery {
	CostedEffect& effect;
    bool timing;
};

struct SuperTypesQuery {
    CardToken& target;
    std::set<CardSuperType> superTypes;
};

struct TypesQuery {
    CardToken& target;
    std::set<CardType> types;
};

struct SubTypesQuery {
    CardToken& target;
    std::set<CardSubType> subTypes;
};

struct ColorsQuery {
    CardToken& target;
    std::set<Color> colors;
};

struct ControllerQuery {
    Targetable& target;
    xg::Guid controller;
};

// CodeReview: How to handle losing static/continuous abilities
struct ActivatedAbilitiesQuery {
    CardToken& target;
    std::vector<std::shared_ptr<ActivatedAbility>> abilities;
};

using StateQuery = std::variant<PowerQuery, ToughnessQuery, TimingQuery, SuperTypesQuery, TypesQuery, SubTypesQuery,
                                ColorsQuery, ControllerQuery, ActivatedAbilitiesQuery>;

#endif
