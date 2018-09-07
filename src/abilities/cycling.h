#ifndef _CYCLING_H_
#define _CYCLING_H_

#include "../ability.h"
#include "../cost.h"
#include "../effect.h"
#include "../enum.h"

struct CyclingCost : public Cost {
	bool canPay(const Player& player, const Environment& env, const SourceType& source) const override {
		xg::Guid id = getBaseClassPtr<const Targetable>(source)->id;
		return (bool)env.hands.at(player.id)->findObject(id);
	}
	Changeset payCost(const Player& player, const Environment& env, const SourceType& source) const override {
		xg::Guid id = getBaseClassPtr<const Targetable>(source)->id;
		Changeset changes;
		if (env.hands.at(player.id)->findObject(id)) {
			changes.moves.push_back(ObjectMovement{ id, env.hands.at(player.id)->id, env.graveyards.at(player.id)->id, 0, CYCLING });
		}
	}

	Cost& operator+=(const Cost& other) { return *this; }

	Cost& operator-=(const Cost& other) { return *this; }

	CostValue createCostValue() const { return CostValue(*this); }
};

struct CyclingAbility : public clone_inherit<CyclingAbility, ActivatedAbility> {
	CyclingAbility(CostValue cost)
		: clone_inherit(LambdaEffects([](xg::Guid source, const Environment& env) -> std::optional<Changeset> { return Changeset::drawCards(1, env.getController(source)); }),
						std::make_shared<NoTargets>(), { cost += CyclingCost() }, {}, std::shared_ptr<const std::set<ZoneType>>(new std::set{ HAND }))
	{ }
};

#endif