#ifndef _COUNTERUNLESS_H_
#define _COUNTERUNLESS_H_

#include "../ability.h"
#include "../cost.h"
#include "../effect.h"
#include "../enum.h"

struct CounterUnlessAbility : public clone_inherit<CounterUnlessAbility, Ability> {
	CounterUnlessAbility(CostValue cost, std::shared_ptr<TargetingRestriction> targeting)
		: clone_inherit(LambdaEffects([this](xg::Guid s, const Environment& env) -> std::optional<Changeset>
	{
		Changeset counter;
		std::shared_ptr<Targetable> target = env.gameObjects.at(env.targets.at(s)[0]);
		if (std::dynamic_pointer_cast<Card>(target)) {
			counter.push_back(ObjectMovement{ target->id, env.stack->id, env.graveyards.at(target->owner)->id, 0, COUNTER });
		}
		else {
			counter.push_back(RemoveObject{ target->id, env.stack->id });
		}
		Player& controller = *std::dynamic_pointer_cast<Player>(env.gameObjects.at(env.getController(target)));
		Changeset change = counter;
		if (this->cost.canPay(controller, env, {})) {
			Changeset payCost = this->cost.payCost(controller, env, std::shared_ptr<const Card>());
			change = controller.strategy->chooseOne({ payCost, change }, controller, env);
		}

		return change;
	}), targeting), cost(cost)
	{ }

	std::optional<Changeset> operator()(xg::Guid s, const Environment& env) {
		return this->effect.getChangeset(s, env);
	}

private:
	CostValue cost;
};

#endif