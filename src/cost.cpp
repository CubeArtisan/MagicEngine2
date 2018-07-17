#include "card.h"
#include "cost.h"
#include "environment.h"

ManaCost::ManaCost(Mana mana)
    : mana(mana)
{}

bool ManaCost::canPay(Player& player, Environment& env, SourceType) {
    return env.manaPools.at(player.id).contains(this->mana);
}

Changeset ManaCost::payCost(Player& player, Environment&, SourceType) {
    Changeset changes;
    changes.removeMana.push_back(RemoveMana{player.id, this->mana});
    return changes;
}

bool LandPlayCost::canPay(Player& player, Environment& env, SourceType) {
    return env.landPlays[player.id] < env.getLandPlays(player.id);
}

Changeset LandPlayCost::payCost(Player&, Environment&, SourceType) {
    return Changeset();
}

bool TapCost::canPay(Player&, Environment&, SourceType source) {
    return !getBaseClassPtr<CardToken>(source)->is_tapped;
}

Changeset TapCost::payCost(Player&, Environment&, SourceType source) {
    Changeset tap;
    tap.tap.push_back(TapTarget{getBaseClassPtr<Targetable>(source)->id, true});
    return tap;
}
