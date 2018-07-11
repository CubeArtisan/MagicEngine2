#ifndef _CHANGESET_H_
#define _CHANGESET_H_

#include <variant>
#include <vector>

#include "guid.hpp"

#include "mana.h"

struct Environment;

struct Targetable {
    xg::Guid id;

    Targetable();
};

template<typename T, typename Variant>
T getBaseClass(Variant variant){
    // For each index see if its there and if so extract and return
    auto visitor = [](T& base) -> T&{ return base; };
    return std::visit(visitor, variant);
}

enum StepOrPhase {
    UNTAP,
    UPKEEP,
    DRAW,
    PRECOMBATMAIN,
    BEGINCOMBAT,
    DECLAREATTACKERS,
    DECLAREBLOCKERS,
    FIRSTSTRIKEDAMAGE,
    COMBATDAMAGE,
    ENDCOMBAT,
    POSTCOMBATMAIN,
    END,
    CLEANUP
};

enum PlayerCounterType {
    POISONCOUNTER
};

enum PermanentCounterType {
    PLUSONEPLUSONECOUNTER
};

struct ObjectMovement {
    xg::Guid object;
    xg::Guid sourceZone;
    xg::Guid destinationZone;
};

struct AddPlayerCounter {
    xg::Guid object;
    PlayerCounterType counterType;
    int amount;
};

struct AddPermanentCounter {
    xg::Guid object;
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
    Targetable created;
};

struct ControlChange {
    xg::Guid object;
    xg::Guid originalController;
    xg::Guid newController;
};

struct RemoveObject {
    xg::Guid object;
    xg::Guid zone;
};

struct StepOrPhaseChange {
    StepOrPhase starting;
};

struct Changeset {
    std::vector<ObjectMovement> moves;
    std::vector<AddPlayerCounter> playerCounters;
    std::vector<AddPermanentCounter> permanentCounters;
    std::vector<ObjectCreation> create;
    std::vector<RemoveObject> remove;
    std::vector<ControlChange> controlChanges;
    AddMana addMana;
    RemoveMana removeMana;
    bool millOut;

    static Changeset drawCards(xg::Guid player, unsigned int amount, Environment& env);
};
#endif
