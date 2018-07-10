#ifndef _CHANGESET_H_
#define _CHANGESET_H_

#include <variant>
#include <vector>

#include "guid.hpp"

class Environment;

class Targetable {
public:
    xg::Guid id;
};

enum PlayerCounterType {
    POISONCOUNTER
};

enum PermanentCounterType {
    PLUSONEPLUSONECOUNTER
};

class ObjectMovement {
public:
    xg::Guid sourceZone;
    xg::Guid destinationZone;
    xg::Guid object;
};

class AddPlayerCounter {
public:
    xg::Guid object;
    PlayerCounterType counterType;
    int amount;
};

class AddPermanentCounter {
public:
    xg::Guid object;
    PermanentCounterType counterType;
    int amount;
};

class ObjectCreation {
public:
    xg::Guid zone;
    Targetable created;
};

class Changeset {
    std::vector<ObjectMovement> moves;
    std::vector<AddPlayerCounter> playerCounters;
    std::vector<AddPermanentCounter> permanentCounters;
    std::vector<ObjectCreation> creation;
};

#endif
