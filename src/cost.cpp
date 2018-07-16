#include "card.h"
#include "cost.h"
#include "environment.h"

ManaCost::ManaCost(Mana mana)
    : mana(mana)
{}

bool ManaCost::canPay(Player& player, Environment& env) {
    return env.manaPools[player.id].contains(this->mana);
}

Changeset ManaCost::payCost(Player& player, Environment&) {
    Changeset changes;
    changes.removeMana.push_back(RemoveMana{player.id, this->mana});
    return changes;
}

bool LandPlayCost::canPay(Player& player, Environment& env) {
    return env.landPlays[player.id] > 0;
}

Changeset LandPlayCost::payCost(Player&, Environment&) {
    return Changeset();
}

TapCost::TapCost(CardToken& object)
    : object(object)
{}

bool TapCost::canPay(Player&, Environment&) {
    return !object.is_tapped;
}

Changeset TapCost::payCost(Player&, Environment&) {
    Changeset tap;
    tap.tap.push_back(TapTarget{object.id, true});
    return tap;
}
