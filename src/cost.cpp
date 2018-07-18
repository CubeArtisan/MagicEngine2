#include "card.h"
#include "cost.h"
#include "environment.h"

ManaCost::ManaCost(Mana mana)
    : mana(mana)
{}

bool ManaCost::canPay(const Player& player, const Environment& env, const SourceType&)  const {
    return env.manaPools.at(player.id).contains(this->mana);
}

Changeset ManaCost::payCost(const Player& player, const Environment&, const SourceType&)  const {
    Changeset changes;
    changes.removeMana.push_back(RemoveMana{player.id, this->mana});
    return changes;
}

bool LandPlayCost::canPay(const Player& player, const Environment& env, const SourceType&)  const {
    return env.landPlays.at(player.id) < env.getLandPlays(player.id);
}

Changeset LandPlayCost::payCost(const Player&, const Environment&, const SourceType&)  const {
    return Changeset();
}

bool TapCost::canPay(const Player&, const Environment&, const SourceType& source)  const {
    return !getBaseClassPtr<const CardToken>(source)->is_tapped;
}

Changeset TapCost::payCost(const Player&, const Environment&, const SourceType& source)  const {
    Changeset tap;
    tap.tap.push_back(TapTarget{getBaseClassPtr<const Targetable>(source)->id, true});
    return tap;
}
