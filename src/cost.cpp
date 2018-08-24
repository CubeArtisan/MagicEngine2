#include "card.h"
#include "cost.h"
#include "environment.h"

Cost& CostValue::operator+=(const Cost& other) {
	if (typeid(other) == typeid(this->value()) || typeid(this->value()) == typeid(CombineCost)) {
		this->value().operator+=(other);
	}
	else {
		*this = CombineCost(other, this->value());
	}
	return *this;
}

Cost& CostValue::operator-=(const Cost& other) {
	if (typeid(other) == typeid(this->value()) || typeid(this->value()) == typeid(CombineCost)) {
		this->value().operator-=(other);
	}
	return *this;
}

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

ManaCost& ManaCost::operator+=(const Cost& other) {
	if (typeid(other) == typeid(CostValue)) {
		return *this += dynamic_cast<const CostValue&>(other).value();
	}
	else if (typeid(other) == typeid(ManaCost)) {
		this->mana += dynamic_cast<const ManaCost&>(other).mana;
	}
	else {
		throw "Tried to add non-ManaCost to ManaCost";
	}
	return *this;
}

ManaCost& ManaCost::operator-=(const Cost& other) {
	if (typeid(other) == typeid(CostValue)) {
		return *this -= dynamic_cast<const CostValue&>(other).value();
	}
	else if (typeid(other) == typeid(ManaCost)) {
		this->mana -= dynamic_cast<const ManaCost&>(other).mana;
	}
	else {
		throw "Tried to subtract non-ManaCost from ManaCost";
	}
	return *this;
}

bool LandPlayCost::canPay(const Player& player, const Environment& env, const SourceType&)  const {
    return env.landPlays.at(player.id) < env.getLandPlays(player.id);
}

Changeset LandPlayCost::payCost(const Player&, const Environment&, const SourceType&)  const {
    return Changeset();
}

bool TapCost::canPay(const Player&, const Environment&, const SourceType& source)  const {
    return !getBaseClassPtr<const CardToken>(source)->isTapped;
}

Changeset TapCost::payCost(const Player&, const Environment&, const SourceType& source)  const {
    Changeset tap;
    tap.tap.push_back(TapTarget{getBaseClassPtr<const Targetable>(source)->id, true});
    return tap;
}

TapCost& TapCost::operator+=(const Cost& other) {
	if (typeid(other) == typeid(CostValue)) {
		return *this += dynamic_cast<const CostValue&>(other).value();
	}
	else if (typeid(other) == typeid(TapCost)) {
		this->applies |= dynamic_cast<const TapCost&>(other).applies;
	}
	else {
		throw "Tried to add non-TapCost to TapCost";
	}
	return *this;
}

TapCost& TapCost::operator-=(const Cost& other) {
	if (typeid(other) == typeid(CostValue)) {
		return *this -= dynamic_cast<const CostValue&>(other).value();
	}
	else if (typeid(other) == typeid(TapCost)) {
		this->applies &= !dynamic_cast<const TapCost&>(other).applies;
	}
	else {
		throw "Tried to subtract non-TapCost from TapCost";
	}
	return *this;
}