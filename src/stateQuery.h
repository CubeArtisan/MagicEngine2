#ifndef _STATEQUERY_H_
#define _STATEQUERY_H_

#include <variant>

#include "enum.h"
class Ability;

struct PowerQuery {
    xg::Guid target;
    int currentValue;
};

struct ToughnessQuery {
    xg::Guid target;
    int currentValue;
};

struct TimingQuery {
    xg::Guid target;
    bool timing;
};

struct SuperTypesQuery {
    xg::Guid target;
    std::set<CardSuperType> supertypes;
};

struct TypesQuery {
    xg::Guid target;
    std::set<CardType> types;
};

struct SubTypesQuery {
    xg::Guid target;
    std::set<CardSubType> subtypes;
};

struct ColorsQuery {
    xg::Guid target;
    std::set<Color> colors;
};

struct ControllerQuery {
    xg::Guid target;
    xg::Guid controller;
};

struct AbilitiesQuery {
    xg::Guid target;
    std::vector<std::reference_wrapper<Ability>> abilities;
};

using StateQuery = std::variant<PowerQuery, ToughnessQuery, TimingQuery, SuperTypesQuery, TypesQuery, SubTypesQuery,
                                ColorsQuery, ControllerQuery, AbilitiesQuery>;

#endif
