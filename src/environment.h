#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <functional>
#include <map>
#include <set>
#include <variant>
#include <vector>

#include "guid.hpp"

#include "changeset.h"
#include "player.h"
#include "card.h"
#include "ability.h"

enum ZoneType {
    HAND,
    GRAVEYARD,
    LIBRARY,
    BATTLEFIELD,
    STACK,
    EXILE,
    COMMAND
};

template<typename T>
class Zone : public Targetable {
public:
    ZoneType type;
    std::vector<T> objects;
};

template<typename T>
using PrivateZone = std::map<xg::Guid, Zone<T>>;

class Environment {
public:
    std::map<xg::Guid, std::reference_wrapper<Targetable>> gameObjects;

    PrivateZone<std::variant<std::reference_wrapper<Card>, std::reference_wrapper<Token>>> hands;
    PrivateZone<std::variant<std::reference_wrapper<Card>, std::reference_wrapper<Token>>> libraries;
    Zone<std::variant<std::reference_wrapper<Card>, std::reference_wrapper<Token>>> graveyard;
    Zone<std::variant<std::reference_wrapper<Card>, std::reference_wrapper<Token>>> battlefield;
    Zone<std::variant<std::reference_wrapper<Card>, std::reference_wrapper<Token>, std::reference_wrapper<Ability>>> stack;
    Zone<std::variant<std::reference_wrapper<Card>, std::reference_wrapper<Token>>> exile;
    Zone<std::variant<std::reference_wrapper<Card>, std::reference_wrapper<Emblem>>> command;
    
    std::map<xg::Guid, Mana> manaPools;
    // CodeReview: Does this maintain across undoing turn changes
    std::map<xg::Guid, unsigned int> landPlays;
    std::vector<Player> players;

    std::map<xg::Guid, std::map<PermanentCounterType, unsigned int>> permanentCounters;
    std::map<xg::Guid, std::map<PlayerCounterType, unsigned int>> playerCounters;
    std::map<xg::Guid, int> lifeTotals;

    std::vector<std::reference_wrapper<EventHandler>> triggerHandlers;
    std::vector<std::reference_wrapper<EventHandler>> replacementEffects;
    std::vector<std::reference_wrapper<StateQueryHandler>> propertyHandlers;

    std::vector<Changeset> changes;

    StepOrPhase currentPhase;
    unsigned int currentPlayer;
    unsigned int turnPlayer;

    Changeset passPriority(xg::Guid player);
};

#endif
